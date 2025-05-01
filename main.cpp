#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstdlib>

#include <nlohmann/json.hpp>

#include <locale.h>
#include <libintl.h>

#define _(String) gettext(String)

namespace fs = std::filesystem;
using json = nlohmann::json;

struct PackageInfo {
    std::string name;
    std::string code_name;  // Yeni eklendi
    std::string version;
    std::string description;
    std::vector<std::string> dependencies;
    std::map<std::string, std::string> binaries;
    std::map<std::string, std::string> libraries;
};

PackageInfo parseMetadata(const json& metadata) {
    PackageInfo info;
    info.name = metadata["name"];
    info.version = metadata["version"];
    info.description = metadata["description"];
    
    // code_name kontrolü ve validasyonu
    if (metadata.contains("code_name")) {
        std::string code = metadata["code_name"];
        // Sadece alfanumerik ve alt çizgi karakterlerine izin ver
        if (std::all_of(code.begin(), code.end(), [](char c) {
            return std::isalnum(c) || c == '_';
        })) {
            info.code_name = code;
        } else {
            std::cerr << _("Warning: code_name can only contain alphanumeric characters!") << std::endl;
            info.code_name = "unnamed_package";
        }
    } else {
        std::cout << _("Warning: code_name not found, using default value") << std::endl;
        info.code_name = "unnamed_package";
    }
    
    if (metadata.contains("dependencies")) {
        for (const auto& dep : metadata["dependencies"]) {
            info.dependencies.push_back(dep);
        }
    }

    if (metadata.contains("binaries")) {
        for (const auto& [name, path] : metadata["binaries"].items()) {
            info.binaries[name] = path;
        }
    }

    if (metadata.contains("libraries")) {
        for (const auto& [name, path] : metadata["libraries"].items()) {
            info.libraries[name] = path;
        }
    }

    return info;
}

// .pako paketini aç ve içeriğini hedef dizine kopyala
void installPackage(const std::string& packagePath) {
    std::string workDir = std::string(std::getenv("HOME")) + "/.pako";
    std::string extractPath = workDir + "/tmp/pako_extract";
    std::string rootDir = workDir + "/root";
    std::string metadataFile = extractPath + "/metadata.json";
    
    std::cout << _("DEBUG: Working directory: ") << workDir << std::endl;
    std::cout << _("DEBUG: Package path: ") << packagePath << std::endl;

    // Gerekli dizinleri oluştur
    fs::create_directories(extractPath);
    fs::create_directories(rootDir + "/bin");
    fs::create_directories(rootDir + "/lib");
    fs::create_directories(rootDir + "/etc");
    fs::create_directories(workDir + "/archive");

    // Paketi aç
    std::string command = "tar -I zstd -xf " + packagePath + " -C " + extractPath;
    if (system(command.c_str()) != 0) {
        std::cerr << _("Error: Package couldn't be extracted!") << std::endl;
        return;
    }

    // Metadata kontrolü ve okuma
    if (!fs::exists(metadataFile)) {
        std::cerr << _("Error: Metadata not found!") << std::endl;
        fs::remove_all(extractPath);
        return;
    }

    try {
        std::ifstream metadataStream(metadataFile);
        json metadata;
        metadataStream >> metadata;
        
        PackageInfo pkgInfo = parseMetadata(metadata);
        
        // Arşiv dosya adını oluştur
        std::string archiveFileName = pkgInfo.code_name + "+" + pkgInfo.version + ".pako";
        std::string archivePath = workDir + "/archive/" + archiveFileName;
        
        std::cout << _("DEBUG: Archive file name: ") << archiveFileName << std::endl;
        
        // Paketi arşivle
        if (!fs::copy_file(packagePath, archivePath, fs::copy_options::overwrite_existing)) {
            std::cerr << _("Error: Package couldn't be archived!") << std::endl;
            return;
        }
        
        std::cout << _("DEBUG: Package archived: ") << archivePath << std::endl;

        // Paket bilgilerini göster
        std::cout << _("Package: ") << pkgInfo.name << " v" << pkgInfo.version << std::endl;
        std::cout << _("Description: ") << pkgInfo.description << std::endl;

        // Bağımlılıkları kontrol et
        if (!pkgInfo.dependencies.empty()) {
            std::cout << _("Dependencies:") << std::endl;
            for (const auto& dep : pkgInfo.dependencies) {
                std::cout << "- " << dep << std::endl;
            }
        }

        // Binary dosyaları kopyala
        for (const auto& [name, path] : pkgInfo.binaries) {
            std::string srcPath = extractPath + "/" + path;
            std::string destPath = rootDir + "/bin/" + name;
            
            if (fs::exists(srcPath)) {
                fs::copy(srcPath, destPath, fs::copy_options::overwrite_existing);
                fs::permissions(destPath, fs::perms::owner_exec | fs::perms::owner_write | 
                                       fs::perms::owner_read, fs::perm_options::add);
            }
        }

        // Kütüphaneleri kopyala
        for (const auto& [name, path] : pkgInfo.libraries) {
            std::string srcPath = extractPath + "/" + path;
            std::string destPath = rootDir + "/lib/" + name;
            
            if (fs::exists(srcPath)) {
                fs::copy(srcPath, destPath, fs::copy_options::overwrite_existing);
            }
        }

        std::cout << pkgInfo.name << _(" successfully installed!") << std::endl;

    } catch (const std::exception& e) {
        std::cerr << _("Error: ") << e.what() << std::endl;
        std::cout << _("DEBUG: Exception caught!") << std::endl;
    }

    // Temizlik
    fs::remove_all(extractPath);
}

