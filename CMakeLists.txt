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


# Define the Executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/MainWindow.h   
    src/MainWindow.cpp 

    # Components
    src/components/CodeEditor.h
    src/components/CodeEditor.cpp
    src/components/WelcomeWidget.h
    
    src/components/ProjectSidebar.h   
    src/components/ProjectSidebar.cpp 

    src/components/EditorArea.h   
    src/components/EditorArea.cpp 

    src/components/RichTextEditor.h
    src/components/RichTextEditor.cpp

    src/components/CustomRichTextBoard.h
    src/components/CustomRichTextBoard.cpp

    src/components/ImageResizeWidget.h
    src/components/ImageResizeWidget.cpp
    
    src/components/ImageCropDialog.h
    src/components/ImageCropDialog.cpp

    src/components/CommonTooltip.h
    src/components/CommonTooltip.cpp

    src/components/DiffViewDialog.h
    src/components/DiffViewDialog.cpp

    # Core
    src/core/Highlighter.h
    src/core/Highlighter.cpp

    # Utils
    src/utils/DiffHelpers.h
    src/utils/DiffHelpers.cpp

    
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/components
    ${CMAKE_SOURCE_DIR}/src/core
)

# 6. Link the Libraries
# Connect our app to the Qt6 Widgets module found in Step 3
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        Qt6::Widgets
)