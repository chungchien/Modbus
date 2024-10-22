set(MODBUS_SERIAL_HEADER_FILES ${MODBUS_HEADER_FILES_DIR}/Serial/connection.hpp)
set(MODBUS_SERIAL_SOURCE_FILES connection.cpp serialportimpl.cpp)

find_package(Boost REQUIRED CONFIG)

add_library(Modbus_Serial)
target_include_directories(Modbus_Serial PUBLIC ${MODBUS_HEADER_FILES_DIR} ${Boost_INCLUDE_DIRS})
target_link_libraries(Modbus_Serial Modbus_Core)
target_sources(Modbus_Serial PRIVATE ${MODBUS_SERIAL_SOURCE_FILES} PUBLIC ${MODBUS_SERIAL_HEADER_FILES})
