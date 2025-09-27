#include "VerifyJob.hpp"

#include "GameFiles.hpp"
#include "util.hpp"

#include <psp2/io/fcntl.h>
#include <psp2/kernel/clib.h>

VerifyJob::VerifyJob(const GameVersion* gameVersion, const paf::string& cdFolder, const paf::string& diskFolder)
	: paf::job::JobItem("Verify")
{
	this->game_version = gameVersion;
	this->cd_folder = cdFolder;
	this->disk_folder = diskFolder;
}

int VerifyJob::verifyFile(
	const GameFile* file,
	EVerifyResult& result,
	std::function<void(uint64_t, float)> OnFileProgress
)
{
	paf::vector<uint8_t> buffer;
	buffer.resize(512_KiB);

	const paf::string& folder = file->folder == EDir::CD ? this->cd_folder : this->disk_folder;
	const paf::string filename = folder + "/" + file->filename;
	SceUID fd = sceIoOpen(filename.c_str(), SCE_O_RDONLY, 0666);
	if (fd < 0) {
		result = EVerifyResult::MISSING;
		return 0;
	}

	SHA1Context context;
	SHA1Reset(&context);
	uint64_t file_done = 0;
	while (true) {
		int ret = sceIoRead(fd, buffer.data(), buffer.size());
		if (ret < 0) {
			sceIoClose(fd);
			return ret;
		}
		if (ret == 0) {
			break;
		}
		SHA1Input(&context, buffer.data(), ret);
		file_done += ret;
		OnFileProgress(file_done, file->size);
	}
	sceIoClose(fd);

	uint8_t hash1[20];
	SHA1Result(&context, hash1);
	uint8_t hash2[20];
	hexDecode(file->sha1, hash2);
	if (sce_paf_memcmp(hash1, hash2, sizeof(hash1)) != 0) {
		result = EVerifyResult::HASH_FAILED;
		return 0;
	}
	result = EVerifyResult::OK;
	return 0;
}

int32_t VerifyJob::Run()
{
	paf::vector<VerifyResult> results;
	uint64_t total_done = 0;

	int validCount = 0;
	int brokenCount = 0;
	int missingCount = 0;

	for (int i = 0; i < this->game_version->file_count; i++) {
		const GameFile* file = &this->game_version->files[i];

		this->OnFileStart(file->filename);

		auto OnFileProgress = [this, total_done](uint64_t file_done, uint64_t file_size) {
			this->OnProgress(total_done + file_done, file_done, file_size);
		};

		sceClibPrintf("verifying: %s\n", file->filename.c_str());

		VerifyResult verifyResult;
		verifyResult.gameFile = file;
		verifyResult.result = EVerifyResult::INVALID;
		int ret = this->verifyFile(file, verifyResult.result, OnFileProgress);
		if (ret < 0) {
			verifyResult.result = EVerifyResult::ERROR;
		}
		results.push_back(verifyResult);
		total_done += file->size;

		switch (verifyResult.result) {
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
		this->OnFileVerified(validCount, brokenCount, missingCount);
	}

	this->OnComplete(results);
	return 0;
}
