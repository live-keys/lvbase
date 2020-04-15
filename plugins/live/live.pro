PLUGIN_NAME = live
PLUGIN_PATH = live

# PLUGIN_NAME and PLUGIN_PATH must be set up prior to including this config file
include($$getGlobalFile(plugin.pri))

DEFINES += Q_LIVE_LIB

uri = live

linkLocalLibrary(lvbase,      lvbase)
linkLocalLibrary(lvview,      lvview)
linkLocalLibrary(lveditor,    lveditor)
linkLocalLibrary(lveditqmljs, lveditqmljs)

# Source

unix:!macx{
    QMAKE_LFLAGS += \
	'-Wl,-rpath,\'\$$ORIGIN/../../link\''


    createlinkdir.commands += $${QMAKE_MKDIR_CMD} $$shell_path($${DEPLOY_PATH}/link)
    QMAKE_EXTRA_TARGETS    += createlinkdir
    POST_TARGETDEPS        += createlinkdir
}

include($$PWD/src/live.pri)
include($$PWD/include/liveheaders.pri)

# Deploy samples

samplescopy.commands = $$deployDirCommand($$PWD/samples, $$PLUGIN_DEPLOY_PATH/$$PLUGIN_PATH/samples)
first.depends = $(first) samplescopy
export(first.depends)
export(samplescopy.commands)
QMAKE_EXTRA_TARGETS += first samplescopy

OTHER_FILES += \
    qml/*.qml \
    qml/qmldir \
    qml/plugins.qmltypes \
    doc/*.md \
    samples/*.qml

DISTFILES += \
    qml/PathInputBox.qml \
    qml/ResizeArea.qml \
    qml/live.package.json \
    qml/live.plugin.json \
    qml/forknode.qml
