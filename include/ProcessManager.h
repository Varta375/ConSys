#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <windows.h>
#include <psapi.h>
#include <sddl.h>
#include <string>
#include <tlhelp32.h>
#include <vector>


#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "psapi.lib")

/**
 * @struct ProcessInfo
 * @brief Структура для хранения информации об ID, имени процесса и определения его принадлежности к системным процессам
 */
struct ProcessInfo {
    DWORD pid; ///< ID процесса
    std::string name; ///< Имя процесса
    bool isSystem{ false }; ///< Фильтр процесса (системный или пользовательский)
};

/**
 * @class ProcessManager
 * @brief Класс для создания снимка действующих процессов системы
 * @details Программа использует библиотеку Windows.h для создания снимка системы. Позволяет получать список процессов и обновлять его при необходимости.
 *          Кроме того, существует возможность приостановить процесс (плавно, с таймаутом) из списка.
 * @warning Приостановка процесса некоторых программ (например, вида UWP) не сработает по причине его передачи ApplicationFrameHost.exe.
 *          При его закрытии закроются все зависимые от него программы.
 * @see https://learn.microsoft.com/ru-ru/windows/uwp/get-started/universal-application-platform-guide
 */
class ProcessManager {
public:
    /**
    * @brief Отвечает за получение списка процессов и его обновление
    * @details Метод создает снэпшот состояния системы путем вызова CreateToolhelp32Snapshot с флагом TH32CS_SNAPPROCESS и ID 0 (сохранение всех процессов).
    *          После этого пробегается по полученному снэпшоту, фильтрует его и сохраняет во внутренний вектор Processes.
    *          Работа кода строится на Windows API (библиотека Tool Help Library).
    * @return Вектор процессов системы, прошедших фильтрацию. В случае критического сбоя возвращается пустой или частично заполненный вектор.
    * @throw std::exception Метод не выбрасывает данное исключение наружу. Любые возникающие стандартные
    *        исключения перехватываются внутри, выбрасываются с помощью std::cerr.
    * @see https://learn.microsoft.com/ru-ru/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes
    * @see https://learn.microsoft.com/ru-ru/windows/win32/api/tlhelp32/nf-tlhelp32-process32first?source=recommendations
    * @see https://learn.microsoft.com/ru-ru/windows/win32/api/tlhelp32/nf-tlhelp32-process32nextw
    * @see https://learn.microsoft.com/ru-ru/windows/win32/api/stringapiset/nf-stringapiset-widechartomultibyte
    */
    const std::vector<ProcessInfo>& Refresh();

    /**
    * @brief Отвечает за получение списка процессов системы
    * @details Запускает Refresh()
    * @param refresh Метод возвращает кэшированный список при false или возвращает обновленный список при true
    * @return Вектор процессов системы, прошедших фильтрацию.
    * @throw std::exception Исключения не выбрасываются наружу. Все потенциальные исключения при обновлении
    *        списка перехватываются и обрабатываются внутри метода Refresh().
    */
    const std::vector<ProcessInfo>& GetList(bool refresh = false);

    /**
    * @brief Отвечает за двухэтапное завершение процесса (мягкое и жесткое закрытие, с таймаутом)
    * @details Сначала ищется PID в векторе Processes. Если он нашелся, то открывается его дескриптор.
    *          PROCESS_TERMINATE отвечает за принудительное завершение процесса. Срабатывает только в том случае,
    *          если не сработает мягкое закрытие WM_CLOSE.
    * @param pid Значение ID процесса для его приостановки
    * @see https://learn.microsoft.com/ru-ru/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocess
    * @see https://learn.microsoft.com/ru-ru/windows/win32/api/winuser/nf-winuser-enumwindows
    * @see https://learn.microsoft.com/ru-ru/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject
    * @return Флаг, указывающий на успех (true) или неудачу (false) приостановки процесса.
    * @throw std::exception Метод не выбрасывает исключений наружу. Любые ошибки или исключения при обработке
    *        процесса перехватываются внутри, открытый дескриптор безопасно закрывается, а метод возвращает false.
    */
    bool StopProcess(DWORD pid);

private:
    std::vector<ProcessInfo> Processes; ///< Вектор для сохранения информации о процессах
};

#endif