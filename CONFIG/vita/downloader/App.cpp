#include "App.hpp"

#include "DeleteFilesJob.hpp"
#include "Dialog.hpp"
#include "DownloadJob.hpp"
#include "DownloadPage.hpp"
#include "StartupPage.hpp"
#include "VerifyJob.hpp"
#include "VerifyPage.hpp"
#include "config.h"
#include "util.hpp"

#include <iniparser.h>
#include <psp2/appmgr.h>
#include <psp2/io/fcntl.h>
#include <psp2/kernel/processmgr.h>

App* App::instance = nullptr;

App::App(paf::Framework* framework)
{
	App::instance = this;

	this->framework = framework;
	this->plugin = nullptr;

	this->current_page = nullptr;

	this->installed_version = EVersion::Invalid;
	this->selected_version = EVersion::Invalid;

	this->loadPlugin();
}

void App::loadPlugin()
{
	paf::Plugin::InitParam pluginParam;
	pluginParam.name = "downloader_plugin";
	pluginParam.caller_name = "__main__";
	pluginParam.resource_file = "app0:/downloader_plugin.rco";
	pluginParam.locale = Locale_EN;
	pluginParam.start_func = App::startFunc;
	pluginParam.stop_func = nullptr;
	pluginParam.exit_func = nullptr;
	paf::Plugin::LoadSync(pluginParam, nullptr);
}

void App::startFunc(paf::Plugin* plugin)
{
	auto app = App::GetInstance();
	app->plugin = plugin;
	app->LoadConfig();
	app->DetectVersion();
	paf::vector<const GameFile*> missingFiles;
	uint64_t installed_size;
	app->ScanFolders(app->installed_version, missingFiles, installed_size);

	paf::Plugin::PageOpenParam openParam;
	plugin->PageOpen("BackgroundPage", openParam);

	app->OpenStartup(installed_size, missingFiles.size());
}

void App::SwitchPage(Page* next_page)
{
	Page* prev_page = this->current_page;
	if (prev_page != nullptr) {
		prev_page->Close();
		delete prev_page;
	}

	sceClibPrintf("OpenPage(%s)\n", next_page->PageId().GetID().c_str());
	this->current_page = next_page;
	this->current_page->Open(this->plugin);
}

void App::enqueueJob(paf::job::JobItem* job)
{
	paf::common::SharedPtr<paf::job::JobItem> jobShared(job);
	paf::job::JobQueue::DefaultQueue()->Enqueue(jobShared);
}

void App::LoadConfig()
{
	this->cd_folder = default_cd_path;
	this->disk_folder = default_disk_path;
	dictionary* dict = iniparser_load("ux0:data/isledecomp/isle/isle.ini");
	if (dict != nullptr) {
		this->cd_folder = iniparser_getstring(dict, "isle:cdpath", default_cd_path);
		this->disk_folder = iniparser_getstring(dict, "isle:diskpath", default_disk_path);
		dictionary_del(dict);
	}
	if (this->cd_folder.c_str()[this->cd_folder.size() - 1] != '/') {
		this->cd_folder += "/";
	}
	if (this->disk_folder.c_str()[this->disk_folder.size() - 1] != '/') {
		this->disk_folder += "/";
	}
}

static int hashFileSha1(const paf::string& filename, paf::vector<uint8_t>& hash)
{
	paf::vector<uint8_t> buffer;
	buffer.resize(512_KiB);

	SceUID fd = sceIoOpen(filename.c_str(), SCE_O_RDONLY, 0666);
	if (fd < 0) {
		return fd;
	}
	int ret = 0;
	SHA1Context context;
	SHA1Reset(&context);
	while (true) {
		ret = sceIoRead(fd, buffer.data(), buffer.size());
		if (ret <= 0) {
			break;
		}
		SHA1Input(&context, buffer.data(), ret);
	}
	sceIoClose(fd);

	hash.resize(20);
	SHA1Result(&context, hash.data());
	return 0;
}

void App::DetectVersion()
{
	this->installed_version = EVersion::None;
	const paf::string filename = this->cd_folder + "/Lego/Scripts/NOCD.SI";

	paf::vector<uint8_t> hash1;
	int ret = hashFileSha1(filename, hash1);
	if (ret < 0) {
		this->installed_version = EVersion::None;
	}
	else {
		for (const auto& gameVersion : GetAllVersions()) {
			const GameFile* gameFile = nullptr;
			for (const auto& file : gameVersion.files) {
				if (file.filename == "Lego/Scripts/NOCD.SI") {
					gameFile = &file;
				}
			}

			uint8_t hash2[20];
			hexDecode(gameFile->sha1, hash2);
			bool eq = sce_paf_memcmp(hash1.data(), hash2, hash1.size()) == 0;
			if (eq) {
				this->installed_version = gameVersion.version;
				break;
			}
		}
		if (this->installed_version == EVersion::None) {
			this->installed_version = EVersion::Unknown;
		}
	}

	// set selected version
	if (this->selected_version == EVersion::Invalid) {
		if (this->installed_version != EVersion::None) {
			this->selected_version = this->installed_version;
		}
		else {
			this->selected_version = EVersion::English_1_1;
		}
	}
}

