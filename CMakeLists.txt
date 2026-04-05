cmake_minimum_required(VERSION 3.10)
project(compilator_translator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(${PROJECT_NAME} src/main.cpp)

# Флаги компиляции
target_compile_options(${PROJECT_NAME} PRIVATE
    -D_DEBUG -ggdb3 -O0 -Wall -Wextra -Weffc++
    -Wmissing-declarations
    -Wcast-align -Wcast-qual -Wchar-subscripts
    -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal
    -Wformat=2 -Wformat-nonliteral -Wformat-security -Wformat-signedness
    -Wlogical-op -Wnon-virtual-dtor -Woverloaded-virtual
    -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-conversion
    -Wstrict-overflow=2 -Wswitch-default -Wswitch-enum
    -Wunused -Wuseless-cast -Wvariadic-macros
    -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing
    -fno-omit-frame-pointer -fstack-protector
    -fsanitize=address,undefined
)

target_link_options(${PROJECT_NAME} PRIVATE
    -fsanitize=address,undefined
)
