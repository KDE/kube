/*
 * Copyright (C) 2015 Michael Bohlender <michael.bohlender@kdemail.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.4

ListModel {
    ListElement {
        sender: "mighty@mail.com"
        senderName: "Mighty Monkey"
        subject: "I feel weak without my bananas"
        date: "19:21"
        unread: true
    }
    ListElement {
        sender: "benni@bandana.org"
        senderName: "Ben Bandana"
        subject: "Check this out"
        date: "16:01"
        unread: true
    }
    ListElement {
        sender: "alice@mail.com"
        senderName: "Alice Cheng"
        subject: "Re: We need more food"
        date: "12:55"
        unread: false
    }
    ListElement {
        sender: "bob@bandana.org"
        senderName: "Bob Ross"
        subject: "KDE Rocks"
        date: "Sat"
        unread: true
    }
    ListElement {
        sender: "tiny@mail.com"
        senderName: "Tiny"
        subject: "Huge success!!"
        date: "Sat"
        unread: false
    }
    ListElement {
        sender: "bob@bandana.org"
        senderName: "Bob Ross"
        subject: "KDE Rocks"
        date: "Fr"
        unread: false
    }
    ListElement {
        sender: "Laura@mail.com"
        senderName: "Laura B"
        subject: ":)"
        date: "Thu"
        unread: false
    }
    ListElement {
        sender: "Andreas@stars.org"
        senderName: "Andreas Rockstar"
        subject: "KDE Rocks"
        date: "Wed"
        unread: false
    }
    ListElement {
        sender: "alice@mail.com"
        senderName: "Alice Cheng"
        subject: "Re: We need more food"
        date: "Wed"
    }
    ListElement {
        sender: "bob@bandana.org"
        senderName: "Bob Ross"
        subject: "KDE Rocks"
        date: "Mon"
    }
    ListElement {
        sender: "mighty@mail.com"
        senderName: "Mighty Monkey"
        subject: "I feel weak without my bananas"
        date: "Nov 20"
    }
    ListElement {
        sender: "benni@bandana.org"
        senderName: "Ben Bandana"
        subject: "Check this out"
        date: "Nov 15"
    }
    ListElement {
        sender: "alice@mail.com"
        senderName: "Alice Cheng"
        subject: "Re: We need more food"
        date: "Sep 29"
    }
    ListElement {
        sender: "bob@bandana.org"
        senderName: "Bob Ross"
        subject: "KDE Rocks"
        date: "Sep 14"
    }
    ListElement {
        sender: "tiny@mail.com"
        senderName: "Tiny"
        subject: "Huge success!!"
        date: "Sep 14"
    }
    ListElement {
        sender: "bob@bandana.org"
        senderName: "Bob Ross"
        subject: "KDE Rocks"
        date: "Sep 5"
    }
    ListElement {
        sender: "Laura@mail.com"
        senderName: "Laura B"
        subject: ":)"
        date: "Sep 4"
    }
    ListElement {
        sender: "Andreas@stars.org"
        senderName: "Andreas Rockstar"
        subject: "KDE Rocks"
        date: "May 25"
    }
    ListElement {
        sender: "alice@mail.com"
        senderName: "Alice Cheng"
        subject: "Re: We need more food"
        date: "May 3"
    }
    ListElement {
        sender: "bob@bandana.org"
        senderName: "Bob Ross"
        subject: "Board Task: Write draft email to people with KDE accounts commiting to Qt repositories"
        date: "Dec 2014"
    }
}