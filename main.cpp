
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <ctime>

#include <nlohmann/json.hpp>
#include <openssl/sha.h>

#include <locale.h>
#include <libintl.h>
#include <unistd.h>
namespace fs = std::filesystem;
using json = nlohmann::json;

struct PackageInfo {
    std::string name;
    std::string code_name;
    std::string version;
    std::string description;
    std::vector<std::string> dependencies;
    std::map<std::string, std::string> binaries;
    std::map<std::string, std::string> libraries;
    std::string mainbinary;
};

// Global path değişkenleri
std::string prefix = std::string(getenv("HOME")) + "/.local/pako";
std::string varDir = prefix + "/var";
std::string storeDir = prefix + "/store";
std::string tmpDir = prefix + "/tmp";
std::string binDir = prefix + "/bin";
const std::string dbPath = varDir + "/db/packages.json";

#define _(String) gettext(String)

namespace fs = std::filesystem;
using json = nlohmann::json;

// Metadata okuma ve doğrulama fonksiyonu
PackageInfo parseMetadata(const json& metadata) {
    PackageInfo info;
    info.name = metadata.value("name", "unknown");
    info.version = metadata.value("version", "unknown");
    info.description = metadata.value("description", "");

    std::string code = metadata.value("code_name", "unnamed_package");
    if (std::all_of(code.begin(), code.end(), [](char c){ return std::isalnum(c) || c=='_'; })) {
        info.code_name = code;
    } else {
        std::cerr << _("Warning: code_name can only contain alphanumeric characters!") << std::endl;
        info.code_name = "unnamed_package";
    }

    if(metadata.contains("dependencies")) for(const auto& dep : metadata["dependencies"]) info.dependencies.push_back(dep);
    if(metadata.contains("binaries")) for(const auto& [name, path] : metadata["binaries"].items()) info.binaries[name]=path;
    if(metadata.contains("libraries")) for(const auto& [name, path] : metadata["libraries"].items()) info.libraries[name]=path;

    info.mainbinary = metadata.value("mainbinary", "");
    return info;
}

std::string getPackageFolder(const std::string& hash, const std::string& code_name, const std::string& version) {
    return storeDir + "/" + hash + "-" + code_name + "-" + version;
}

void updatePackageDB(const std::string& hash, const PackageInfo& pkgInfo) {
    try {
        std::ifstream dbInput(dbPath); json db; if(dbInput) dbInput >> db; dbInput.close();
        db[hash] = {
            {"name", pkgInfo.name},
            {"code_name", pkgInfo.code_name},
            {"version", pkgInfo.version},
            {"hash", hash},
            {"installed_files", json(pkgInfo.binaries)},
            {"dependencies", pkgInfo.dependencies},
            {"description", pkgInfo.description}
        };
        std::ofstream dbOutput(dbPath); dbOutput << db.dump(4); dbOutput.close();
    } catch(const std::exception& e) {
        std::cerr << "Error updating package database: " << e.what() << std::endl;
    }
}

// Hash oluşturma fonksiyonu (SHA-256 kullanarak)
std::string generateHash(const std::string& input) {
    // Şu anki zamanı al (saat ve dakika)
    std::time_t t = std::time(nullptr);
    std::tm* tm_ptr = std::localtime(&t);
    int hour = tm_ptr->tm_hour;
    int min = tm_ptr->tm_min;

    std::ostringstream combined;
    combined << input << "_" << std::setw(2) << std::setfill('0') << hour
             << std::setw(2) << std::setfill('0') << min;

    std::string data = combined.str();

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), hash);

    std::ostringstream hashStream;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        hashStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    return hashStream.str().substr(0, 8); // İlk 8 karakteri al
}

// Binary ve symlink izinlerini ayarlar
void setPermissions(const std::string& path, fs::perms permissions) {
    try {
        fs::permissions(path, permissions, fs::perm_options::add);
    } catch (const std::exception& e) {
        std::cerr << "Error setting permissions for: " << path << " - " << e.what() << std::endl;
    }
}

