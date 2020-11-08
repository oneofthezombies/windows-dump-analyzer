#include <iostream>
#include "DumpFile.h"

int main()
{
    using namespace wda;

    DumpFileResult dumpFileResult = openDumpFile("chrome.dmp");
    if (!dumpFileResult) {
        std::cout << dumpFileResult.getFailureResult();
    } else {
        auto& dumpFile = dumpFileResult.getSuccessResult();
        auto exceptionInfoResult = dumpFile.readExceptionStream();
        if (exceptionInfoResult) {
            std::cout << "dump has exception info.\n";
        } else {
            std::cout << "dump has not exception info.\n";
        }
        auto systemInfoResult = dumpFile.readSystemInfo();
        if (systemInfoResult) {
            std::cout << "dump has system info.\n";
        } else {
            std::cout << "dump has not system info.\n";
        }
        auto threadListResult = dumpFile.readThreadList();
        if (threadListResult) {
            std::cout << "dump has thread list.\n";
        } else {
            std::cout << "dump has not thread list.\n";
        }
        auto moduleListResult = dumpFile.readModuleList();
        if (moduleListResult) {
            std::cout << "dump has module list.\n";
        } else {
            std::cout << "dump has not module list.\n";
        }
    }

    return 0;
}
