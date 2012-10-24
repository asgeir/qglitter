cmake_minimum_required(VERSION 2.8)
project(QGLITTERTOOL)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(Qt4 REQUIRED COMPONENTS QtCore QtGui QtNetwork)

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

set(SOURCES
	main.cpp)

add_executable(qglitter-tool ${SOURCES})
target_link_libraries(qglitter-tool QGlitter ${QT_LIBRARIES} ${PLATFORM_LIBS})
