/*
 *  Copyright (C) 2018 Christian Mollekopf, <mollekopf@kolabsys.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


import QtQuick 2.7
import QtQuick.Controls 2.0

StackView {
    id: root
    property string currentViewName: currentItem ? currentItem.objectName : ""
    property variant extensionModel: null
    property bool dontFocus: false

    /*
     * We maintain the view's lifetimes separately from the StackView in the viewDict.
     */
    property var viewDict: new Object

    onCurrentItemChanged: {
        if (currentItem && !dontFocus) {
            currentItem.forceActiveFocus()
        }
    }

    function pushView(view, properties, name) {
        var item = push(view, properties, StackView.Immediate)
        item.parent = root
        item.anchors.fill = root
        item.objectName = name
    }

    function createComponent(name) {
        //Creating a new view
        var source = extensionModel.findSource(name, "View.qml");
        //On windows it will be async anyways, so just always create it async
        return Qt.createComponent(source, Qt.Asynchronous)
    }

    function createView(name, properties, push) {
        var component = createComponent(name)
        function finishCreation() {
            if (component.status == Component.Ready) {
                var view = component.createObject(root, properties ? properties : {});
                viewDict[name] = view
                if (push) {
                    pushView(view, properties, name)
                }
            } else {
                console.error("Error while loading the component: ", source, "\nError: ", component.errorString())
            }
        }

        if (component.status == Component.Loading) {
            component.statusChanged.connect(finishCreation);
        } else {
            finishCreation();
        }
    }

    function showOrReplaceView(name, properties, replace) {
        if (currentItem && currentItem.objectName == name) {
            return
        }
        if (root.depth > 0) {
            root.pop(StackView.Immediate)
        }
        //Avoid trying to push the same item again, if its on top after pop
        if (currentItem && currentItem.objectName == name) {
            return
        }

        var item = name in viewDict ? viewDict[name] : null
        if (item) {
            if (replace) {
                if (item.aborted) {
                    //Call the aborted hook on the view
                    item.aborted()
                }
                //Fall through to create new component to replace this one
            } else {
                pushView(item, properties, name)
                return
            }
        }

        createView(name, properties, true)
    }

    function showView(name, properties) {
        showOrReplaceView(name, properties, false)
    }

    function prepareViewInBackground(name, properties) {
        createView(name, properties, false)
    }

    function replaceView(name, properties) {
        showOrReplaceView(name, properties, true)
    }

    function closeView() {
        //The initial view remains
        if (root.depth > 1) {
            var item = root.pop(StackView.Immediate)
            viewDict[item.objectName] = null
            item.destroy()
        }
    }
}
