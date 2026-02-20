cmake_minimum_required(VERSION 4.1)
project(Loni_toolbox)

include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "Loni_Toolbox")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_CONTACT "Kyyous@outlook.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A small toolbox")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" CACHE FILEPATH "License file")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -mwindows")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.10.2/llvm-mingw_64")

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)


add_executable(Loni_toolbox main.cpp
        src/main_window.h
        src/main_window.cpp
        src/backend.h
        src/backend.cpp
        src/lib_e/switch_button.h
        src/lib_e/switch_button.cpp
)

target_link_libraries(Loni_toolbox
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION .
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/asset"
        DESTINATION .
)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/help"
        DESTINATION .
)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
        DESTINATION .
)

if (WIN32)
    set(DEBUG_SUFFIX "")
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif()
    endif()
    foreach (QT_LIB Core Gui Widgets)
        set(_qt_dll "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll")
        if (EXISTS "${_qt_dll}")
            install(FILES "${_qt_dll}" DESTINATION .)
        endif()
    endforeach()
    install(FILES "./include/libunwind.dll" DESTINATION .)
    install(FILES "./include/libc++.dll" DESTINATION .)
    install(FILES "./include/libwinpthread-1.dll" DESTINATION .)
    set(_platform_dll "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
    if (EXISTS "${_platform_dll}")
        install(FILES "${_platform_dll}" DESTINATION plugins/platforms)
    endif()
endif()

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Loni_Toolbox")
set(CPACK_NSIS_DISPLAY_NAME "Loni Toolbox")
set(CPACK_NSIS_INSTALLED_ICON_NAME "./Loni_toolbox.exe")

message(STATUS "CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH}")
message(STATUS "CPACK_GENERATOR = ${CPACK_GENERATOR}")

include(CPack)