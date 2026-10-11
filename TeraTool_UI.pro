QT += core gui widgets svg
CONFIG += c++14 release
TARGET = TeraTool_UI
TEMPLATE = app
SOURCES += main.cpp mainwindow.cpp theme_manager.cpp
HEADERS += shell.h mainwindow.h theme_manager.h
FORMS += ui/mainwindow.ui
RESOURCES += artwork.qrc themes.qrc
