/*
    Copyright (c) 2016 Christian Mollekopf <mollekopf@kolabsys.com>

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
#include "messageparser.h"

#include "stringhtmlwriter.h"
#include "objecttreesource.h"
#include "csshelper.h"

#include <QFile>
#include <QImage>
#include <QDebug>
#include <QTime>
#include <MessageViewer/ObjectTreeParser>

MessageParser::MessageParser(QObject *parent)
    : QObject(parent)
{

}

QString MessageParser::html() const
{
    return mHtml;
}

QVariant MessageParser::message() const
{
    return QVariant();
}

void MessageParser::setMessage(const QVariant &message)
{
    QTime time;
    time.start();
    const auto mailData = KMime::CRLFtoLF(message.toByteArray());
    KMime::Message::Ptr msg(new KMime::Message);
    msg->setContent(mailData);
    msg->parse();
    qWarning() << "parsed: " << time.elapsed();

    // render the mail
    StringHtmlWriter htmlWriter;
    QImage paintDevice;
    CSSHelper cssHelper(&paintDevice);
    //temporary files only have the lifetime of the nodehelper, so we keep it around until the mail changes.
    mNodeHelper = std::make_shared<MessageViewer::NodeHelper>();
    ObjectTreeSource source(&htmlWriter, &cssHelper);
    MessageViewer::ObjectTreeParser otp(&source, mNodeHelper.get());

    htmlWriter.begin(QString());
    htmlWriter.queue(cssHelper.htmlHead(false));

    otp.parseObjectTree(msg.data());

    htmlWriter.queue(QStringLiteral("</body></html>"));
    htmlWriter.end();

    mHtml = htmlWriter.html();
    emit htmlChanged();
}
