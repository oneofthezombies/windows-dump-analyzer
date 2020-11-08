#include "Common.h"
#include <ostream>

namespace wda {
    Failure::Failure(DWORD code, std::string&& message)
        : code(static_cast<uint32_t>(code))
        , message(std::move(message)) {
    }

    std::ostream& operator<<(std::ostream& ostream, const Failure& failure) {
        ostream << "failure code: " << failure.code << ", message: " << failure.message;
        return ostream;
    }

    StringBuilder::StringBuilder() {
    }

    StringBuilder::operator std::string() const {
        return std::move(_stringstream.str());
    }

    void WindowsHandleDeleter::operator()(Type handle) {
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

    void MappedViewOfFileDeleter::operator()(Type startingAddress) {
        if (startingAddress == nullptr) {
            return;
        }

        if (::UnmapViewOfFileEx(startingAddress, MEM_UNMAP_WITH_TRANSIENT_BOOST) == TRUE) {
            return;
        }

        // critical error
    }
}