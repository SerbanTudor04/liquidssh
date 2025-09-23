cmake_minimum_required(VERSION 3.20)
project(liquidssh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# --- Sources/headers
add_executable(liquidssh
        src/main.cpp
        src/MainWindow.cpp          src/MainWindow.h
        src/Glass.h
        src/Sidebar.cpp             src/Sidebar.h
        src/InfoTab.cpp             src/InfoTab.h
        src/TabArea.cpp             src/TabArea.h
        src/TerminalTab.cpp         src/TerminalTab.h
        src/TerminalView.cpp        src/TerminalView.h
        src/CustomTabBar.cpp        src/CustomTabBar.h
        src/CustomTabWidget.cpp     src/CustomTabWidget.h
        src/GlassCloseButton.cpp    src/GlassCloseButton.h
        src/SSHWorker.cpp
        src/SSHWorker.h
)

# Platform-specific glass
if(APPLE)
    target_sources(liquidssh PRIVATE src/Glass_mac.mm)
    set_source_files_properties(src/Glass_mac.mm PROPERTIES COMPILE_FLAGS "-fobjc-arc")
    target_link_libraries(liquidssh PRIVATE "-framework Cocoa" "-framework AppKit")
elseif(WIN32)
    target_sources(liquidssh PRIVATE src/Glass_win.cpp)
    target_link_libraries(liquidssh PRIVATE Dwmapi)
else()
    target_sources(liquidssh PRIVATE src/Glass_stub.cpp)
endif()

# Qt
target_link_libraries(liquidssh PRIVATE Qt6::Widgets)
target_include_directories(liquidssh PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- libssh via pkg-config (after add_executable!)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSSH REQUIRED IMPORTED_TARGET libssh)
# This single line adds include dirs, link dirs, and -lssh cleanly:
target_link_libraries(liquidssh PRIVATE PkgConfig::LIBSSH)

# Bundle niceties
if(APPLE)
    set_target_properties(liquidssh PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER com.tgssoftware.liquidssh
    )
elseif(WIN32)
    set_target_properties(liquidssh PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
