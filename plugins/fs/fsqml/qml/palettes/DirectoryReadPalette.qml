/****************************************************************************
**
** Copyright (C) 2014-2019 Dinu SV.
** (contact: mail@dinusv.com)
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

import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.4
import editor 1.0
import live 1.0
import base 1.0
import visual.input 1.0 as Input

CodePalette{
    id: palette

    type: "qml/fs#DirectoryRead"

    property QtObject theme: lk.layers.workspace.themes.current

    item: Item{
        id: directoryReadItem
        width: 100
        height: 25

        property var current : null

        Input.TextButton{
            id: textButton
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.left: parent.left
            anchors.leftMargin: 10
            height: 28
            width: 80
            style: theme.formButtonStyle
            text: "Start"

            onClicked: {
                if (directoryReadItem.state == "NOT_RUNNING"){
                    directoryReadItem.current.start()
                }
            }
        }

        state: "NOT_RUNNING"

        states: [
            State {
                name: "NOT_RUNNING"
                PropertyChanges {
                    target: textButton
                    text: "Start"
                }
            },
            State {
                name: "RUNNING"
                PropertyChanges {
                    target: textButton
                    text: "Stop"
                }
            }
        ]
    }

    onValueFromBindingChanged: {
        directoryReadItem.current = value
    }
    onInit: {
        directoryReadItem.current = value
        editFragment.whenBinding = function(){}
    }

}
