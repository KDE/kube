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
#include <attachmentstrategy.h>
#include <bodypartformatter.h>
#include <bodypartformatterbasefactory.h>
#include <messagepartrenderer.h>
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
    TestObjectTreeSource(MimeTreeParser::HtmlWriter *writer)
        : mWriter(writer)
        , mAttachmentStrategy(QStringLiteral("smart"))
        , mPreferredMode(Util::Html)
        , mHtmlLoadExternal(false)
        , mDecryptMessage(false)
    {
    }

    MimeTreeParser::HtmlWriter *htmlWriter() Q_DECL_OVERRIDE {
        return mWriter;
    }

    bool htmlLoadExternal() const Q_DECL_OVERRIDE
    {
        return mHtmlLoadExternal;
    }

    void setHtmlLoadExternal(bool loadExternal)
    {
        mHtmlLoadExternal = loadExternal;
    }

    void setAttachmentStrategy(QString strategy)
    {
        mAttachmentStrategy = strategy;
    }

    const AttachmentStrategy *attachmentStrategy() Q_DECL_OVERRIDE {
        return  AttachmentStrategy::create(mAttachmentStrategy);
    }

    bool autoImportKeys() const Q_DECL_OVERRIDE
    {
        return true;
    }

    bool showEmoticons() const Q_DECL_OVERRIDE
    {
        return false;
    }

    bool showExpandQuotesMark() const Q_DECL_OVERRIDE
    {
        return false;
    }

    const BodyPartFormatterBaseFactory *bodyPartFormatterFactory() Q_DECL_OVERRIDE {
        return &mBodyPartFormatterBaseFactory;
    }

    bool decryptMessage() const Q_DECL_OVERRIDE
    {
        return mDecryptMessage;
    }

    void setAllowDecryption(bool allowDecryption)
    {
        mDecryptMessage = allowDecryption;
    }

    void setShowSignatureDetails(bool showSignatureDetails)
    {
        mShowSignatureDetails = showSignatureDetails;
    }

    bool showSignatureDetails() const Q_DECL_OVERRIDE
    {
        return mShowSignatureDetails;
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

    int levelQuote() const Q_DECL_OVERRIDE
    {
        return 1;
    }

    const QTextCodec *overrideCodec() Q_DECL_OVERRIDE {
        return nullptr;
    }

    QString createMessageHeader(KMime::Message *message) Q_DECL_OVERRIDE {
        Q_UNUSED(message);
        return QString(); //do nothing
    }

    QObject *sourceObject() Q_DECL_OVERRIDE {
        return nullptr;
    }

    Interface::MessagePartRenderer::Ptr messagePartTheme(Interface::MessagePart::Ptr msgPart) Q_DECL_OVERRIDE {
        Q_UNUSED(msgPart);
        return  Interface::MessagePartRenderer::Ptr();
    }
    bool isPrinting() const Q_DECL_OVERRIDE
    {
        return false;
    }
private:
    MimeTreeParser::HtmlWriter *mWriter;
    QString mAttachmentStrategy;
    BodyPartFormatterBaseFactory mBodyPartFormatterBaseFactory;
    MimeTreeParser::Util::HtmlMode mPreferredMode;
    bool mHtmlLoadExternal;
    bool mDecryptMessage;
    bool mShowSignatureDetails;
};

}

}

#endif
