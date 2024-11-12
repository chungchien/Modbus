// Modbus for c++ <https://github.com/Mazurel/Modbus>
// Copyright (c) 2020 Mateusz Mazur aka Mazurel
// Licensed under: MIT License <http://opensource.org/licenses/MIT>

#include "MB/modbusException.hpp"
#include "MB/modbusRequest.hpp"
#include "MB/modbusResponse.hpp"
#include "Serial/connection.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <thread>
#include <chrono>

using namespace std::literals;

std::ostream &operator<<(std::ostream &stream, const MB::ModbusRequest &request) {
    stream << request.toString();
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const MB::ModbusResponse &response) {
    stream << response.toString();
    return stream;
}

std::ostream &operator<<(std::ostream &stream, const std::vector<uint8_t> &raw) {
    std::for_each(raw.begin(), raw.end(), [&stream](const uint8_t &byte){
        stream << " 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    });
    return stream;
}


void createRequest() {
    // Create simple request
    MB::ModbusRequest request(1, MB::utils::ReadDiscreteOutputCoils, 100, 10);

    std::cout << "Stringed Request: " << request.toString() << std::endl;

    std::cout << "Raw request:" << std::endl;

    // Get raw represenatation for request
    std::vector<uint8_t> rawed = request.toRaw();

    // Method for showing byte
    auto showByte = [](const uint8_t &byte) {
        std::cout << " 0x" << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(byte);
    };

    // Show all bytes
    std::for_each(rawed.begin(), rawed.end(), showByte);
    std::cout << std::endl;

    // Create CRC and pointer to its bytes
    uint16_t CRC = MB::utils::calculateCRC(rawed);
    auto CRCptr  = reinterpret_cast<uint8_t *>(&CRC);

    // Show byted CRC for request
    std::cout << "CRC for the above code: ";
    std::for_each(CRCptr, CRCptr + 2, showByte);
    std::cout << std::endl;

    auto request1 = MB::ModbusRequest::fromRaw(rawed);
    std::cout << "Stringed Request 1 after rawed: " << request1.toString() << std::endl;

    // Add CRC to the end of raw request so that it can be loaded with CRC check
    rawed.insert(rawed.end(), CRCptr, CRCptr + 2);
    auto request2 = MB::ModbusRequest::fromRawCRC(rawed); // Throws on invalid CRC
    std::cout << "Stringed Request 2 after rawed: " << request2.toString() << std::endl;
}

int main() { 
    MB::Serial::Connection conn;

    MB::ModbusRequest request_status(1, MB::utils::ReadDiscreteInputContacts, 0, 2);
    MB::ModbusRequest request_measure(1, MB::utils::WriteSingleAnalogOutputRegister, 0x100, 1, {MB::ModbusCell::initReg(0x00)});
    MB::ModbusRequest request_result(1, MB::utils::ReadAnalogInputRegisters, 0x1000, 1024);

    std::cout << "Request status: " << request_status.toString() << std::endl;
    std::cout << "Request measure: " << request_measure.toString() << std::endl;
    std::cout << "Request result: " << request_result.toString() << std::endl;

    try {
        conn.connect("COM46");
        conn.setBaudRate(115200);
        conn.setDataBits(8);
        conn.setParity(MB::Serial::Connection::Parity::None);
        conn.setStopBits(MB::Serial::Connection::StopBits::One);

        std::cout << "MEAS RAW:" << conn.sendRequest(request_measure) << std::endl;
        std::cout << "MEAS ACK:" << std::get<1>(conn.awaitResponse()) << std::endl;
        while(true) {
            auto raw = conn.sendRequest(request_status);
            std::cout << "Send Raw: " << raw << std::endl;
            auto tuple = conn.awaitResponse();
            MB::ModbusResponse response = std::get<0>(tuple);
            std::cout << "Received status: " << response << std::endl
                << "RAW: " << std::get<1>(tuple) << std::endl;
            auto status = response.registerValues();
            std::cout << status[0].coil() << " " << status[1].coil() << std::endl;
            if (status.at(0).coil()) {
                std::this_thread::sleep_for(50ms);
                continue;
            }
            if (status.at(1).coil()) {
                break;
            } else {
                std::cerr << "Error: measure failed" << std::endl;
                return 1;
            }
        }

        conn.sendRequest(request_result);
        auto tuple = conn.awaitResponse();
        auto result = std::get<0>(tuple);
        std::cout << "Result: " << result.toString() << std::endl;

    } catch(MB::ModbusException &ex) {
        std::cerr << ex.toString() << std::endl;
    } catch (std::exception &ex) {
        std::cerr << ex.what() << std::endl;
    }

    return 0;
}
