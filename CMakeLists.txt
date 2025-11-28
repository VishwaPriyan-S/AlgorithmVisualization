cmake_minimum_required(VERSION 3.16)

project(AlgorithmVisualizaion VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# -------------------------------------------------------------------------
# 1. BUILD LUA FROM SOURCE
# This runs the CMake file you created in Step 1 inside thirdparty/lua
# -------------------------------------------------------------------------
add_subdirectory(thirdparty/lua)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(AlgorithmVisualizaion
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        step_data.h step_data.cpp
        algorithm_base.h algorithm_base.cpp
        algorithm_manager.h algorithm_manager.cpp
        Widgets/code_highlighter.h Widgets/code_highlighter.cpp
        Widgets/visualization_widget.h Widgets/visualization_widget.cpp
        Widgets/array_visualizer.h Widgets/array_visualizer.cpp
        Widgets/graph_visualizer.h Widgets/graph_visualizer.cpp
        Widgets/control_panel.h Widgets/control_panel.cpp
        interpreter/lexer.h
        interpreter/vis_event.h
        interpreter/lexer.cpp
        interpreter/ast.h
        interpreter/parser.h
        interpreter/parser.cpp
        interpreter/interpreter.h
        interpreter/interpreter.cpp
        interpreter/interpreter_algorithm.h
        interpreter/LuaParserEngine.h
        interpreter/LuaParserEngine.cpp
    )
else()
    if(ANDROID)
        add_library(AlgorithmVisualizaion SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(AlgorithmVisualizaion
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# -------------------------------------------------------------------------
# 2. LINK LUA
# We link the target "lua" defined in the subdirectory.
# Note: We do NOT need target_include_directories, it is handled automatically.
# -------------------------------------------------------------------------
target_link_libraries(AlgorithmVisualizaion PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    lua
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.AlgorithmVisualizaion)
endif()

set_target_properties(AlgorithmVisualizaion PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS AlgorithmVisualizaion
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(AlgorithmVisualizaion)
endif()
