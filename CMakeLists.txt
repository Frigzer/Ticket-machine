cmake_minimum_required(VERSION 3.25)

project(ticket-machine
    VERSION 0.1.0
    DESCRIPTION "Ticket machine client/server application"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)
include(CTest)

option(TICKET_MACHINE_BUILD_CLIENT "Build ticket machine client executable" ON)
option(TICKET_MACHINE_BUILD_SERVER "Build ticket machine server executable" ON)

find_package(asio CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(ticket_machine
    src/change_calculator.cpp
    src/coin_inventory.cpp
    src/console_client_app.cpp
    src/network_protocol.cpp
    src/server_console_app.cpp
    src/server_seed_data.cpp
    src/ticket_machine_client.cpp
    src/ticket_server.cpp
    src/ticket_server_host.cpp
)

add_library(ticket_machine::ticket_machine ALIAS ticket_machine)

target_compile_features(ticket_machine
    PUBLIC
        cxx_std_23
)

target_include_directories(ticket_machine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(ticket_machine
    PUBLIC
        TICKET_MACHINE_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(ticket_machine
    PUBLIC
        asio::asio
        nlohmann_json::nlohmann_json
)

if(TICKET_MACHINE_BUILD_CLIENT)
    add_executable(ticket-machine
        app/main.cpp
    )

    target_link_libraries(ticket-machine
        PRIVATE
            ticket_machine::ticket_machine
    )
endif()

if(TICKET_MACHINE_BUILD_SERVER)
    add_executable(ticket-machine-server
        app/server_main.cpp
    )

    target_link_libraries(ticket-machine-server
        PRIVATE
            ticket_machine::ticket_machine
    )
endif()

if(BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    find_package(GTest CONFIG REQUIRED)

    file(GLOB TICKET_MACHINE_TEST_SOURCES CONFIGURE_DEPENDS
        tests/*.cpp
    )

    if(TICKET_MACHINE_TEST_SOURCES)
        add_executable(ticket-machine-tests
            ${TICKET_MACHINE_TEST_SOURCES}
        )

        target_link_libraries(ticket-machine-tests
            PRIVATE
                ticket_machine::ticket_machine
                GTest::gtest_main
        )

        include(GoogleTest)
        gtest_discover_tests(ticket-machine-tests)
    endif()
endif()

install(
    TARGETS ticket_machine
    EXPORT ticket-machine-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT ticket-machine-targets
    NAMESPACE ticket_machine::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ticket-machine
)