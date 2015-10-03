######################################################################
# Automatically generated by qmake (2.01a) Sat May 17 16:29:31 2014
######################################################################

TEMPLATE = app
TARGET = d2ng-pomcp
DEPENDPATH += . src
INCLUDEPATH += . src
LIBS += -lboost_program_options

CONFIG -= qt debug
INCLUDEPATH -= qt

# Input
HEADERS += src/battleship.h \
           src/beliefstate.h \
           src/coord.h \
           src/experiment.h \
           src/grid.h \
           src/history.h \
           src/mcts.h \
           src/memorypool.h \
           src/network.h \
           src/node.h \
           src/pocman.h \
           src/rocksample.h \
           src/simulator.h \
           src/statistic.h \
           src/tag.h \
           src/testsimulator.h \
           src/utils.h \
           src/distribution.h
SOURCES += src/battleship.cpp \
           src/beliefstate.cpp \
           src/coord.cpp \
           src/experiment.cpp \
           src/main.cpp \
           src/mcts.cpp \
           src/network.cpp \
           src/node.cpp \
           src/pocman.cpp \
           src/rocksample.cpp \
           src/simulator.cpp \
           src/tag.cpp \
           src/testsimulator.cpp \
           src/utils.cpp \
           src/distribution.cpp
