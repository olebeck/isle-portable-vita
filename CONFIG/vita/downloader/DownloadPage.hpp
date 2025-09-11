#include "Page.hpp"

class DownloadPage : public Page {
private:
    paf::ui::ProgressBar* progress_bar_total;
    paf::ui::ProgressBar* progress_bar_file;
    paf::ui::Text* dialog_text1;
    paf::ui::Text* error_message;

    uint64_t download_size;
    int mb_done; 
    paf::string filename;

    void updateDialogText();

public:
    DownloadPage(uint64_t download_size);

    void DisplayError(const paf::string& filename, const paf::string& errorMessage);
    void UpdateFile(const paf::string& filename);
    void UpdateProgress(uint64_t total_downloaded, float total_progress, float file_progress);

    paf::IDParam PageId() override { return "DownloadPage"; };
    void Mount() override;
};