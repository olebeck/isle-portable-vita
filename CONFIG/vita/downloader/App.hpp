#include <paf.h>

#include "Page.hpp"
#include "GameFiles.hpp"
#include "VerifyJob.hpp"

class App {
    paf::Framework* framework;
    paf::job::JobQueue* jobs;
    paf::Plugin* plugin;
    paf::ui::Widget* scene;

    Page* current_page;
    Page* next_page;
    std::function<void()> after_open;

    paf::string cd_folder;
    paf::string disk_folder;
    bool found_any_files;
    paf::vector<const GameFile*> missing_files;
    paf::vector<VerifyResult> verify_results;
    uint64_t free_space;
    uint64_t installed_size;

    static App* instance;

public:
    App(paf::Framework* framework);
    static inline App* GetInstance() {
        return App::instance;
    };

private:
    void loadPlugin();
    static void startFunc(paf::Plugin* plugin);
    static void switchPageTask(void* userdata);
    void SwitchPage(Page* page, std::function<void()> after_open);
    void enqueueJob(paf::job::JobItem* job);

    void LoadConfig();
    void ScanFolders();
    void FixBroken();

    void OpenStartup();
    void OpenDownload();
    void OpenVerify();
};