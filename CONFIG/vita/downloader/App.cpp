#include "App.hpp"

#include "Dialog.hpp"
#include "DownloadJob.hpp"
#include "DownloadPage.hpp"
#include "StartupPage.hpp"
#include "VerifyJob.hpp"
#include "VerifyPage.hpp"
#include "util.hpp"
#include "config.h"

#include <iniparser.h>
#include <psp2/appmgr.h>
#include <psp2/io/fcntl.h>

App* App::instance;

App::App(paf::Framework* framework)
{
	App::instance = this;
	this->framework = framework;
	this->jobs = paf::job::JobQueue::DefaultQueue();
	this->current_page = nullptr;
	this->installed_size = 0;
	this->loadPlugin();
	paf::common::MainThreadCallList::Register(App::switchPageTask, this);
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
	app->free_space = GetFreeSpace("ux0:");
	app->ScanFolders();
	app->OpenStartup();
}

void App::SwitchPage(Page* next_page, std::function<void()> after_open)
{
	this->next_page = next_page;
	this->after_open = after_open;
}

void App::switchPageTask(void* userdata)
{
	App* app = static_cast<App*>(userdata);

	if (app->next_page == nullptr) {
		return;
	}

	// close previous page
	Page* prev_page = app->current_page;
	if (prev_page != nullptr) {
		prev_page->Close();
		delete prev_page;
	}

	// open new page
	sceClibPrintf("OpenPage(%s)\n", app->next_page->PageId().GetID().c_str());
	app->current_page = app->next_page;
	app->current_page->Open(app->plugin);
	if (app->after_open != nullptr) {
		app->after_open();
	}
	app->after_open = nullptr;
	app->next_page = nullptr;
}

void App::enqueueJob(paf::job::JobItem* job)
{
	paf::common::SharedPtr<paf::job::JobItem> jobShared(job);
	this->jobs->Enqueue(jobShared);
}

void App::LoadConfig()
{
	// defaults
	this->cd_folder = default_cd_path;
	this->disk_folder = default_disk_path;
	dictionary* dict = iniparser_load("ux0:data/isledecomp/isle/isle.ini");
	if (dict != nullptr) {
		this->cd_folder = iniparser_getstring(dict, "isle:cdpath", default_cd_path);
		this->disk_folder = iniparser_getstring(dict, "isle:diskpath", default_disk_path);
		dictionary_del(dict);
	}
	if(this->cd_folder.c_str()[this->cd_folder.size()-1] != '/') {
		this->cd_folder += "/";
	}
	if(this->disk_folder.c_str()[this->disk_folder.size()-1] != '/') {
		this->disk_folder += "/";
	}
}

void App::ScanFolders()
{
	this->found_any_files = false;
	this->installed_size = 0;
	this->missing_files.clear();
	paf::vector<paf::string> found_cd_files;
    paf::vector<paf::string> found_disk_files;

	listFilesRecursive(this->cd_folder, "", found_cd_files);
	listFilesRecursive(this->disk_folder, "", found_disk_files);
	sceClibPrintf("Found %d cd files, %d disk files\n", found_cd_files.size(), found_disk_files.size());

	for (const GameFile& file : gameFiles) {
		paf::vector<paf::string>* found_files = (file.folder == DIR_CD) ? &found_cd_files : &found_disk_files;
		bool missing = paf::find(found_files->begin(), found_files->end(), file.filename) == found_files->end();
		if (missing || always_download) {
			this->missing_files.push_back(&file);
		}
		else {
			this->installed_size += file.size;
			this->found_any_files = true;
		}
	}
	sceClibPrintf(
		"Missing %d files\ninstalled_size: %lld\n",
		this->missing_files.size(),
		this->installed_size
	);
}

void App::OpenStartup()
{
	uint64_t data_todo = 0;
	for (const auto& gameFile : gameFiles) {
		data_todo += gameFile.size;
	}
	data_todo -= this->installed_size;

	bool allDownloaded = this->missing_files.size() == 0;

	StartupPage* startupPage = new StartupPage(data_todo, this->cd_folder, this->disk_folder);
	startupPage->OnDownloadButton = [this, startupPage, allDownloaded, data_todo]() {
		ScopeLockMain lock();

		if (allDownloaded && !disable_install_check) {
			auto dialog = new Dialog();
			dialog->SetTitle("Already Installed");
			dialog->SetText("You already have all required files installed.");
			dialog->SetButton1("Ok");
			dialog->Show(this->plugin, startupPage->Root());
			return;
		}

		sceClibPrintf(
			"data_todo: %lld free_space: %lld installed_size: %lld\n",
			data_todo,
			this->free_space,
			this->installed_size
		);

		int need_mb = (data_todo + MIN_FREE_SPACE) / 1000000;
		int free_mb = this->free_space / 1000000;

		if (free_mb < need_mb && !disable_free_space_check) {
			char buf[128];
			sceClibSnprintf(
				buf,
				sizeof(buf),
				"Not enough storage left to download this game.\n Need: %dMB, Free: %dMB",
				need_mb,
				free_mb
			);
			auto dialog = new Dialog();
			dialog->SetTitle("Storage Full");
			dialog->SetText(buf);
			dialog->SetButton1("Ok", nullptr);
			dialog->Show(this->plugin, startupPage->Root());
			return;
		}

		this->OpenDownload();
	};

	startupPage->OnVerifyButton = [this, startupPage]() {
		ScopeLockMain lock();

		if (!this->found_any_files) {
			auto dialog = new Dialog();
			dialog->SetTitle("Nothing Installed");
			dialog->SetText("No Game Files found\nNothing to Verify.");
			dialog->SetButton1("Ok", nullptr);
			dialog->Show(this->plugin, startupPage->Root());
			return;
		}

		this->OpenVerify();
	};

	this->SwitchPage(startupPage, nullptr);
}

