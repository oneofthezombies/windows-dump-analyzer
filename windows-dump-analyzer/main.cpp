#include <iostream>
#include "DumpFile.h"

int main()
{
    using namespace wda;

    DumpFileResult dumpFileResult = openDumpFile("chrome.dmp");
    if (!dumpFileResult) {
        std::cout << dumpFileResult.error();
    } else {
        auto exceptionInfoResult = dumpFileResult->readExceptionStream();
        if (exceptionInfoResult) {
            std::cout << "dump has exception info.\n";
        } else {
            std::cout << "dump has not exception info.\n";
        }
        auto systemInfoResult = dumpFileResult->readSystemInfo();
        if (systemInfoResult) {
            std::cout << "dump has system info.\n";
        } else {
            std::cout << "dump has not system info.\n";
        }
        auto threadListResult = dumpFileResult->readThreadList();
        if (threadListResult) {
            std::cout << "dump has thread list.\n";
        } else {
            std::cout << "dump has not thread list.\n";
        }
        auto moduleListResult = dumpFileResult->readModuleList();
        if (moduleListResult) {
            std::cout << "dump has module list.\n";
        } else {
            std::cout << "dump has not module list.\n";
        }
    }

    return 0;
}
