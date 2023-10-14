cmake_minimum_required(VERSION 3.14)

# задаем используемый стандарт С++
set(CMAKE_CXX_STANDARD 20)

# TODO: переименовать проект
project(ftpv2_server)

# задаем имя исполняемого файла основной программы
set(FTPv2_server ${PROJECT_NAME})

# if (MSVC)
#     # warning level 4 and all warnings as errors
#     add_compile_options(/W4 /WX)
# else()
#     # lots of warnings and all warnings as errors
#     add_compile_options(-Wall -Wextra -pedantic -Werror)
# endif()

find_package(Boost REQUIRED COMPONENTS filesystem system)

add_library(corelib OBJECT src/backend/ftpv2_server.cpp src/backend/client_info.cpp)

add_executable(${FTPv2_server} src/backend/main.cpp)
target_link_libraries(${FTPv2_server} corelib Boost::filesystem Boost::system)