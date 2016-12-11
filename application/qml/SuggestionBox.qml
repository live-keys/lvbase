import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import Cv 1.0

Rectangle{
    id: root

    width: 250
    height: 280
    color: "#111"
    opacity: 0.95

    property alias suggestionCount : pluginList.count
    property alias model : pluginList.model
    property string fontFamily: 'Courier New, Courier'
    property int fontSize: 11
    property int smallFontSize: 9

    function getCompletion(){
        if ( pluginList.currentItem ){
            return pluginList.currentItem.completion
        }
        return ""
    }

    function highlightNext(){
        if ( pluginList.currentIndex + 1 <  pluginList.count ){
            pluginList.currentIndex++;
        } else {
            pluginList.currentIndex = 0;
        }
    }
    function highlightPrev(){
        if ( pluginList.currentIndex > 0 ){
            pluginList.currentIndex--;
        } else {
            pluginList.currentIndex = pluginList.count - 1;
        }
    }

    function highlightNextPage(){
        var noItems = Math.floor(pluginList.height / 25)
        if ( pluginList.currentIndex + noItems < pluginList.count ){
            pluginList.currentIndex += noItems;
        } else {
            pluginList.currentIndex = pluginList.count - 1;
        }
    }
    function highlightPrevPage(){
        var noItems = Math.floor(pluginList.height / 25)
        if ( pluginList.currentIndex - noItems >= 0 ){
            pluginList.currentIndex -= noItems;
        } else {
            pluginList.currentIndex = 0;
        }
    }

    ScrollView{
        anchors.top : parent.top

        height : root.height
        width: root.width

        style: ScrollViewStyle {
            transientScrollBars: false
            handle: Item {
                implicitWidth: 10
                implicitHeight: 10
                Rectangle {
                    color: "#0b1f2e"
                    anchors.fill: parent
                }
            }
            scrollBarBackground: Item{
                implicitWidth: 10
                implicitHeight: 10
                Rectangle{
                    anchors.fill: parent
                    color: "#091823"
                }
            }
            decrementControl: null
            incrementControl: null
            frame: Rectangle{color: "transparent"}
            corner: Rectangle{color: "#091823"}
        }

        ListView{
            id : pluginList
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

            delegate: Component{

                Rectangle{
                    property string completion: model.completion

                    width : pluginList.width
                    height : 25
                    color : ListView.isCurrentItem ? "#444" : "transparent"
                    Text{
                        id: label
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.verticalCenter: parent.verticalCenter

                        font.family: root.fontFamily
                        font.pixelSize: root.fontSize

                        color: "#fafafa"
                        text: model.label
                    }
                    Text{
                        id: category
                        anchors.right: parent.right
                        anchors.rightMargin: 10
                        anchors.verticalCenter: parent.verticalCenter

                        font.family: root.fontFamily
                        font.pixelSize: root.smallFontSize

                        color: "#666688"
                        text: model.info + "|" + model.category
                    }

                    MouseArea{
                        anchors.fill: parent
                    }
                }
            }

        }
    }
}