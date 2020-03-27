#include "Helpers.hpp"
#include <sstream>
#include <windows.h>

namespace Help
{

#if defined(_WIN32)
#include <intrin.h>
#elif defined(__linux__)
#include <x86intrin.h>
#endif
inline uint64_t rdtsc()
{
    return __rdtsc();
}

// Homegrown ;)
std::vector<std::string> TokenizeString(std::string const& str, std::string const& delims)
{
    std::vector<std::string> result;
    size_t                   offset = 0;
    size_t                   len    = 0;

    for (int i = 0; i < str.size(); ++i) {
        bool delim_found = false;
        for (auto const c : delims) {
            if (str[i] == c) {
                delim_found = true;
                break;
            }
        }

        if (delim_found) {
            if (len > 0) {
                result.push_back(str.substr(offset, len));
            }
            offset = i + 1;
            len    = 0;
        } else
            len++;
    }

    // If end of string doesn't contain a delimiter then there could still be data in buffer
    if (len > 0)
        result.push_back(str.substr(offset, len));

    return result;
}

std::vector<std::string> GetCOMPorts()
{
    std::vector<std::string> ret_ports;

    for (int i = 0; i < 255; ++i) {
        auto port_name     = "COM" + std::to_string(i);
        auto win_port_name = "\\\\.\\" + port_name;
        auto hnd           = CreateFile(win_port_name.c_str(), GENERIC_READ | GENERIC_WRITE,
                              0,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL);

        if (hnd != INVALID_HANDLE_VALUE) {
            CloseHandle(hnd);
            ret_ports.push_back(port_name);
        }
    }

    return ret_ports;
}

} // namespace Help