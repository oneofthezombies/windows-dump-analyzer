// windows-dump-analyzer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <Windows.h>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

struct Error {
    Error(const DWORD code, std::string&& message)
        : code(static_cast<uint32_t>(code))
        , message(std::move(message)) {}

    uint32_t code;
    std::string message;
};

std::ostream& operator<<(std::ostream& ostream, const Error& error) {
    ostream << "error code: " << error.code << ", message: " << error.message;
    return ostream;
}

struct WindowsHandle {
    struct Deleter {
        void operator()(HANDLE handle) {
            if (handle == nullptr) {
                return;
            }

            if (handle == INVALID_HANDLE_VALUE) {
                return;
            }

            if (::CloseHandle(handle) == TRUE) {
                return;
            }

            // critical error
        }
    };

    using Pointer = std::unique_ptr<std::remove_pointer<HANDLE>::type, Deleter>;

    WindowsHandle() = delete;
    WindowsHandle& operator=(const WindowsHandle&) = delete;
};

struct MappedViewOfFile {
    struct Deleter {
        void operator()(PVOID startingAddress) {
            if (startingAddress == nullptr) {
                return;
            }

            if (::UnmapViewOfFileEx(startingAddress, MEM_UNMAP_WITH_TRANSIENT_BOOST) == TRUE) {
                return;
            }

            // critical error
        }
    };

    using Pointer = std::unique_ptr<std::remove_pointer<PVOID>::type, Deleter>;

    MappedViewOfFile() = delete;
    MappedViewOfFile& operator=(const MappedViewOfFile&) = delete;
};

struct StringBuilder {
    StringBuilder() {}

    template<typename T>
    StringBuilder& operator<<(T&& value) {
        stringstream_ << std::forward<T>(value);
        return *this;
    }

    operator std::string() const {
        return std::move(stringstream_.str());
    }

private:
    std::stringstream stringstream_;
};

struct DumpFile
{
    using Pointer = std::unique_ptr<DumpFile>;

    DumpFile(const std::string& dumpPath, const int64_t dumpFileSize, MappedViewOfFile::Pointer&& mappedViewOfFile)
        : dumpPath_(dumpPath)
        , dumpFileSize_(dumpFileSize)
        , mappedViewOfFile_(std::move(mappedViewOfFile)) {
    }

    static DumpFile::Pointer open(const std::string& dumpFilePath, std::function<void(Error)> onError = std::function<void(Error)>{}) {
        const WindowsHandle::Pointer fileHandle(::CreateFileA(
            dumpFilePath.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL));
        if (fileHandle.get() == INVALID_HANDLE_VALUE) {
            onError(Error(::GetLastError(), StringBuilder() << "failed to open dump file. path: " << dumpFilePath));
            return nullptr;
        }

        FILE_STANDARD_INFO fileStandardInfo{};
        if (::GetFileInformationByHandleEx(
            fileHandle.get(),
            FileStandardInfo,
            &fileStandardInfo,
            sizeof(fileStandardInfo)) == FALSE) {
            onError(Error(::GetLastError(), StringBuilder() << "failed to get file information. path: " << dumpFilePath));
            return nullptr;
        }

        if (fileStandardInfo.AllocationSize.QuadPart == 0) {
            onError(Error(::GetLastError(), StringBuilder() << "file size is zero. path: " << dumpFilePath));
            return nullptr;
        }

        const int64_t fileSize = fileStandardInfo.EndOfFile.QuadPart;

        const WindowsHandle::Pointer mappingHandle(::CreateFileMappingA(
            fileHandle.get(),
            NULL,
            PAGE_READONLY,
            NULL,
            NULL,
            NULL));
        if (mappingHandle.get() == NULL) {
            onError(Error(::GetLastError(), StringBuilder() << "failed to map file. path: " << dumpFilePath));
            return nullptr;
        }

        MappedViewOfFile::Pointer mappedViewOfFile(::MapViewOfFileEx(
            mappingHandle.get(),
            FILE_MAP_READ,
            NULL,
            NULL,
            NULL,
            NULL));
        if (mappedViewOfFile.get() == NULL) {
            onError(Error(::GetLastError(), StringBuilder() << "failed to map view of file. path: " << dumpFilePath));
            return nullptr;
        }

        const char minidumpHeader[] = "MDMP";
        const size_t minidumpHeaderSize = sizeof(minidumpHeader) - 1;
        if (::memcmp(mappedViewOfFile.get(), minidumpHeader, minidumpHeaderSize) != 0) {
            onError(Error(::GetLastError(), StringBuilder() << "not a minidump file. path: " << dumpFilePath));
            return nullptr;
        }

        return std::make_unique<DumpFile>(dumpFilePath, fileSize, std::move(mappedViewOfFile));
    }

private:
    std::string dumpPath_;
    int64_t dumpFileSize_;
    MappedViewOfFile::Pointer mappedViewOfFile_;
};


int main()
{
    std::cout << "Hello World!\n";
    DumpFile::Pointer dumpFile = DumpFile::open("chrome.dmp", [](Error error) {
        std::cout << error;
        });

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