void App::ScanFolders(EVersion version, paf::vector<const GameFile*>& missing_files, uint64_t& installed_size)
{
	installed_size = 0;

	paf::vector<paf::string> found_cd_files;
	paf::vector<paf::string> found_disk_files;
	listFilesRecursive(this->cd_folder, "", found_cd_files);
	listFilesRecursive(this->disk_folder, "", found_disk_files);
	sceClibPrintf("Found %d cd files, %d disk files\n", found_cd_files.size(), found_disk_files.size());

	const GameVersion* gameVersion = GetGameVersion(version);
	if (gameVersion == nullptr) {
		dialog::OpenError(this->plugin, -1, L"Invalid game version selected");
		return;
	}

	for (const GameFile& file : gameVersion->files) {
		paf::vector<paf::string>* found_files = (file.folder == EDir::CD) ? &found_cd_files : &found_disk_files;
		bool missing = paf::find(found_files->begin(), found_files->end(), file.filename) == found_files->end();
		if (missing || always_download) {
			missing_files.push_back(&file);
		}
		else {
			installed_size += file.size;
		}
	}
	sceClibPrintf("Missing %d files\ninstalled_size: %lld\n", missing_files.size(), installed_size);
}

bool App::StartDownload(EVersion version)
{
	sceClibPrintf("StartDownload %d\n", (int) version);

	if (this->installed_version == EVersion::Unknown) {
		dialog::OpenOk(this->plugin, L"Unknown Version Installed", L"Select a version to manage it using this app.");
		return false;
	}

	const GameVersion* gameVersion = GetGameVersion(version);
	if (!gameVersion) {
		dialog::OpenError(this->plugin, -1, L"invalid version");
		return false;
	}

	paf::vector<const GameFile*> missingFiles;
	uint64_t installed_size;
	this->ScanFolders(version, missingFiles, installed_size);

	bool allDownloaded = missingFiles.empty();
	if (allDownloaded && !disable_install_check) {
		dialog::OpenOk(
			this->plugin,
			L"Already Installed",
			L"You already have all required files installed for the selected version."
		);
		return false;
	}

	uint64_t data_todo = 0;
	for (const auto& gameFile : gameVersion->files) {
		data_todo += gameFile.size;
	}
	data_todo -= installed_size;

	uint64_t free_space = GetFreeSpace("ux0:");
	sceClibPrintf("data_todo: %lld free_space: %lld installed_size: %lld\n", data_todo, free_space, installed_size);

	int need_mb = (data_todo + MIN_FREE_SPACE) / 1000000;
	int free_mb = free_space / 1000000;

	if (free_mb < need_mb && !disable_free_space_check) {
		wchar_t wbuf[128];
		sce_paf_swprintf(
			wbuf,
			sizeof(wbuf),
			L"Not enough storage left to download this game.\n Need: %dMB, Free: %dMB",
			need_mb,
			free_mb
		);
		dialog::OpenOk(this->plugin, L"Storage Full", wbuf);
		return false;
	}

	this->OpenDownload(version, missingFiles, installed_size);
	return true;
}

