// Modbus for c++ <https://github.com/Mazurel/Modbus>
// Copyright (c) 2020 Mateusz Mazur aka Mazurel
// Licensed under: MIT License <http://opensource.org/licenses/MIT>

#pragma once

#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "MB/modbusException.hpp"
#include "MB/modbusRequest.hpp"
#include "MB/modbusResponse.hpp"
#include "MB/modbusUtils.hpp"

namespace MB::Serial {

class SerialPortImpl;

class Connection {
  public:
    // Pretty high timeout
    static const unsigned int DefaultSerialTimeout = 100;

  private:
    SerialPortImpl *_serial = nullptr;
    int _timeout = Connection::DefaultSerialTimeout;

  public:
    explicit Connection();
    explicit Connection(const std::string &path);
    explicit Connection(const Connection &) = delete;
    explicit Connection(Connection &&) noexcept;
    Connection &operator=(Connection &&);
    ~Connection();

    void connect();

    std::vector<uint8_t> sendRequest(const MB::ModbusRequest &request);
    std::vector<uint8_t> sendResponse(const MB::ModbusResponse &response);
    std::vector<uint8_t> sendException(const MB::ModbusException &exception);

    /**
     * @brief Sends data through the serial
     * @param data - Vectorized data
     */
    std::vector<uint8_t> send(std::vector<uint8_t> data);

    void clearInput();

    [[nodiscard]] std::tuple<MB::ModbusResponse, std::vector<uint8_t>> awaitResponse();
    [[nodiscard]] std::tuple<MB::ModbusRequest, std::vector<uint8_t>> awaitRequest();

    [[nodiscard]] std::vector<uint8_t> awaitRawMessage();

    void disableParity();

    void setEvenParity();

    void setOddParity();

    void setTwoStopBits(const bool two);

    void setBaudRate(int speed);

    int getTimeout() const { return _timeout; }

    void setTimeout(int timeout) { _timeout = timeout; }
};
} // namespace MB::Serial
