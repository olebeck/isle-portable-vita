#pragma once

#include <paf.h>
#include "GameFiles.hpp"

struct DownloadFile {
    paf::string url;
    paf::string directory;
    paf::string filename;
    uint64_t size;
};

class DownloadJob : public paf::job::JobItem {
private:
    paf::vector<DownloadFile> files;
    const char* acceptLanguage;
    int tmpl;
    int conn;

public:
    std::function<void(const paf::string&)> OnFileStart;
    std::function<void(uint64_t, uint64_t, uint64_t, uint64_t)> OnProgress; // total, total_size, file, file_size
    std::function<void(uint64_t)> OnComplete;
    std::function<void(const paf::string&, const paf::string&)> OnError;

    DownloadJob(const paf::string& baseUrl, const char* acceptLanguage, const paf::vector<DownloadFile>& files);
    ~DownloadJob();

    int32_t Run() override;
    void Cancel() override {};
    void Finish(int32_t result) override {};
};
