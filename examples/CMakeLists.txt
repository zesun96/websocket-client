# cmake needs this line
cmake_minimum_required(VERSION 3.8)

project(examples)

include_directories(../include)

add_executable(client-static client-static/client.cpp)
target_link_libraries(client-static websocketclient-static)

add_executable(client-shared client-shared/client.cpp)
target_link_libraries(client-shared websocketclient)
if(MSVC)
	add_custom_command(TARGET client-shared POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE_DIR:websocketclient>/websocketclient.dll
		$<TARGET_FILE_DIR:client-shared>
    COMMAND_EXPAND_LISTS
	)
endif()
