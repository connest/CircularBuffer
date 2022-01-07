TEMPLATE = app
CONFIG += console c++11 thread
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -pedantic-errors -Wall -Wextra -Weffc++

#LIBS += -lgcov
#QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage

SOURCES += \
        main.cpp

HEADERS += \
    include/circular_buffer/circular_buffer_blocked_mrmw.h \
    include/circular_buffer/circular_buffer_fwd.h \
    include/circular_buffer/circular_buffer_lockfree_mrmw.h \
    include/circular_buffer/circular_buffer_lockfree_srsw.h

INCLUDEPATH += $$PWD/include
