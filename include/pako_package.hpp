#pragma once

#include "pako_archive.hpp"
#include "pako_generatehash.hpp"
#include "pako_metadata.hpp"

std::string getPackageFolder(const std::string& hash);

// Paket kurulum fonksiyonu
void installPackage(const std::string& packagePath);

// Paket oluşturma fonksiyonu hash eklenerek yeniden düzenlendi
void createPackage(const std::string& packageDir);

// Paket bilgilerini gösterme fonksiyonu
void showPackageInfo(const std::string& packagePath, bool jsonOutput = false);