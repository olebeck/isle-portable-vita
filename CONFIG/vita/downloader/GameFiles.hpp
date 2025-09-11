#pragma once

#include <paf.h>

enum EDir {
    DIR_CD = 1,
    DIR_DISK = 2
};

struct GameFile {
    EDir folder;
    paf::string filename;
    paf::string sha256;
    size_t size;
};

extern const paf::vector<GameFile> gameFiles;
