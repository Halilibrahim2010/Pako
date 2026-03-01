#pragma once
#include <string>
#include <vector>
#include <map>

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