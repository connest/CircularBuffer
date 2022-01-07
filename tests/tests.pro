GOOGLETEST_DIR = $$PWD/dependencies/googletest
include(gtest_dependency.pri)

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG += thread
CONFIG -= qt

LIBS += -lgcov
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage

HEADERS += \
        tst_circular_buffer_blocked_mrmw.h \
        tst_circular_buffer_lockfree_mrmw.h \
        tst_circular_buffer_lockfree_srsw.h

SOURCES += \
        main.cpp

INCLUDEPATH += $$PWD/../include