void App::OpenStartup(uint64_t installed_size, int missingCount)
{
	StartupPage* startupPage = new StartupPage(this->installed_version, missingCount);

	startupPage->OnChangeVersion = [this](EVersion newVersion) {
		ScopeLockMain lock();

		if (newVersion == this->selected_version) {
			dialog::OpenOk(this->plugin, L"Already Selected", L"That Version is already Selected");
			return;
		}
		this->selected_version = newVersion;

		bool installing = false;
		if (this->installed_version == EVersion::None) {
			installing = this->StartDownload(newVersion);
		}
		else if (this->installed_version == EVersion::Invalid) {
			installing = this->StartDownload(newVersion);
		}
		else if (this->installed_version == newVersion) {
			dialog::OpenOk(this->plugin, L"Already Installed", L"That Version is already Installed");
			installing = false;
		}
		else {
			wchar_t wbuf[128];
			sce_paf_swprintf(
				wbuf,
				sizeof(wbuf),
				L"Installing %s will DELETE your current install of the %s version.",
				StringToWString(EVersionToString(newVersion)).c_str(),
				StringToWString(EVersionToString(this->installed_version)).c_str()
			);
			dialog::OpenYesNo(this->plugin, L"Are you sure", wbuf, [this](dialog::ButtonCode button) {
				ScopeLockMain lock();
				if (button == dialog::ButtonCode_No) {
					this->selected_version = this->installed_version;
					return;
				}
				if (button == dialog::ButtonCode_Yes) {
					// dialog::OpenPleaseWait(this->plugin, L"Deleting Previous Version", L"");
					auto deleteJob = new DeleteFilesJob(
						this->cd_folder,
						this->disk_folder,
						this->selected_version,
						this->installed_version
					);
					deleteJob->OnProgress = [](float progress) {
						// progressBar->SetValueAsync(progress);
					};
					deleteJob->OnComplete = [this]() {
						ScopeLockMain lock();
						EVersion old_version = this->installed_version;
						this->installed_version = EVersion::None;
						dialog::Close();
						if (!this->StartDownload(this->selected_version)) {
							this->selected_version = old_version;
							this->installed_version = old_version;
						}
					};
					this->enqueueJob(deleteJob);
				}
			});
			installing = true;
		}

		if (!installing) {
			this->selected_version = this->installed_version;
		}
	};

	startupPage->OnDownloadButton = [this]() {
		ScopeLockMain lock();
		this->StartDownload(this->selected_version);
	};

	startupPage->OnVerifyButton = [this, installed_size]() {
		ScopeLockMain lock();
		this->OpenVerify(installed_size);
	};

	this->SwitchPage(startupPage);
}

void App::OpenDownload(EVersion version, paf::vector<const GameFile*>& missingFiles, uint64_t installed_size)
{
	uint64_t download_size = 0;
	for (const auto& file : missingFiles) {
		download_size += file->size;
	}

	DownloadPage* downloadPage = new DownloadPage(download_size);
	this->SwitchPage(downloadPage);

	paf::vector<DownloadFile> downloadFiles;
	for (const auto& gameFile : missingFiles) {
		paf::string url = download_server + "LEGO/" + gameFile->filename.substr(5, gameFile->filename.size());
		paf::string directory = gameFile->folder == EDir::CD ? this->cd_folder : this->disk_folder;
		downloadFiles.push_back(
			DownloadFile{.url = url, .directory = directory, .filename = gameFile->filename, .size = gameFile->size}
		);
	}

	const char* acceptLanguage = EVersionToAcceptLanguage(version);
	sceClibPrintf("Downloading %d files\n", downloadFiles.size());
	auto downloadJob = new DownloadJob(download_server, acceptLanguage, downloadFiles);

	downloadJob->OnFileStart = [downloadPage](const paf::string& filename) {
		ScopeLockMain lock();
		downloadPage->UpdateFile(filename);
	};

	downloadJob->OnProgress =
		[this,
		 downloadPage,
		 installed_size](uint64_t total_done, uint64_t total_size, uint64_t file_done, uint64_t file_size) {
			ScopeLockMain lock();
			float total_progress = (float) (total_done + installed_size) / (float) (total_done + installed_size) * 100;
			float file_progress = (float) (file_done) / (float) (file_size) * 100;
			sceClibPrintf("Progress: %f%% %f%%\n", total_progress, file_progress);
			downloadPage->UpdateProgress(total_done, total_progress, file_progress);
		};

	downloadJob->OnError = [downloadPage](const paf::string& filename, const paf::string& errorMessage) {
		ScopeLockMain lock();
		downloadPage->DisplayError(filename, errorMessage);
	};

	downloadJob->OnComplete = [this, downloadPage](uint64_t total_downloaded) {
		ScopeLockMain lock();

		wchar_t wbuf[128];
		sce_paf_swprintf(
			wbuf,
			sizeof(wbuf),
			L"Successfully downloaded game data\n%dMB Downloaded",
			(int) total_downloaded / 1000000
		);

		dialog::OpenTwoButton(
			this->plugin,
			L"Download Complete",
			wbuf,
			paf::IDParam("msg_launch"),
			paf::IDParam("msg_exit"),
			[](dialog::ButtonCode button) {
				if (button == dialog::ButtonCode_Button1) {
					sceAppMgrLoadExec("app0:/eboot.bin", NULL, NULL);
				}
				if (button == dialog::ButtonCode_Button2) {
					sceKernelExitProcess(0);
				}
			}
		);
	};

	this->enqueueJob(downloadJob);
}

