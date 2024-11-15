#pragma

#include <string>
#include <vector>
#include <cstdint>

struct SerialPortInfo {
    std::string port;
    std::string description;
    std::string manufacturer;
    uint16_t vid;
    uint16_t pid;

    static std::vector<SerialPortInfo> listPorts();
};
