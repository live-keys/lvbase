/****************************************************************************
**
** Copyright (C) 2022 Dinu SV.
**
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

#include <QCoreApplication>
#include <QTest>

#include "live/applicationcontext.h"
#include "live/elements/engine.h"

#include "testrunner.h"
#include "eventtest.h"
#include "jsobjecttest.h"
#include "jspropertytest.h"
#include "jseventtest.h"
#include "jsfunctiontest.h"
#include "jsmethodtest.h"
#include "jstypestest.h"
#include "jsmemorytest.h"
#include "jslisttest.h"
#include "jserrorhandlingtest.h"
#include "jsinheritancetest.h"
#include "mlnodetojstest.h"
#include "lvcompiletest.h"
#include "jstupletest.h"
#include "lvimportstest.h"
#include "metaobjecttypeinfotest.h"
#include "lvelparseddocumenttest.h"
#include "lvellanguageinfoserializationtest.h"

int main(int argc, char *argv[]){
    lv::ApplicationContext::initialize({});

    QCoreApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);

    lv::el::Engine::InitializeScope initializer("");

//    return lv::TestRunner::runTest(LvParseErrorTest::testIndex, argc, argv);
//    return lv::TestRunner::runTest(LvParseTest::testIndex, argc, argv);
//    return lv::TestRunner::runTest(LvCompileTest::testIndex, argc, argv);
//    return lv::TestRunner::runTest(LvImportsTest::testIndex, argc, argv);
//    return lv::TestRunner::runTest(JsErrorHandlingTest::testIndex, argc, argv);
    return lv::TestRunner::runTests(argc, argv);
}
