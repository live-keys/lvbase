
#//TODO Check if LIVECV_DEV_PATH, LIVECV_BIN_PATH have been set and exist

BUILD_PATH  = $$shadowed($$PROJECT_ROOT)
DEPLOY_PATH = $$LIVECV_BIN_PATH

macx:PLUGIN_DEPLOY_PATH = $$DEPLOY_PATH/plugins
else:PLUGIN_DEPLOY_PATH = $$DEPLOY_PATH/PlugIns

include($$LIVECV_DEV_PATH/project/functions.pri)
