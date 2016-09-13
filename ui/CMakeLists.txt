cmake_minimum_required(VERSION 3.2)

find_package(Qt5Widgets REQUIRED)
find_package(PkgConfig REQUIRED)

set(CMAKE_AUTORCC ON)

set(SOURCE_FILES main.cpp X_misc.c)

set(LINK_LIBS
        Qt5::Widgets
        Qt5::Gui
        -lX11
)

find_package(Qt5WebKitWidgets REQUIRED)

add_executable(fcitx-touchscreen-helper main.cpp X_misc.c html.qrc)

target_link_libraries(fcitx-touchscreen-helper ${LINK_LIBS} Qt5::WebKitWidgets)

install(TARGETS fcitx-touchscreen-helper DESTINATION "/usr/bin")
