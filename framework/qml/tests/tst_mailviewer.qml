/*
 *   Copyright 2017 Christian Mollekopf <mollekopf@kolabsystems.com>
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
import "../mailpartview" as MPV


TestCase {
    id: testCase
    width: 400
    height: 400
    name: "MailViewer"
    when: windowShown
    visible: true

    Kube.File {
        id: file
        path: "/src/kube/framework/qml/tests/kraken.eml"
    }

    Component {
        id: viewerComponent
        Kube.MailViewer {
            anchors.fill: parent
            visible: true
            unread: false
            trash: false
            draft: false
            sent: false
            incomplete: false
            current: true
            autoLoadImages: true
        }
    }

    function test_1htmlMail() {
        var viewer = createTemporaryObject(viewerComponent, testCase, {message: file.data})

        var htmlButton = findChild(viewer, "htmlButton");
        verify(htmlButton)
        tryVerify(function(){ return htmlButton.visible })
        htmlButton.clicked()

        var htmlView = findChild(viewer, "htmlView");
        verify(htmlView)
        tryVerify(function(){ return htmlView.loadProgress == 100})

        tryVerify(function(){ return (1300 > htmlView.width && htmlView.width > 1200)})
        tryVerify(function(){ return (2700 > htmlView.height && htmlView.height > 2400)})
    }

    Component {
        id: htmlComponent
        MPV.HtmlPart {
            anchors.fill: parent
        }
    }

    Component {
        id: textComponent
        MPV.TextPart {
            anchors.fill: parent
        }
    }

    function test_2htmlMail() {
        var data =
"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">
<html><head><title></title><style>
body {
  overflow:hidden;
  font-family: \"Noto Sans\" ! important;
  color: #31363b ! important;
  background-color: #fcfcfc ! important
}
blockquote {
  border-left: 2px solid #bdc3c7 ! important;
}
</style></head>
<body>
<table><tr><td style=\"\">john added a comment.
</td></tr></table><br /><div><div><blockquote style=\"border-left: 3px solid #a7b5bf; color: #464c5c; font-style: italic; margin: 4px 0 12px 0; padding: 4px 12px; background-color: #f8f9fc;\"><blockquote style=\"border-left: 3px solid #a7b5bf; color: #464c5c; font-style: italic; margin: 4px 0 12px 0; padding: 4px 12px; background-color: #f8f9fc;\"><p>This is some text that is quoted.</p></blockquote></blockquote>

<p>I&#039;m afraid due to technical limitations of how connections from us are done that&#039;s not trivial.</p>

<p>I would still suggest scheduling a meeting</p></div></div><br /><div><strong>TASK DETAIL</strong><div><a href=\"https://test.com/T336176\">https://test.com/T336176</a></div></div><br /><div><strong>EMAIL PREFERENCES</strong><div><a href=\"https://test.com/settings/panel/emailpreferences/\">https://test.com/settings/panel/emailpreferences/</a></div></div><br /><div><strong>To: </strong>mollekopf, john<br /><strong>Cc: </strong>john, test, Support<br /></div></body></html>"

        var part = createTemporaryObject(htmlComponent, testCase, {content: data})
        var htmlView = findChild(part, "htmlView");
        verify(htmlView)
        tryVerify(function(){ return htmlView.loadProgress == 100})

        tryVerify(function(){ console.warn(htmlView.width, htmlView.height); return (400 > htmlView.width && htmlView.width > 300)})
        tryVerify(function(){ return (1300 > htmlView.height && htmlView.height > 1200)})

        var part = createTemporaryObject(textComponent, testCase, {content: data})
        var textView = findChild(part, "textView");
        verify(textView)
        tryVerify(function(){ console.warn(textView.width, textView.height); return (500 > textView.width && textView.width > 300)})
        tryVerify(function(){ console.warn(textView.width, textView.height); return (500 > textView.height && textView.height > 400)})
    }
