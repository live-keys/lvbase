/****************************************************************************
**
** Copyright (C) 2014-2018 Dinu SV.
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

#ifndef COMMANDS_H
#define COMMANDS_H

#include <QObject>
#include <QHash>
#include <QJSValue>

#include "live/keymap.h"

namespace lv{

class Commands : public QObject{

    Q_OBJECT

    class Node{
    public:
        QJSValue function;
        QJSValue enabled;
        KeyMap::KeyCode key;
    };

public:
    explicit Commands(QObject *parent = 0);
    ~Commands();

    QString dump();

public slots:
    QString add(QObject* object, const QJSValue& command);
    void removeCommandsFor(QObject* object);
    void execute(const QString& command);

private:
    QStringList getCommandChain(QObject *object);

    QHash<QString, Node*> m_commands;
};

}// namespace

#endif // COMMANDS_H