void App::OpenVerify(uint64_t installed_size)
{
	if (this->installed_version == EVersion::None) {
		dialog::OpenOk(this->plugin, L"Nothing Installed", L"No Game Files found\nNothing to Verify.");
		return;
	}

	if (this->installed_version == EVersion::Unknown) {
		dialog::OpenOk(this->plugin, L"Unknown Version Installed", L"Select a version to manage it using this app.");
		return;
	}

	const GameVersion* gameVersion = GetGameVersion(this->installed_version);
	if (!gameVersion) {
		dialog::OpenError(this->plugin, -1, L"invalid version");
		return;
	}

	VerifyPage* verifyPage = new VerifyPage(installed_size);
	this->SwitchPage(verifyPage);

	auto verifyJob = new VerifyJob(gameVersion, this->cd_folder, this->disk_folder);

	verifyJob->OnFileStart = [verifyPage](const paf::string& filename) {
		ScopeLockMain lock();
		verifyPage->UpdateFile(filename);
	};

	verifyJob->OnProgress =
		[this, verifyPage, installed_size](uint64_t total_done, uint64_t file_done, uint64_t file_size) {
			ScopeLockMain lock();
			float total_progress = (float) total_done / (float) installed_size * 100;
			float file_progress = (float) file_done / (float) file_size * 100;
			sceClibPrintf("Progress: %f%% %f%%\n", total_progress, file_progress);
			verifyPage->UpdateProgress(total_done, total_progress, file_progress);
		};

	verifyJob->OnFileVerified = [this, verifyPage](int validCount, int brokenCount, int missingCount) {
		ScopeLockMain lock();
		verifyPage->UpdateVerifiedStatus(validCount, brokenCount, missingCount);
	};

	verifyJob->OnComplete = [this, verifyPage, installed_size](paf::vector<VerifyResult>& results) {
		ScopeLockMain lock();
		int validCount = 0;
		int brokenCount = 0;
		int missingCount = 0;

		auto verifyResults = new paf::vector<VerifyResult>();
		for (const auto& result : results) {
			verifyResults->push_back(result);
			switch (result.result) {
			case EVerifyResult::OK:
				validCount++;
				break;
			case EVerifyResult::INVALID:
			case EVerifyResult::ERROR:
			case EVerifyResult::HASH_FAILED:
				brokenCount++;
				break;
			case EVerifyResult::MISSING:
				missingCount++;
			}
		}

		if (validCount == results.size()) {
			dialog::OpenTwoButton(
				this->plugin,
				L"Verify Complete",
				L"All OK.",
				paf::IDParam("msg_launch"),
				paf::IDParam("msg_exit"),
				[](dialog::ButtonCode button) {
					if (button == dialog::ButtonCode_Button1) {
						sceAppMgrLoadExec("app0:/eboot.bin", NULL, NULL);
					}
					if (button == dialog::ButtonCode_Button2) {
						sceKernelExitProcess(0);
					}
				}
			);
			return;
		}

		wchar_t wbuf[128];
		sce_paf_swprintf(
			wbuf,
			sizeof(wbuf),
			L"Valid: %d\nBroken: %d\nMissing: %d\nPress 'Download' to Fix",
			validCount,
			brokenCount,
			missingCount
		);
		dialog::OpenTwoButton(
			this->plugin,
			L"Verify Complete",
			wbuf,
			paf::IDParam("msg_download"),
			paf::IDParam("msg_back"),
			[this, verifyResults, installed_size, missingCount](dialog::ButtonCode button) {
				ScopeLockMain lock();
				if (button == dialog::ButtonCode_Button1) {
					this->FixBroken(*verifyResults);
					delete verifyResults;
				}
				if (button == dialog::ButtonCode_Button2) {
					this->OpenStartup(installed_size, missingCount);
				}
			}
		);
	};

	this->enqueueJob(verifyJob);
}

void App::FixBroken(paf::vector<VerifyResult>& verifyResults)
{
	for (const auto& result : verifyResults) {
		if (result.result == EVerifyResult::OK || result.result == EVerifyResult::MISSING) {
			continue;
		}

		const paf::string& folder = result.gameFile->folder == EDir::CD ? this->cd_folder : this->disk_folder;
		paf::string filename = folder + result.gameFile->filename;
		sceIoRemove(filename.c_str());
	}

	paf::vector<const GameFile*> missingFiles;
	uint64_t installed_size;
	this->ScanFolders(this->installed_version, missingFiles, installed_size);
	this->OpenDownload(this->installed_version, missingFiles, installed_size);
}
