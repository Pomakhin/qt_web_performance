TEMPLATE = app

SOURCES += main.cpp ../../common/cookiejar.cpp
HEADERS += ../../common/cookiejar.h

include(../../common.pri)
QT += sql
