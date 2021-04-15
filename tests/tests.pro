GOOGLETEST_DIR = $$PWD/../googletest
include(gtest_dependency.pri)

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG += thread
CONFIG -= qt

HEADERS += \
        ../circular_buffer.h \
        ../circular_buffer_lockfree.h \
        tst_circular_buffer.h \
        tst_circular_buffer_lockfree.h

SOURCES += \
        main.cpp
