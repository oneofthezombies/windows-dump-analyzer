#include "DumpFile.h"

namespace wda {
    DumpFile::DumpFile(const std::string& dumpPath, const int64_t dumpFileSize, MappedViewOfFile&& mappedViewOfFile)
        : _dumpPath(dumpPath)
        , _dumpFileSize(dumpFileSize)
        , _mappedViewOfFile(std::move(mappedViewOfFile)) {
    }

    DumpFileResult openDumpFile(const std::string& dumpFilePath) {
        const WindowsHandle fileHandle(::CreateFileA(
            dumpFilePath.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL));
        if (fileHandle.get() == INVALID_HANDLE_VALUE) {
            return Failure(::GetLastError(), StringBuilder() << "failed to open dump file. path: " << dumpFilePath);
        }

        FILE_STANDARD_INFO fileStandardInfo{};
        if (::GetFileInformationByHandleEx(
            fileHandle.get(),
            FileStandardInfo,
            &fileStandardInfo,
            sizeof(fileStandardInfo)) == FALSE) {
            return Failure(::GetLastError(), StringBuilder() << "failed to get file information. path: " << dumpFilePath);
        }

        if (fileStandardInfo.AllocationSize.QuadPart == 0) {
            return Failure(::GetLastError(), StringBuilder() << "file size is zero. path: " << dumpFilePath);
        }

        const int64_t fileSize = fileStandardInfo.EndOfFile.QuadPart;

        const WindowsHandle mappingHandle(::CreateFileMappingA(
            fileHandle.get(),
            NULL,
            PAGE_READONLY,
            NULL,
            NULL,
            NULL));
        if (mappingHandle.get() == NULL) {
            return Failure(::GetLastError(), StringBuilder() << "failed to map file. path: " << dumpFilePath);
        }

        MappedViewOfFile mappedViewOfFile(::MapViewOfFileEx(
            mappingHandle.get(),
            FILE_MAP_READ,
            NULL,
            NULL,
            NULL,
            NULL));
        if (mappedViewOfFile.get() == NULL) {
            return Failure(::GetLastError(), StringBuilder() << "failed to map view of file. path: " << dumpFilePath);
        }

        const char minidumpHeader[] = "MDMP";
        const size_t minidumpHeaderSize = sizeof(minidumpHeader) - 1;
        if (::memcmp(mappedViewOfFile.get(), minidumpHeader, minidumpHeaderSize) != 0) {
            return Failure(::GetLastError(), StringBuilder() << "not a minidump file. path: " << dumpFilePath);
        }

        return DumpFile(dumpFilePath, fileSize, std::move(mappedViewOfFile));
    }
}