/*
   Copyright (c) 2015 Sandro Knauß <sknauss@kde.org>

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

#include "queuehtmlwriter.h"

#include "mimetreeparser_debug.h"

#include<QByteArray>
#include<QString>

using namespace MimeTreeParser;

QueueHtmlWriter::QueueHtmlWriter(HtmlWriter *base)
    : HtmlWriter()
    , mBase(base)
{
}

QueueHtmlWriter::~QueueHtmlWriter()
{
}

void QueueHtmlWriter::setBase(HtmlWriter *base)
{
    mBase = base;
}

void QueueHtmlWriter::begin(const QString &css)
{
    Command cmd;
    cmd.type = Command::Begin;
    cmd.s = css;
    mQueue.append(cmd);
}

void QueueHtmlWriter::end()
{
    Command cmd;
    cmd.type = Command::End;
    mQueue.append(cmd);
}

void QueueHtmlWriter::reset()
{
    Command cmd;
    cmd.type = Command::Reset;
    mQueue.append(cmd);
}

void QueueHtmlWriter::write(const QString &str)
{
    Command cmd;
    cmd.type = Command::Write;
    cmd.s = str;
    mQueue.append(cmd);
}

void QueueHtmlWriter::queue(const QString &str)
{
    Command cmd;
    cmd.type = Command::Queue;
    cmd.s = str;
    mQueue.append(cmd);
}

void QueueHtmlWriter::flush()
{
    Command cmd;
    cmd.type = Command::Flush;
    mQueue.append(cmd);
}

void QueueHtmlWriter::replay()
{
    foreach (const auto &entry, mQueue) {
        switch (entry.type) {
        case Command::Begin:
            mBase->begin(entry.s);
            break;
        case Command::End:
            mBase->end();
            break;
        case Command::Reset:
            mBase->reset();
            break;
        case Command::Write:
            mBase->write(entry.s);
            break;
        case Command::Queue:
            mBase->queue(entry.s);
            break;
        case Command::Flush:
            mBase->flush();
            break;
        case Command::EmbedPart:
            mBase->embedPart(entry.b, entry.s);
            break;
        case Command::ExtraHead:
            mBase->extraHead(entry.s);
            break;
        }
    }
}

void QueueHtmlWriter::embedPart(const QByteArray &contentId, const QString &url)
{
    Command cmd;
    cmd.type = Command::EmbedPart;
    cmd.s = url;
    cmd.b = contentId;
    mQueue.append(cmd);
}
void QueueHtmlWriter::extraHead(const QString &extra)
{
    Command cmd;
    cmd.type = Command::ExtraHead;
    cmd.s = extra;
    mQueue.append(cmd);
}

