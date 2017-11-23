/****************************************************************************
**
** Copyright (C) 2014-2017 Dinu SV.
** (contact: mail@dinusv.com)
** This file is part of Live CV Application.
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

#include "live/plugincontext.h"
#include "live/exception.h"
#include "live/engine.h"
#include "live/settings.h"
#include <QQmlEngine>
#include <QQmlContext>

namespace lv{

lv::Engine*   PluginContext::m_engine   = 0;
lv::Settings* PluginContext::m_settings = 0;

void PluginContext::initFromEngine(QQmlEngine *engine){
    QObject* livecv = engine->rootContext()->contextProperty("livecv").value<QObject*>();
    if ( !livecv )
        THROW_EXCEPTION(lv::Exception, "Failed to load livecv context property.", 0);

    m_engine   = qobject_cast<lv::Engine*>(livecv->property("engine").value<QObject*>());
    m_settings = qobject_cast<lv::Settings*>(livecv->property("settings").value<QObject*>());
    if ( !m_engine || !m_settings )
        THROW_EXCEPTION(lv::Exception, "Failed to load properties from context", 0);
}

}// namespace
