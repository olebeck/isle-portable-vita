#pragma once

#include <paf.h>

#include "GameFiles.hpp"

class DeleteFilesJob : public paf::job::JobItem {
private:
    paf::string cd_folder;
    paf::string disk_folder;
    EVersion selected_version;
    EVersion installed_version;
public:
    std::function<void(float)> OnProgress;
    std::function<void()> OnComplete;

    DeleteFilesJob(
        const paf::string& cdFolder,
        const paf::string& diskFolder,
        EVersion selectedVersion,
        EVersion installedVersion
    );

    int32_t Run() override;
    void Cancel() override {};
    void Finish(int32_t result) override {};
};
