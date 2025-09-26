#include "util.hpp"

#include <psp2/io/devctl.h>
#include <psp2/io/dirent.h>
#include <psp2/kernel/clib.h>

void listFilesRecursive(const paf::string& base, const paf::string& dir, paf::vector<paf::string>& filePaths)
{
	paf::string full_dir = base + "/" + dir;
	SceUID dir_fd = sceIoDopen(full_dir.c_str());
	if (dir_fd < 0) {
		return;
	}
	while (true) {
		SceIoDirent dir_ent;
		int ret = sceIoDread(dir_fd, &dir_ent);
		if (ret == 0) {
			break;
		}
		if (ret < 0) {
			break;
		}
		paf::string filename = paf::string(dir_ent.d_name);
		if (dir.size()) {
			filename = dir + "/" + filename;
		}
		if (SCE_S_ISDIR(dir_ent.d_stat.st_mode)) {
			listFilesRecursive(base, filename, filePaths);
			continue;
		}
		filePaths.push_back(filename);
	}
	sceIoDclose(dir_fd);
}

paf::string removeFileName(const paf::string& path)
{
	paf::string& path_ = (paf::string&) path;
	size_t last_index = path_.rfind('/', 0xffffffff);
	if (last_index == 0xffffffff) {
		return "";
	}
	return path_.substr(0, last_index);
}

int mkdirAll(const paf::string& dir)
{
	if (dir.empty()) {
		return 0;
	}
	int ret = sceIoMkdir(dir.c_str(), 0777);
	if (ret == 0x80010002) { // ENOENT
		paf::string sub = removeFileName(dir);
		ret = mkdirAll(sub);
		if (ret < 0) {
			return ret;
		}
		ret = sceIoMkdir(dir.c_str(), 0777);
	}
	if (ret == 0x80010011) { // EEXIST
		ret = 0;
	}
	return ret;
}

paf::wstring StringToWString(const paf::string& str)
{
	paf::wstring wstr;
	wstr.resize(str.length());
	paf::mbstowcs(wstr.c_str(), str.c_str(), str.size());
	return wstr;
}

uint64_t GetFreeSpace(const char* device)
{
	uint64_t free_space = 0;

	// host0 will always report as 0 bytes free
	if (sceClibStrcmp("host0:", device) == 0) {
		return 0xFFFFFFFFFFFFFFFFull;
	}

	SceIoDevInfo info;
	int res = sceIoDevctl(device, 0x3001, NULL, 0, &info, sizeof(SceIoDevInfo));
	if (res < 0) {
		free_space = 0;
	}
	else {
		free_space = info.free_size;
	}

	return free_space;
}

static int getHexValue(char hexChar)
{
	if (hexChar >= '0' && hexChar <= '9') {
		return hexChar - '0';
	}
	if (hexChar >= 'A' && hexChar <= 'F') {
		return 10 + (hexChar - 'A');
	}
	if (hexChar >= 'a' && hexChar <= 'f') {
		return 10 + (hexChar - 'a');
	}
	return -1;
}

bool hexDecode(const paf::string& hex, uint8_t* out)
{
	if (hex.length() % 2 != 0) {
		return false;
	}

	for (size_t i = 0; i < hex.length(); i += 2) {
		int highNibble = getHexValue(hex.c_str()[i]);
		int lowNibble = getHexValue(hex.c_str()[i + 1]);

		if (highNibble == -1 || lowNibble == -1) {
			return false;
		}

		uint8_t byte = (highNibble << 4) | lowNibble;
		*out = byte;
		out++;
	}

	return true;
}

ButtonListener::ButtonListener(std::function<void()> OnClick)
{
	this->OnClick = OnClick;
}

int32_t ButtonListener::Do(int32_t type, paf::ui::Handler* self, paf::ui::Event* e)
{
	this->OnClick();
	return 0;
}

void AddClickCallback(paf::ui::ButtonBase* button, std::function<void()> func)
{
	button->AddEventListener(paf::ui::ButtonBase::CB_BTN_DECIDE, new ButtonListener(func));
}
