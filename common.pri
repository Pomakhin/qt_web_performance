# header files and dependencies
INCLUDEPATH += $$PWD/common
DEPENDPATH += $$PWD/common
QT += testlib \
    network \
    webkit \
    sql
QT += widgets
QT += webenginewidgets

# install target
isEmpty(INSTALL_DIR):INSTALL_DIR = $$[QT_INSTALL_BINS]
target.path += $$INSTALL_DIR/qtwebkit-benchmark
INSTALLS += target

INCLUDEPATH += $$PWD/bin/common
debug_and_release: {
    CONFIG(debug, debug|release) QMAKE_LIBDIR += $$PWD/common/debug
    else: QMAKE_LIBDIR += $$PWD/common/release
} else:
    QMAKE_LIBDIR += $$PWD/bin/common

!symbian: {
LIBS += -lcommon
}

# Don't use Application bundles on Mac OS X
CONFIG -= app_bundle
