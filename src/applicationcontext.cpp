/****************************************************************************
**
** Copyright (C) 2022 Dinu SV.
** This file is part of Livekeys Application.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
****************************************************************************/

#include "live/applicationcontext.h"
#include "live/exception.h"
#include "live/mlnode.h"

namespace lv{

/**
  \class lv::ApplicationContext
  \ingroup lvbase

  \brief Contains the application running context data.

  */

/// \private
class ApplicationContextPrivate{

public:
    static std::unique_ptr<ApplicationContext> instance;

    std::string applicationPath;
    std::string executablePath;
    std::string releasePath;
    std::string applicationFilePath;
    std::string linkPath;
    std::string pluginPath;
    std::string docsPath;
    std::string externalPath;
    std::string librariesPath;
    std::string developmentPath;
    std::string configPath;
    std::string appDataPath;

    MLNode      defaults;

    std::vector<std::string> scriptArguments;
};

std::unique_ptr<ApplicationContext> ApplicationContextPrivate::ApplicationContextPrivate::instance;

/**
 * Default destructor
 */
ApplicationContext::~ApplicationContext(){
    delete m_d;
}

/**
 * Initalizes the singleton instance of the context
 */
void ApplicationContext::initialize(const lv::MLNode &defaults){
    ApplicationContextPrivate::instance.reset(new ApplicationContext(defaults));
}

ApplicationContext::ApplicationContext(const lv::MLNode &defaults)
    : m_d(new ApplicationContextPrivate)
{
    m_d->defaults = defaults;
    initializePaths();
}

void ApplicationContext::initializePaths(){

    m_d->applicationFilePath = applicationFilePathImpl();

#if defined(__APPLE__)
    m_d->applicationPath = Path::resolve(Path::parent(m_d->applicationFilePath) + "/..");
    m_d->executablePath = Path::resolve(Path::parent(m_d->applicationFilePath));
    m_d->releasePath = Path::resolve(Path::parent(m_d->applicationFilePath) + "/../../..");
    m_d->linkPath = applicationPath() + "/Link";
    m_d->pluginPath = applicationPath() + "/PlugIns";
    m_d->externalPath = applicationPath() + "/External";
    m_d->docsPath = applicationPath() + "/Docs";
    m_d->librariesPath =  applicationPath() + "/Libraries";
    m_d->developmentPath = applicationPath() + "/Dev";
    m_d->configPath = applicationPath() + "/config";
#else
    m_d->applicationPath = Path::parent(m_d->applicationFilePath);
    m_d->executablePath = Path::parent(m_d->applicationFilePath);
    m_d->releasePath = Path::parent(m_d->applicationFilePath);
    m_d->linkPath = applicationPath() + "/link";
    m_d->pluginPath = applicationPath() + "/plugins";
    m_d->docsPath = applicationPath() + "/docs";
    m_d->externalPath = applicationPath() + "/external";
    m_d->librariesPath = applicationPath() + "/libraries";
    m_d->developmentPath = applicationPath() + "/dev";
    m_d->configPath = applicationPath() + "/config";
#endif
}

/** Getter of the singleton instance */
ApplicationContext &ApplicationContext::instance(){
    if ( !ApplicationContextPrivate::instance )
        THROW_EXCEPTION(lv::Exception, "Application context has not been initialized", 1);
    return *ApplicationContextPrivate::instance;
}

/** Application path getter */
const std::string &ApplicationContext::applicationPath(){
    return m_d->applicationPath;
}

/** Release path getter */
const std::string &ApplicationContext::releasePath(){
    return m_d->releasePath;
}

/** Application file path getter */
const std::string &ApplicationContext::applicationFilePath(){
    return m_d->applicationFilePath;
}

/** Link path getter */
const std::string &ApplicationContext::linkPath(){
    return m_d->linkPath;
}

/** Docs path */
const std::string &ApplicationContext::docsPath(){
    return m_d->docsPath;
}

/** Plugin path getter */
const std::string &ApplicationContext::pluginPath(){
    return m_d->pluginPath;
}

/** External path getter */
const std::string &ApplicationContext::externalPath(){
    return m_d->externalPath;
}

/** Libraries path getter */
const std::string &ApplicationContext::librariesPath(){
    return m_d->librariesPath;
}

/** Development path getter */
const std::string &ApplicationContext::developmentPath(){
    return m_d->developmentPath;
}

/** Config path getter */
const std::string &ApplicationContext::configPath(){
    return m_d->configPath;
}

/**
 * \brief Returns the startup configuration for Livekeys
 */
const MLNode &ApplicationContext::startupConfiguration(){
    return m_d->defaults;
}

void ApplicationContext::setScriptArguments(const std::vector<std::string> &args){
    m_d->scriptArguments = args;
}

const std::vector<std::string> ApplicationContext::scriptArguments() const{
    return m_d->scriptArguments;
}

/** Executable path getter */
const std::string &ApplicationContext::executablePath(){
    return m_d->executablePath;
}

}// namespace
