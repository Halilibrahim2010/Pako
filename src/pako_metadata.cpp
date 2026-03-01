#include "pako_metadata.hpp"
#include <algorithm>
#include <iostream>

#define _(String) gettext(String)

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
