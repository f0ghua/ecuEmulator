#-------------------------------------------------
#
# Project created by QtCreator 2017-06-24T11:07:37
#
#-------------------------------------------------

QT       += core gui serialport network
#QT       += axcontainer
#DEFINES  += QAXOBJECT_SUPPORT

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ecuSimulator
TEMPLATE = app

top_srcdir = $$PWD
top_builddir = $$shadowed($$PWD)

# auto touch aboutdialog to update release datetime
#versionTarget.target = extra
#versionTarget.depends = FORCE
#win32: versionTarget.commands = touch.exe $$top_srcdir/src/aboutdialog.cpp
#else:  versionTarget.commands = touch $$top_srcdir/src/aboutdialog.cpp
#PRE_TARGETDEPS += extra
#QMAKE_EXTRA_TARGETS += versionTarget

CONFIG(release, debug|release): {
    DEFINES += F_NO_DEBUG
    #DEFINES += F_ENABLE_TRACEFILE
    # We add debug info in release f/w so that we can get dig crash issues
    # the release bin should be stripped after "objdump -S"
    QMAKE_CXXFLAGS_RELEASE += -g
    # prevent from linker strip(no '-s')
    QMAKE_LFLAGS_RELEASE =
}

CONFIG(debug, debug|release): {
    DEFINES += F_DISABLE_CRASHDUMP
}

QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter -Wno-unused-variable

win32 {
	# windows only
	QMAKE_CXXFLAGS += -march=i686
	LIBS += $${top_srcdir}/libftd/ftd2xx.lib -lwinmm
	#DEFINES += Q_PLATFORM_WIN
}

SOURCES += \
    src/main.cpp\
    src/mainwindow.cpp \
    src/ftworker.cpp \
    src/hal.cpp \
    src/xbusframe.cpp \
    src/utils.cpp \
    src/xbusmgr.cpp \
    src/connectdialog.cpp \
    src/xframelogger.cpp \
    src/xcmdframe.cpp \
    src/busengine.cpp \
    src/deviceconfig.cpp \
    src/serialworker.cpp \
    src/aboutdialog.cpp \
    src/xframesender.cpp

HEADERS  += \
    src/mainwindow.h \
    src/ftworker.h \
    src/hal.h \
    src/xbusframe.h \
    src/xcommdefine.h \
    src/utils.h \
    src/xbusmgr.h \
    src/connectdialog.h \
    src/xframelogger.h \
    src/xcmdframe.h \
    src/busengine.h \
    src/deviceconfig.h \
    src/serialworker.h \
    src/aboutdialog.h \
    src/xframesender.h

FORMS    += \
    ui/mainwindow.ui \
    ui/connectdialog.ui \
    ui/deviceconfig.ui

RESOURCES += \
    res/ctt.qrc

#RC_FILE = ecuSimulator.rc
DESTDIR += ../output/
INCLUDEPATH += src ../libQtDbc/inc libftd
LIBS+= -L../output/libQtDbc -lQtDbc

