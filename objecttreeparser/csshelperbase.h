/*  -*- c++ -*-
    csshelper.h

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>

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

#ifndef MESSAGEVIEWER_CSSHELPERBASE_H
#define MESSAGEVIEWER_CSSHELPERBASE_H

#include "messageviewer_export.h"
#include <QColor>
#include <QFont>

class QString;
class QPaintDevice;

namespace MessageViewer
{

class MESSAGEVIEWER_EXPORT CSSHelperBase
{
public:
    /** Construct a CSSHelper object and set its font and color settings to
        default values.
        Sub-Classes should put their config loading here.
     */
    explicit CSSHelperBase(const QPaintDevice *pd);

    /** @return HTML head including style sheet definitions and the
        &gt;body&lt; tag */
    QString htmlHead(bool fixedFont = false) const;

    /** @return The collected CSS definitions as a string */
    QString cssDefinitions(bool fixedFont = false) const;

    /** @return a &lt;div&gt; start tag with embedded style
        information suitable for quoted text with quote level @p level */
    QString quoteFontTag(int level) const;
    /** @return a &lt;div&gt; start tag with embedded style
        information suitable for non-quoted text */
    QString nonQuotedFontTag() const;

    QFont bodyFont(bool fixedFont = false, bool printing = false) const;

    void setBodyFont(const QFont &font);
    void setPrintFont(const QFont &font);

    /** @return the quote color for the given level, where level ranges from 0 to 2 **/
    QColor quoteColor(int level);

    QColor pgpWarnColor() const;

protected:
    /** Recalculate PGP frame and body colors (should be called after changing
        color settings) */
    void recalculatePGPColors();

protected:
    QFont mBodyFont;
    QFont mPrintFont;
    QFont mFixedFont;
    QFont mFixedPrintFont;
    QFont mQuoteFont[3];
    QColor mQuoteColor[3];
    bool mRecycleQuoteColors;
    bool mBackingPixmapOn;
    bool mShrinkQuotes;
    QString mBackingPixmapStr;
    QColor mForegroundColor;
    QColor mLinkColor;
    QColor mVisitedLinkColor;
    QColor mBackgroundColor;
    // colors for PGP (Frame, Header, HeaderText, Body)
    QColor cPgpOk1F;
    QColor cPgpOk1H;
    QColor cPgpOk1HT;
    QColor cPgpOk1B;
    QColor cPgpOk0F;
    QColor cPgpOk0H;
    QColor cPgpOk0HT;
    QColor cPgpOk0B;
    QColor cPgpWarnF;
    QColor cPgpWarnH;
    QColor cPgpWarnHT;
    QColor cPgpWarnB;
    QColor cPgpErrF;
    QColor cPgpErrH;
    QColor cPgpErrHT;
    QColor cPgpErrB;
    QColor cPgpEncrF;
    QColor cPgpEncrH;
    QColor cPgpEncrHT;
    QColor cPgpEncrB;
    // color of frame of warning preceding the source of HTML messages
    QColor cHtmlWarning;

private:
    int fontSize(bool fixed, bool print = false) const;
    // returns CSS rules specific to the print media type
    QString printCssDefinitions(bool fixed) const;
    // returns CSS rules specific to the screen media type
    QString screenCssDefinitions(const CSSHelperBase *helper, bool fixed) const;
    // returns CSS rules common to both screen and print media types
    QString commonCssDefinitions() const;

private:
    const QPaintDevice *mPaintDevice;

};

}

#endif
