#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace Marshal::IO {

struct MappedView {
    const char* data = nullptr;
    size_t size = 0;
#ifdef _WIN32
    HANDLE file = INVALID_HANDLE_VALUE;
    HANDLE mapping = nullptr;
#else
    int fd = -1;
#endif

    MappedView() = default;
    MappedView(const MappedView&) = delete;
    MappedView& operator=(const MappedView&) = delete;
    ~MappedView() { Unmap(); }

    bool Map(const std::string& path) {
        Unmap();
#ifdef _WIN32
        file = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) return false;
        LARGE_INTEGER sz;
        if (!GetFileSizeEx(file, &sz) || sz.QuadPart <= 0) return false;
        size = static_cast<size_t>(sz.QuadPart);
        mapping = CreateFileMappingA(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
        if (!mapping) return false;
        data = static_cast<const char*>(MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0));
        return data != nullptr;
#else
        fd = ::open(path.c_str(), O_RDONLY);
        if (fd < 0) return false;
        struct stat st {};
        if (fstat(fd, &st) != 0 || st.st_size <= 0) return false;
        size = static_cast<size_t>(st.st_size);
        void* p = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (p == MAP_FAILED) return false;
        data = static_cast<const char*>(p);
        return true;
#endif
    }

    void Unmap() {
#ifdef _WIN32
        if (data) { UnmapViewOfFile(const_cast<char*>(data)); data = nullptr; }
        if (mapping) { CloseHandle(mapping); mapping = nullptr; }
        if (file != INVALID_HANDLE_VALUE) { CloseHandle(file); file = INVALID_HANDLE_VALUE; }
#else
        if (data && size) { munmap(const_cast<char*>(data), size); data = nullptr; size = 0; }
        if (fd >= 0) { close(fd); fd = -1; }
#endif
    }
};

inline std::vector<size_t> ScanLineOffsets(const MappedView& mv) {
    std::vector<size_t> starts;
    if (!mv.data || !mv.size) return starts;
    starts.reserve(mv.size / 18 + 8);
    starts.push_back(0);
    for (size_t i = 0; i < mv.size; ++i)
        if (mv.data[i] == '\n') starts.push_back(i + 1);
    return starts;
}

inline bool ParseLineView(const char* b, const char* e, double& out) {
    while (b < e && (*b == ' ' || *b == '\t' || *b == '\r')) ++b;
    if (b >= e) return false;
    char* end = nullptr;
    out = std::strtod(b, &end);
    return end != b;
}

}  // namespace Marshal::IO
