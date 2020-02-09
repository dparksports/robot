cmake_minimum_required(VERSION 3.15)
project(robot)

set(CMAKE_CXX_STANDARD 17)

find_package( OpenCV REQUIRED )
set(SOURCE_FILES main.cpp)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} )

find_package (Eigen3 3.3 REQUIRED NO_MODULE)
target_link_libraries (${PROJECT_NAME} Eigen3::Eigen)

#find_package(Boost 1.72 REQUIRED)
#include_directories( ${Boost_INCLUDE_DIR} )
#target_link_libraries(${PROJECT_NAME} Public Boost::boost)
#TARGET_LINK_LIBRARIES(${PROJECT_NAME} LINK_PUBLIC ${Boost_LIBRARIES} )

find_package(Boost 1.72 REQUIRED COMPONENTS
        date_time filesystem iostreams)
#add_executable(foo foo.cc)
target_link_libraries(${PROJECT_NAME} Boost::date_time Boost::filesystem
        Boost::iostreams)