void createPackage(const std::string& packageDir) {
    std::cout << _("DEBUG: Package creation started. Directory: ") << packageDir << std::endl;
    std::string metadataPath = packageDir + "/metadata.json";
    
    if (!fs::exists(metadataPath)) {
        std::cerr << _("Error: Metadata not found!") << std::endl;
        return;
    }

    try {
        std::ifstream metadataStream(metadataPath);
        json metadata;
        metadataStream >> metadata;
        
        PackageInfo pkgInfo = parseMetadata(metadata);
        
        std::cout << _("DEBUG: Metadata read:") << std::endl;
        std::cout << "- " << _("Name: ") << pkgInfo.name << std::endl;
        std::cout << "- " << _("Code name: ") << pkgInfo.code_name << std::endl;
        std::cout << "- " << _("Version: ") << pkgInfo.version << std::endl;

        std::string tarCommand = "tar -cf " + pkgInfo.name + ".tar -C " + packageDir + " .";
        std::string zstdCommand = "zstd " + pkgInfo.name + ".tar -o " + pkgInfo.name + ".pako";
        std::string cleanCommand = "rm -f " + pkgInfo.name + ".tar";

        std::cout << "Paket oluşturuluyor: " << pkgInfo.name << " v" << pkgInfo.version << std::endl;
        
        if (system(tarCommand.c_str()) != 0) {
            std::cerr << _("Error: tar archive creation failed!") << std::endl;
            return;
        }
        
        if (system(zstdCommand.c_str()) != 0) {
            std::cerr << _("Error: zstd compression failed!") << std::endl;
            return;
        }
        
        if (system(cleanCommand.c_str()) == 0) {
            std::cout << _("Package successfully created: ") << pkgInfo.name << ".pako" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << _("Error: ") << e.what() << std::endl;
        std::cout << _("DEBUG: Package creation error!") << std::endl;
    }
}

void showPackageInfo(const std::string& packagePath, bool jsonOutput = false) {
    std::string workDir = fs::current_path().string();
    std::string extractPath = workDir + "/tmp/pako_info";
    std::string metadataFile = extractPath + "/metadata.json";

    fs::create_directories(extractPath);

    std::string command = "tar -I zstd -xf " + packagePath + " -C " + extractPath;
    if (system(command.c_str()) != 0) {
        std::cerr << _("Error: Package couldn't be extracted!") << std::endl;
        fs::remove_all(extractPath);
        return;
    }

    try {
        std::ifstream metadataStream(metadataFile);
        json metadata;
        metadataStream >> metadata;

        if (jsonOutput) {
            std::cout << metadata.dump(4) << std::endl;
        } else {
            PackageInfo info = parseMetadata(metadata);
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

    fs::remove_all(extractPath);
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "");
    bindtextdomain("pako", "./locale"); // çeviriler burada
    textdomain("pako");
    if (argc < 2) {
        std::cerr << _("Usage:\n")
            << _("  pako install -y <package.pako>\n")
            << _("  pako create <directory>\n")
            << _("  pako info <package.pako> [-json]") << std::endl;
        return 1;
    }

    std::string command = argv[1];
    
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
    else {
        std::cerr << _("Error: Invalid command!") << std::endl;
        return 1;
    }

    return 0;
}
