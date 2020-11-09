#pragma once
#include "Common.h"

namespace wda {
    struct DumpFile
    {
        DumpFile(const std::string& dumpPath, const int64_t dumpFileSize, MappedViewOfFile&& mappedViewOfFile);

        template<typename T>
        DumpInfoResult<T> read(const MINIDUMP_STREAM_TYPE type) const {
            MINIDUMP_DIRECTORY* minidumpDirectory = nullptr;
            void* streamPointer = nullptr;
            if (!::MiniDumpReadDumpStream(_mappedViewOfFile.get(), type, &minidumpDirectory, &streamPointer, nullptr)) {
                return Error(::GetLastError(), StringBuilder() << "minidump stream information does not exist. type: " << type);
            }

            return DumpInfo(minidumpDirectory, static_cast<T*>(streamPointer));
        }

        DumpInfoResult<MINIDUMP_EXCEPTION_STREAM> readExceptionStream() const {
            return read<MINIDUMP_EXCEPTION_STREAM>(ExceptionStream);
        }

        DumpInfoResult<MINIDUMP_SYSTEM_INFO> readSystemInfo() const {
            return read<MINIDUMP_SYSTEM_INFO>(SystemInfoStream);
        }

        DumpInfoResult<MINIDUMP_THREAD_LIST> readThreadList() const {
            return read<MINIDUMP_THREAD_LIST>(ThreadListStream);
        }

        DumpInfoResult<MINIDUMP_MODULE_LIST> readModuleList() const {
            return read<MINIDUMP_MODULE_LIST>(ModuleListStream);
        }

    private:
        std::string _dumpPath;
        int64_t _dumpFileSize;
        MappedViewOfFile _mappedViewOfFile;
    };

    using DumpFileResult = Result<DumpFile>;

    DumpFileResult openDumpFile(const std::string& dumpFilePath);
}