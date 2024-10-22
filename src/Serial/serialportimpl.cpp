
#include <vector>
#include <iostream>
#include <functional>
#include <chrono>
#include "serialportimpl.hpp"

#define TAG "SerialPortImpl"

namespace MB::Serial {

using serial_port = boost::asio::serial_port;
using milliseconds =  std::chrono::milliseconds;

SerialPortImpl::SerialPortImpl() 
	: m_serial_port(m_io_context)
{

}

SerialPortImpl::~SerialPortImpl() {
	if (isOpen()) {
		close();
	}
}

bool SerialPortImpl::setBaudRate(uint32_t baudRate)
{
	boost::system::error_code ec;
	m_serial_port.set_option(serial_port::baud_rate(baudRate), ec);
	return ec == boost::system::errc::success;
}

bool SerialPortImpl::setDataBits(uint32_t dataBits)
{
	boost::system::error_code ec;
	m_serial_port.set_option(serial_port::character_size(dataBits), ec);
	return ec == boost::system::errc::success;
}

bool SerialPortImpl::setParity(Parity parity)
{
	boost::system::error_code ec;
	m_serial_port.set_option(serial_port::parity(static_cast<serial_port::parity::type>(parity)), ec);
	return ec == boost::system::errc::success;
}

bool SerialPortImpl::setStopBits(StopBits stopBits)
{
	boost::system::error_code ec;
	m_serial_port.set_option(serial_port::stop_bits(static_cast<serial_port::stop_bits::type>(stopBits)));
	return ec == boost::system::errc::success;
}

bool SerialPortImpl::setFlowControl(FlowControl flowControl)
{
	boost::system::error_code ec;
	m_serial_port.set_option(serial_port::flow_control(static_cast<serial_port::flow_control::type>(flowControl)), ec);
	return ec == boost::system::errc::success;
}

bool SerialPortImpl::open(const char *portName) { 
	if (m_serial_port.is_open()) {
		return false;
	}
	boost::system::error_code ec;
	m_serial_port.open(portName, ec);
	if (ec == boost::system::errc::success) {
		m_serial_port.async_read_some(boost::asio::buffer(m_one_byte_buffer, 1), 
			std::bind(&SerialPortImpl::onReceived, this, std::placeholders::_1, std::placeholders::_2));
		// 创建线程，用于异步读取数据
		m_thread = std::thread([this]() {
			m_io_context.run();
			// LOG_ERROR(TAG, "io_context stopped");
		});
		return true;
	}
	return false;
}

void SerialPortImpl::close() {
	if (m_serial_port.is_open()) {
		boost::system::error_code ec;
		m_serial_port.close(ec);
		if (ec != boost::system::errc::success) {
			// LOG_ERROR(TAG, "close error: %s", ec.what().c_str());
		}
		// LOG_DEBUG(TAG, "waiting thread to join ...");
		m_thread.join();
		// LOG_DEBUG(TAG, "thread joined");
		// LOG_DEBUG(TAG, "serial port closed");
	}
}

bool SerialPortImpl::isOpen() const { 
	return m_serial_port.is_open();
}

int SerialPortImpl::write(const char *data, int length) {
	boost::system::error_code ec;
    size_t n = m_serial_port.write_some(boost::asio::buffer(data, length), ec);
	if (ec != boost::system::errc::success) {
		// LOG_ERROR(TAG, "write error: %s", ec.what());
		return -1;
	}
	return static_cast<int>(n);
}

int SerialPortImpl::read(char *buffer, int length, milliseconds timeout) { 
	std::unique_lock lock{m_buffer_mutex};

    // 先读取已有的，如果不够再等待

	int num_bytes_read = 0;
    auto tp =  timeout != milliseconds::max() ? (std::chrono::steady_clock::now() + timeout) : std::chrono::steady_clock::time_point::max();
    m_num_bytes_required = length;
    while (m_num_bytes_required > 0) {
        size_t n = m_buffer.size();
		if (n > 0) {
			if (n > m_num_bytes_required) {
				n = m_num_bytes_required;
			}

			std::copy(m_buffer.begin(), m_buffer.begin() + n, buffer + num_bytes_read);
			m_buffer.erase_begin(n);
			
			num_bytes_read += static_cast<int>(n);
			m_num_bytes_required = length - n;
			if (m_num_bytes_required == 0) {
				break;
			}
		}
        if (std::chrono::steady_clock::now() >= tp) {
            break;
        }
		if (m_cond.wait_until(lock, tp) == std::cv_status::timeout) {
			// LOG_INFO(TAG, "read timeout");
		}
    }

	m_num_bytes_required = 0;

    lock.unlock();
	return num_bytes_read;
}



int SerialPortImpl::readLine(char *buffer, int size, milliseconds timeout) {
	int num_bytes_read = 0;
	std::unique_lock lock{m_buffer_mutex};

    // 先读取已有的，如果不够再等待

    auto tp =  timeout != milliseconds::max() ? (std::chrono::steady_clock::now() + timeout) : std::chrono::steady_clock::time_point::max();

	while (true) {
		// 查找换行符
		auto it = std::find(m_buffer.begin(), m_buffer.end(), '\n');
		int n;
		if (it == m_buffer.end()) {
		    n = (int)m_buffer.size();
		} else {
			++it;
		    n = static_cast<int>(std::distance(m_buffer.begin(), it));
		}
        if (n > 0) {
            if (n > size) {
                n = size;
            }
            std::copy(m_buffer.begin(), m_buffer.begin() + n, buffer + num_bytes_read);
            m_buffer.erase_begin(n);
            num_bytes_read += n;
            size -= n;
            if (size == 0 || buffer[num_bytes_read - 1] == '\n') {
                break;
            }
        }
		if (std::chrono::steady_clock::now() >= tp) {
			break;
		}

        m_num_bytes_required = SIZE_MAX; // 读取到换行符为止
		if (m_cond.wait_until(lock, tp) == std::cv_status::timeout) {
			// LOG_INFO(TAG, "read timeout");
		}
	}
	m_num_bytes_required = 0;
    lock.unlock();
	return num_bytes_read;
}

void SerialPortImpl::clearInputs() {
	std::lock_guard lock{m_buffer_mutex};
	m_buffer.clear();
}

void SerialPortImpl::onReceived(const boost::system::error_code &ec,
                                size_t bytes_transferred) {
	if (ec == boost::system::errc::success) {
		{
			std::lock_guard lock{m_buffer_mutex};
			m_buffer.push_back(m_one_byte_buffer[0]);
		}
		if (m_num_bytes_required > 0) {
			if (m_buffer.size() >= m_num_bytes_required || (m_one_byte_buffer[0] == '\n')) {
				m_cond.notify_one();
			}
		}
		m_serial_port.async_read_some(boost::asio::buffer(m_one_byte_buffer, 1),
			std::bind(&SerialPortImpl::onReceived, this, std::placeholders::_1, std::placeholders::_2));
	} 
	// else if (ec.value() != boost::system::errc::no_such_device_or_address) {
	// 	LOG_INFO(TAG, "serial port read error: %s (catage: %s, value: %d)", ec.message().c_str(),
	// 		ec.category().name(), ec.value());
	// }
}

}
