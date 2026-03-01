#include "pako_archive.hpp"
#include <random>

void createArchive(const std::string& sourceDir,
                   const std::string& outputFile) {

    struct archive* a = archive_write_new();
    archive_write_add_filter_zstd(a);
    archive_write_set_format_pax_restricted(a); // tar
    archive_write_open_filename(a, outputFile.c_str());

    for (auto& p : fs::recursive_directory_iterator(sourceDir)) {

        if (fs::is_directory(p)) continue;

        struct archive_entry* entry = archive_entry_new();

        std::string relative =
            fs::relative(p.path(), sourceDir).string();

        archive_entry_set_pathname(entry, relative.c_str());
        archive_entry_set_size(entry, fs::file_size(p));
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0644);

        archive_write_header(a, entry);

        std::ifstream file(p.path(), std::ios::binary);
        char buffer[8192];
        while (file.read(buffer, sizeof(buffer)) || file.gcount()) {
            archive_write_data(a, buffer, file.gcount());
        }

        archive_entry_free(entry);
    }

    archive_write_close(a);
    archive_write_free(a);
}

void extractArchiveTo(const std::string& archivePath, const std::string& destDir) {
    struct archive* a = archive_read_new();
    struct archive* ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM |
                                          ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS);

    archive_read_support_format_tar(a);
    archive_read_support_filter_zstd(a);

    if (archive_read_open_filename(a, archivePath.c_str(), 10240) != ARCHIVE_OK) {
        throw std::runtime_error("Archive open failed!");
    }

    struct archive_entry* entry;
    int r;
    while ((r = archive_read_next_header(a, &entry)) == ARCHIVE_OK) {
        archive_entry_set_pathname(entry, (destDir + "/" + archive_entry_pathname(entry)).c_str());
        r = archive_write_header(ext, entry);
        if (r != ARCHIVE_OK) throw std::runtime_error(archive_error_string(ext));

        const void* buff;
        size_t size;
        la_int64_t offset;
        while (archive_read_data_block(a, &buff, &size, &offset) == ARCHIVE_OK) {
            if (archive_write_data_block(ext, buff, size, offset) != ARCHIVE_OK)
                throw std::runtime_error(archive_error_string(ext));
        }
    }

    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
}

std::string createTmpDir() {
    // tmpfs /dev/shm kullan
    std::string base = "/dev/shm/pako_tmp";
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(10000, 99999);

    std::string tmp = base + "-" + std::to_string(getpid()) + "-" + std::to_string(dist(mt));
    fs::create_directories(tmp);
    return tmp;
}