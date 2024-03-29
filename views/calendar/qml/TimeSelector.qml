/*
 *  Copyright (C) 2017 Michael Bohlender, <bohlender@kolabsys.com>
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
import QtQuick.Layouts 1.2
import org.kube.framework 1.0 as Kube
import Qt.labs.calendar 1.0

import "dateutils.js" as DateUtils

Kube.ComboBox {
    id: root

    property int delta: 15
    property date dateTime
    property var notBefore: new Date(0)
    property var notBeforeRounded: DateUtils.roundToMinutes(notBefore, delta)


    onNotBeforeRoundedChanged: {
        root.setNotBefore(notBeforeRounded)
    }

    function generateTimes(start, delta) {
        var d = new Date(2000, 1, 1, start.getHours(), start.getMinutes(), start.getSeconds())
        var list = []
        while (d.getDate() == 1) {
            list.push(dateToString(d))
            d = DateUtils.addMinutesToDate(d, delta)
        }
        return list
    }

    function setNotBefore(notBefore) {
        availableTimes = generateTimes(DateUtils.sameDay(notBefore, root.dateTime) ? notBefore : new Date(2000, 1, 1, 0, 0, 0), root.delta)
        model = availableTimes
        currentIndex = findCurrentIndex(root.dateTime, root.delta)
        if (currentIndex >= 0) {
            setTimeFromIndex(currentIndex)
        } else {
            currentIndex = 0
            setTimeFromIndex(currentIndex)
        }
    }

    property var availableTimes: null

    function dateToString(date) {
        return date.toLocaleTimeString(Qt.locale(), "hh:mm")
    }

    function findCurrentIndex(date, delta) {
        var s = dateToString(DateUtils.roundToMinutes(date, delta))
        //Find does not reliably work if we just reset the model
        return availableTimes.indexOf(s)
        // return find(s)
    }

    function setTimeFromIndex(index) {
        var date = Date.fromLocaleTimeString(Qt.locale(), availableTimes[index], "hh:mm")
        //Intermediate variable is necessary for binding to be updated
        var newDate = root.dateTime
        newDate.setHours(date.getHours(), date.getMinutes())
        root.dateTime = newDate
    }

    Component.onCompleted: {
        setNotBefore(root.notBeforeRounded)
    }

    onActivated: {
        setTimeFromIndex(index)
    }

    model: availableTimes
}
