cmake_minimum_required(VERSION 3.14)

# задаем используемый стандарт С++
set(CMAKE_CXX_STANDARD 20)

# TODO: переименовать проект
project(selfdetecting)

# задаем имя исполняемого файла основной программы
set(SelfDetecting ${PROJECT_NAME})

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_library(corelib OBJECT src/backend/multicast_sender.cpp src/backend/multicast_receiver.cpp)

add_executable(${SelfDetecting} src/backend/selfdetector.cpp)
target_link_libraries(${SelfDetecting} corelib)