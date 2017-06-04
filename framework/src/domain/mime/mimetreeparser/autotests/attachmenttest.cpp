/*
  Copyright (c) 2015 Volker Krause <vkrause@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/
#include "objecttreeparser.h"
#include "util.h"

#include "setupenv.h"

#include <qtest.h>
#include <QDebug>

using namespace MimeTreeParser;

class AttachmentTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testEncryptedAttachment_data();
    void testEncryptedAttachment();
};

QTEST_MAIN(AttachmentTest)

void AttachmentTest::initTestCase()
{
    MimeTreeParser::Test::setupEnv();
}

void AttachmentTest::testEncryptedAttachment_data()
{
    QTest::addColumn<QString>("mbox");
    QTest::newRow("encrypted") << "openpgp-encrypted-two-attachments.mbox";
    QTest::newRow("signed") << "openpgp-signed-two-attachments.mbox";
    QTest::newRow("signed+encrypted") << "openpgp-signed-encrypted-two-attachments.mbox";
    QTest::newRow("encrypted+partial signed") << "openpgp-encrypted-partially-signed-attachments.mbox";
}

void AttachmentTest::testEncryptedAttachment()
{
    QFETCH(QString, mbox);
    auto msg = readAndParseMail(mbox);
    NodeHelper nodeHelper;
    ObjectTreeParser otp(&nodeHelper);
    otp.parseObjectTree(msg.data());
    otp.decryptParts();
    otp.print();

    auto attachmentParts = otp.collectAttachmentParts();
    QCOMPARE(attachmentParts.size(), 2);
}

#include "attachmenttest.moc"
