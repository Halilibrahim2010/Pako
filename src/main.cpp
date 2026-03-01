#include <iostream>
#include <cstdlib>
#include <sstream>
#include <ctime>
#include <string>

#include <openssl/evp.h>

#include <locale.h>
#include <libintl.h>
#include <unistd.h>

#include "pako_archive.hpp"
#include "pako_package.hpp"

// Global path değişkenleri
std::string getPrefix() {
    const char* home = getenv("HOME");
    if (!home) throw std::runtime_error("HOME environment variable not set");
    return std::string(home) + "/.local/pako";
}

std::string prefix = getPrefix();
std::string varDir = prefix + "/var";
std::string storeDir = prefix + "/store";
std::string tmpDir = prefix + "/tmp";
std::string binDir = prefix + "/bin";
extern std::string dbPath = varDir + "/db/packages.json";

#define _(String) gettext(String)

// izinleri ayarlar
void setPermissions(const std::string& path, fs::perms permissions) {
    try {
        fs::permissions(path, permissions, fs::perm_options::add);
    } catch (const std::exception& e) {
        std::cerr << "Error setting permissions for: " << path << " - " << e.what() << std::endl;
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
    std::string oldPkgFolder = getPackageFolder(currentHash);
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
    const std::string VERSION = "1.2";
    const std::string CODENAME = "Meridyen";

    setlocale(LC_ALL, "");
    bindtextdomain("pako", "./locale"); // çeviriler burada
    textdomain("pako");

    if (argc >= 2) {
        std::string command = argv[1];

        if (command == "--version") {
            std::cout << VERSION << " Stable: " << CODENAME << std::endl;
            return 0;
        }
        if (command == "--only-version") {
            std::cout << VERSION << std::endl;
            return 0;
        }
    }

    if (argc < 2) {
        std::cerr << _("Pako 1.2 - Usage:\n")
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

        std::string pkgFolder = getPackageFolder(foundHash);
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
        char* argv[] = { const_cast<char*>(binPath.c_str()), nullptr };
        char* envp[] = {
            const_cast<char*>(("LD_LIBRARY_PATH=" + ldLibPath).c_str()),
            const_cast<char*>(("XDG_DATA_DIRS=" + xdgDataDirs).c_str()),
            nullptr
        };

        execve(binPath.c_str(), argv, envp);

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