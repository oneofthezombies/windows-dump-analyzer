#pragma once
#include <Windows.h>
#include <DbgHelp.h>
#include <sstream>
#include <variant>
#include <string>
#include <memory>

namespace wda {
    struct Failure {
        Failure(DWORD code, std::string&& message);

        uint32_t code;
        std::string message;
    };

    std::ostream& operator<<(std::ostream& ostream, const Failure& failure);

    template<typename T>
    struct Result {
        Result(T&& value) : _value(std::forward<T>(value)) {
        }

        Result(Failure&& failure) : _value(std::move(failure)) {
        }

        operator bool() const {
            return isSuccess();
        }

        bool isSuccess() const {
            return std::get_if<T>(&_value) != nullptr;
        }

        const T& getSuccessResult() const {
            return *std::get_if<T>(&_value);
        }

        const Failure& getFailureResult() const {
            return *std::get_if<Failure>(&_value);
        }

    private:
        std::variant<T, Failure> _value;
    };

    struct StringBuilder {
        StringBuilder();

        template<typename T>
        StringBuilder& operator<<(T&& value) {
            _stringstream << std::forward<T>(value);
            return *this;
        }

        operator std::string() const;

    private:
        std::stringstream _stringstream;
    };

    struct WindowsHandleDeleter {
        using Type = HANDLE;
        void operator()(Type handle);
    };

    using WindowsHandle = std::unique_ptr<std::remove_pointer<WindowsHandleDeleter::Type>::type, WindowsHandleDeleter>;

    struct MappedViewOfFileDeleter {
        using Type = PVOID;
        void operator()(Type startingAddress);
    };

    using MappedViewOfFile = std::unique_ptr<std::remove_pointer<MappedViewOfFileDeleter::Type>::type, MappedViewOfFileDeleter>;

    template<typename T>
    struct DumpInfo {
        DumpInfo(MINIDUMP_DIRECTORY* minidumpDirectory, T* info)
            : minidumpDirectory(minidumpDirectory)
            , info(info) {
        }

        MINIDUMP_DIRECTORY* minidumpDirectory;
        T* info;
    };

    template<typename T>
    using DumpInfoResult = Result<DumpInfo<T>>;
}