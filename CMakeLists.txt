cmake_minimum_required(VERSION 3.20)
project(liquidssh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ---------------- macOS build settings ----------------
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "macOS deployment target")
    # For universal builds, uncomment:
    # set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Universal")
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

        # resources/liquidssh.qrc  # <- add if you have one
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

# ---------------- libssh via pkg-config (macOS/Linux) ----------------
find_package(PkgConfig REQUIRED)
if(APPLE OR UNIX)
    pkg_check_modules(LIBSSH REQUIRED IMPORTED_TARGET libssh)
    target_link_libraries(liquidssh PRIVATE PkgConfig::LIBSSH)
endif()

# ---------------- macOS bundle niceties ----------------
if(APPLE)
    set(LIQUIDSSH_BUNDLE_ID "com.tgssoftware.liquidssh")
    set(LIQUIDSSH_VERSION    "0.0.0")
    set(LIQUIDSSH_SHORTVER   "0.0.0")

    set(MACOSX_BUNDLE_ICON_FILE "LiquidSSH.icns")
    set_source_files_properties(resources/icons/LiquidSSH.icns PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(liquidssh PRIVATE resources/icons/LiquidSSH.icns)

    set(INFO_PLIST_IN  "${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist.in")
    set(INFO_PLIST_OUT "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")
    configure_file("${INFO_PLIST_IN}" "${INFO_PLIST_OUT}" @ONLY)

    set_target_properties(liquidssh PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER "${LIQUIDSSH_BUNDLE_ID}"
            MACOSX_BUNDLE_INFO_PLIST "${INFO_PLIST_OUT}"
    )
endif()

# ---------------- Install (optional) ----------------
install(TARGETS liquidssh
        BUNDLE DESTINATION .
        RUNTIME DESTINATION bin
)

# ====================================================================================
# Debug vs Release behavior (macOS)
# - Debug: run-from-SDK/Homebrew, no bundling/no signing; smooth CLion run.
# - Release: bundle libssh + Qt into .app and deep ad-hoc sign.
# Works with single-config (Ninja) and multi-config (Xcode) via generator expressions.
# ====================================================================================
if(APPLE)
    # Useful paths (tweak if your setup differs)
    set(HOMEBREW_PREFIX "/opt/homebrew")            # Intel: /usr/local
    set(QT_HINT "$ENV{Qt6_DIR}")
    if(NOT QT_HINT OR QT_HINT STREQUAL "")
        set(QT_HINT "$ENV{QT_ROOT_DIR}")
    endif()

    # Common rpath defaults (both configs keep Frameworks searchable)
    set_target_properties(liquidssh PROPERTIES
            BUILD_WITH_INSTALL_RPATH ON
    )

    # ---------------- DEBUG / dev: no bundling, no signing ----------------
    # If single-config: CMAKE_BUILD_TYPE == Debug
    # If multi-config: handled by genex below.
    set_property(TARGET liquidssh APPEND PROPERTY
            INSTALL_RPATH
            "$<IF:$<CONFIG:Debug>,${HOMEBREW_PREFIX}/lib;${QT_HINT}/lib;@executable_path/../Frameworks;@loader_path/../Frameworks,@executable_path/../Frameworks;@loader_path/../Frameworks>"
    )

    # ---------------- RELEASE / dist: bundle + macdeployqt + sign ----------------

    # 1) Bundle libssh from Homebrew inside the app (Release only)
    # We guard copy/fix commands with $<CONFIG:...> so they trigger only for Release/RelWithDebInfo.
    find_library(LIBSSH_DYLIB
            NAMES ssh libssh libssh.4
            HINTS "${HOMEBREW_PREFIX}/lib" "/usr/local/lib"
            NO_DEFAULT_PATH
    )
    if(NOT LIBSSH_DYLIB)
        message(STATUS "Note: libssh not found at ${HOMEBREW_PREFIX}/lib or /usr/local/lib. If you build Release, brew install libssh.")
    endif()

    if(LIBSSH_DYLIB)
        add_custom_command(TARGET liquidssh POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E env CONFIG=$<CONFIG>
                bash -lc '[ "$$CONFIG" = "Release" -o "$$CONFIG" = "RelWithDebInfo" ] || exit 0; \
                mkdir -p "$<TARGET_BUNDLE_CONTENT_DIR:liquidssh>/Frameworks"; \
                BASENAME="$(basename "${LIBSSH_DYLIB}")"; \
                cp -f "${LIBSSH_DYLIB}" "$<TARGET_BUNDLE_CONTENT_DIR:liquidssh>/Frameworks/$$BASENAME"; \
                install_name_tool -id "@rpath/$$BASENAME" "$<TARGET_BUNDLE_CONTENT_DIR:liquidssh>/Frameworks/$$BASENAME"; \
                install_name_tool -add_rpath "@executable_path/../Frameworks" "$<TARGET_FILE:liquidssh>" || true; \
                install_name_tool -change "${LIBSSH_DYLIB}" "@rpath/$$BASENAME" "$<TARGET_FILE:liquidssh>" || true'
                COMMENT "Release: bundle libssh and fix install names"
                VERBATIM
        )
    endif()

    # 2) Bundle Qt frameworks/plugins with macdeployqt (Release only)
    find_program(MACDEPLOYQT macdeployqt
            HINTS "$ENV{Qt6_DIR}/../../../bin" "$ENV{QT_ROOT_DIR}/bin" "${QT_HINT}/bin"
            "/Users/serbantudor04/Qt/6.9.2/macos/bin"   # <- tweak if needed
    )
    if(MACDEPLOYQT)
        add_custom_command(TARGET liquidssh POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E env CONFIG=$<CONFIG>
                bash -lc '[ "$$CONFIG" = "Release" -o "$$CONFIG" = "RelWithDebInfo" ] || exit 0; \
                "${MACDEPLOYQT}" "$<TARGET_BUNDLE_DIR:liquidssh>" -verbose=2'
                COMMENT "Release: macdeployqt for Qt frameworks/plugins"
                VERBATIM
        )
    endif()

    # 3) Deep ad-hoc sign the app bundle (Release only, ALWAYS last)
    find_program(CODESIGN_EXECUTABLE codesign)
    if(CODESIGN_EXECUTABLE)
        add_custom_command(TARGET liquidssh POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E env CONFIG=$<CONFIG>
                bash -lc '[ "$$CONFIG" = "Release" -o "$$CONFIG" = "RelWithDebInfo" ] || exit 0; \
                codesign --force --deep --timestamp=none -s - "$<TARGET_BUNDLE_DIR:liquidssh>"'
                COMMENT "Release: deep ad-hoc codesign"
                VERBATIM
        )
    endif()
endif()
