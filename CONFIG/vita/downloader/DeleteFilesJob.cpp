#include "DeleteFilesJob.hpp"

#include <psp2/io/fcntl.h>

DeleteFilesJob::DeleteFilesJob(
	const paf::string& cdFolder,
	const paf::string& diskFolder,
	EVersion selectedVersion,
	EVersion installedVersion
)
	: paf::job::JobItem("DeleteFiles")
{
	this->cd_folder = cdFolder;
	this->disk_folder = diskFolder;
	this->selected_version = selectedVersion;
	this->installed_version = installedVersion;
}

// delete all files that arent equal
int32_t DeleteFilesJob::Run()
{
	sceClibPrintf(
		"Deleting files %s to %s\n",
		EVersionToString(this->installed_version).c_str(),
		EVersionToString(this->selected_version).c_str()
	);

	auto selectedVer = GetGameVersion(this->selected_version);
	auto installedVer = GetGameVersion(this->installed_version);

	paf::vector<const GameFile*> filesToDelete;
	for (const auto& file1 : installedVer->files) {
		bool do_delete = true;
		for (const auto& file2 : selectedVer->files) {
			bool is_same = file1.folder == file2.folder && file1.filename == file2.filename;
			if (is_same) {
				if (file1.sha1 == file2.sha1) {
					do_delete = false;
					break;
				}
				do_delete = true;
			}
		}
		filesToDelete.push_back(&file1);
	}

	int i = 0;
	for (const auto& file : filesToDelete) {
		const paf::string& folder = (file->folder == EDir::CD) ? this->cd_folder : this->disk_folder;
		paf::string filepath = folder + file->filename;
		sceClibPrintf("Deleting %s\n", filepath.c_str());
		// sceIoRemove(filepath.c_str());

		float progress = (float) i / (float) filesToDelete.size() * 100;
		this->OnProgress(progress);
	}
	this->OnComplete();
	return 0;
}
