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
    paf::string cd_folder;
    paf::string disk_folder;

    int verifyFile(const GameFile& file, EVerifyResult& result, std::function<void(uint64_t, float)> OnFileProgress);
public:
    std::function<void(paf::string)> OnFileStart;
    std::function<void(uint64_t, float, float)> OnProgress;
    std::function<void(paf::vector<VerifyResult>&)> OnComplete;

    VerifyJob(const paf::string& cdFolder, const paf::string& diskFolder);
    
    void Run() override;
    void Cancel() override {};
    void Finish() override {};
};