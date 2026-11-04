#pragma once
#include <Windows.h>
#include <string>

namespace subprocess {

    struct PipeHandle {
        HANDLE handle = nullptr;

        inline void set_inherit(bool value) noexcept {
            if (handle) {
                SetHandleInformation(handle, HANDLE_FLAG_INHERIT, value ? HANDLE_FLAG_INHERIT : 0);
            }
        }

        inline void close() noexcept {
            if (handle) {
                CloseHandle(handle);
                handle = nullptr;
            }
        }
    };

    struct PipePair {
        PipeHandle m_read;
        PipeHandle m_write;

        static PipePair create(bool inheritable, DWORD buffer_size = 4194304) noexcept {
            SECURITY_ATTRIBUTES security = {};
            security.nLength = sizeof(security);
            security.bInheritHandle = inheritable;
            HANDLE read, write;
            CreatePipe(&read, &write, &security, buffer_size);
            return { { read }, { write } };
        }

        inline bool write(const void* const data, size_t size) noexcept {
            DWORD bytes_written;
            BOOL success = WriteFile(m_write.handle, data, static_cast<DWORD>(size), &bytes_written, nullptr);
            return success && (bytes_written == size);
        }

        inline void close() noexcept {
            m_read.close();
            m_write.close();
        }
    };

    class Popen {
    public:
        PipePair m_stdin;
        PROCESS_INFORMATION m_proc_info{};
    public:
        Popen() noexcept = default;
        
        Popen(const std::string& command, bool hide_window = true) {
            STARTUPINFOW start_info = {};
            start_info.cb = sizeof(start_info);
            start_info.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            start_info.hStdError = GetStdHandle(STD_ERROR_HANDLE);
            start_info.dwFlags |= STARTF_USESTDHANDLES;

            m_stdin = PipePair::create(true, 4194304);
            start_info.hStdInput = m_stdin.m_read.handle;
            m_stdin.m_write.set_inherit(false);

            int size = MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, nullptr, 0);
            std::wstring wcommand(size, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, &wcommand[0], size);

            DWORD flags = hide_window ? CREATE_NO_WINDOW : 0;

            CreateProcessW(nullptr, &wcommand[0], nullptr, nullptr, true, flags, nullptr, nullptr, &start_info, &m_proc_info);

            m_stdin.m_read.close();
        }

        inline int wait() noexcept {
            WaitForSingleObject(m_proc_info.hProcess, INFINITE);
            DWORD exit_code;
            GetExitCodeProcess(m_proc_info.hProcess, &exit_code);
            return exit_code;
        }

        inline int close(bool should_wait = true) noexcept {
            int exit_code = 0;
            m_stdin.close();
            if (should_wait) exit_code = wait();
            if (m_proc_info.hProcess) {
                CloseHandle(m_proc_info.hProcess);
                m_proc_info.hProcess = nullptr;
            }
            if (m_proc_info.hThread) {
                CloseHandle(m_proc_info.hThread);
                m_proc_info.hThread = nullptr;
            }
            return exit_code;
        }
    };
}