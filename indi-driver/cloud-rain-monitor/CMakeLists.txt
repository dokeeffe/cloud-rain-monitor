cmake_minimum_required(VERSION 2.4.7)
PROJECT(indi_cloud_rain_monitor CXX C)

set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
set (VERSION_MAJOR 0)
set (VERSION_MINOR 2)
 
find_package(INDI REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${INDI_INCLUDE_DIR})

################ Cloud rain monitor ################
set(cloudrainmonitor_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/cloudrainmonitor.cpp
       ${CMAKE_CURRENT_SOURCE_DIR}/gason.cpp
   )

add_executable(indi_cloud_rain_monitor ${cloudrainmonitor_SRCS})
target_link_libraries(indi_cloud_rain_monitor ${INDI_DRIVER_LIBRARIES} curl)
install(TARGETS indi_cloud_rain_monitor RUNTIME DESTINATION bin )
install(FILES indi_cloud_rain_monitor.xml DESTINATION ${INDI_DATA_DIR})

