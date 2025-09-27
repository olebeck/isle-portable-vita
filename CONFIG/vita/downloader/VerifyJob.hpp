#pragma once

#include <paf.h>

#include "GameFiles.hpp"

enum class EVerifyResult {
    INVALID = 0,
    OK = 1,
    HASH_FAILED = 2,
    MISSING = 3,
    ERROR = 4
};

struct VerifyResult {
    const GameFile* gameFile;
    EVerifyResult result;
};

class VerifyJob : public paf::job::JobItem {
    const GameVersion* game_version;
    paf::string cd_folder;
    paf::string disk_folder;

    int verifyFile(const GameFile* file, EVerifyResult& result, std::function<void(uint64_t, float)> OnFileProgress);
public:
    std::function<void(const paf::string&)> OnFileStart;
    std::function<void(int, int, int)> OnFileVerified;
    std::function<void(uint64_t, uint64_t, uint64_t)> OnProgress;
    std::function<void(paf::vector<VerifyResult>&)> OnComplete;

    VerifyJob(const GameVersion* gameVersion, const paf::string& cdFolder, const paf::string& diskFolder);
    
    int32_t Run() override;
    void Cancel() override {};
    void Finish(int32_t result) override {};
};