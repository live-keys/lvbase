import QtQuick 2.3
import QtQuick.Controls 2.2
import workspace 1.0
import visual.input 1.0 as Input

Rectangle{
    id: root

    width: 180
    height: listView.contentHeight > maxHeight ? maxHeight : listView.contentHeight + 5

    property double maxHeight: 200
    property alias delegate: listView.delegate
    property alias model: listView.model

    color: style.backgroundColor
    opacity: style.opacity
    border.width: style.borderWidth
    border.color: style.borderColor

    property alias currentIndex: listView.currentIndex
    property alias currentItem: listView.currentItem

    property QtObject defaultStyle: Input.SelectableListViewStyle{}
    property QtObject style: defaultStyle

    signal triggered(int index)
    signal cancelled()

    function selectIndex(index){
        listView.currentIndex = index
    }

    function highlightNext(){
        if ( listView.currentIndex + 1 <  listView.count ){
            listView.currentIndex++;
        } else {
            listView.currentIndex = 0;
        }
    }
    function highlightPrev(){
        if ( listView.currentIndex > 0 ){
            listView.currentIndex--;
        } else {
            listView.currentIndex = listView.count - 1;
        }
    }

    function highlightNextPage(){
        var noItems = Math.floor(listView.height / 25)
        if ( listView.currentIndex + noItems < listView.count ){
            listView.currentIndex += noItems;
        } else {
            listView.currentIndex = listView.count - 1;
        }
    }
    function highlightPrevPage(){
        var noItems = Math.floor(listView.height / 25)
        if ( listView.currentIndex - noItems >= 0 ){
            listView.currentIndex -= noItems;
        } else {
            listView.currentIndex = 0;
        }
    }

    focus: false

    Keys.onPressed: {
        if ( event.key === Qt.Key_PageUp ){
            highlightPrevPage()
            event.accepted = true
        } else if ( event.key === Qt.Key_PageDown ){
            highlightNextPage()
            event.accepted = true
        } else if ( event.key === Qt.Key_Down ){
            highlightNext()
            event.accepted = true
        } else if ( event.key === Qt.Key_Up ){
            highlightPrev()
            event.accepted = true
        } else if ( event.key === Qt.Key_Return || event.key === Qt.Key_Enter ){
            root.triggered(listView.currentIndex)
            event.accepted = true
        } else if ( event.key === Qt.Key_Escape ){
            root.cancelled()
            event.accepted = true
        }
    }

    ScrollView{
        id: scrollView
        anchors.top : parent.top

        height : root.height
        width: root.width

        ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        ScrollBar.vertical.contentItem: Input.ScrollbarHandle{
            color: root.style.scrollbarColor
            visible: scrollView.contentHeight > scrollView.height
        }
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOn
        ScrollBar.horizontal.contentItem: Input.ScrollbarHandle{
            color: root.style.scrollbarColor
            visible: scrollView.contentWidth > scrollView.width
        }

        ListView{
            id : listView
            anchors.fill: parent
            anchors.rightMargin: 2
            anchors.bottomMargin: 5
            anchors.topMargin: 0
            visible: true
            opacity: root.opacity

            currentIndex: 0
            onCountChanged: currentIndex = 0

            boundsBehavior : Flickable.StopAtBounds
            highlightMoveDuration: 100
        }
    }
}

