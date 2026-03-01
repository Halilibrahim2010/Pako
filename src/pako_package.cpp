#include "pako_package.hpp"
#include <iostream>

extern std::string prefix;
extern std::string varDir;
extern std::string storeDir;
extern std::string tmpDir;
extern std::string binDir;
const std::string dbPath = varDir + "/db/packages.json";

#define _(String) gettext(String)

std::string getPackageFolder(const std::string& hash) {
    return storeDir + "/" + hash;
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


// Paket kurulum fonksiyonu
void installPackage(const std::string& packagePath) {
    std::string tmpExtractPath = createTmpDir(); // RAM üzerinde temp
    std::string metadataFile = tmpExtractPath + "/metadata.json";

    try {
        // 1️⃣ Paket RAM'de açılıyor
        extractArchiveTo(packagePath, tmpExtractPath);

        // Metadata kontrolü
        if(!fs::exists(metadataFile)) {
            throw std::runtime_error("Metadata not found!");
        }

        std::ifstream metadataStream(metadataFile);
        json metadata; 
        metadataStream >> metadata;
        PackageInfo pkgInfo = parseMetadata(metadata);

        // Paket hash
        std::string hash = generateHash(packagePath);
        std::string pkgFolder = getPackageFolder(hash);

        if(fs::exists(dbPath)) {
            std::ifstream dbInput(dbPath);
            json db;
            if(dbInput) dbInput >> db;
            dbInput.close();

            if(db.contains(hash)) {
                std::cerr << "Bu paket zaten kurulmuş: " << pkgInfo.name
                        << " (hash: " << hash << ")" << std::endl;
                fs::remove_all(tmpExtractPath);
                return; // Paketi tekrar kurmayı engelle
            }
        }

        // Klasörler
        fs::create_directories(pkgFolder + "/bin");
        fs::create_directories(pkgFolder + "/lib");
        fs::create_directories(pkgFolder + "/share");
        fs::create_directories(varDir + "/db");

        auto safeCopy = [&](const fs::path& src, const fs::path& dest, fs::perms perms) {
            fs::path absDest = fs::absolute(dest);
            fs::path absPkgFolder = fs::absolute(pkgFolder);

            // Yol manipülasyonu kontrolü
            if(absDest.string().find(absPkgFolder.string()) != 0) {
                throw std::runtime_error("Invalid path detected: " + absDest.string());
            }

            // Atomik yazma: tmp file -> rename
            fs::path tmpFile = absDest;
            tmpFile += ".tmp";

            fs::copy(src, tmpFile, fs::copy_options::overwrite_existing);
            fs::rename(tmpFile, absDest);

            // Yetkiler
            fs::permissions(absDest, perms);
        };

        // Binaries
        for(const auto& [name, path] : pkgInfo.binaries) {
            fs::path src = tmpExtractPath + "/" + path;
            fs::path dest = pkgFolder + "/bin/" + name;
            if(fs::exists(src)) {
                safeCopy(src, dest, fs::perms::owner_exec | fs::perms::owner_read |
                                  fs::perms::group_read | fs::perms::others_read);
            }
        }

        // Libraries
        for(const auto& [name, path] : pkgInfo.libraries) {
            fs::path src = tmpExtractPath + "/" + path;
            fs::path dest = pkgFolder + "/lib/" + name;
            if(fs::exists(src)) {
                safeCopy(src, dest, fs::perms::owner_read | fs::perms::group_read | fs::perms::others_read);
            }
        }

        // Metadata
        fs::path metadataDest = pkgFolder + "/metadata.json";
        safeCopy(metadataFile, metadataDest, fs::perms::owner_read | fs::perms::group_read | fs::perms::others_read);

        // DB update
        if(!fs::exists(dbPath)) std::ofstream(dbPath) << "{}";
        updatePackageDB(hash, pkgInfo);

        std::cout << pkgInfo.name << " successfully installed!" << std::endl;

        // RAM temizliği
        fs::remove_all(tmpExtractPath);

    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        fs::remove_all(tmpExtractPath);
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

        // Dosya adı
        std::string pakoName = pkgInfo.name + "-" + pkgInfo.version + ".pako";

        std::cout << _("Paket oluşturuluyor: ")
                  << pkgInfo.name << " v" << pkgInfo.version << std::endl;

        createArchive(packageDir, pakoName); // libarchive + zstd kullanılıyor

        std::cout << _("Paket başarıyla oluşturuldu: ") << pakoName << std::endl;

    } catch (const std::exception& e) {
        std::cerr << _("Hata: ") << e.what() << std::endl;
    }
}

// Paket bilgilerini gösterme fonksiyonu
void showPackageInfo(const std::string& packagePath, bool jsonOutput) {
    // packagePath bir .pako dosyası ise, metadata.json içeriğini okur ve code_name + version ile hash'i DB'den bulur
    std::string code_name, version;
    std::string hash = "";

    // .pako dosyasını geçici dizine açıp metadata.json okuma
    std::string tmpExtractPath = createTmpDir();
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