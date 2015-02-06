TEMPLATE = subdirs

SUBDIRS += common

# tests
TESTNAMES = loading painting scrolling cycler layout \
            graphicspainting graphicsscrolling zooming \
            loading_one_qnetworkaccessmanager \
            cycler_one_qnetworkaccessmanager \

SUBDIRS += tests_loading tests_painting tests_scrolling tests_cycler tests_layout \
           tests_graphicspainting tests_graphicsscrolling tests_zooming \
           tests_loading_one_qnetworkaccessmanager \
           tests_cycler_one_qnetworkaccessmanager \
           tests_cycler_one_qnetworkaccessmanager_with_pipelining \

!symbian: {
TESTNAMES += continuous_animation
SUBDIRS += tests_continuous_animation
}

tests_loading.subdir = tests/loading
tests_loading.depends = common
tests_painting.subdir = tests/painting
tests_painting.depends = common
tests_scrolling.subdir = tests/scrolling
tests_scrolling.depends = common
tests_cycler.subdir = tests/cycler
tests_cycler.depends = common
tests_layout.subdir = tests/layout
tests_layout.depends = common
tests_graphicspainting.subdir = tests/graphicspainting
tests_graphicspainting.depends = common
tests_graphicsscrolling.subdir = tests/graphicsscrolling
tests_graphicsscrolling.depends = common
tests_zooming.subdir = tests/zooming
tests_zooming.depends = common
tests_loading_one_qnetworkaccessmanager.subdir = tests/loading_one_qnetworkaccessmanager
tests_loading_one_qnetworkaccessmanager.depends = common
tests_cycler_one_qnetworkaccessmanager.subdir = tests/cycler_one_qnetworkaccessmanager
tests_cycler_one_qnetworkaccessmanager.depends = common
tests_cycler_one_qnetworkaccessmanager_with_pipelining.subdir = tests/cycler_one_qnetworkaccessmanager_with_pipelining
tests_cycler_one_qnetworkaccessmanager_with_pipelining.depends = common

tests_continuous_animation.subdir = tests/continuous_animation
tests_continuous_animation.depends = common


!symbian: {
# reductions
!lessThan(QT_MINOR_VERSION, 6): {
    SUBDIRS += tests_loading_standalone
    tests_loading_standalone.subdir = reductions/loading_standalone
    tests_loading_standalone.depends = common
}

# host tools
SUBDIRS += host-tools/http_server ht_mirror
ht_mirror.subdir = host-tools/mirror
ht_mirror.depends = common
}

symbian: {
load(data_caging_paths)

TARGET.CAPABILITY = NetworkServices LocalServices ReadUserData WriteUserData UserEnvironment
RSS_RULES ="group_name=\"QtBenchmarks\";"

vendorinfo = \
    "; Localised Vendor name" \
    "%{\"Nokia, Qt\"}" \
    " " \
    "; Unique Vendor name" \
    ":\"Nokia, Qt\"" \
    " "

default_deployment.pkg_prerules += vendorinfo

for(tname,TESTNAMES) {
  testsexe.sources += tst_$${tname}.exe
}

testsexe.path = c:PROGRAMS_DIR

for(tname,TESTNAMES) {
  testrsc.sources += $${EPOCROOT}$$HW_ZDIR$$APP_RESOURCE_DIR/tst_$${tname}.rsc
}
testrsc.path = c:$$APP_RESOURCE_DIR

for(tname,TESTNAMES) {
  testregrsc.sources += $${EPOCROOT}$$HW_ZDIR$$REG_RESOURCE_IMPORT_DIR/tst_$${tname}_reg.rsc
}
testregrsc.path = $$REG_RESOURCE_IMPORT_DIR

exists(perfurls.txt) {
testurls.sources = perfurls.txt
testurls.path = c:/data/Others
}

DEPLOYMENT += testrsc testregrsc testsexe testurls
}
