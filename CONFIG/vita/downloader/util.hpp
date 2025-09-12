#pragma once

#include <paf.h>

class ScopeLockMain {
    ScopeLockMain() {
        paf::thread::RMutex::MainThreadMutex()->Lock();
    };
    ~ScopeLockMain() {
        paf::thread::RMutex::MainThreadMutex()->Unlock();
    }
};

void listFilesRecursive(const paf::string& base, const paf::string& dir, paf::vector<paf::string>& filePaths);

paf::string removeFileName(const paf::string& path);

int mkdirAll(const paf::string& dir);

paf::wstring StringToWString(const paf::string& str);

uint64_t GetFreeSpace(const char* device);

constexpr std::size_t operator""_KiB(unsigned long long int x) {
  return 1024ULL * x;
}

constexpr std::size_t operator""_MiB(unsigned long long int x) {
  return 1024_KiB * x;
}

constexpr std::size_t operator""_GiB(unsigned long long int x) {
  return 1024_MiB * x;
}

constexpr std::size_t operator""_TiB(unsigned long long int x) {
  return 1024_GiB * x;
}

constexpr std::size_t operator""_PiB(unsigned long long int x) {
  return 1024_TiB * x;
}

bool hexDecode(const paf::string& hex, uint8_t* out_bytes);
