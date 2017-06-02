/*
  Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>

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

#ifndef MESSAGECORE_TESTS_UTIL_H
#define MESSAGECORE_TESTS_UTIL_H

#include <gpgme++/key.h>
#include <bodypartformatter.h>
#include <bodypartformatterbasefactory.h>
#include <objecttreesource.h>

namespace MimeTreeParser
{

namespace Test
{

/**
* setup a environment variables for tests:
* * set LC_ALL to C
* * set KDEHOME
*/
void setupEnv();

// We can't use EmptySource, since we need to control some emelnets of the source for tests to also test
// loadExternal and htmlMail.
class TestObjectTreeSource : public MimeTreeParser::Interface::ObjectTreeSource
{
public:
    TestObjectTreeSource()
        : mPreferredMode(Util::Html)
    {
    }

    bool autoImportKeys() const Q_DECL_OVERRIDE
    {
        return true;
    }

    const BodyPartFormatterBaseFactory *bodyPartFormatterFactory() Q_DECL_OVERRIDE {
        return &mBodyPartFormatterBaseFactory;
    }

    void setHtmlMode(MimeTreeParser::Util::HtmlMode mode, const QList<MimeTreeParser::Util::HtmlMode> &availableModes) Q_DECL_OVERRIDE {
        Q_UNUSED(mode);
        Q_UNUSED(availableModes);
    }

    MimeTreeParser::Util::HtmlMode preferredMode() const Q_DECL_OVERRIDE
    {
        return mPreferredMode;
    }

    void setPreferredMode(MimeTreeParser::Util::HtmlMode mode)
    {
        mPreferredMode = mode;
    }

    const QTextCodec *overrideCodec() Q_DECL_OVERRIDE {
        return nullptr;
    }

    QObject *sourceObject() Q_DECL_OVERRIDE {
        return nullptr;
    }
private:
    BodyPartFormatterBaseFactory mBodyPartFormatterBaseFactory;
    MimeTreeParser::Util::HtmlMode mPreferredMode;
};

}

}

#endif
