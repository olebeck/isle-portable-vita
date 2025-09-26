#pragma once

#include "Page.hpp"
#include "GameFiles.hpp"

class StartupPage : public Page {
private:
    EVersion version;
    int missingCount;

    void OpenVersionsList();
public:
    std::function<void(EVersion)> OnChangeVersion;
    std::function<void()> OnDownloadButton;
    std::function<void()> OnVerifyButton;

    StartupPage(EVersion version, int missingCount);

    paf::IDParam PageId() override { return "StartupPage"; };
    void Mount() override;
};