#include "./commandlineutils.h"
#include "./argumentparserprivate.h"

#include "../io/ansiescapecodes.h"

#include <iostream>
#include <string>

#include <fcntl.h>
#ifdef PLATFORM_WINDOWS
#include <cstring>
#include <io.h>
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

using namespace std;

namespace CppUtilities {

/*!
 * \brief Prompts for confirmation displaying the specified \a message.
 */
bool confirmPrompt(const char *message, Response defaultResponse)
{
    cout << message;
    cout << ' ' << '[';
    cout << (defaultResponse == Response::Yes ? 'Y' : 'y');
    cout << '/' << (defaultResponse == Response::No ? 'N' : 'n');
    cout << ']' << ' ';
    cout.flush();
    for (string line;;) {
        getline(cin, line);
        if (line == "y" || line == "Y" || (defaultResponse == Response::Yes && line.empty())) {
            return true;
        } else if (line == "n" || line == "N" || (defaultResponse == Response::No && line.empty())) {
            return false;
        } else {
            cout << "Please enter [y] or [n]: ";
            cout.flush();
        }
    }
}

/*!
 * \brief Returns whether the specified env variable is set to a non-zero and non-white-space-only value.
 */
std::optional<bool> isEnvVariableSet(const char *variableName)
{
    const char *envValue = std::getenv(variableName);
    if (!envValue) {
        return std::nullopt;
    }
    for (; *envValue; ++envValue) {
        switch (*envValue) {
        case '0':
        case ' ':
            break;
        default:
            return true;
        }
    }
    return false;
}

/*!
 * \brief Returns the current size of the terminal.
 * \remarks Unknown members of the returned TerminalSize are set to zero.
 */
TerminalSize determineTerminalSize()
{
    TerminalSize size;
#ifndef PLATFORM_WINDOWS
    ioctl(STDOUT_FILENO, TIOCGWINSZ, reinterpret_cast<winsize *>(&size));
#else
    CONSOLE_SCREEN_BUFFER_INFO consoleBufferInfo;
    if (const HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE)) {
        GetConsoleScreenBufferInfo(stdHandle, &consoleBufferInfo);
        if (consoleBufferInfo.dwSize.X > 0) {
            size.columns = static_cast<unsigned short>(consoleBufferInfo.dwSize.X);
        }
        if (consoleBufferInfo.dwSize.Y > 0) {
            size.rows = static_cast<unsigned short>(consoleBufferInfo.dwSize.Y);
        }
    }
#endif
    return size;
}

#ifdef PLATFORM_WINDOWS
/*!
 * \brief Enables virtual terminal processing (and thus processing of ANSI escape codes) of the console
 *        determined by the specified \a nStdHandle.
 * \sa https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
 */
static bool enableVirtualTerminalProcessing(DWORD nStdHandle)
{
    auto stdHandle = GetStdHandle(nStdHandle);
    if (stdHandle == INVALID_HANDLE_VALUE) {
        return false;
    }
    auto dwMode = DWORD();
    if (!GetConsoleMode(stdHandle, &dwMode)) {
        return false;
    }
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    return SetConsoleMode(stdHandle, dwMode);
}

/*!
 * \brief Enables virtual terminal processing (and thus processing of ANSI escape codes) of the console
 *        or disables use of ANSI escape codes if that's not possible.
 */
bool handleVirtualTerminalProcessing()
{
    // try to enable virtual terminal processing
    if (enableVirtualTerminalProcessing(STD_OUTPUT_HANDLE) && enableVirtualTerminalProcessing(STD_ERROR_HANDLE)) {
        return true;
    }
    // disable use on ANSI escape codes otherwise if it makes sense
    const char *const msyscon = std::getenv("MSYSCON");
    if (msyscon && std::strstr(msyscon, "mintty")) {
        return false; // no need to disable escape codes if it is just mintty
    }
    const char *const term = std::getenv("TERM");
    if (term && std::strstr(term, "xterm")) {
        return false; // no need to disable escape codes if it is some xterm-like terminal
    }
    return EscapeCodes::enabled = false;
}

/*!
 * \brief Closes stdout, stdin and stderr and stops the console.
 * \remarks Internally used by startConsole() to close the console when the application exits.
 */
