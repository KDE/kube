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
/*
* TODO
* * Only replace composer if necessary (on reply, edit draft, ...)
* * Shutdown procedure to save draft before destruction
* * Separate logging from view and and make accessible to log (initialize()) call?
*/
StackView {
    id: root
    property string currentViewName: currentItem ? currentItem.objectName : ""
    property variant extensionModel: null
    property bool dontFocus: false

    property var viewDict: new Object
    function getView(name, replaceView) {
        if (!replaceView && name in viewDict) {
            var item = viewDict[name]
            if (item) {
                return item
            }
        }
        var v = Qt.createComponent(extensionModel.findSource(name, "View.qml"))
        v = v.createObject(root)
        viewDict[name] = v
        return v;
    }

    onCurrentItemChanged: {
        if (currentItem && !dontFocus) {
            currentItem.forceActiveFocus()
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
        var view = getView(name, replace)
        var item = push(view, properties, StackView.Immediate)
        item.parent = root
        item.anchors.fill = root
        item.objectName = name
    }

    function showView(name, properties) {
        showOrReplaceView(name, properties, false)
    }

    function replaceView(name, properties) {
        showOrReplaceView(name, properties, true)
    }

    function closeView() {
        //The initial view remains
        if (kubeViews.depth > 1) {
            var item = kubeViews.pop(StackView.Immediate)
            viewDict[item.objectName] = null
            item.destroy()
        }
    }
}
