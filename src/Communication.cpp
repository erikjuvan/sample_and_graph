#include "Communication.hpp"
#include "Helpers.hpp"
#include <iostream>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#endif

Communication::Communication() :
    m_serial("", 460800)
{
}

Communication::~Communication()
{
    Disconnect();
}

bool Communication::Connect(const std::string& port)
{
    std::scoped_lock<std::mutex> sl(m_mtx);

    bool ret = true;

    try {
        m_serial.setPort(port);
        m_serial.open();
    } catch (std::invalid_argument) {
        ret = false;
    } catch (serial::SerialException) {
        ret = false;
    } catch (serial::IOException) {
        ret = false;
    }

    if (ret) {
        m_is_connected = true;
    }

    return ret;
}

void Communication::Disconnect()
{
    std::scoped_lock<std::mutex> sl(m_mtx);
    m_is_connected = false;
    m_serial.close();
}

size_t Communication::GetRxBufferLen()
{
    std::scoped_lock<std::mutex> sl(m_mtx);
    if (IsConnected())
        return m_serial.available();
    else
        return 0;
}

size_t Communication::Write(const void* buffer, int size)
{
    std::scoped_lock<std::mutex> sl(m_mtx);
    if (IsConnected())
        return m_serial.write((uint8_t*)buffer, size);
    else
        return 0;
}

size_t Communication::Write(const std::string& buffer)
{
    std::scoped_lock<std::mutex> sl(m_mtx);
    if (IsConnected())
        return m_serial.write(buffer);
    else
        return 0;
}

size_t Communication::Read(void* buffer, int size)
{
    std::scoped_lock<std::mutex> sl(m_mtx);
    if (IsConnected())
        return m_serial.read((uint8_t*)buffer, size);
    else
        return 0;
}

size_t Communication::Read(std::vector<uint8_t>& buffer, size_t size)
{
    std::scoped_lock<std::mutex> sl(m_mtx);
    if (IsConnected())
        return m_serial.read(buffer, size);
    else
        return 0;
}

std::string Communication::Readline()
{
    std::scoped_lock<std::mutex> sl(m_mtx);
    if (IsConnected())
        return m_serial.readline();
    else
        return std::string("");
}

void Communication::Flush()
{
    std::scoped_lock<std::mutex> sl(m_mtx);
    if (IsConnected())
        m_serial.flush();
}

void Communication::Purge()
{
    std::scoped_lock<std::mutex> sl(m_mtx);
    if (IsConnected())
        m_serial.purge();
}

void Communication::SetTimeout(int ms)
{
    std::scoped_lock<std::mutex> sl(m_mtx);

    auto to = serial::Timeout::simpleTimeout(ms);
    m_serial.setTimeout(to);
}

std::vector<serial::PortInfo> Communication::ListAllPorts()
{
    return serial::list_ports();
}

std::vector<std::string> Communication::ListFreePorts()
{
    std::vector<std::string> ret_ports;
#ifdef _WIN32
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
#endif
    return ret_ports;
}

void Communication::ConfirmTransmission(std::string const& str)
{
    auto ret_str    = Readline();
    auto tokens     = Help::TokenizeString(str, ", \n");
    auto tokens_ret = Help::TokenizeString(ret_str, ", \n");
    bool fault      = false;
    auto tmp_str    = str;
    if (tmp_str.back() == '\n')
        tmp_str.pop_back();
    std::string error_msg = "Transmission failed when sending: \"" + tmp_str + "\": ";

    // Command name
    if (tokens_ret[0] != tokens[0]) {
        error_msg += "Command name mismatch: expected: '" + tokens[0] + "' received: '" + tokens_ret[0] + "'\n";
        throw std::runtime_error(error_msg);
    }

    // Check for number of tokens mismatch
    if (tokens.size() != tokens_ret.size()) {
        error_msg += "Argument number mismatch: expected: " + std::to_string(tokens.size() - 1) + " received: " + std::to_string(tokens_ret.size() - 1) + "\n";
        throw std::runtime_error(error_msg);
    }

    // Check arguments
    if (tokens.size() > 1) {
        for (int i = 1; i < tokens.size(); ++i) {
            // Arguments are always numeric
            auto f     = std::stof(tokens[i]);
            auto f_ret = std::stof(tokens_ret[i]);
            if (std::abs(f - f_ret) > 0.1) {
                error_msg += "Argument number " + std::to_string(i) + " mismatch: expected: " + tokens[i] + " received: " + tokens_ret[i] + "\n";
                throw std::runtime_error(error_msg);
            }
        }
    }
}

std::vector<std::string> Communication::WriteAndTokenizeResult(std::string const& str)
{
    Write(str);
    auto ret_data = Readline();
    auto str_vec  = Help::TokenizeString(ret_data, ",\n");

    return str_vec;
}
