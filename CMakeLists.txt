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

# --- Base target sources (shared for all platforms)
add_executable(liquidssh
        src/main.cpp
        src/MainWindow.cpp
        src/MainWindow.h
        src/Glass.h
        src/Sidebar.cpp
        src/Sidebar.h
        src/InfoTab.cpp
        src/InfoTab.h
        src/TabArea.cpp
        src/TabArea.h
        src/TerminalTab.cpp
        src/TerminalTab.h
        src/CustomTabBar.cpp
        src/CustomTabBar.h
        src/CustomTabWidget.cpp
        src/CustomTabWidget.h
)

# --- Platform-specific implementation
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

# --- Qt link
target_link_libraries(liquidssh PRIVATE Qt6::Widgets)

# --- Include dir
target_include_directories(liquidssh PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- Platform niceties
if(APPLE)
    set_target_properties(liquidssh PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER com.tgssoftware.liquidssh
    )
elseif(WIN32)
    set_target_properties(liquidssh PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
