/*
 * Copyright (C) 2016 Michael Bohlender <michael.bohlender@kdemail.net>
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
        date: "Today 19:21"
        unread: true
    }
    ListElement {
        sender: "benni@bandana.org"
        senderName: "Ben Bandana"
        subject: "Check this out"
        date: "Today 16:01"
        unread: true
    }
    ListElement {
        sender: "alice@mail.com"
        senderName: "Alice Cheng"
        subject: "Re: We need more food"
        date: "Today 12:55"
        unread: false
    }
    ListElement {
        sender: "bob@bandana.org"
        senderName: "Bob Ross"
        subject: "KDE Rocks"
        date: "Today 07:32"
        unread: true
    }
    ListElement {
        sender: "tiny@mail.com"
        senderName: "Tiny"
        subject: "Huge success!!"
        date: "Today 00:11"
        unread: false
    }
    ListElement {
        sender: "bob@bandana.org"
        senderName: "Bob Ross"
        subject: "KDE Rocks"
        date: "Yesterday 20:54"
        unread: false
    }
    ListElement {
        sender: "Laura@mail.com"
        senderName: "Laura B"
        subject: ":)"
        date: "Monday 12:37"
        unread: false
    }
    ListElement {
        sender: "Andreas@stars.org"
        senderName: "Andreas Rockstar"
        subject: "KDE Rocks"
        date: "Monday 12:37"
        unread: false
    }
    ListElement {
        sender: "alice@mail.com"
        senderName: "Alice Cheng"
        subject: "Re: We need more food"
        date: "Monday 12:37"
    }
    ListElement {
        sender: "bob@bandana.org"
        senderName: "Bob Ross"
        subject: "KDE Rocks"
        date: "Monday 12:37"
    }
    ListElement {
        sender: "mighty@mail.com"
        senderName: "Mighty Monkey"
        subject: "I feel weak without my bananas"
        date: "2 hours ago"
    }
    ListElement {
        sender: "benni@bandana.org"
        senderName: "Ben Bandana"
        subject: "Check this out"
        date: "8 hours ago"
    }
    ListElement {
        sender: "alice@mail.com"
        senderName: "Alice Cheng"
        subject: "Re: We need more food"
        date: "2 hours ago"
    }
    ListElement {
        sender: "bob@bandana.org"
        senderName: "Bob Ross"
        subject: "KDE Rocks"
        date: "8 hours ago"
    }
    ListElement {
        sender: "tiny@mail.com"
        senderName: "Tiny"
        subject: "Huge success!!"
        date: "2 hours ago"
    }
    ListElement {
        sender: "bob@bandana.org"
        senderName: "Bob Ross"
        subject: "KDE Rocks"
        date: "8 hours ago"
    }
    ListElement {
        sender: "Laura@mail.com"
        senderName: "Laura B"
        subject: ":)"
        date: "2 hours ago"
    }
    ListElement {
        sender: "Andreas@stars.org"
        senderName: "Andreas Rockstar"
        subject: "KDE Rocks"
        date: "8 hours ago"
    }
    ListElement {
        sender: "alice@mail.com"
        senderName: "Alice Cheng"
        subject: "Re: We need more food"
        date: "2 hours ago"
    }
    ListElement {
        sender: "bob@bandana.org"
        senderName: "Bob Ross"
        subject: "Board Task: Write draft email to people with KDE accounts commiting to Qt repositories"
        date: "8 hours ago"
    }
}
