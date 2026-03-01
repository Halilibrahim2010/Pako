#pragma once

#include <nlohmann/json.hpp>

#include "pako_packageinfo.hpp"

using json = nlohmann::json;

// Metadata okuma ve doğrulama fonksiyonu
PackageInfo parseMetadata(const json& metadata);