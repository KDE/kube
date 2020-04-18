/*
 *   Copyright 2017 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.7
import QtTest 1.0
import org.kube.framework 1.0 as Kube
import org.kube.test 1.0
import "../qml"

ViewTestCase {
    id: viewmanagerTestcase
    width: 400
    height: 400
    name: "ViewManager"

    Component {
        id: viewManagerComponent
        ViewManager {
            id: viewManager
            anchors.fill: parent

            function createComponent(name) {
                return testViewComponent
            }

            Component {
                id: testViewComponent
                Rectangle {
                    property string test: "not initialized"
                }
            }
        }
    }


    function test_1testStack() {
        var viewManager = createTemporaryObject(viewManagerComponent, viewmanagerTestcase, {})

        viewManager.showView("view1", {test: "test1"})
        compare(viewManager.currentViewName, "view1")
        compare(viewManager.currentItem.test, "test1")
        compare(viewManager.depth, 1)

        viewManager.showView("view2", {test: "test2"})
        compare(viewManager.currentViewName, "view2")
        compare(viewManager.currentItem.test, "test2")
        compare(viewManager.depth, 2)

        viewManager.replaceView("view3", {test: "test3"})
        compare(viewManager.currentViewName, "view3")
        compare(viewManager.currentItem.test, "test3")
        compare(viewManager.depth, 2)

        //Switch back to the old view (properties are maintained)
        viewManager.showView("view2")
        compare(viewManager.currentViewName, "view2")
        compare(viewManager.currentItem.test, "test2")
        compare(viewManager.depth, 2)

        //And replace view3 (properties are not maintained)
        viewManager.replaceView("view3", {test: "test4"})
        compare(viewManager.currentViewName, "view3")
        compare(viewManager.currentItem.test, "test4")
        compare(viewManager.depth, 2)

        viewManager.closeView()
        compare(viewManager.currentViewName, "view1")
        compare(viewManager.currentItem.test, "test1")
        compare(viewManager.depth, 1)

        //Never close the last view
        viewManager.closeView()
        compare(viewManager.depth, 1)
    }

    function test_2testBackgroundView() {
        var viewManager = createTemporaryObject(viewManagerComponent, viewmanagerTestcase, {})

        viewManager.prepareViewInBackground("backgroundView", {test: "background"})

        viewManager.showView("view1", {test: "test1"})
        compare(viewManager.currentViewName, "view1")
        compare(viewManager.currentItem.test, "test1")
        compare(viewManager.depth, 1)

        viewManager.showView("backgroundView")
        compare(viewManager.currentViewName, "backgroundView")
        compare(viewManager.currentItem.test, "background")
        compare(viewManager.depth, 2)

        viewManager.closeView()
        compare(viewManager.currentViewName, "view1")
        compare(viewManager.currentItem.test, "test1")
        compare(viewManager.depth, 1)

        //Never close the last view
        viewManager.closeView()
        compare(viewManager.currentViewName, "view1")
        compare(viewManager.currentItem.test, "test1")
        compare(viewManager.depth, 1)
    }
}
