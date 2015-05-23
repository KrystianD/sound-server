cmake_minimum_required (VERSION 2.6)
get_filename_component(CURRENT_DIR ${CMAKE_CURRENT_LIST_FILE} PATH) # for cmake before 2.8.3

project(client C CXX ASM)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

if(CROSS)
	set(CMAKE_C_COMPILER "i686-w64-mingw32-gcc")
	set(CMAKE_CXX_COMPILER "i686-w64-mingw32-g++")
endif()

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

include_directories(${CURRENT_DIR})

if(WIN32)
  set(PROJECT_PORT_DIR ${CURRENT_DIR}/port/windows)
  set(CMAKE_C_FLAGS "-DPLATFORM_WIN32 -D_CRT_SECURE_NO_WARNINGS -std=c99")
  set(CMAKE_CXX_FLAGS "-DPLATFORM_WIN32 -D_CRT_SECURE_NO_WARNINGS -std=c++11")

	include_directories(${CURRENT_DIR}/port/windows/)
	link_directories(${CURRENT_DIR}/port/windows/)
elseif(UNIX)
  set(PROJECT_PORT_DIR ${CURRENT_DIR}/port/linux)
  set(CMAKE_C_FLAGS "-DPLATFORM_LINUX -std=c99")
  set(CMAKE_CXX_FLAGS "-DPLATFORM_LINUX -std=c++11")

	include_directories(${CURRENT_DIR}/port/linux/)
endif()
include_directories(${CURRENT_DIR}/../kdutils)
include_directories(${CURRENT_DIR}/../common)

set(COMMON_SOURCES main.cpp ../kdutils/UdpSocket.cpp ../kdutils/kdutils.cpp ../common/resample.cpp)

if(WIN32)
	add_definitions(-D_WIN32_WINNT=0x0600)
	if(CROSS)
		set(TARGET_NAME sound.exe)
	else()
		set(TARGET_NAME sound)
	endif()

	add_executable(${TARGET_NAME} port/windows/capture.cpp
		${COMMON_SOURCES})

	target_link_libraries(${TARGET_NAME} pthread ole32 oleaut32 ksguid ws2_32)
	
	add_definitions(-DWIN32)

elseif(UNIX)
	set(TARGET_NAME sound)
	add_definitions(-DUNIX)

	add_executable(${TARGET_NAME} port/linux/capture.cpp ${COMMON_SOURCES})

	target_link_libraries(${TARGET_NAME} pthread pulse pulse-simple)
endif()

if(DEBUG)
	set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_FLAGS "-g" LINK_FLAGS "-g")
endif()
