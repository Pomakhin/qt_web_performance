TEMPLATE = app

SUBDIRS+=$$PWD/../../common

TARGET = tst_tiledpainting
SOURCES += $$_PRO_FILE_PWD_/$${TARGET}.cpp

include($$PWD/../../common.pri)


