QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
TARGET = texteditor
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    codeeditor.cpp \
    finddialog.cpp

HEADERS += \
    mainwindow.h \
    codeeditor.h \
    finddialog.h

FORMS += \
    mainwindow.ui
