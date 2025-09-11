#pragma once

#include "Page.hpp"

class StartupPage : public Page {
private:
    uint64_t data_todo;
    paf::string cd_folder;
    paf::string disk_folder;
public:
    std::function<void()> OnDownloadButton;
    std::function<void()> OnVerifyButton;

    StartupPage(uint64_t dataTodo, const paf::string& cd_folder, const paf::string& disk_folder) : data_todo(dataTodo) {};

    paf::IDParam PageId() override { return "StartupPage"; };
    void Mount() override;
};