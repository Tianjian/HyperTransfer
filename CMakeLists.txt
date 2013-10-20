PROJECT(ftp_server)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

add_definitions(-g -Wall)

include_directories (/opt/libevent-static/include)
include_directories (/opt/log4cplus/include)

link_directories (/opt/libevent-static/lib)
link_directories (/opt/log4cplus/lib)
link_directories (/usr/local/lib)

set( server_src ftp_server_main.cpp ConfigFile.cpp logger.cpp util.cpp
TCP_interface.cpp TCP_ctrl_event.cpp TCP_data_event.cpp TCP_manager.cpp
finite_state_machine.cpp fsm_file.cpp fsm_transfer.cpp
session.cpp session_manager.cpp session_thread_pool.cpp
file_index.cpp disk_manager.cpp disk_thread_pool.cpp disk_fsm.cpp)

add_executable( hyper_transfer_server ${server_src})

target_link_libraries (hyper_transfer_server log4cplus event pthread rt crypto)

