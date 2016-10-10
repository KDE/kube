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

ListModel {

    ListElement {
        type: "encrypted"
        securityLevel: "GREEN"
        content: [
        ListElement {
            type: "plaintext"
            textContent: "Moin, \n find the forwarded mail below. \n \n - M"
            embeded: false
        },
        ListElement {
            type: "embeded"
            sender: "Senderson"
            date: "05/05/2055"
            content: [
            ListElement{
                type: "plaintext"
                textContent: "sender mc senderson is a sender. sender mc senderson is a sender. sender mc senderson is a  mc senderson is a sender sender mc senderson is a sender sender mc senderson is a sender sender mc senderson is a sender sender mc senderson is a sender sender mc   a sender sender mc  is a sender sender mc senderson is a sendersender mc senderson is a sender"
                embeded: true
            }]
        }
        ]
    }
    ListElement {
        type: "plaintext"
        textContent: "footer mc footerson"
        embeded: false
    }
}
