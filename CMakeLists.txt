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
     set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Universal")
endif()

# ---------------- Qt ----------------
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

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
        src/NewHostDialog.cpp        src/NewHostDialog.h
        src/types/SSHHost.h
        src/store/HostsStore.cpp        src/store/HostsStore.h
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
target_link_libraries(liquidssh PRIVATE Qt6::Sql)
target_include_directories(liquidssh PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ---------------- libssh via pkg-config (macOS/Linux) ----------------
find_package(PkgConfig REQUIRED)
if(APPLE OR UNIX)
    pkg_check_modules(LIBSSH REQUIRED IMPORTED_TARGET libssh)
    target_link_libraries(liquidssh PRIVATE PkgConfig::LIBSSH)
endif()

# ---------------- macOS bundle niceties ----------------
if(APPLE)
    # App identity & version for Info.plist
    set(LIQUIDSSH_BUNDLE_ID "ro.serbantudor04.liquidssh")
    set(LIQUIDSSH_VERSION   "0.0.1")
    set(LIQUIDSSH_SHORTVER  "0.0.1")

    # Icon
    set(MACOSX_BUNDLE_ICON_FILE "LiquidSSH")
    set_source_files_properties(resources/icons/LiquidSSH.icns PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(liquidssh PRIVATE resources/icons/LiquidSSH.icns)

    # Info.plist from template with @VAR@ tokens
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
# macOS: Debug vs Release behavior
# - Debug/dev (CLion): run-from-SDK/Homebrew; no bundling, no signing; add RPATHs
# - Release/RelWithDebInfo: bundle libssh + Qt via macdeployqt; deep ad-hoc sign (last)
# ====================================================================================
if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)

    # Paths (adjust if needed)
    set(HOMEBREW_PREFIX "/opt/homebrew")      # Intel Macs: /usr/local
    set(QT_HINT "$ENV{Qt6_DIR}")
    if(NOT QT_HINT OR QT_HINT STREQUAL "")
        set(QT_HINT "$ENV{QT_ROOT_DIR}")
    endif()

    # Base rpath: inside the app bundle
    set(BASE_RPATH "@executable_path/../Frameworks;@loader_path/../Frameworks")
    set_target_properties(liquidssh PROPERTIES
            BUILD_WITH_INSTALL_RPATH ON
            INSTALL_RPATH "${BASE_RPATH}"
    )

    # In Debug, also look in Homebrew + Qt SDK (so CLion runs from build tree)
    if(NOT CMAKE_CONFIGURATION_TYPES) # single-config (Ninja, CLion)
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            set_target_properties(liquidssh PROPERTIES
                    INSTALL_RPATH "${BASE_RPATH};${HOMEBREW_PREFIX}/lib;${QT_HINT}/lib"
            )
        endif()
    else() # multi-config (Xcode)
        set_target_properties(liquidssh PROPERTIES
                INSTALL_RPATH_DEBUG "${BASE_RPATH};${HOMEBREW_PREFIX}/lib;${QT_HINT}/lib"
        )
    endif()

    # -------- Release helpers: scripts we will generate and run with cmake -P --------
    set(_SCRIPT_DIR "${CMAKE_CURRENT_BINARY_DIR}/cmake_scripts")
    file(MAKE_DIRECTORY "${_SCRIPT_DIR}")

    # 1) bundle_libssh.cmake: copy Homebrew libssh into Contents/Frameworks and fix install names
    file(WRITE "${_SCRIPT_DIR}/bundle_libssh.cmake" [=[
    if(NOT DEFINED CFG OR (CFG STREQUAL "Debug"))
      return()
    endif()
    if(NOT (CFG STREQUAL "Release" OR CFG STREQUAL "RelWithDebInfo"))
      return()
    endif()

    if(NOT DEFINED BUNDLE_CONTENTS OR NOT DEFINED EXE OR NOT DEFINED LIBSSH_DYLIB)
      message(FATAL_ERROR "bundle_libssh.cmake: missing args")
    endif()

    file(MAKE_DIRECTORY "${BUNDLE_CONTENTS}/Frameworks")
    get_filename_component(_LIBSSH_BASENAME "${LIBSSH_DYLIB}" NAME)
    file(COPY "${LIBSSH_DYLIB}" DESTINATION "${BUNDLE_CONTENTS}/Frameworks")
    set(_BUNDLED "${BUNDLE_CONTENTS}/Frameworks/${_LIBSSH_BASENAME}")

    # Set the dylib's id to @rpath/...
    execute_process(COMMAND install_name_tool -id "@rpath/${_LIBSSH_BASENAME}" "${_BUNDLED}"
                    RESULT_VARIABLE r1)
    if(r1)
      message(FATAL_ERROR "install_name_tool -id failed: ${r1}")
    endif()

    # Ensure exe has rpath to Frameworks
    execute_process(COMMAND install_name_tool -add_rpath "@executable_path/../Frameworks" "${EXE}"
                    RESULT_VARIABLE r2)
    # ignore r2 nonzero if rpath exists already

    # Rewrite exe's reference to Homebrew lib -> @rpath/...
    execute_process(COMMAND install_name_tool -change "${LIBSSH_DYLIB}" "@rpath/${_LIBSSH_BASENAME}" "${EXE}"
                    RESULT_VARIABLE r3)
    # ignore r3 nonzero if not referenced directly (ok)
  ]=])

    # 2) macdeployqt.cmake: bundle Qt frameworks/plugins
    file(WRITE "${_SCRIPT_DIR}/macdeployqt.cmake" [=[
    if(NOT DEFINED CFG OR (CFG STREQUAL "Debug"))
      return()
    endif()
    if(NOT (CFG STREQUAL "Release" OR CFG STREQUAL "RelWithDebInfo"))
      return()
    endif()

    if(NOT DEFINED BUNDLE_DIR OR NOT DEFINED MACDEPLOYQT)
      message(FATAL_ERROR "macdeployqt.cmake: missing args")
    endif()

    execute_process(COMMAND "${MACDEPLOYQT}" "${BUNDLE_DIR}" -verbose=2
                    RESULT_VARIABLE r)
    if(r)
      message(FATAL_ERROR "macdeployqt failed: ${r}")
    endif()
  ]=])

    # 3) codesign.cmake: deep ad-hoc sign the app (must be LAST)
    file(WRITE "${_SCRIPT_DIR}/codesign.cmake" [=[
    if(NOT DEFINED CFG OR (CFG STREQUAL "Debug"))
      return()
    endif()
    if(NOT (CFG STREQUAL "Release" OR CFG STREQUAL "RelWithDebInfo"))
      return()
    endif()

    if(NOT DEFINED BUNDLE_DIR OR NOT DEFINED CODESIGN)
      message(FATAL_ERROR "codesign.cmake: missing args")
    endif()

    execute_process(COMMAND "${CODESIGN}" --force --deep --timestamp=none -s - "${BUNDLE_DIR}"
                    RESULT_VARIABLE r)
    if(r)
      message(FATAL_ERROR "codesign failed: ${r}")
    endif()
  ]=])

    # -------- Release: locate libssh + macdeployqt + codesign tools --------
    find_library(LIBSSH_DYLIB
            NAMES ssh libssh libssh.4
            HINTS "${HOMEBREW_PREFIX}/lib" "/usr/local/lib"
            NO_DEFAULT_PATH
    )
    # Not fatal here; script handles missing case gracefully (but you'll want libssh for Release)

    find_program(MACDEPLOYQT macdeployqt
            HINTS "$ENV{Qt6_DIR}/../../../bin" "$ENV{QT_ROOT_DIR}/bin" "${QT_HINT}/bin"
            "/Users/serbantudor04/Qt/6.9.2/macos/bin"  # adjust if needed
    )
    find_program(CODESIGN_EXECUTABLE codesign)

    # -------- Attach the post-build steps (Release only via CFG var) --------
    # Bundle libssh
    add_custom_command(TARGET liquidssh POST_BUILD
            COMMAND "${CMAKE_COMMAND}"
            -DCFG=$<CONFIG>
            -DBUNDLE_CONTENTS=$<TARGET_BUNDLE_CONTENT_DIR:liquidssh>
            -DEXE=$<TARGET_FILE:liquidssh>
            -DLIBSSH_DYLIB=${LIBSSH_DYLIB}
            -P "${_SCRIPT_DIR}/bundle_libssh.cmake"
            COMMENT "Release: bundle libssh & fix install names"
    )

    # macdeployqt
    add_custom_command(TARGET liquidssh POST_BUILD
            COMMAND "${CMAKE_COMMAND}"
            -DCFG=$<CONFIG>
            -DBUNDLE_DIR=$<TARGET_BUNDLE_DIR:liquidssh>
            -DMACDEPLOYQT=${MACDEPLOYQT}
            -P "${_SCRIPT_DIR}/macdeployqt.cmake"
            COMMENT "Release: macdeployqt frameworks/plugins"
    )

    # deep ad-hoc codesign (last)
    add_custom_command(TARGET liquidssh POST_BUILD
            COMMAND "${CMAKE_COMMAND}"
            -DCFG=$<CONFIG>
            -DBUNDLE_DIR=$<TARGET_BUNDLE_DIR:liquidssh>
            -DCODESIGN=${CODESIGN_EXECUTABLE}
            -P "${_SCRIPT_DIR}/codesign.cmake"
            COMMENT "Release: deep ad-hoc codesign"
    )
endif()
