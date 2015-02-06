TEMPLATE = app
TARGET = image_cycling_webcore
SOURCES = main.cpp \
          ../shared/IntSizeQt.cpp ../shared/IntRectQt.cpp ../shared/IntPointQt.cpp
SOURCES += \
        ../shared/PNGImageDecoder.cpp \
        ../shared/JPEGImageDecoder.cpp \
        ../shared/GIFImageReader.cpp \
        ../shared/GIFImageDecoder.cpp \
        ../shared/ImageDecoder.cpp
RESOURCES = images.qrc
LIBS += -lpng -ljpeg

# We can not clean image data in RGBA32Buffer
DEFINES += WEBCORE_DECODER

include(../../common.pri)
INCLUDEPATH += ../shared ../shared/WebCore/

