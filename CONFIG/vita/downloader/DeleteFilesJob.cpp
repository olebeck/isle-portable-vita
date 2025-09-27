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
	for (int i = 0; i < installedVer->file_count; i++) {
		const GameFile* installed_file = &installedVer->files[i];
		bool do_delete = true;
		for (int j = 0; j < selectedVer->file_count; j++) {
			const GameFile* selected_file = &selectedVer->files[i];
			bool is_same =
				installed_file->folder == selected_file->folder && installed_file->filename == selected_file->filename;
			if (is_same) {
				if (installed_file->sha1 == selected_file->sha1) {
					do_delete = false;
					break;
				}
				do_delete = true;
			}
		}
		if (!do_delete) {
			continue;
		}
		filesToDelete.push_back(installed_file);
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
