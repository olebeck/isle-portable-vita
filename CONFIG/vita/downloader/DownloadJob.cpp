#include "DownloadJob.hpp"

#include "util.hpp"

#include <psp2/io/fcntl.h>
#include <psp2/kernel/clib.h>
#include <psp2/net/http.h>

static int SaveToFile(int reqId, const paf::string& filename, std::function<void(uint64_t)> OnProgress)
{
	int ret = 0;

	paf::vector<uint8_t> buffer;
	buffer.resize(512_KiB);

	uint64_t content_length = 0;
	sceHttpGetResponseContentLength(reqId, &content_length);

	// open
	sceClibPrintf("SaveToFile: %s\n", filename.c_str());
	SceUID fd = sceIoOpen(filename.c_str(), SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666);
	if (fd < 0) {
		return fd;
	}

	uint64_t downloaded_bytes = 0;
	while (true) {
		ret = sceHttpReadData(reqId, buffer.data(), buffer.size());
		if (ret <= 0) {
			break;
		}
		ret = sceIoWrite(fd, buffer.data(), ret);
		if (ret < 0) {
			break;
		}
		downloaded_bytes += ret;
		OnProgress(downloaded_bytes);
	}
	sceIoClose(fd);
	return ret;
}

static int DoDownload(
	const DownloadFile& file,
	int connId,
	const char* acceptLanguage,
	paf::string& errorMessage,
	std::function<void(uint64_t)> OnProgress
)
{
	char buf[0x100];
	sceClibPrintf("Downloading: %s\n", file.filename.c_str());
	paf::string filepath = file.directory + file.filename;
	paf::string tmpPath = filepath + ".tmp";

	int ret = mkdirAll(removeFileName(filepath));
	if (ret < 0) {
		errorMessage = "failed to mkdir";
		return ret;
	}

	// create request
	sceClibPrintf("Get: %s\n", file.url.c_str());
	int reqId = sceHttpCreateRequestWithURL(connId, SCE_HTTP_METHOD_GET, file.url.c_str(), 0);
	if (reqId < 0) {
		errorMessage = "failed to create request";
		return ret;
	}

	sceHttpAddRequestHeader(reqId, "Accept-Language", acceptLanguage, SCE_HTTP_HEADER_OVERWRITE);

	// send request
	ret = sceHttpSendRequest(reqId, nullptr, 0);
	if (ret < 0) {
		sceHttpDeleteRequest(reqId);
		sceClibSnprintf(buf, sizeof(buf), "failed to send request %08x", ret);
		errorMessage = paf::string(buf);
		return ret;
	}

	// check status
	int statusCode;
	ret = sceHttpGetStatusCode(reqId, &statusCode);
	if (ret < 0) {
		sceHttpDeleteRequest(reqId);
		errorMessage = "failed to get status";
		return ret;
	}
	if (statusCode != 200) {
		sceHttpDeleteRequest(reqId);
		sceClibSnprintf(buf, sizeof(buf), "status code %d", statusCode);
		errorMessage = paf::string(buf);
		return -1;
	}

	ret = SaveToFile(reqId, tmpPath, OnProgress);
	if (ret < 0) {
		sceHttpDeleteRequest(reqId);
		sceClibSnprintf(buf, sizeof(buf), "failed to save file %08x", ret);
		errorMessage = paf::string(buf);
		return ret;
	}

	sceHttpDeleteRequest(reqId);
	sceIoRename(tmpPath.c_str(), filepath.c_str());
	return 0;
}

DownloadJob::DownloadJob(
	const paf::string& baseUrl,
	const char* acceptLanguage,
	const paf::vector<DownloadFile>& files
)
	: paf::job::JobItem("Download")
{
	this->files = files;
	this->acceptLanguage = acceptLanguage;
	this->tmpl = sceHttpCreateTemplate("downloader", SCE_HTTP_VERSION_1_1, 1);
	this->conn = sceHttpCreateConnectionWithURL(this->tmpl, baseUrl.c_str(), 1);
}

DownloadJob::~DownloadJob()
{
	sceHttpDeleteTemplate(this->tmpl);
	sceHttpDeleteConnection(this->conn);
}

int32_t DownloadJob::Run()
{
	uint64_t total_size = 0;
	uint64_t total_downloaded = 0;

	for (const auto& file : this->files) {
		total_size += file.size;
	}

	for (const auto& file : this->files) {
		this->OnFileStart(file.filename);

		while (true) {
			paf::string errorMessage;
			int ret = DoDownload(
				file,
				this->conn,
				this->acceptLanguage,
				errorMessage,
				[this, total_downloaded, total_size, file](uint64_t downloaded_file) {
					this->OnProgress(total_downloaded + downloaded_file, total_size, downloaded_file, file.size);
				}
			);
			if (ret < 0) {
				sceClibPrintf("failed to download %s: %s\n", file.filename.c_str(), errorMessage.c_str());
				this->OnError(file.filename, errorMessage);
				paf::thread::Sleep(3000);
				continue;
			}
			total_downloaded += file.size;
			break;
		}
	}

	this->OnComplete(total_downloaded);
	return 0;
}
