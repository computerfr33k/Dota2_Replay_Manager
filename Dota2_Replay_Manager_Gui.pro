#-------------------------------------------------
#
# Project created by QtCreator 2013-04-12T00:31:09
#
#-------------------------------------------------

QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Dota2_Replay_Manager_Gui
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    edittitle.cpp \
    preferences.cpp

HEADERS  += mainwindow.h \
    edittitle.h \
    preferences.h

FORMS    += mainwindow.ui \
    edittitle.ui \
    preferences.ui
