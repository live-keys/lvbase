import QtQuick 2.3
import QtQuick.Controls 2.2
import visual.input 1.0 as Input

Rectangle{
    id: root
    visible: false
    color: "transparent"

    onVisibleChanged: {
        if ( visible ){
            show()
        } else {
            close()
        }

    }

    width: 400
    height: 400
    z: 400

    function show(){
        commandsMenuInput.forceActiveFocus()
    }

    function close(){
        commandsView.currentIndex = 0
        commandsMenuInput.text = ''
        visible = false
    }

    Rectangle{
        id: commandsMenuInputBox
        anchors.top: parent.top
        anchors.left: parent.left
        width: parent.width
        height: 30

        color: "#05090e"

        border.color: "#181d24"
        border.width: 1

        TextInput{
            id : commandsMenuInput
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 6
            anchors.right: parent.right
            anchors.rightMargin: 6

            clip: true

            width: parent.width

            color : "#afafaf"
            font.family: "Source Code Pro, Ubuntu Mono, Courier New, Courier"
            font.pixelSize: 12
            font.weight: Font.Normal

            selectByMouse: true

            text: ""
            onTextChanged: {
                lk.layers.workspace.commands.model.setFilter(text)
            }

            MouseArea{
                anchors.fill: parent
                acceptedButtons: Qt.NoButton
                cursorShape: Qt.IBeamCursor
            }

            z: 300
            Keys.onPressed: {
                if ( event.key === Qt.Key_Down ){
                    commandsView.highlightNext()
                    event.accepted = true
                } else if ( event.key === Qt.Key_Up ){
                    commandsView.highlightPrev()
                    event.accepted = true
                } else if ( event.key === Qt.Key_PageDown ){
                    commandsView.highlightNextPage()
                    event.accepted = true
                } else if ( event.key === Qt.Key_PageUp ){
                    commandsView.highlightPrevPage()
                    event.accepted = true
                } else if ( event.key === Qt.Key_K && ( event.modifiers & Qt.ControlModifier) ){
                    root.close()
                    root.cancel()
                    event.accepted = true
                }
            }
            Keys.onReturnPressed: {
                if (commandsView.currentItem) {
                    lk.layers.workspace.commands.execute(commandsView.currentItem.command)
                    root.close()
                }
                event.accepted = true
            }
            Keys.onEscapePressed: {
                root.close()
                event.accepted = true
            }
        }
    }

    ScrollView{
        id: scrollView
        z: 300
        width: parent.width
        height: commandsView.count * commandsView.delegateHeight > parent.height - commandsMenuInputBox.height ?
                    parent.height - commandsMenuInputBox.height :
                    commandsView.count * commandsView.delegateHeight
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: commandsMenuInputBox.height

        ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        ScrollBar.vertical.contentItem: Input.ScrollbarHandle{
            color: "#1f2227"
            visible: scrollView.contentHeight > scrollView.height
        }
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOn
        ScrollBar.horizontal.contentItem: Input.ScrollbarHandle{
            color: "#1f2227"
            visible: scrollView.contentWidth > scrollView.width
        }

        ListView{
            id: commandsView
            model : lk.layers.workspace.commands.model
            width: parent.width
            height: parent.height
            clip: true
            currentIndex : 0
            onCountChanged: currentIndex = 0

            boundsBehavior : Flickable.StopAtBounds
            highlightMoveDuration: 100

            property int delegateHeight : 20

            function highlightNext(){
                if ( commandsView.currentIndex + 1 <  commandsView.count ){
                    commandsView.currentIndex++;
                } else {
                    commandsView.currentIndex = 0;
                }
            }

            function highlightPrev(){
                if ( commandsView.currentIndex > 0 ){
                    commandsView.currentIndex--;
                } else {
                    commandsView.currentIndex = commandsView.count - 1;
                }
            }

            function highlightNextPage(){
                var noItems = Math.floor(commandsView.height / commandsView.delegateHeight)
                if ( commandsView.currentIndex + noItems < commandsView.count ){
                    commandsView.currentIndex += noItems;
                } else {
                    commandsView.currentIndex = commandsView.count - 1;
                }
            }
            function highlightPrevPage(){
                var noItems = Math.floor(commandsView.height / commandsView.delegateHeight)
                if ( commandsView.currentIndex - noItems >= 0 ){
                    commandsView.currentIndex -= noItems;
                } else {
                    commandsView.currentIndex = 0;
                }
            }

            delegate: Rectangle{

                property string command: model.command

                z: 400
                color: ListView.isCurrentItem ? "#292d34" : "#05080a"
                width: root.width
                height: commandsView.delegateHeight
                Text{
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    text: model.description
                    color: "#ebebeb"

                    font.family: "Open Sans, sans-serif"
                    font.pixelSize: 12
                    font.weight: Font.Normal
                }

                Text{
                    anchors.right: parent.right
                    anchors.rightMargin: 20

                    text: model.shortcuts
                    color: "#aaa5a5"

                    font.family: "Open Sans, sans-serif"
                    font.pixelSize: 12
                    font.weight: Font.Normal
                }

                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        lk.layers.workspace.commands.execute(model.command)
                        root.close()
                    }
                }
            }
        }
    }

    Rectangle{
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: commandsMenuInputBox.height
        visible: !commandsView.currentItem
        z: 400
        color: "#0a131a"
        width: root.width
        height: commandsView.delegateHeight
        Text{
            anchors.left: parent.left
            anchors.leftMargin: 20
            text: "No available commands"
            color: "#ebebeb"

            font.family: "Open Sans, sans-serif"
            font.pixelSize: 12
            font.weight: Font.Normal
            font.italic: true
        }

    }
}
