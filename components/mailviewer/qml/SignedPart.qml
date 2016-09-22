/*
  Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>

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

import QtQuick 2.4
import QtQuick.Controls 1.5

Item {

    height: mailPart.height + 20
    width: mailPart.width + 20

        BorderImage {

        anchors.fill: parent
        border { left: 40; top: 40; right: 40; bottom: 40 }
        horizontalTileMode: BorderImage.Round
        verticalTileMode: BorderImage.Round

        source: "securityborders" + model.securityLevel + ".png"
    }

    MailPart {
        id: mailPart

        anchors.centerIn: parent

    }
}

