#pragma once

#include <archive.h>
#include <archive_entry.h>

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

void createArchive(const std::string& sourceDir,
                   const std::string& outputFile);

void extractArchiveTo(const std::string& archivePath, 
                      const std::string& destDir);

std::string createTmpDir();