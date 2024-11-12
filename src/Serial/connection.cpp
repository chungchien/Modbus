// Modbus for c++ <https://github.com/Mazurel/Modbus>
// Copyright (c) 2020 Mateusz Mazur aka Mazurel
// Licensed under: MIT License <http://opensource.org/licenses/MIT>

#include "Serial/connection.hpp"
#include "modbusUtils.hpp"
#include "modbusLog.hpp"
#include "serialportimpl.hpp"

using namespace MB::Serial;

Connection::Connection()
    : _impl{new SerialPortImpl}
{
}

void Connection::connect(const std::string &path) {
    if (!_impl->open(path.c_str())) {
        throw std::runtime_error("Cannot open serial port " + path);
    }
    _impl->setBaudRate(115200);
    _impl->setDataBits(8);
    _impl->setParity(static_cast<SerialPortImpl::Parity>(NOPARITY));
    _impl->setStopBits(static_cast<SerialPortImpl::StopBits>(ONESTOPBIT));
    _impl->setFlowControl(SerialPortImpl::FlowControl::NONE);
}

Connection::~Connection() {
    if (_impl->isOpen()) {
        _impl->close();
    }
}

std::vector<uint8_t> Connection::sendRequest(const MB::ModbusRequest &request) {
    return send(request.toRaw());
}

std::vector<uint8_t> Connection::sendResponse(const MB::ModbusResponse &response) {
    return send(response.toRaw());
}

std::vector<uint8_t> Connection::sendException(const MB::ModbusException &exception) {
    return send(exception.toRaw());
}

std::vector<uint8_t> Connection::awaitRawMessage() {
    std::vector<uint8_t> data(1024);
    if (!_impl->isOpen()) {
        throw MB::ModbusException(MB::utils::ConnectionClosed);
    }

    int number_of_bytes_read = _impl->read((void *)data.data(), static_cast<int>(data.size()), SerialPortImpl::milliseconds(_timeout));
    if (number_of_bytes_read < 0) {
        throw std::runtime_error("Error while reading from serial port");
    }
    if (number_of_bytes_read == 0) {
        throw MB::ModbusException(MB::utils::Timeout);
    }
    data.resize(number_of_bytes_read);
    data.shrink_to_fit();

    return data;
}

// TODO: Figure out how to return raw data when exception is being thrown
std::tuple<MB::ModbusResponse, std::vector<uint8_t>> Connection::awaitResponse() {
    std::vector<uint8_t> data;
    data.reserve(8);

    MB::ModbusResponse response(0, MB::utils::ReadAnalogInputRegisters);

    while (true) {
        try {
            auto tmpResponse = awaitRawMessage();
            data.insert(data.end(), tmpResponse.begin(), tmpResponse.end());

            if (MB::ModbusException::exist(data))
                throw MB::ModbusException(data);

            response = MB::ModbusResponse::fromRawCRC(data);
            break;
        } catch (const MB::ModbusException &ex) {
            if (MB::utils::isStandardErrorCode(ex.getErrorCode()) ||
                ex.getErrorCode() == MB::utils::Timeout ||
                ex.getErrorCode() == MB::utils::SlaveDeviceFailure)
                throw ex;
            continue;
        }
    }

    return std::tie(response, data);
}

std::tuple<MB::ModbusRequest, std::vector<uint8_t>> Connection::awaitRequest() {
    std::vector<uint8_t> data;
    data.reserve(8);

    MB::ModbusRequest request(0, MB::utils::ReadAnalogInputRegisters);

    while (true) {
        try {
            auto tmpResponse = awaitRawMessage();
            data.insert(data.end(), tmpResponse.begin(), tmpResponse.end());

            request = MB::ModbusRequest::fromRawCRC(data);
            break;
        } catch (const MB::ModbusException &ex) {
            if (ex.getErrorCode() == MB::utils::Timeout ||
                ex.getErrorCode() == MB::utils::SlaveDeviceFailure)
                throw ex;
            continue;
        }
    }

    return std::tie(request, data);
}

std::vector<uint8_t> Connection::send(std::vector<uint8_t> data) {
    data.reserve(data.size() + 2);
    const auto crc = utils::calculateCRC(data.data(), data.size());

    data.push_back(reinterpret_cast<const uint8_t *>(&crc)[0]);
    data.push_back(reinterpret_cast<const uint8_t *>(&crc)[1]);

    int nwriten = _impl->write(data.data(), (int)data.size());
    if (nwriten != data.size()) {
        throw std::runtime_error("Failed to send data");
    }

    return data;
}

Connection::Connection(Connection &&moved) noexcept {
    _impl = moved._impl;
    moved._impl = nullptr;
    _timeout = moved._timeout;
}

Connection &Connection::operator=(Connection &&moved) {
    if (this == &moved)
        return *this;

    _impl = moved._impl;
    moved._impl = NULL;
    _timeout = moved._timeout;
    return *this;
}


    
void Connection::setParity(Parity parity)
{
    if (!_impl->setParity(static_cast<SerialPortImpl::Parity>(parity))) {
        throw std::runtime_error("Failed to set comm state");
    }
}

void Connection::setStopBits(StopBits stopBits)
{
    if (!_impl->setStopBits(static_cast<SerialPortImpl::StopBits>(stopBits))) {
        throw std::runtime_error("Failed to set comm state");
    }
}

void Connection::setDataBits(int dataBits)
{
    if (!_impl->setDataBits(dataBits)) {
        throw std::runtime_error("Failed to set comm state");
    }
}

void Connection::setBaudRate(uint32_t speed)
{
    if (!_impl->setBaudRate(speed)) {
        throw std::runtime_error("Failed to set comm state");
    }
}
