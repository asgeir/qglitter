cmake_minimum_required(VERSION 2.8)
project(QGLITTER)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(OpenSSL REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${QGLITTER_BINARY_DIR})

if(APPLE)
	set(PLATFORM_SOURCES
		Platform/Mac/Mac.cpp)
endif()

if(WIN32)
	set(PLATFORM_SOURCES
		Platform/Win32/Win32.cpp)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    SET(LINUX ON CACHE BOOL "Linux system" FORCE)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(LINUX)
	set(PLATFORM_SOURCES
		Platform/Linux/Linux.cpp)
endif()

find_package(Qt4 REQUIRED COMPONENTS QtCore QtGui QtNetwork QtWebKit)

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

set(SOURCES
	QGlitterAppcast.cpp
	QGlitterAppcastItem.cpp
	QGlitterAutomaticUpdateAlert.cpp
	QGlitterDownloader.cpp
	QGlitterUpdateAlert.cpp
	QGlitterUpdateCheckStatus.cpp
	QGlitterUpdater.cpp
	QGlitterUpdateStatus.cpp
	Crypto/OpenSSLCrypto.cpp
	${PLATFORM_SOURCES})

set(HEADERS
	QGlitterAutomaticUpdateAlert.h
	QGlitterDownloader.h
	QGlitterUpdateAlert.h
	QGlitterUpdateCheckStatus.h
	QGlitterUpdater.h
	QGlitterUpdateStatus.h)

set(UI_FILES
	Designer/QGlitterAutomaticUpdateAlert.ui
	Designer/QGlitterUpdateAlert.ui
	Designer/QGlitterUpdateStatus.ui)

QT4_WRAP_CPP(MOC_SOURCES ${HEADERS})
QT4_WRAP_UI(UI_SOURCES ${UI_FILES})

add_library(QGlitter STATIC ${SOURCES} ${MOC_SOURCES} ${UI_SOURCES})
target_link_libraries(QGlitter ${QT_LIBRARIES} ${OPENSSL_LIBRARIES} ${PLATFORM_LIBS})
