TEMPLATE = app
TARGET = tst_javascript_parsing
SOURCES = main.cpp

include($$(QTDIR)/src/3rdparty/webkit/WebKit.pri)
include($$(QTDIR)/src/3rdparty/webkit/JavaScriptCore/JavaScriptCore.pri)

include(../../common.pri)
INCLUDEPATH += ../shared ../shared/WebCore/
