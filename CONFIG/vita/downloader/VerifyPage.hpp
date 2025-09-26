#pragma once

#include <paf.h>

#include "Page.hpp"

class VerifyPage : public Page {
private:
    paf::ui::ProgressBar* progress_bar_total;
    paf::ui::ProgressBar* progress_bar_file;
    paf::ui::Text* dialog_text1;

    paf::string filename;
    uint64_t total_size;
    int mb_done;

    int validCount;
    int brokenCount;
    int missingCount;

    void updateDialogText();
public:
    VerifyPage(uint64_t total_size);

    void UpdateFile(const paf::string& filename);
    void UpdateProgress(uint64_t bytes, float total, float file);
    void UpdateVerifiedStatus(int validCount, int brokenCount, int missingCount);

    paf::IDParam PageId() override { return "VerifyPage"; };
    void Mount() override;
};
