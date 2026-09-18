cmake_minimum_required(VERSION 3.16)

project(Strategix
    VERSION 2.0.0
    DESCRIPTION "Plateforme de jeux de stratégie (Dames & Go) en C++17 et Qt avec IA et multilinguisme"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Qt automatic tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt (Compatible with both Qt6 and Qt5)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

# Localization sources
set(LOCALIZATION_SOURCES
    src/localization/LanguageManager.h
    src/localization/LanguageManager.cpp
)

# Checkers core sources
set(CHECKERS_CORE_SOURCES
    src/core/checkers/CheckersBoard.h
    src/core/checkers/CheckersBoard.cpp
    src/core/checkers/CheckersGame.h
    src/core/checkers/CheckersGame.cpp
    src/core/checkers/CheckersAI.h
    src/core/checkers/CheckersAI.cpp
)

# Go core sources
set(GO_CORE_SOURCES
    src/core/go/GoBoard.h
    src/core/go/GoBoard.cpp
    src/core/go/GoGame.h
    src/core/go/GoGame.cpp
    src/core/go/GoAI.h
    src/core/go/GoAI.cpp
)

# Connect Four core sources
set(CONNECTFOUR_CORE_SOURCES
    src/core/connectfour/ConnectFourBoard.h
    src/core/connectfour/ConnectFourBoard.cpp
    src/core/connectfour/ConnectFourGame.h
    src/core/connectfour/ConnectFourGame.cpp
    src/core/connectfour/ConnectFourAI.h
    src/core/connectfour/ConnectFourAI.cpp
)

# Reversi core sources
set(REVERSI_CORE_SOURCES
    src/core/reversi/ReversiBoard.h
    src/core/reversi/ReversiBoard.cpp
    src/core/reversi/ReversiGame.h
    src/core/reversi/ReversiGame.cpp
    src/core/reversi/ReversiAI.h
    src/core/reversi/ReversiAI.cpp
)

# Quoridor core sources
set(QUORIDOR_CORE_SOURCES
    src/core/quoridor/QuoridorBoard.h
    src/core/quoridor/QuoridorBoard.cpp
    src/core/quoridor/QuoridorGame.h
    src/core/quoridor/QuoridorGame.cpp
    src/core/quoridor/QuoridorAI.h
    src/core/quoridor/QuoridorAI.cpp
)

# Nine Men's Morris core sources
set(MORRIS_CORE_SOURCES
    src/core/morris/MorrisBoard.h
    src/core/morris/MorrisBoard.cpp
    src/core/morris/MorrisGame.h
    src/core/morris/MorrisGame.cpp
    src/core/morris/MorrisAI.h
    src/core/morris/MorrisAI.cpp
)

# Dots and Boxes core sources
set(DOTSANDBOXES_CORE_SOURCES
    src/core/dotsandboxes/DotsAndBoxesBoard.h
    src/core/dotsandboxes/DotsAndBoxesBoard.cpp
    src/core/dotsandboxes/DotsAndBoxesGame.h
    src/core/dotsandboxes/DotsAndBoxesGame.cpp
    src/core/dotsandboxes/DotsAndBoxesAI.h
    src/core/dotsandboxes/DotsAndBoxesAI.cpp
)

# UI sources
set(UI_SOURCES
    src/ui/GameSelectionWidget.h
    src/ui/GameSelectionWidget.cpp
    src/ui/checkers/CheckersWidget.h
    src/ui/checkers/CheckersWidget.cpp
    src/ui/go/GoBoardWidget.h
    src/ui/go/GoBoardWidget.cpp
    src/ui/connectfour/ConnectFourWidget.h
    src/ui/connectfour/ConnectFourWidget.cpp
    src/ui/reversi/ReversiWidget.h
    src/ui/reversi/ReversiWidget.cpp
    src/ui/quoridor/QuoridorWidget.h
    src/ui/quoridor/QuoridorWidget.cpp
    src/ui/morris/MorrisWidget.h
    src/ui/morris/MorrisWidget.cpp
    src/ui/dotsandboxes/DotsAndBoxesWidget.h
    src/ui/dotsandboxes/DotsAndBoxesWidget.cpp
    src/ui/dialogs/RulesDialog.h
    src/ui/dialogs/RulesDialog.cpp
    src/ui/dialogs/AboutDialog.h
    src/ui/dialogs/AboutDialog.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
)

# Main Application Executable
add_executable(strategix
    src/main.cpp
    ${LOCALIZATION_SOURCES}
    ${CHECKERS_CORE_SOURCES}
    ${GO_CORE_SOURCES}
    ${CONNECTFOUR_CORE_SOURCES}
    ${REVERSI_CORE_SOURCES}
    ${QUORIDOR_CORE_SOURCES}
    ${MORRIS_CORE_SOURCES}
    ${DOTSANDBOXES_CORE_SOURCES}
    ${UI_SOURCES}
    resources.qrc
)

target_include_directories(strategix PRIVATE src)

target_link_libraries(strategix PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
)

if(MSVC)
    target_compile_options(strategix PRIVATE /W4)
else()
    target_compile_options(strategix PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Unit Tests (CTest)
enable_testing()

# Test Checkers
add_executable(test_checkers
    tests/test_checkers.cpp
    ${CHECKERS_CORE_SOURCES}
)
target_include_directories(test_checkers PRIVATE src)
if(NOT MSVC)
    target_compile_options(test_checkers PRIVATE -Wall -Wextra)
endif()
add_test(NAME CheckersTests COMMAND test_checkers)

# Test Go
add_executable(test_go
    tests/test_go.cpp
    ${GO_CORE_SOURCES}
)
target_include_directories(test_go PRIVATE src)
if(NOT MSVC)
    target_compile_options(test_go PRIVATE -Wall -Wextra)
endif()
add_test(NAME GoTests COMMAND test_go)

# Test Connect Four
add_executable(test_connectfour
    tests/test_connectfour.cpp
    ${CONNECTFOUR_CORE_SOURCES}
)
target_include_directories(test_connectfour PRIVATE src)
if(NOT MSVC)
    target_compile_options(test_connectfour PRIVATE -Wall -Wextra)
endif()
add_test(NAME ConnectFourTests COMMAND test_connectfour)

# Test Reversi
add_executable(test_reversi
    tests/test_reversi.cpp
    ${REVERSI_CORE_SOURCES}
)
target_include_directories(test_reversi PRIVATE src)
if(NOT MSVC)
    target_compile_options(test_reversi PRIVATE -Wall -Wextra)
endif()
add_test(NAME ReversiTests COMMAND test_reversi)

# Test Quoridor
add_executable(test_quoridor
    tests/test_quoridor.cpp
    ${QUORIDOR_CORE_SOURCES}
)
target_include_directories(test_quoridor PRIVATE src)
if(NOT MSVC)
    target_compile_options(test_quoridor PRIVATE -Wall -Wextra)
endif()
add_test(NAME QuoridorTests COMMAND test_quoridor)

# Test Nine Men's Morris
add_executable(test_morris
    tests/test_morris.cpp
    ${MORRIS_CORE_SOURCES}
)
target_include_directories(test_morris PRIVATE src)
if(NOT MSVC)
    target_compile_options(test_morris PRIVATE -Wall -Wextra)
endif()
add_test(NAME MorrisTests COMMAND test_morris)

# Test Dots and Boxes
add_executable(test_dotsandboxes
    tests/test_dotsandboxes.cpp
    ${DOTSANDBOXES_CORE_SOURCES}
)
target_include_directories(test_dotsandboxes PRIVATE src)
if(NOT MSVC)
    target_compile_options(test_dotsandboxes PRIVATE -Wall -Wextra)
endif()
add_test(NAME DotsAndBoxesTests COMMAND test_dotsandboxes)

