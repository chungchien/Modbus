

find_package(Boost COMPONENTS log log_setup CONFIG)

add_library(zjlog zjlog.cpp zjlog.h)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(zjlog PRIVATE ZJLOG_EXPORTS SHARED_LIBRARY)
endif(BUILD_SHARED_LIBS)


target_link_libraries(zjlog PRIVATE Boost::log Boost::log_setup)

target_include_directories(zjlog
    PRIVATE ${Boost_INCLUDE_DIRS}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

# set_target_properties(zjlog PROPERTIES
#     POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS}
#     CXX_VISIBILITY_PRESET hidden
# )

set_target_properties(zjlog PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
