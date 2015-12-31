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

#ifndef __MESSAGEVIEWER_QUEUEHTMLWRITER_H__
#define __MESSAGEVIEWER_QUEUEHTMLWRITER_H__

#include "messageviewer_export.h"
#include "interfaces/htmlwriter.h"

#include<QVector>
#include<QVariant>

class QString;
class QByteArray;

namespace MessageViewer
{
/**
\brief Cache HTML output and not write them directy.

This class is needed to make it possible to first process the mime tree and
afterwards render the HTML.

Please do not use this class - it is only added to make it possible to slowly
move ObjectTreeParser to a process fist / render later.

*/
struct Command {
    enum { Begin, End, Reset, Write, Queue, Flush, EmbedPart, ExtraHead } type;
    QString s;
    QByteArray b;
};

class QueueHtmlWriter : public HtmlWriter
{
public:
    explicit QueueHtmlWriter(MessageViewer::HtmlWriter *base);
    virtual ~QueueHtmlWriter();

    void begin(const QString &cssDefs) Q_DECL_OVERRIDE;
    void end() Q_DECL_OVERRIDE;
    void reset() Q_DECL_OVERRIDE;
    void write(const QString &str) Q_DECL_OVERRIDE;
    void queue(const QString &str) Q_DECL_OVERRIDE;
    void flush() Q_DECL_OVERRIDE;
    void embedPart(const QByteArray &contentId, const QString &url) Q_DECL_OVERRIDE;
    void extraHead(const QString &str) Q_DECL_OVERRIDE;

    void replay();

private:
    HtmlWriter *mBase;
    QVector<Command> mQueue;
};

} // namespace MessageViewer

#endif // __MESSAGEVIEWER_QUEUEHTMLWRITER_H__
