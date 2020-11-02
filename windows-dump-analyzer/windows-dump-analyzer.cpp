// windows-dump-analyzer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <Windows.h>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

class DumpFile
{
public:
    DumpFile(const std::string& dumpPath)
        : dumpPath_(dumpPath) {
        std::cout << "Dump()\n";

    }

    ~DumpFile() {
        std::cout << "~Dump()\n";

    }

private:
    std::string dumpPath_;
};

struct Error {
    Error(const uint32_t code, std::string&& message)
        : code(code), message(std::move(message)) {}

    uint32_t code;
    std::string message;
};

struct WindowsHandle {
    WindowsHandle(HANDLE handle) 
        : handle(handle) {}

    ~WindowsHandle() {
        if (handle != nullptr) {
            if (::CloseHandle(handle) == FALSE) {
                // critical error
            }
        }
    }

    HANDLE handle;
};

struct WindowsMappedViewOfFile {
    WindowsMappedViewOfFile(PVOID startingAddress)
        : startingAddress(startingAddress) {}

    ~WindowsMappedViewOfFile() {
        if (startingAddress != nullptr) {
            if (::UnmapViewOfFileEx(startingAddress, MEM_UNMAP_WITH_TRANSIENT_BOOST) == FALSE) {
                // critical error
            }
        }
    }

    PVOID startingAddress;
};

class StringBuilder {
public:
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

void create(const std::string& dumpFilePath, std::function<void(DumpFile)> onSuccess, std::function<void(Error)> onError = std::function<void(Error)>{}) {
    WindowsHandle fileHandle(::CreateFileA(
        dumpFilePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL));
    if (fileHandle.handle == INVALID_HANDLE_VALUE) {
        onError(Error(static_cast<uint32_t>(::GetLastError()), StringBuilder() << "failed to open dump file. path: " << dumpFilePath));
        return;
    }

    FILE_STANDARD_INFO fileStandardInfo{};
    if (::GetFileInformationByHandleEx(
        fileHandle.handle,
        FileStandardInfo,
        &fileStandardInfo,
        sizeof(fileStandardInfo)) == FALSE) {
        onError(Error(static_cast<uint32_t>(::GetLastError()), StringBuilder() << "failed to get file information. path: " << dumpFilePath));
        return;
    }

    if (fileStandardInfo.AllocationSize.QuadPart == 0) {
        onError(Error(static_cast<uint32_t>(::GetLastError()), StringBuilder() << "file size is zero. path: " << dumpFilePath));
        return;
    }

    WindowsHandle mappingHandle(::CreateFileMappingA(
        fileHandle.handle,
        NULL,
        PAGE_READONLY,
        NULL,
        NULL,
        NULL));
    if (mappingHandle.handle == NULL) {
        onError(Error(static_cast<uint32_t>(::GetLastError()), StringBuilder() << "failed to map file. path: " << dumpFilePath));
        return;
    }

    WindowsMappedViewOfFile mappedViewOfFile(::MapViewOfFileEx(
        mappingHandle.handle, 
        FILE_MAP_READ, 
        NULL, 
        NULL, 
        NULL, 
        NULL));
    if (mappedViewOfFile.startingAddress == NULL) {
        onError(Error(static_cast<uint32_t>(GetLastError()), StringBuilder() << "failed to map view of file. path: " << dumpFilePath));
        return;
    }

    const std::string minidumpHeader("MDMP");
    if (::memcmp(mappedViewOfFile.startingAddress, minidumpHeader.c_str(), minidumpHeader.size()) != 0) {
        onError(Error(static_cast<uint32_t>(GetLastError()), StringBuilder() << "not a minidump file. path: " << dumpFilePath));
        return;
    }


}

int main()
{
    std::cout << "Hello World!\n";
    create("test2.txt", [](DumpFile file) {
        }, [](Error error) {
            int i = 0;
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