void App::OpenDownload()
{
	uint64_t download_size = 0;
	for (const auto& file : this->missing_files) {
		download_size += file->size;
	}

	DownloadPage* downloadPage = new DownloadPage(download_size);
	this->SwitchPage(downloadPage, [this, downloadPage]() {
		paf::vector<DownloadFile> downloadFiles;

		for(const auto& gameFile : this->missing_files) {
			// always uppercase LEGO, even on cd
			paf::string url = paf::string(download_server) + "LEGO/" + gameFile->filename.substr(5, gameFile->filename.size());
			paf::string directory = gameFile->folder == DIR_CD ? this->cd_folder : this->disk_folder;
			downloadFiles.push_back(DownloadFile{
				.url = url,
				.directory = directory,
				.filename = gameFile->filename,
				.size = gameFile->size
			});
		}

		sceClibPrintf("Downloading %d files\n", downloadFiles.size());
		auto downloadJob = new DownloadJob(download_server, downloadFiles);

		downloadJob->OnFileStart = [downloadPage](const paf::string& filename) {
			ScopeLockMain lock();
			downloadPage->UpdateFile(filename);
		};

		downloadJob->OnProgress = [this, downloadPage](uint64_t total_downloaded, uint64_t total_size, uint64_t file_downloaded, uint64_t file_size) {
			ScopeLockMain lock();
			float total_progress = (float)(total_downloaded + this->installed_size) / (float)(total_size + this->installed_size) * 100;
			float file_progress = (float)(file_downloaded) / (float)(file_size) * 100;
			sceClibPrintf("Progress: %f%% %f%%\n", total_progress, file_progress);
			downloadPage->UpdateProgress(total_downloaded, total_progress, file_progress);
		};

		downloadJob->OnError = [downloadPage](const paf::string& filename, const paf::string& errorMessage) {
			ScopeLockMain lock();
			downloadPage->DisplayError(filename, errorMessage);
		};

		downloadJob->OnComplete = [this, downloadPage](uint64_t total_downloaded) {
			ScopeLockMain lock();

			auto dialog = new Dialog();
			dialog->SetTitle("Download Complete");
			char buf[128];
			sceClibSnprintf(
				buf,
				sizeof(buf),
				"Successfully downloaded game data\n%dMB Downloaded",
				(int) total_downloaded / 1000000
			);
			dialog->SetText(buf);
			dialog->SetButton1("Launch", []() { sceAppMgrLoadExec("app0:/eboot.bin", NULL, NULL); });
			dialog->SetButton2("Exit", []() { sceKernelExitProcess(0); });
			dialog->Show(this->plugin, downloadPage->Root());
		};

		this->enqueueJob(downloadJob);
	});
}

void App::OpenVerify()
{
	this->verify_results.clear();
	VerifyPage* verifyPage = new VerifyPage(this->installed_size);
	this->SwitchPage(verifyPage, [this, verifyPage]() {
		auto verifyJob = new VerifyJob(this->cd_folder, this->disk_folder);

		verifyJob->OnFileStart = [verifyPage](const paf::string& filename) {
			ScopeLockMain lock();
			verifyPage->UpdateFile(filename);
		};

		verifyJob->OnProgress = [this, verifyPage](uint64_t total_done, uint64_t file_done, uint64_t file_size) {
			ScopeLockMain lock();
			float total_progress = (float)(total_done) / (float)(this->installed_size) * 100;
			float file_progress = (float)(file_done) / (float)(file_size) * 100;
			sceClibPrintf("Progress: %f%% %f%%\n", total_progress, file_progress);
			verifyPage->UpdateProgress(total_done, total_progress, file_progress);
		};

		verifyJob->OnComplete = [this, verifyPage](paf::vector<VerifyResult>& results) {
			ScopeLockMain lock();
			int validCount = 0;
			int brokenCount = 0;
			int missingCount = 0;
			this->verify_results.clear();
			for (const auto& result : results) {
				this->verify_results.push_back(result);
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
				auto dialog = new Dialog();
				dialog->SetTitle("Verify Complete");
				dialog->SetText("All OK.");
				dialog->SetButton1("Exit", []() { sceKernelExitProcess(0); });
				dialog->Show(this->plugin, verifyPage->Root());
				return;
			}

			auto dialog = new Dialog();
			dialog->SetTitle("Verify Complete");
			char buf[256];
			sceClibSnprintf(
				buf,
				sizeof(buf),
				"Valid: %d\nBroken: %d\nMissing: %d\nPress 'Download' to Fix",
				validCount,
				brokenCount,
				missingCount
			);
			dialog->SetText(buf);
			dialog->SetButton1("Download", [this]() {
				ScopeLockMain lock();
				this->FixBroken();
			});
			dialog->SetButton2("Exit", []() { sceKernelExitProcess(0); });
			dialog->Show(this->plugin, verifyPage->Root());
		};

		this->enqueueJob(verifyJob);
	});
}

void App::FixBroken()
{
	// delete all broken files
	for (const auto& result : this->verify_results) {
		if (result.result == EVerifyResult::OK || result.result == EVerifyResult::MISSING) {
			continue;
		}

		const paf::string& folder = result.gameFile->folder == DIR_CD ? this->cd_folder : this->disk_folder;
		paf::string filename = folder + result.gameFile->filename;
		sceIoRemove(filename.c_str());
	}

	// rescan, download
	this->ScanFolders();
	this->OpenDownload();
}
