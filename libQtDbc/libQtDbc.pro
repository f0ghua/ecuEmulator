#-------------------------------------------------
#
# Project created by QtCreator 2016-08-19T18:44:04
#
#-------------------------------------------------

QT       -= gui

TARGET = QtDbc
TEMPLATE = lib

macx {
# mac only
}
unix:!macx{
# linux only
QMAKE_CXX=ccache g++
}
win32 {
    # windows only
    QMAKE_CXXFLAGS += -march=i686
}

DEFINES += LIBQTDBC_LIBRARY

SOURCES += src/DbcHandle.cpp\
        src/File.cpp\
        src/Network.cpp \
    src/Node.cpp \
    src/Message.cpp \
    src/Signal.cpp \
    src/AttributeDefinition.cpp \
    src/Attribute.cpp \
    src/SignalGroup.cpp \
    src/Utility.cpp \
	src/DBCHelper.cpp

HEADERS += inc/DbcHandle.h\
    inc/libqtdbc_global.h\
    inc/Attribute.h \
    inc/AttributeDefinition.h \
    inc/AttributeValueType.h \
    inc/BitTiming.h \
    inc/ByteOrder.h \
    inc/DbcHandle.h \
    inc/EnvironmentVariable.h \
    inc/ExtendedMultiplexor.h \
    inc/File.h \
    inc/Message.h \
    inc/Network.h \
    inc/Node.h \
    inc/platform.h \
    inc/SignalDbc.h \
    inc/SignalGroup.h \
    inc/Status.h \
    inc/Utility.h \
    inc/ValueDescriptions.h \
    inc/ValueTable.h \
    inc/ValueType.h \
    inc/vector_dbc_export.h \
	inc/DBCHelper.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += ./inc
DESTDIR += ../output/libQtDbc

DISTFILES += \
    .tags \
    .tags_sorted_by_file
