TEMPLATE = app
TARGET = image_cycling
SOURCES = main.cpp \
          ../shared/ImageDecoderQt.cpp ../shared/RGBA32BufferQt.cpp ../shared/IntSizeQt.cpp ../shared/IntRectQt.cpp ../shared/IntPointQt.cpp
RESOURCES = images.qrc

include(../../common.pri)
INCLUDEPATH += ../shared ../shared/WebCore/

