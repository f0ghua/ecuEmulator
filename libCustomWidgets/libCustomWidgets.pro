
QT += widgets

#RESOURCES += customwidgets.qrc 

HEADERS += \
    xwidgetwithbackground.h \
    xled.h \
    xledswitcher.h

SOURCES += \
    xwidgetwithbackground.cpp \
    xled.cpp \
    xledswitcher.cpp

TARGET = customwidgets

CONFIG += \
	release \
	warn_on \
	staticlib
TEMPLATE = lib
INCLUDEPATH += ./
DESTDIR = ../output/

WIN32{
  DEFINES+= WIN32
}
