#include <paf.h>

#include "Page.hpp"
#include "GameFiles.hpp"
#include "VerifyJob.hpp"

class App {
    static App* instance;

    paf::Framework* framework;
    paf::Plugin* plugin;

    Page* current_page;

    paf::string cd_folder;
    paf::string disk_folder;

    EVersion installed_version;
    EVersion selected_version;

    static void startFunc(paf::Plugin* plugin);
    void enqueueJob(paf::job::JobItem* job);
    void SwitchPage(Page* next_page);
    bool StartDownload(EVersion version);

public:
    App(paf::Framework* framework);
    App(const App&) = delete;

    static inline App* GetInstance() {
        return App::instance;
    };

    void loadPlugin();
    void LoadConfig();
    void DetectVersion();
    void ScanFolders(EVersion version, paf::vector<const GameFile*>& missing_files, uint64_t& installed_size);
    void FixBroken(paf::vector<VerifyResult>& verifyResults);

    void OpenStartup(uint64_t installed_size, int missingCount);
    void OpenDownload(EVersion version, paf::vector<const GameFile*>& missingFiles, uint64_t installed_size);
    void OpenVerify(uint64_t installed_size);

    void OnChangeVersion(EVersion newVersion);
};