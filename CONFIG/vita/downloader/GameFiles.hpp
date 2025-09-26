#pragma once

#include <paf.h>

enum class EDir {
    CD = 1,
    DISK = 2
};

enum class EVersion {
    None = -1,
    Invalid = 0,
    English,
    English_1_1,
    Danish,
    French,
    German,
    Italian,
    Japanese,
    Korean,
    Portuguese,
    Russian,
    Spanish,
    Unknown,
    MAX,
};

paf::string EVersionToString(EVersion version);

const char* EVersionToAcceptLanguage(EVersion version);

struct GameFile {
    EDir folder;
    paf::string filename;
    paf::string sha1;
    size_t size;
};

struct GameVersion {
    EVersion version;
    paf::vector<GameFile> files;
};

const GameVersion* GetGameVersion(EVersion version);
const paf::vector<GameVersion>& GetAllVersions();
