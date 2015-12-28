/* Copyright 2009 Thomas McGuire <mcguire@kde.org>
             2009 Torgny Nyblom <nyblom@kde.org>

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

#include "htmlquotecolorer.h"
#include "settings/messageviewersettings.h"

#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>

using namespace MessageViewer;

HTMLQuoteColorer::HTMLQuoteColorer()
    : mEnableHtmlQuoteColorer(true)
{
}

void HTMLQuoteColorer::setEnableHtmlQuoteColorer(bool enabled)
{
    mEnableHtmlQuoteColorer = enabled;
}

QString HTMLQuoteColorer::process(const QString &htmlSource, QString &extraHead)
{
    // Create a DOM Document from the HTML source
    QWebPage page(0);
    page.settings()->setAttribute(QWebSettings::JavascriptEnabled, false);
    page.settings()->setAttribute(QWebSettings::JavaEnabled, false);
    page.settings()->setAttribute(QWebSettings::PluginsEnabled, false);

    page.settings()->setAttribute(QWebSettings::AutoLoadImages, false);

    QWebFrame *frame = page.mainFrame();
    frame->setHtml(htmlSource);

    QString script(QLatin1String(
                       "mIsQuotedLine = false;\n"
                       "mIsFirstTextNodeInLine = true;\n"
                       "mQuoteColors = new Array();\n"
                       "mQuoteColors[0] = \"") + mQuoteColors[0].name() + QLatin1String("\";\n"
                               "mQuoteColors[1] = \"") + mQuoteColors[1].name() + QLatin1String("\";\n"
                                       "mQuoteColors[2] = \"") + mQuoteColors[2].name() + QLatin1String("\";\n"

                                               "processNode( document.documentElement );\n"

                                               "function processNode( node ) {\n"
                                               // Below, we determine if the current text node should be quote colored by keeping track of"
                                               // linebreaks and whether this text node is the first one."
                                               "  var textContent = node.textContent;\n"
                                               "  var isTextNode = !textContent.length == 0 && !node.hasChildNodes();\n"
                                               "  if ( isTextNode ) {\n"
                                               "   if ( mIsFirstTextNodeInLine ) {\n"
                                               "     if ( textContent.charAt( 0 ) ==  '>' || textContent.charAt( 0 ) == '|' ) {\n"
                                               "       mIsQuotedLine = true;\n"
                                               "       currentQuoteLength = quoteLength( textContent ) - 1;\n"
                                               "     }\n"
                                               "     else {\n"
                                               "       mIsQuotedLine = false;\n"
                                               "     }\n"
                                               "    }\n"

                                               // All subsequent text nodes are not the first ones anymore"
                                               "    mIsFirstTextNodeInLine = false;\n"
                                               "  }\n"

                                               "  var nodeName = node.nodeName.toLowerCase();\n"
                                               "  var lineBreakNodes = new Array();\n"
                                               "  lineBreakNodes[0] = \"br\"\n"
                                               "  lineBreakNodes[1] = \"p\"\n"
                                               "  lineBreakNodes[2] = \"div\"\n"
                                               "  lineBreakNodes[3] = \"ul\"\n"
                                               "  lineBreakNodes[4] = \"ol\"\n"
                                               "  lineBreakNodes[5] = \"li\"\n"

                                               "  for( i = 0; i < lineBreakNodes.length; i++) {\n"
                                               "    if ( lineBreakNodes[i] == nodeName ) {\n"
                                               "      mIsFirstTextNodeInLine = true;\n"
                                               "      break;\n"
                                               "    }\n"
                                               "  }\n"

                                               "  var returnNode = node;\n"
                                               "  var fontTagAdded = false;\n"
                                               "  if ( mIsQuotedLine && isTextNode ) {\n"

                                               // Ok, we are in a line that should be quoted, so create a font node for the color and replace"
                                               // the current node with it."
                                               "    var font = node.ownerDocument.createElement( \"font\" );\n"
                                               "    font.setAttribute( \"color\", mQuoteColors[ currentQuoteLength ] );\n"
                                               "    node.parentNode.replaceChild( font, node );\n"
                                               "    font.appendChild( node );\n"
                                               "    returnNode = font;\n"
                                               "    fontTagAdded = true;\n"
                                               "  }\n"

                                               // Process all child nodes, but only if we are didn't add those child nodes itself, as otherwise"
                                               // we'll go into an infinite recursion."
                                               "  if ( !fontTagAdded ) {\n"
                                               "    var childNode = node.firstChild;\n"
                                               "    while ( childNode ) {\n"
                                               "      childNode = processNode( childNode );\n"
                                               "      childNode = childNode.nextSibling;\n"
                                               "    }\n"
                                               "  }\n"
                                               "  return returnNode;\n"
                                               "}\n"

                                               "function quoteLength( line )\n"
                                               "{\n"
                                               "  line = line.replace(  / /g, \"\" ).replace( '|', '>' );\n"
                                               "  if ( line.substr( 0, 3 ) == \">>>\" ) return 3;\n"
                                               "  if ( line.substr( 0, 2 ) == \">>\" ) return 2;\n"
                                               "  if ( line.substr( 0, 1 ) == '>' ) return 1;\n"
                                               "  return 0;\n"
                                               "}\n"));

    if (mEnableHtmlQuoteColorer) {
        page.settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
        frame->evaluateJavaScript(script);
        page.settings()->setAttribute(QWebSettings::JavascriptEnabled, false);
    }
    const QWebElement body = frame->documentElement().findFirst(QStringLiteral("body"));
    const QWebElement header = frame->documentElement().findFirst(QStringLiteral("head"));

    extraHead = header.toInnerXml();
    return body.toInnerXml();
}

void HTMLQuoteColorer::setQuoteColor(unsigned int level, const QColor &color)
{
    if (level < 3) {
        mQuoteColors[level] = color;
    }
}

