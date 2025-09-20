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
        # forms/MainWindow.ui   # if you add a .ui later, just uncomment
        # resources.qrc         # same for a .qrc
)

target_link_libraries(liquidssh PRIVATE Qt6::Widgets)

target_include_directories(liquidssh PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- Platform niceties
if(APPLE)
    set_target_properties(liquidssh PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER ro.serbantudor04.liquidssh
    )
elseif(WIN32)
    set_target_properties(liquidssh PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
