/*
  Copyright (C) 2017 Michael Bohlender, <bohlender@kolabsys.com>
  Copyright (C) 2017 Christian Mollekopf, <mollekopf@kolabsys.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

pragma Singleton

import QtQuick 2.7
import org.kube.framework 1.0 as Kube

Item {
    //Ensure the gridUnit is divisble by 4 without remainder, otherwise we'll end up with alignment issues
    property int gridUnit: Math.ceil(fontMetrics.height / 4) * 4
    property int smallSpacing: gridUnit/4
    property int largeSpacing: gridUnit

    property int defaultFontSize: fontMetrics.font.pointSize
    property int smallFontSize: fontMetrics.font.pointSize * 0.9
    property int tinyFontSize: fontMetrics.font.pointSize * 0.8

    property variant fontMetrics: TextMetrics {
        text: "M"
        font.family: Kube.Font.fontFamily
    }
}

