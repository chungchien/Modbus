// Modbus for c++ <https://github.com/Mazurel/Modbus>
// Copyright (c) 2020 Mateusz Mazur aka Mazurel
// Licensed under: MIT License <http://opensource.org/licenses/MIT>

#include "Serial/connection.hpp"
#include "modbusUtils.hpp"
#include "serialportimpl.hpp"

using namespace MB::Serial;

Connection::Connection(const std::string &path)
    : _serial(new SerialPortImpl) {
    if (!_serial->open(path.c_str())) {
        throw std::runtime_error("Cannot open serial port " + path);
    }
}

void Connection::connect() {
    
}

Connection::~Connection() {
    if (_serial) {
        delete _serial;
        _serial = nullptr;
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

    auto size = _serial->read((char *)data.data(), (int)data.size(), std::chrono::milliseconds(_timeout));
    if (size < 0) {
        throw MB::ModbusException(MB::utils::SlaveDeviceFailure);
    }

    data.resize(size);
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

    int ret = _serial->write((const char *)data.data(), data.size());

    return data;
}

Connection::Connection(Connection &&moved) noexcept {
    _serial = moved._serial;
    moved._serial = nullptr;
}

Connection &Connection::operator=(Connection &&moved) {
    if (this == &moved)
        return *this;

    _serial = moved._serial;
    moved._serial = nullptr;
    return *this;
}

void Connection::disableParity()
{
    _serial->setParity(SerialPortImpl::Parity::NONE);
}

void Connection::setEvenParity()
{
    _serial->setParity(SerialPortImpl::Parity::EVEN);
}

void Connection::setOddParity()
{
    _serial->setParity(SerialPortImpl::Parity::ODD);
}

void Connection::setTwoStopBits(const bool two)
{
    _serial->setStopBits(two ? SerialPortImpl::StopBits::TWO : SerialPortImpl::StopBits::ONE);
}

void Connection::setBaudRate(int speed)
{
    _serial->setBaudRate(speed);
}
