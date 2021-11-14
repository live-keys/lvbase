import QtQuick 2.3
import editor 1.0
import workspace.nodeeditor 1.0

WorkspaceExtension{
    id: root
    objectName: "workspace"

    globals: QtObject{
        property ObjectGraphControls objectGraphControls: ObjectGraphControls{}
    }

    function addEditorPane(){
        var pane = lk.layers.workspace.panes.createPane('editor')
        lk.layers.workspace.panes.insertPane(pane, {orientation: Qt.Horizontal} )
    }

    function toggleMainRunViewPane(){
        var panes = lk.layers.workspace.panes.findPanesByType('viewer')
        if ( panes.length > 0 ){
            for ( var i = 0; i < panes.length; ++i ){
                lk.layers.workspace.removePane(panes[i])
            }
        }

        var pane = lk.layers.workspace.panes.createPane('viewer')
        lk.layers.workspace.panes.insertPane(pane)
    }

    property Component editorPane: Component{
        EditorPane{
            id: editorComponent
            panes: lk.layers.workspace.panes
            Component.onCompleted: {
                editorComponent.editor.forceFocus()
            }
        }
    }
    property Component objectPalettePane: Component{
        ObjectPalettePane{
            id: objectPaletteComponent
            panes: root.panes
        }
    }
    property Component palettePane: Component{
        PalettePane{
            id: paletteComponent
            panes: root.panes
        }
    }

    fileFormats: [
        { extension: 'qml', type: 'qml' },
        { extension: 'js', type: 'js' },
        { extension: 'json', type: 'json' },
        { extension: 'csv', type: 'csv' },
        { extension: 'txt', type: 'txt' }
    ]

    commands: {
        'minimize' : [lk.layers.window.handle.minimize, "Minimize"],
        'addEditor' : [root.addEditorPane, "Add editor pane"],
        'toggleFullScreen': [lk.layers.window.handle.toggleFullScreen, "Toggle Fullscreen"],
        "nodeDeleteActive": [globals.objectGraphControls.removeActiveItem, "Deletes the activated item in the node editor."],
        "nodeEditMode" : [globals.objectGraphControls.nodeEditMode, "Switch to node editing mode."],
        "toggleMainRunView" : [root.toggleMainRunViewPane, "MainRunView: Toggle visibility."]
    }

    // quick node editing

    keyBindings: {
        "backspace": {command: "workspace.nodeDeleteActive", whenPane: "editor", whenItem:"objectGraph" },
        "delete": {command: "workspace.nodeDeleteActive", whenPane: "editor", whenItem:"objectGraph" },
        "alt+n": {command: "workspace.nodeEditMode" }
    }

    panes: {
        return {
            "editor" : {
                create: function(p, s){
                    var pane = root.editorPane.createObject(p)
                    if ( s )
                        pane.paneInitialize(s)
                    return pane
                },
                single: false
            },
            "objectPalette" : {
                create: function(p, s){
                    var pane = root.objectPalettePane.createObject(p)
                    if ( s )
                        pane.paneInitialize(s)
                    return pane
                },
                single: false
            },
            "palette" : {
                create: function(p, s){
                    var pane = root.palettePane.createObject(p)
                    if ( s )
                        pane.paneInitialize(s)
                    return pane
                },
                single: false
            }
        }
    }

    menuInterceptors: [
        {
            whenPane: 'editor',
            whenItem: 'editorType',
            intercept: function(pane, item){
                return [
                    {
                        name : "Cut",
                        action : item.textEdit.cut,
                        shortcut: 'ctrl+x',
                        enabled: item.textEdit.selectionStart !== item.textEdit.selectionEnd
                    }, {
                        name : "Copy",
                        action : item.textEdit.copy,
                        shortcut: 'ctrl+c',
                        enabled: item.textEdit.selectionStart !== item.textEdit.selectionEnd
                    }, {
                        name : "Paste",
                        action : item.textEdit.paste,
                        enabled : item.textEdit.canPaste,
                        shortcut: 'ctrl+v'
                    }
                ]
            }
        },
        {
            whenPane: 'log',
            whenItem: 'logList',
            intercept: function(pane, item){
                return [
                    {
                        name : "Copy",
                        action : pane.copySelection,
                        shortcut: 'ctrl+c',
                        enabled: pane.selection.length
                    }, {
                        name : "Open Location",
                        action : pane.openSelectionLocation,
                        shortcut: 'ctrl+l',
                        enabled: pane.selection.length
                    }, {
                        name : "Open Location Externally",
                        action : pane.openSelectionLocationExternally,
                        enabled : pane.selection.length
                    }, {
                        name : "Clear Selection",
                        action : pane.clearSelection,
                        enabled : pane.selection.length
                    }
                ]
            }
        },
        {
            whenPane: 'projectFileSystem',
            intercept: function(pane, item){
                if ( item.entry().isFile ){
                    return [
                        {
                            name : "Edit File",
                            action : item.openFile
                        }, {
                            name : "Monitor file",
                            action : item.monitorFile
                        }, {
                            name : "Set As Active",
                            action : item.setActive
                        }, {
                            name : "Add As Runnable",
                            action : item.addRunnable
                        }, {
                            name : "Rename",
                            action : function(){
                                item.editMode = true
                                item.focusText()
                            }
                        }, {
                            name : "Delete",
                            action : function(){
                                lk.layers.workspace.wizards.removePath(item.entry().path)
                            }
                        }
                    ]
                } else if ( item.path() === lk.layers.workspace.project.rootPath ){
                    return [
                        {
                            name : "Show in Explorer",
                            action : item.openExternally
                        }, {
                            name : "Close Project",
                            action : function(){
                                lk.layers.workspace.commands.execute('window.workspace.project.close')
                            }
                        }, {
                            name : "New Qml Document",
                            action : function(){
                                var newDocument = lk.layers.workspace.project.createDocument({fileSystem: true, type: 'text', format: 'qml'})
                                lk.layers.workspace.environment.fileSystem.openDocumentInEditor(newDocument)
                            }
                        }, {
                            name : "New Runnable",
                            action : function(){
                                var newDocument = lk.layers.workspace.project.createDocument({fileSystem: true, type: 'text', format: 'qml'})
                                lk.layers.workspace.environment.fileSystem.openDocumentInEditor(newDocument)
                                lk.layers.workspace.project.openRunnable(newDocument.path, [newDocument.path])
                            }
                        }, {
                            name : "Add File",
                            action : function(){
                                lk.layers.workspace.wizards.addFile(item.entry().path, {}, function(f){
                                    lk.layers.workspace.environment.openFile(f.path, Document.Edit)
                                })
                            }
                        }, {
                            name : "Add Directory",
                            action : function(){
                                lk.layers.workspace.wizards.addDirectory(item.entry().path, {}, function(d){})
                            }
                        }
                    ]
                } else {
                    return [
                        {
                            name : "Show in Explorer",
                            action : item.openExternally
                        },
                        {
                            name : "Rename",
                            action : function(){
                                item.editMode = true
                                item.focusText()
                            }
                        },
                        {
                            name : "Delete",
                            action : function(){
                                lk.layers.workspace.wizards.removePath(item.entry().path)
                            }
                        }, {
                            name : "Add File",
                            action : function(){
                                lk.layers.workspace.wizards.addFile(item.entry().path, function(f){
                                    lk.layers.workspace.environment.openFile(f.path, ProjectDocument.Edit)
                                })
                            }
                        }, {
                            name : "Add Directory",
                            action : function(){
                                lk.layers.workspace.wizards.addDirectory(item.entry().path, function(d){})
                            }
                        }
                    ]
                }
            }
        },
        {
            whenItem: 'timelineTrackTitle',
            intercept: function(pane, item){
                var track = item.timelineArea.timeline.trackList.trackAt(item.trackIndex)

                var optionsConverted = []

                for ( var i = 0; i < item.menuOptions.length; ++i ){
                    var option = item.menuOptions[i]
                    optionsConverted.push({
                        name : option.name,
                        action : option.action.bind(item, track),
                        enabled: option.enabled
                    })
                }

                optionsConverted.push({
                    name : "Remove Track",
                    action : function(){ item.timelineArea.timeline.removeTrack(item.trackIndex) },
                    enabled: true
                })

                return optionsConverted
            }
        },
        {
            whenItem: 'timelineOptions',
            intercept: function(pane, item){
                var optionsConverted = []

                var menuOptions = item.timelineArea.headerContextMenu

                for ( var i = 0; i < menuOptions.length; ++i ){
                    var option = menuOptions[i]
                    optionsConverted.push({
                        name : option.name,
                        action : option.action,
                        enabled: option.enabled
                    })
                }

                return optionsConverted
            }
        },
    ]

}
