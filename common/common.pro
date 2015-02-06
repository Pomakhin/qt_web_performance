TEMPLATE = lib

QT += testlib network webkit sql
CONFIG += staticlib
TARGET = common

QT += widgets
QT += webkitwidgets

SOURCES += common_init.cpp \
    benchmarkoutputwriter.cpp \
    benchmarkcontroller.cpp \
    benchmark.cc \
    databasenetworkaccessmanager.cpp \
    webpage.cpp
HEADERS += benchmark.h \
    benchmark_p.h \
    benchmarkoutputwriter.h \
    benchmarkcontroller.h \
    common_init.h \
    cookiejar.h \
    databasenetworkaccessmanager.h \
    databasenetworkreply.h \
    databasetests.h \
    urlfilereader.h \
    webpage.h