void stopConsole()
{
    fclose(stdout);
    fclose(stdin);
    fclose(stderr);
    if (auto *const consoleWindow = GetConsoleWindow()) {
        PostMessage(consoleWindow, WM_KEYUP, VK_RETURN, 0);
        FreeConsole();
    }
}

/*!
 * \brief Ensure the process has a console attached and sets its output code page to UTF-8.
 * \remarks
 * - Only available (and required) under Windows where otherwise stdout/stderr is not printed to the console (at
 *   least when using `cmd.exe`).
 * - Used to start a console from a GUI application. Does *not* create a new console if the process already has one.
 * - Closes the console automatically when the application exits.
 * - It breaks redirecting stdout/stderr so this can be opted-out by setting the environment
 *   variable `ENABLE_CONSOLE=0` and/or `ENABLE_CP_UTF8=0`.
 * \sa
 * - https://docs.microsoft.com/en-us/windows/console/AttachConsole
 * - https://docs.microsoft.com/en-us/windows/console/AllocConsole
 * - https://docs.microsoft.com/en-us/windows/console/SetConsoleCP
 * - https://docs.microsoft.com/en-us/windows/console/SetConsoleOutputCP
 */
void startConsole()
{
    // attach to the parent process' console or allocate a new console if that's not possible
    const auto consoleEnabled = isEnvVariableSet("ENABLE_CONSOLE");
    if ((!consoleEnabled.has_value() || consoleEnabled.value()) && (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole())) {
        // redirect stdout
        auto stdHandle = reinterpret_cast<intptr_t>(GetStdHandle(STD_OUTPUT_HANDLE));
        auto conHandle = _open_osfhandle(stdHandle, _O_TEXT);
        auto fp = _fdopen(conHandle, "w");
        *stdout = *fp;
        setvbuf(stdout, nullptr, _IONBF, 0);
        // redirect stdin
        stdHandle = reinterpret_cast<intptr_t>(GetStdHandle(STD_INPUT_HANDLE));
        conHandle = _open_osfhandle(stdHandle, _O_TEXT);
        fp = _fdopen(conHandle, "r");
        *stdin = *fp;
        setvbuf(stdin, nullptr, _IONBF, 0);
        // redirect stderr
        stdHandle = reinterpret_cast<intptr_t>(GetStdHandle(STD_ERROR_HANDLE));
        conHandle = _open_osfhandle(stdHandle, _O_TEXT);
        fp = _fdopen(conHandle, "w");
        *stderr = *fp;
        setvbuf(stderr, nullptr, _IONBF, 0);
        // sync
        ios::sync_with_stdio(true);
        // ensure the console prompt is shown again when app terminates
        atexit(stopConsole);
    }

    // set console character set to UTF-8
    const auto utf8Enabled = isEnvVariableSet("ENABLE_CP_UTF8");
    if (!utf8Enabled.has_value() || utf8Enabled.value()) {
        SetConsoleCP(CP_UTF8);
        SetConsoleOutputCP(CP_UTF8);
    }

    // enable virtual terminal processing or disable ANSI-escape if that's not possible
    handleVirtualTerminalProcessing();
}

/*!
 * \brief Convert command line arguments to UTF-8.
 * \remarks Only available on Windows (on other platforms we can assume passed arguments are already UTF-8 encoded).
 */
pair<vector<unique_ptr<char[]>>, vector<char *>> convertArgsToUtf8()
{
    pair<vector<unique_ptr<char[]>>, vector<char *>> res;
    int argc;

    LPWSTR *argv_w = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv_w || argc <= 0) {
        return res;
    }

    res.first.reserve(static_cast<size_t>(argc));
    res.second.reserve(static_cast<size_t>(argc));
    for (LPWSTR *i = argv_w, *end = argv_w + argc; i != end; ++i) {
        int requiredSize = WideCharToMultiByte(CP_UTF8, 0, *i, -1, nullptr, 0, 0, 0);
        if (requiredSize <= 0) {
            break; // just stop on error
        }

        auto argv = make_unique<char[]>(static_cast<size_t>(requiredSize));
        requiredSize = WideCharToMultiByte(CP_UTF8, 0, *i, -1, argv.get(), requiredSize, 0, 0);
        if (requiredSize <= 0) {
            break;
        }

        res.second.emplace_back(argv.get());
        res.first.emplace_back(move(argv));
    }

    LocalFree(argv_w);
    return res;
}
#endif

} // namespace CppUtilities
