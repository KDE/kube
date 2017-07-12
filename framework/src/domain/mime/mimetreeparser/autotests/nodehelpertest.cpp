/* Copyright 2015 Sandro Knauß <bugs@sandroknauss.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "nodehelpertest.h"

#include "nodehelper.h"

#include <qtest.h>

using namespace MimeTreeParser;

NodeHelperTest::NodeHelperTest()
    : QObject()
{

}

void NodeHelperTest::testFromAsString()
{
    const QString tlSender = QStringLiteral("Foo <foo@example.com>");
    const QString encSender = QStringLiteral("Bar <bar@example.com>");

    NodeHelper helper;

    // msg (KMime::Message)
    //  |- subNode
    //  |- encNode (KMime::Message)
    //      |- encSubNode
    //
    // subNode
    //  |- subExtra
    //
    // encSubNode
    //  |- encSubExtra

    KMime::Message msg;
    msg.from(true)->fromUnicodeString(tlSender, "UTF-8");
    auto node = msg.topLevel();
    auto subNode = new KMime::Content();
    // auto subExtra = new KMime::Content();

    // Encapsulated message
    KMime::Message *encMsg = new KMime::Message;
    encMsg->from(true)->fromUnicodeString(encSender, "UTF-8");
    auto encNode = encMsg->topLevel();
    auto encSubNode = new KMime::Content();
    auto encSubExtra = new KMime::Content();

    node->addContent(subNode);
    node->addContent(encMsg);
    encNode->addContent(encSubNode);

    // helper.attachExtraContent(subNode, subExtra);
    // helper.attachExtraContent(encSubNode, encSubExtra);

    QCOMPARE(helper.fromAsString(node), tlSender);
    QCOMPARE(helper.fromAsString(subNode), tlSender);
    // QCOMPARE(helper.fromAsString(subExtra), tlSender);
    QEXPECT_FAIL("", "Returning sender of encapsulated message is not yet implemented", Continue);
    QCOMPARE(helper.fromAsString(encNode), encSender);
    QEXPECT_FAIL("", "Returning sender of encapsulated message is not yet implemented", Continue);
    QCOMPARE(helper.fromAsString(encSubNode), encSender);
    QEXPECT_FAIL("", "Returning sender of encapsulated message is not yet implemented", Continue);
    QCOMPARE(helper.fromAsString(encSubExtra), encSender);
}

QTEST_GUILESS_MAIN(NodeHelperTest)

