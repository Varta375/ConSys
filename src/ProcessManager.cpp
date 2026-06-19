#include "ProcessManager.h"
#include <algorithm>
#include <exception>
#include <iostream>

// Snapshot source : https://learn.microsoft.com/ru-ru/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes
const std::vector<ProcessInfo>& ProcessManager::Refresh() {
    HANDLE Snapshot = INVALID_HANDLE_VALUE;
    try {
        Processes.clear();

        Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (Snapshot == INVALID_HANDLE_VALUE)
            return Processes;

        PROCESSENTRY32W pe {};
        pe.dwSize = sizeof(pe); // WinAPI требует указывать размер структуры в dwSize

        for (bool more = Process32FirstW(Snapshot, &pe); more; // Контролирует наличие следующего элемента
            more = Process32NextW(Snapshot, &pe)) {
            // Двухэтапная конвертация UTF-16 (wchar_t) в UTF-8 (std::string):
            int len = WideCharToMultiByte(CP_UTF8, 0, pe.szExeFile, -1, nullptr, 0, nullptr,
                nullptr); // Возвращает необходимый размер буфера (включая '\0')

            std::string name(len - 1, '\0');

            WideCharToMultiByte(CP_UTF8, 0, pe.szExeFile, -1, name.data(), len, nullptr, nullptr);

            if (name == "svchost.exe" || name == "dllhost.exe" || name == "ConSys.exe"
                || name == "ApplicationFrameHost.exe") {
                continue;
            }

            HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE,
                pe.th32ProcessID); // Открываем процесс для проверки на его системность
            if (!hProc) {
                continue;
            }

            try {
                ProcessInfo info;
                info.pid = pe.th32ProcessID;
                info.name = name;
                info.isSystem = false;
                Processes.push_back(info);
            } catch (...) {
                CloseHandle(hProc);
                throw;
            }

            CloseHandle(hProc);
        }

        CloseHandle(Snapshot);
    } catch (const std::exception& e) {
        if (Snapshot != INVALID_HANDLE_VALUE) {
            CloseHandle(Snapshot);
        }
        std::cerr << "Exception in Refresh(): " << e.what() << std::endl;
    } catch (...) {
        if (Snapshot != INVALID_HANDLE_VALUE) {
            CloseHandle(Snapshot);
        }
        std::cerr << "Unknown exception in Refresh()" << std::endl;
    }

    return Processes;
}
// Snapshot source : https://learn.microsoft.com/ru-ru/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes

const std::vector<ProcessInfo>& ProcessManager::GetList(bool refresh) {
    try {
        if (refresh)
            return Refresh();
        return Processes;
    } catch (const std::exception& e) {
        std::cerr << "Exception in GetList(): " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception in GetList()" << std::endl;
    }
    return Processes;
}

bool ProcessManager::StopProcess(DWORD pid) {
    HANDLE hProc = nullptr;
    try {
        DWORD timeoutMs { 5000 };

        auto it
            = std::find_if(Processes.begin(), Processes.end(), [pid](const ProcessInfo& p) { return p.pid == pid; });
        if (it == Processes.end() || it->isSystem)
            return false;

        hProc = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, pid);
        if (!hProc)
            return false;

        EnumWindows(
            [](HWND hWnd, LPARAM lParam) -> BOOL {
                DWORD windowPid;
                GetWindowThreadProcessId(hWnd, &windowPid);
                if (windowPid == (DWORD)lParam) {
                    // PostMessage работает асинхронно, отправляя WM_CLOSE в очередь сообщений окна
                    PostMessage(hWnd, WM_CLOSE, 0, 0);
                }
                return TRUE;
            },
            pid);

        DWORD waitResult = WaitForSingleObject(hProc, timeoutMs);

        bool ok;
        if (waitResult == WAIT_OBJECT_0) {
            // Процесс успешно завершился самостоятельно в рамках таймаута
            ok = true;
        } else {
            ok = TerminateProcess(hProc, 1);
        }

        CloseHandle(hProc);
        hProc = nullptr;

        if (ok)
            Refresh();
        return ok;
    } catch (const std::exception& e) {
        if (hProc) {
            CloseHandle(hProc);
        }
        std::cerr << "Exception in StopProcess(): " << e.what() << std::endl;
        return false;
    } catch (...) {
        if (hProc) {
            CloseHandle(hProc);
        }
        std::cerr << "Unknown exception in StopProcess()" << std::endl;
        return false;
    }
}