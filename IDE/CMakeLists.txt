cmake_minimum_required(VERSION 3.16)

project(My_IDE VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 修改此处：添加 Network 组件
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(PROJECT_SOURCES
        main.cpp
        My_IDE.cpp
        My_IDE.h
        cpphighlighter.cpp
        cpphighlighter.h
        LineNumberArea.h
        CodeEditor.h
        LineNumberArea.cpp
        CodeEditor.cpp
        bracketmatcher.h
        bracketmatcher.cpp
        findreplacedialog.h
        findreplacedialog.cpp
        tabwidgetmanager.h
        tabwidgetmanager.cpp
        formatter.h
        formatter.cpp
        Resources.qrc

        debuggermanager.h
        debuggermanager.cpp
        breakpointmanager.h
        breakpointmanager.cpp
        colorrefresher.h
        colorrefresher.cpp
        deepseekchat.h
        deepseekchat.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(My_IDE
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(My_IDE SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(My_IDE
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# 修改此处：链接 Network 模块
target_link_libraries(My_IDE PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.My_IDE)
endif()
set_target_properties(My_IDE PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS My_IDE
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(My_IDE)
endif()
