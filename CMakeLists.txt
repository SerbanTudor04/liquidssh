cmake_minimum_required(VERSION 3.20)
project(liquidssh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ---------------- macOS build settings (NEW) ----------------
if(APPLE)
    # Pick a reasonable minimum (Sonoma = 14.0; Ventura = 13.0). Adjust if you need older.
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "macOS deployment target")
    # If you want universal binaries, uncomment:
    # set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build universal binary")
endif()

# Qt
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# ---------------- Sources ----------------
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
        src/SSHWorker.cpp           src/SSHWorker.h

        # ---- (NEW) bundle resources / icon / plist ----
        # If you have a Qt resource file, add it here:
        # resources/liquidssh.qrc
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

# Qt link + includes
target_link_libraries(liquidssh PRIVATE Qt6::Widgets)
target_include_directories(liquidssh PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ---------------- libssh via pkg-config (guarded) ----------------
find_package(PkgConfig REQUIRED)
if(APPLE OR UNIX) # keep it for macOS; avoids accidental Windows pkg-config issues later
    pkg_check_modules(LIBSSH REQUIRED IMPORTED_TARGET libssh)
    target_link_libraries(liquidssh PRIVATE PkgConfig::LIBSSH)
endif()

# ---------------- macOS bundle niceties (NEW) ----------------
if(APPLE)
    # App identity & version for Info.plist
    set(LIQUIDSSH_BUNDLE_ID "com.tgssoftware.liquidssh")
    set(LIQUIDSSH_VERSION    "0.0.0")        # overridden by CI if you wish
    set(LIQUIDSSH_SHORTVER   "0.0.0")        # CFBundleShortVersionString

    # Icon: put an .icns in resources/icons/LiquidSSH.icns (example path)
    set(MACOSX_BUNDLE_ICON_FILE "LiquidSSH.icns")
    set_source_files_properties(resources/icons/LiquidSSH.icns PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(liquidssh PRIVATE resources/icons/LiquidSSH.icns)

    # Custom Info.plist (recommended). Create file at resources/Info.plist.in with tokens below.
    # Minimal template example:
    #   <?xml version="1.0" encoding="UTF-8"?>
    #   <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
    #   <plist version="1.0"><dict>
    #     <key>CFBundleName</key><string>LiquidSSH</string>
    #     <key>CFBundleDisplayName</key><string>LiquidSSH</string>
    #     <key>CFBundleIdentifier</key><string>@BUNDLE_ID@</string>
    #     <key>CFBundleVersion</key><string>@BUNDLE_VER@</string>
    #     <key>CFBundleShortVersionString</key><string>@BUNDLE_SHORTVER@</string>
    #     <key>LSMinimumSystemVersion</key><string>${CMAKE_OSX_DEPLOYMENT_TARGET}</string>
    #     <key>CFBundleIconFile</key><string>${MACOSX_BUNDLE_ICON_FILE}</string>
    #     <key>NSRequiresAquaSystemAppearance</key><true/>
    #   </dict></plist>

    set(INFO_PLIST_IN  "${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist.in")
    set(INFO_PLIST_OUT "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")
    configure_file("${INFO_PLIST_IN}" "${INFO_PLIST_OUT}" @ONLY)

    set_target_properties(liquidssh PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER "${LIQUIDSSH_BUNDLE_ID}"
            MACOSX_BUNDLE_INFO_PLIST "${INFO_PLIST_OUT}"
    )
endif()

# ---------------- RPATH for macOS (NEW – macdeployqt friendly) ----------------
if(APPLE)
    # Use @rpath inside the app bundle; macdeployqt will relocate Qt frameworks.
    set_target_properties(liquidssh PROPERTIES
            BUILD_WITH_INSTALL_RPATH ON
            INSTALL_RPATH "@executable_path/../Frameworks"
    )
endif()

# ---------------- Install rules (optional but useful) (NEW) ----------------
# Lets you do `cmake --install build --prefix some/dir` (handy locally or for CPack later)
install(TARGETS liquidssh
        BUNDLE DESTINATION .
        RUNTIME DESTINATION bin
)

# If you have extra runtime assets (qss, images, etc.), install them into the bundle Resources:
# install(FILES path/to/your.asset DESTINATION "LiquidSSH.app/Contents/Resources")
