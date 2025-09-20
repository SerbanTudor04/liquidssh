cmake_minimum_required(VERSION 3.20)  # Qt6 needs >=3.16; 3.20+ is safe
project(liquidssh LANGUAGES CXX)

# --- C++23, no extensions
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Qt's auto-moc/uic/rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# --- Qt6
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# --- Target
add_executable(liquidssh
        src/main.cpp
        src/MainWindow.cpp
        src/MainWindow.h
        src/Glass_stub.cpp
        src/Glass_stub.cpp
        src/Glass.cpp
        src/Glass.h
        src/Glass_win.cpp
        # forms/MainWindow.ui   # if you add a .ui later, just uncomment
        # resources.qrc         # same for a .qrc
)

target_link_libraries(liquidssh PRIVATE Qt6::Widgets)

target_include_directories(liquidssh PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- Platform niceties
# macOS: use Objective-C++ for the vibrancy implementation
if(APPLE)
    target_sources(liquidssh PRIVATE src/Glass_mac.mm)
    set_source_files_properties(src/Glass_mac.mm PROPERTIES COMPILE_FLAGS "-fobjc-arc")
    target_link_libraries(liquidssh PRIVATE "-framework Cocoa" "-framework AppKit")
elseif(WIN32)
    target_sources(liquidssh PRIVATE src/Glass_win.cpp)
    target_link_libraries(liquidssh PRIVATE Dwmapi)
endif()
