# header files and dependencies
INCLUDEPATH += $$PWD/common
DEPENDPATH += $$PWD/common
QT += testlib \
    network \
    webkit \
    sql
QT += widgets
QT += webkitwidgets

# install target
isEmpty(INSTALL_DIR):INSTALL_DIR = $$[QT_INSTALL_BINS]
target.path += $$INSTALL_DIR/qtwebkit-benchmark
INSTALLS += target

INCLUDEPATH += $$PWD/common
debug_and_release: {
    CONFIG(debug, debug|release) QMAKE_LIBDIR += $$PWD/common/debug
    else: QMAKE_LIBDIR += $$PWD/common/release
} else:
    QMAKE_LIBDIR += $$PWD/common

!symbian: {
LIBS += -lcommon
}

# Don't use Application bundles on Mac OS X
CONFIG -= app_bundle
