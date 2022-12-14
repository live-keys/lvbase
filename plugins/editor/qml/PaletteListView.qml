import QtQuick 2.3
import QtQuick.Controls 2.2
import base 1.0
import editor.private 1.0
import visual.input 1.0 as Input

Input.SelectableListView{
    id: root

    signal paletteSelected(int index)
    onTriggered: paletteSelected(index)

    delegate: Component{
        Rectangle{
            width : root.width
            height : 25
            color : ListView.isCurrentItem ? root.style.selectionBackgroundColor : "transparent"
            Input.Label{
                id: label
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                textStyle: root.style.labelStyle
                text: modelData.name
            }

            MouseArea{
                anchors.fill: parent
                hoverEnabled: true
                onEntered: root.currentIndex = index
                onClicked: root.triggered(index)
            }
        }
    }
}

