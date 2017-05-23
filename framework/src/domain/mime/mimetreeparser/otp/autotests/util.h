/*
  Copyright (c) 2010 Thomas McGuire <thomas.mcguire@kdab.com>

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
#include "htmlwriter.h"

#include <KMime/Message>

class TestHtmlWriter : public MimeTreeParser::HtmlWriter
{
public:
    explicit TestHtmlWriter() {}
    virtual ~TestHtmlWriter() {}

    void begin(const QString &) Q_DECL_OVERRIDE {}
    void write(const QString &) Q_DECL_OVERRIDE {}
    void end() Q_DECL_OVERRIDE {}
    void reset() Q_DECL_OVERRIDE {}
    void queue(const QString &str) Q_DECL_OVERRIDE {
        html.append(str);
    }
    void flush() Q_DECL_OVERRIDE {}
    void embedPart(const QByteArray &, const QString &) Q_DECL_OVERRIDE {}
    void extraHead(const QString &) Q_DECL_OVERRIDE {}

    QString html;
};

KMime::Message::Ptr readAndParseMail(const QString &mailFile);
