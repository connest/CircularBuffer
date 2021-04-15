TEMPLATE = app
CONFIG += console c++11 thread
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -pedantic-errors -Wall -Wextra -Weffc++

SOURCES += \
        circular_buffer_lockfree.cpp \
        main.cpp

HEADERS += \
    circular_buffer.h \
    circular_buffer_fwd.h \
    circular_buffer_lockfree.h
