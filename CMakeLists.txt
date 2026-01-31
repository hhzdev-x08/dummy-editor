cmake_minimum_required(VERSION 3.16)

# 1. Project Definition
project(qt_editor VERSION 0.1 LANGUAGES CXX)

# 2. Senior Dev Config: Force standard C++17 (Required by Qt6)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 3. Find the Qt6 Libraries
# "REQUIRED" means: "Stop immediately if you can't find Qt"
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# 4. Standard Qt Boilerplate
# AUTOMOC: Handles Qt's "Meta-Object System" (Signals/Slots magic) automatically.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 5. Define the Executable
# Tell CMake to compile main.cpp into an app named "MyQtEditor"
# WIN32: Hides the console window on Windows (optional, keep it off for debug usually)
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/MainWindow.h   
    src/MainWindow.cpp 
    src/Highlighter.h
    src/Highlighter.cpp
    src/CodeEditor.h
    src/CodeEditor.cpp
    src/WelcomeWidget.h
)

# 6. Link the Libraries
# Connect our app to the Qt6 Widgets module found in Step 3
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        Qt6::Widgets
)