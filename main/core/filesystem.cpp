#include "filesystem.h"
#include <sys/stat.h>
#include <sys/unistd.h>
#include <stdio.h>

namespace wg {

bool fs_exists(const char* path) {
    struct stat st;
    return (stat(path, &st) == 0);
}

bool fs_mkdir(const char* path) {
    return (mkdir(path, 0777) == 0);
}

bool fs_write_text(const char* path, const char* text) {
    FILE* f = fopen(path, "w");
    if (!f) return false;
    fputs(text, f);
    fclose(f);
    return true;
}

bool fs_read_text(const char* path, std::string& out) {
    FILE* f = fopen(path, "r");
    if (!f) return false;
    char buf[512];
    out.clear();
    while (fgets(buf, sizeof(buf), f))
        out += buf;
    fclose(f);
    return true;
}

} // namespace wg
