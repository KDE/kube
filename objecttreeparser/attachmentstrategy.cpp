/*  -*- c++ -*-
    attachmentstrategy.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>
    Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
    Copyright (c) 2009 Andras Mantia <andras@kdab.net>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "attachmentstrategy.h"

#include "nodehelper.h"

#include <kmime/kmime_content.h>

#include "messageviewer_debug.h"
namespace MessageViewer
{

static AttachmentStrategy::Display smartDisplay(KMime::Content *node)
{
    const auto cd = node->contentDisposition(false);

    if (cd && cd->disposition() == KMime::Headers::CDinline)
        // explict "inline" disposition:
    {
        return AttachmentStrategy::Inline;
    }
    if (cd && cd->disposition() == KMime::Headers::CDattachment)
        // explicit "attachment" disposition:
    {
        return AttachmentStrategy::AsIcon;
    }

    const auto ct = node->contentType(false);
    if (ct && ct->isText() && ct->name().trimmed().isEmpty() &&
            (!cd || cd->filename().trimmed().isEmpty()))
        // text/* w/o filename parameter:
    {
        return AttachmentStrategy::Inline;
    }
    return AttachmentStrategy::AsIcon;
}

//
// IconicAttachmentStrategy:
//   show everything but the first text/plain body as icons
//

class IconicAttachmentStrategy : public AttachmentStrategy
{
    friend class AttachmentStrategy;
protected:
    IconicAttachmentStrategy() : AttachmentStrategy() {}
    virtual ~IconicAttachmentStrategy() {}

public:
    const char *name() const Q_DECL_OVERRIDE
    {
        return "iconic";
    }

    bool inlineNestedMessages() const Q_DECL_OVERRIDE
    {
        return false;
    }
    Display defaultDisplay(KMime::Content *node) const Q_DECL_OVERRIDE
    {
        if (node->contentType()->isText() &&
                node->contentDisposition()->filename().trimmed().isEmpty() &&
                node->contentType()->name().trimmed().isEmpty())
            // text/* w/o filename parameter:
        {
            return Inline;
        }
        return AsIcon;
    }
};

//
// SmartAttachmentStrategy:
//   in addition to Iconic, show all body parts
//   with content-disposition == "inline" and
//   all text parts without a filename or name parameter inline
//

class SmartAttachmentStrategy : public AttachmentStrategy
{
    friend class AttachmentStrategy;
protected:
    SmartAttachmentStrategy() : AttachmentStrategy() {}
    virtual ~SmartAttachmentStrategy() {}

public:
    const char *name() const Q_DECL_OVERRIDE
    {
        return "smart";
    }

    bool inlineNestedMessages() const Q_DECL_OVERRIDE
    {
        return true;
    }
    Display defaultDisplay(KMime::Content *node) const Q_DECL_OVERRIDE
    {
        return smartDisplay(node);
    }
};

//
// InlinedAttachmentStrategy:
//   show everything possible inline
//

class InlinedAttachmentStrategy : public AttachmentStrategy
{
    friend class AttachmentStrategy;
protected:
    InlinedAttachmentStrategy() : AttachmentStrategy() {}
    virtual ~InlinedAttachmentStrategy() {}

public:
    const char *name() const Q_DECL_OVERRIDE
    {
        return "inlined";
    }

    bool inlineNestedMessages() const Q_DECL_OVERRIDE
    {
        return true;
    }
    Display defaultDisplay(KMime::Content *) const Q_DECL_OVERRIDE
    {
        return Inline;
    }
};

//
// HiddenAttachmentStrategy
//   show nothing except the first text/plain body part _at all_
//

class HiddenAttachmentStrategy : public AttachmentStrategy
{
    friend class AttachmentStrategy;
protected:
    HiddenAttachmentStrategy() : AttachmentStrategy() {}
    virtual ~HiddenAttachmentStrategy() {}

public:
    const char *name() const Q_DECL_OVERRIDE
    {
        return "hidden";
    }

    bool inlineNestedMessages() const Q_DECL_OVERRIDE
    {
        return false;
    }
    Display defaultDisplay(KMime::Content *node) const Q_DECL_OVERRIDE
    {
        if (node->contentType()->isText() &&
                node->contentDisposition()->filename().trimmed().isEmpty() &&
                node->contentType()->name().trimmed().isEmpty())
            // text/* w/o filename parameter:
        {
            return Inline;
        }
        if (!node->parent()) {
            return Inline;
        }

        if (node->parent() && node->parent()->contentType()->isMultipart() &&
                node->parent()->contentType()->subType() == "related") {
            return Inline;
        }

        return None;
    }
};

class HeaderOnlyAttachmentStrategy : public AttachmentStrategy
{
    friend class AttachmentStrategy;
protected:
    HeaderOnlyAttachmentStrategy() : AttachmentStrategy() {}
    virtual ~HeaderOnlyAttachmentStrategy() {}

public:
    const char *name() const Q_DECL_OVERRIDE
    {
        return "headerOnly";
    }

    bool inlineNestedMessages() const Q_DECL_OVERRIDE
    {
        return true;
    }

    Display defaultDisplay(KMime::Content *node) const Q_DECL_OVERRIDE
    {
        if (NodeHelper::isInEncapsulatedMessage(node)) {
            return smartDisplay(node);
        }

        NodeHelper::AttachmentDisplayInfo info = NodeHelper::attachmentDisplayInfo(node);
        if (info.displayInHeader) {
            // The entire point about this attachment strategy: Hide attachments in the body that are
            // already displayed in the attachment quick list
            return None;
        } else {
            return smartDisplay(node);
        }
    }

    bool requiresAttachmentListInHeader() const Q_DECL_OVERRIDE
    {
        return true;
    }
};

//
// AttachmentStrategy abstract base:
//

AttachmentStrategy::AttachmentStrategy()
{

}

AttachmentStrategy::~AttachmentStrategy()
{

}

const AttachmentStrategy *AttachmentStrategy::create(Type type)
{
    switch (type) {
    case Iconic:     return iconic();
    case Smart:      return smart();
    case Inlined:    return inlined();
    case Hidden:     return hidden();
    case HeaderOnly: return headerOnly();
    }
    qCCritical(MESSAGEVIEWER_LOG) << "Unknown attachment startegy ( type =="
                                  << (int)type << ") requested!";
    return 0; // make compiler happy
}

const AttachmentStrategy *AttachmentStrategy::create(const QString &type)
{
    const QString lowerType = type.toLower();
    if (lowerType == QLatin1String("iconic")) {
        return iconic();
    }
    //if ( lowerType == "smart" )    return smart(); // not needed, see below
    if (lowerType == QLatin1String("inlined")) {
        return inlined();
    }
    if (lowerType == QLatin1String("hidden")) {
        return hidden();
    }
    if (lowerType == QLatin1String("headeronly")) {
        return headerOnly();
    }
    // don't kFatal here, b/c the strings are user-provided
    // (KConfig), so fail gracefully to the default:
    return smart();
}

static const AttachmentStrategy *iconicStrategy = 0;
static const AttachmentStrategy *smartStrategy = 0;
static const AttachmentStrategy *inlinedStrategy = 0;
static const AttachmentStrategy *hiddenStrategy = 0;
static const AttachmentStrategy *headerOnlyStrategy = 0;

const AttachmentStrategy *AttachmentStrategy::iconic()
{
    if (!iconicStrategy) {
        iconicStrategy = new IconicAttachmentStrategy();
    }
    return iconicStrategy;
}

const AttachmentStrategy *AttachmentStrategy::smart()
{
    if (!smartStrategy) {
        smartStrategy = new SmartAttachmentStrategy();
    }
    return smartStrategy;
}

const AttachmentStrategy *AttachmentStrategy::inlined()
{
    if (!inlinedStrategy) {
        inlinedStrategy = new InlinedAttachmentStrategy();
    }
    return inlinedStrategy;
}

const AttachmentStrategy *AttachmentStrategy::hidden()
{
    if (!hiddenStrategy) {
        hiddenStrategy = new HiddenAttachmentStrategy();
    }
    return hiddenStrategy;
}

const AttachmentStrategy *AttachmentStrategy::headerOnly()
{
    if (!headerOnlyStrategy) {
        headerOnlyStrategy = new HeaderOnlyAttachmentStrategy();
    }
    return headerOnlyStrategy;
}

bool AttachmentStrategy::requiresAttachmentListInHeader() const
{
    return false;
}

}