// Paket kurulum fonksiyonu
void installPackage(const std::string& packagePath) {
    std::string tmpExtractPath = tmpDir + "/pako_extract";
    std::string metadataFile = tmpExtractPath + "/metadata.json";

    try {
        fs::create_directories(tmpExtractPath);

        std::string command = "tar -I zstd -xf " + packagePath + " -C " + tmpExtractPath;
        if(system(command.c_str())!=0) {
            std::cerr << _("Error: Package couldn't be extracted!") << std::endl;
            fs::remove_all(tmpExtractPath);
            return;
        }

        if(!fs::exists(metadataFile)) {
            std::cerr << _("Error: Metadata not found!") << std::endl;
            fs::remove_all(tmpExtractPath);
            return;
        }

        std::ifstream metadataStream(metadataFile);
        json metadata; metadataStream >> metadata;
        PackageInfo pkgInfo = parseMetadata(metadata);

        std::string hash = generateHash(pkgInfo.name + pkgInfo.version + pkgInfo.code_name);
        std::string pkgFolder = getPackageFolder(hash, pkgInfo.code_name, pkgInfo.version);
        //printf("DEBUG: Installing package: %s v%s (code_name: %s)\n", pkgInfo.name.c_str(), pkgInfo.version.c_str(), pkgInfo.code_name.c_str());

        fs::create_directories(pkgFolder + "/bin");
        fs::create_directories(pkgFolder + "/lib");
        fs::create_directories(pkgFolder + "/share");
        fs::create_directories(varDir + "/db"); // DB varsa zaten atlar

        for(const auto& [name, path] : pkgInfo.binaries) {
            std::string src = tmpExtractPath + "/" + path;
            std::string dest = pkgFolder + "/bin/" + name;
            if(fs::exists(src)) {
                fs::copy(src, dest, fs::copy_options::overwrite_existing);
                setPermissions(dest, fs::perms::owner_all | fs::perms::group_read | fs::perms::others_read);
            }
        }

        for(const auto& [name, path] : pkgInfo.libraries) {
            std::string src = tmpExtractPath + "/" + path;
            std::string dest = pkgFolder + "/lib/" + name;
            if(fs::exists(src)) fs::copy(src, dest, fs::copy_options::overwrite_existing);
        }

        fs::copy(metadataFile, pkgFolder + "/metadata.json", fs::copy_options::overwrite_existing);
        if(!fs::exists(dbPath)) std::ofstream(dbPath) << "{}";
        updatePackageDB(hash, pkgInfo);

        std::cout << pkgInfo.name << _(" successfully installed!") << std::endl;
        fs::remove_all(tmpExtractPath);
    } catch(const std::exception& e) {
        std::cerr << _("Error: ") << e.what() << std::endl;
    }
}

