#include "VerifyJob.hpp"

#include "GameFiles.hpp"
#include "sha256.h"
#include "util.hpp"

#include <psp2/io/fcntl.h>
#include <psp2/kernel/clib.h>

VerifyJob::VerifyJob(const paf::string& cdFolder, const paf::string& diskFolder) : paf::job::JobItem("Verify")
{
	this->cd_folder = cdFolder;
	this->disk_folder = diskFolder;
}

int VerifyJob::verifyFile(
	const GameFile& file,
	EVerifyResult& result,
	std::function<void(uint64_t, float)> OnFileProgress
)
{
	paf::vector<uint8_t> buffer;
	buffer.resize(512_KiB);

	const paf::string& folder = file.folder == DIR_CD ? this->cd_folder : this->disk_folder;
	const paf::string filename = folder + "/" + file.filename;
	SceUID fd = sceIoOpen(filename.c_str(), SCE_O_RDONLY, 0666);
	if (fd < 0) {
		result = EVerifyResult::MISSING;
		return 0;
	}

	// hash the file
	crypto_hash_sha256_state state;
	crypto_hash_sha256_init(&state);
	uint64_t file_done = 0;
	int prev_percent = -1;
	while (true) {
		int ret = sceIoRead(fd, buffer.data(), buffer.size());
		if (ret < 0) {
			sceIoClose(fd);
			return ret;
		}
		if (ret == 0) {
			break;
		}
		crypto_hash_sha256_update(&state, buffer.data(), ret);
		file_done += ret;

		int new_percent = (float) file_done / (float) file.size * 100;
		if (prev_percent != new_percent) {
			OnFileProgress(file_done, new_percent);
		}
		prev_percent = new_percent;
	}
	sceIoClose(fd);

	uint8_t hash1[32];
	crypto_hash_sha256_final(&state, hash1);

	uint8_t hash2[32];
	hexDecode(file.sha256, hash2);

	if (sce_paf_memcmp(hash1, hash2, sizeof(hash1)) != 0) {
		result = EVerifyResult::HASH_FAILED;
		return 0;
	}
	result = EVerifyResult::OK;
	return 0;
}

void VerifyJob::Run()
{
	paf::vector<VerifyResult> results;
	uint64_t total_size = 0;
	uint64_t total_done = 0;

	for (const auto& file : gameFiles) {
		total_size += file.size;
	}

	for (const auto& file : gameFiles) {
		this->OnFileStart(file.filename);

		uint64_t file_done = 0;
		auto OnFileProgress = [this, &total_done, &file_done, total_size](uint64_t done, float file_percent) {
			uint64_t added = done - file_done;
			total_done += added;
			file_done = done;
			float total_percent = ((float) total_done / (float) total_size) * 100;
			this->OnProgress(total_done, total_percent, file_percent);
		};

		sceClibPrintf("verifying: %s\n", file.filename.c_str());

		VerifyResult verifyResult;
		verifyResult.gameFile = &file;
		verifyResult.result = EVerifyResult::INVALID;
		int ret = this->verifyFile(file, verifyResult.result, OnFileProgress);
		if (ret < 0) {
			verifyResult.result = EVerifyResult::ERROR;
		}
		results.push_back(verifyResult);
	}

	this->OnComplete(results);
}
