#pragma once

#include <string>
#include <stdint.h>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/circular_buffer.hpp>


namespace MB::Serial {

class SerialPortImpl {
public:
    using buffer_type = boost::circular_buffer<char>;
    using milliseconds = std::chrono::milliseconds;

    enum class Parity {
        NONE = boost::asio::serial_port::parity::none,
        ODD = boost::asio::serial_port::parity::odd,
        EVEN = boost::asio::serial_port::parity::even,
    };
    enum class StopBits {
        ONE = boost::asio::serial_port::stop_bits::one,
        ONE_POINT_FIVE = boost::asio::serial_port::stop_bits::onepointfive,
        TWO = boost::asio::serial_port::stop_bits::two,
    };
    enum class FlowControl {
        NONE = boost::asio::serial_port::flow_control::none,
        HARDWARE = boost::asio::serial_port::flow_control::hardware,
        SOFTWARE = boost::asio::serial_port::flow_control::software,
    };

    // 读缓存大小
    static constexpr size_t READING_BUFFER_SIZE = 4096;

    SerialPortImpl();
    ~SerialPortImpl();

    SerialPortImpl(const SerialPortImpl&) = delete;
    SerialPortImpl& operator=(const SerialPortImpl&) = delete;

    bool open(const char* portName);
    bool setBaudRate(uint32_t baudRate);
    bool setDataBits(uint32_t dataBits);
    bool setParity(Parity parity);
    bool setStopBits(StopBits stopBits);
    bool setFlowControl(FlowControl flowControl);
    void close();
    bool isOpen() const;
    int write(const void* data, int length);
    int read(void* buffer, int length, milliseconds timeout=milliseconds(-1));
    int readLine(char* buffer, int length, milliseconds timeout=milliseconds(-1));
    void clearInputs();
    void flush();
    void clearRxBuffer();
    void clearTxBuffer();

private:
    void onReceived(const boost::system::error_code &ec, size_t bytes_transferred);

    boost::asio::io_service m_io_context;
    boost::asio::serial_port m_serial_port;
    std::thread m_thread;
    std::condition_variable m_cond;
    std::mutex m_buffer_mutex;
    buffer_type m_buffer = buffer_type(READING_BUFFER_SIZE);   // 读缓存
    char m_one_byte_buffer[1]; // 1个字节缓存
    size_t m_num_bytes_required = 0;
    boost::system::error_code m_rd_error;
};
}