// Paket oluşturma fonksiyonu hash eklenerek yeniden düzenlendi
void createPackage(const std::string& packageDir) {
    std::cout << _("DEBUG: Paket oluşturma başladı. Klasör: ") << packageDir << std::endl;
    std::string metadataPath = packageDir + "/metadata.json";
    try {
        if (!fs::exists(metadataPath)) {
            std::cout << _("Metadata bulunamadı! Boş bir şablon oluşturuluyor.") << std::endl;
            json metadata;
            metadata["name"] = "";
            metadata["code_name"] = "";
            metadata["version"] = "";
            metadata["description"] = "";
            metadata["dependencies"] = json::array();
            metadata["mainbinary"] = "";
            metadata["binaries"] = json::object();
            metadata["libraries"] = json::object();
            std::ofstream metaOut(metadataPath);
            metaOut << metadata.dump(4);
            metaOut.close();
            std::cout << "Boş metadata.json oluşturuldu! Lütfen düzenleyin." << std::endl;
            return;
        }
        std::ifstream metadataStream(metadataPath);
        json metadata;
        metadataStream >> metadata;
        PackageInfo pkgInfo = parseMetadata(metadata);
        std::cout << _("DEBUG: Metadata okundu:") << std::endl;
        std::cout << "- " << _("Ad: ") << pkgInfo.name << std::endl;
        std::cout << "- " << _("Kod adı: ") << pkgInfo.code_name << std::endl;
        std::cout << "- " << _("Sürüm: ") << pkgInfo.version << std::endl;
        // Hash oluşturma
        std::string hash = generateHash(pkgInfo.name + pkgInfo.version + pkgInfo.code_name);
        // Paket arşivleme
        std::string tarCommand = "tar -cf " + hash + "-" + pkgInfo.name + ".tar -C " + packageDir + " .";
        std::string zstdCommand = "zstd " + hash + "-" + pkgInfo.name + ".tar -o " + hash + "-" + pkgInfo.name + ".pako";
        std::string cleanCommand = "rm -f " + hash + "-" + pkgInfo.name + ".tar";
        std::cout << _("Paket oluşturuluyor: ") << pkgInfo.name << " v" << pkgInfo.version << std::endl;
        if (system(tarCommand.c_str()) != 0) {
            std::cerr << _("Hata: tar arşivleme başarısız oldu!") << std::endl;
            return;
        }
        if (system(zstdCommand.c_str()) != 0) {
            std::cerr << _("Hata: zstd sıkıştırma başarısız oldu!") << std::endl;
            return;
        }
        if (system(cleanCommand.c_str()) == 0) {
            std::cout << _("Paket başarıyla oluşturuldu: ") << hash + "-" + pkgInfo.name + ".pako" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << _("Hata: ") << e.what() << std::endl;
        std::cout << _("DEBUG: Paket oluşturma hatası!") << std::endl;
    }
}

// Sistem seviyesinde geçici dizin kullanımı için düzenleme
std::string generateTmpDir() {
    std::string tmpPath = tmpDir + "/pako_tmp_" + std::to_string(std::time(nullptr));
    fs::create_directories(tmpPath);
    return tmpPath;
}

// Paket bilgilerini gösterme fonksiyonu
void showPackageInfo(const std::string& packagePath, bool jsonOutput = false) {
    // packagePath bir .pako dosyası ise, metadata.json içeriğini okur ve code_name + version ile hash'i DB'den bulur
    std::string code_name, version;
    std::string hash = "";

    // .pako dosyasını geçici dizine açıp metadata.json okuma
    std::string tmpExtractPath = generateTmpDir();
    std::string metadataFile = tmpExtractPath + "/metadata.json";
    std::string command = "tar -I zstd -xf " + packagePath + " -C " + tmpExtractPath;
    if (system(command.c_str()) != 0) {
        std::cerr << _("Error: Package couldn't be extracted!") << std::endl;
        fs::remove_all(tmpExtractPath);
        return;
    }
    if (!fs::exists(metadataFile)) {
        std::cerr << _("Error: Metadata not found!") << std::endl;
        fs::remove_all(tmpExtractPath);
        return;
    }
    try {
        std::ifstream metadataStream(metadataFile);
        json metadata;
        metadataStream >> metadata;
        code_name = metadata.value("code_name", "unnamed_package");
        version = metadata.value("version", "unknown");

        // DB'den hash'i bul
        std::ifstream dbInput(dbPath);
        json db;
        if (dbInput) dbInput >> db;
        dbInput.close();
        for (const auto& [key, value] : db.items()) {
            if (value.value("code_name", "") == code_name && value.value("version", "") == version) {
                hash = key;
                break;
            }
        }
        fs::remove_all(tmpExtractPath);
        if (hash.empty()) {
            std::cerr << _("Error: Package not found in DB!") << std::endl;
            return;
        }
        std::string pkgFolder = storeDir + "/" + hash + "-" + code_name + "-" + version;
        std::string metadataStoreFile = pkgFolder + "/metadata.json";
        if (!fs::exists(metadataStoreFile)) {
            std::cerr << _("Error: Metadata not found in store!") << std::endl;
            return;
        }
        std::ifstream metadataStoreStream(metadataStoreFile);
        json metadataStore;
        metadataStoreStream >> metadataStore;
        if (jsonOutput) {
            std::cout << metadataStore.dump(4) << std::endl;
        } else {
            PackageInfo info = parseMetadata(metadataStore);
            std::cout << _("Package Information:") << std::endl;
            std::cout << _("Name: ") << info.name << std::endl;
            std::cout << _("Version: ") << info.version << std::endl;
            std::cout << _("Description: ") << info.description << std::endl;
            if (!info.dependencies.empty()) {
                std::cout << _("Dependencies:") << std::endl;
                for (const auto& dep : info.dependencies) {
                    std::cout << "- " << dep << std::endl;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << _("Error: ") << e.what() << std::endl;
    }
}

// Bağımlılık ve yükseltme yönetimi için fonksiyonlar
bool checkDependencies(const std::vector<std::string>& dependencies) {
    bool allResolved = true;
    for (const auto& dep : dependencies) {
        // Bağımlılık kontrolü
        std::ifstream dbInput(dbPath);
        json db;
        if (dbInput) {
            dbInput >> db;
        }
        dbInput.close();

        bool found = false;
        for (const auto& [key, value] : db.items()) {
            if (value["name"] == dep) {
                found = true;
                break;
            }
        }

        if (!found) {
            std::cerr << _("Error: Missing dependency: ") << dep << std::endl;
            allResolved = false;
        }
    }
    return allResolved;
}

void upgradePackage(const std::string& packageName) {
    std::ifstream dbInput(dbPath);
    json db;
    if (dbInput) {
        dbInput >> db;
    }
    dbInput.close();

    std::string currentHash;
    for (const auto& [key, value] : db.items()) {
        if (value["name"] == packageName) {
            currentHash = key;
            break;
        }
    }

    if (currentHash.empty()) {
        std::cerr << _("Error: Package not found: ") << packageName << std::endl;
        return;
    }

    std::cout << _("Upgrading package: ") << packageName << std::endl;
    // HENÜZ PROTOTİP. ÇALIŞIR DURUMDA DEĞİL!

    std::cout << _("Package upgraded successfully: ") << packageName << std::endl;
}

void listPackages() {
    if (!fs::exists(dbPath)) {
        std::cout << _( "Installed packages:") << std::endl;
        return;
    }
    std::ifstream dbInput(dbPath);
    json db;
    if (dbInput) {
        dbInput >> db;
    }
    dbInput.close();
    std::cout << _( "Installed packages:") << std::endl;
    for (const auto& [key, value] : db.items()) {
        std::cout << "- " << value["name"] << " v" << value["version"] << std::endl;
    }
}

void uninstallPackage(const std::string& packageName) {
    if (!fs::exists(dbPath)) {
        std::cerr << _( "Error: Package DB not found!") << std::endl;
        return;
    }
    std::ifstream dbInput(dbPath);
    json db;
    if (dbInput) {
        dbInput >> db;
    }
    dbInput.close();
    std::string hashToRemove;
    for (const auto& [key, value] : db.items()) {
        if (value["name"] == packageName) {
            hashToRemove = key;
            break;
        }
    }
    if (hashToRemove.empty()) {
        std::cerr << _( "Error: Package not found: ") << packageName << std::endl;
        return;
    }
    std::string pkgFolder = storeDir + "/" + hashToRemove;
    fs::remove_all(pkgFolder);
    db.erase(hashToRemove);
    std::ofstream dbOutput(dbPath);
    dbOutput << db.dump(4);
    dbOutput.close();
    std::cout << _( "Package uninstalled: ") << packageName << std::endl;
}

void updatePackage(const std::string& packageName) {
    std::cout << _("Updating package: ") << packageName << std::endl;
    // DB'den mevcut paketi bul
    if (!fs::exists(dbPath)) {
        std::cerr << _( "Error: Package DB not found!") << std::endl;
        return;
    }
    std::ifstream dbInput(dbPath);
    json db;
    if (dbInput) dbInput >> db;
    dbInput.close();
    std::string currentHash;
    std::string currentVersion;
    std::string code_name;
    for (const auto& [key, value] : db.items()) {
        if (value.value("name", "") == packageName) {
            currentHash = key;
            currentVersion = value.value("version", "");
            code_name = value.value("code_name", "");
            break;
        }
    }
    if (currentHash.empty()) {
        std::cerr << _( "Error: Package not found: ") << packageName << std::endl;
        return;
    }
    // Yeni sürüm dosyasını bul (örnek: <hash>-<name>.pako dosyası)
    std::string newPackageFile = storeDir + "/" + packageName + ".pako";
    if (!fs::exists(newPackageFile)) {
        std::cerr << _( "Error: New package file not found: ") << newPackageFile << std::endl;
        return;
    }
    // Yeni sürümü kur
    installPackage(newPackageFile);
    // Eski sürümü kaldır
    std::string oldPkgFolder = storeDir + "/" + currentHash + "-" + code_name + "-" + currentVersion;
    if (fs::exists(oldPkgFolder)) {
        fs::remove_all(oldPkgFolder);
    }
    db.erase(currentHash);
    std::ofstream dbOutput(dbPath);
    dbOutput << db.dump(4);
    dbOutput.close();
    std::cout << _( "Package upgraded successfully: ") << packageName << std::endl;
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "");
    bindtextdomain("pako", "./locale"); // çeviriler burada
    textdomain("pako");
    if (argc < 2) {
        std::cerr << _("Pako 1 - Usage:\n")
            << _("  pako -[version] <package name>\n")
            << _("  pako install -y <package.pako>\n")
            << _("  pako create <directory>\n")
            << _("  pako info <package.pako> [-json]\n")
            << _("  pako uninstall <package name>\n")
            << _("  pako list") << std::endl;
        return 1;
    }

    std::string command = argv[1];

    // pako -[sürüm] <uygulama>[.altbinary] çalıştırma
    if (command[0] == '-') {
        std::string version = command.length() > 1 ? command.substr(1) : "";
        if (argc < 3) {
            std::cerr << "Kullanım: pako -[sürüm] <uygulama>[.altbinary]" << std::endl;
            return 1;
        }

        std::string appArg = argv[2];
        std::string appName = appArg;
        std::string altBinary;
        size_t dotPos = appArg.find('.');
        if (dotPos != std::string::npos) {
            appName = appArg.substr(0, dotPos);
            altBinary = appArg.substr(dotPos + 1);
        }

        // DB'den hash ve code_name ile uygun paketi bul
        std::ifstream dbInput(dbPath);
        json db;
        if (dbInput) dbInput >> db;
        dbInput.close();

        std::string foundHash, foundCodeName, foundVersion;
        for (const auto& [key, value] : db.items()) {
            if (value.value("name", "") == appName) {
                if (version.empty() || value.value("version", "") == version) {
                    foundHash = key;
                    foundCodeName = value.value("code_name", "");
                    foundVersion = value.value("version", "");
                    break;
                }
            }
        }

        if (foundHash.empty()) {
            std::cerr << "Uygulama bulunamadı: " << appName << (version.empty() ? "" : (" (sürüm: " + version + ")")) << std::endl;
            return 1;
        }

        std::string pkgFolder = getPackageFolder(foundHash, foundCodeName, foundVersion);
        std::string metaPath = pkgFolder + "/metadata.json";
        std::ifstream metaIn(metaPath);
        json meta;
        if (metaIn) {
            try {
                metaIn >> meta;
            } catch (...) {
                std::cerr << "Error: metadata.json okunamadı veya bozuk!" << std::endl;
                return 1;
            }
        } else {
            std::cerr << "Error: metadata.json okunamadı" << std::endl;
            std::cout << "DEBUG: pkgFolder = " << pkgFolder << std::endl;
            std::cout << "DEBUG: metaPath = " << metaPath << std::endl;
            return 1;
        }

        // mainbinary kontrolü
        std::string mainbinary;
        if (meta.contains("mainbinary") && !meta["mainbinary"].is_null()) {
            mainbinary = meta["mainbinary"].get<std::string>();
        } else {
            mainbinary = "";
        }

        std::string binToRun = altBinary.empty() ? (mainbinary.empty() ? appName : mainbinary) : altBinary;
        std::string binPath = pkgFolder + "/bin/" + binToRun;

        if (!fs::exists(binPath)) {
            std::cerr << "Çalıştırılabilir dosya bulunamadı: " << binPath << std::endl;
            return 1;
        }

        // ----- lib klasörü -----
        std::string ldLibPath = pkgFolder + "/lib"; 
        if (meta.contains("libs") && meta["libs"].is_array()) {
            for (const auto& lib : meta["libs"]) {
                std::string libName = lib.get<std::string>();
                std::string fullLibPath = pkgFolder + "/lib/" + libName;
                if (!fs::exists(fullLibPath)) {
                    std::cerr << "Uyarı: Library bulunamadı: " << fullLibPath << std::endl;
                }
            }
        }

        // ----- share klasörü -----
        std::string xdgDataDirs = pkgFolder + "/share";
        const char* oldXdg = getenv("XDG_DATA_DIRS");
        if ((oldXdg == NULL || oldXdg[0] == '\0')) {
        }
        else {
            xdgDataDirs += ":";
            xdgDataDirs += oldXdg;  // eski dizinleri koru
        }

        // Çalıştırma: LD_LIBRARY_PATH ve XDG_DATA_DIRS ile
        execl("/usr/bin/env", "env",
            ("LD_LIBRARY_PATH=" + ldLibPath).c_str(),
            ("XDG_DATA_DIRS=" + xdgDataDirs).c_str(),
            binPath.c_str(),
            nullptr);

        std::cerr << "Çalıştırma başarısız: " << binPath << std::endl;
        return 1;
    }

    if ((command == "indir" || command == "install") && argc >= 4 && std::string(argv[2]) == "-y") {
        installPackage(argv[3]);
    }
    else if ((command == "tasarla" || command == "create") && argc >= 3) {
        createPackage(argv[2]);
    }
    else if ((command == "bilgi" || command == "info") && argc >= 3) {
        bool jsonOutput = (argc >= 4 && std::string(argv[3]) == "-json");
        showPackageInfo(argv[2], jsonOutput);
    }
    else if ((command == "liste" || command == "list")) {
        listPackages();
    }
    else if ((command == "sil" || command == "uninstall") && argc >= 3) {
        uninstallPackage(argv[2]);
    }
    else if ((command == "guncelle" || command == "update") && argc >= 3) {
        updatePackage(argv[2]);
    }
    else {
        std::cerr << _("Error: Invalid command!") << std::endl;
        return 1;
    }

    return 0;
}

// Function to create symlinks for binaries and libraries
void createSymlink(const std::string& target, const std::string& linkPath) {
    try {
        if (fs::exists(linkPath)) {
            fs::remove(linkPath);
        }
        fs::create_symlink(target, linkPath);
    } catch (const std::exception& e) {
        std::cerr << "Error creating symlink: " << e.what() << std::endl;
    }
}