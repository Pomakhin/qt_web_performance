TEMPLATE = app

TARGET = tst_$$TARGET
SOURCES += $$_PRO_FILE_PWD_/$${TARGET}.cpp

QT += widgets
QT += webkitwidgets

QMAKE_CXXFLAGS += -std=c++11
CONFIG+=c++11

exists($$_PRO_FILE_PWD_/$${TARGET}.qrc):RESOURCES += $$_PRO_FILE_PWD_/$${TARGET}.qrc

include($$PWD/../common.pri)

