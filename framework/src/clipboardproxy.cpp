/*
    Copyright (c) 2016 Christian Mollekofp <mollekopf@kolabsys.com>

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
#include "clipboardproxy.h"

#include <QClipboard>
#include <QApplication>

ClipboardProxy::ClipboardProxy(QObject *parent)
    : QObject(parent)
{
    QClipboard *clipboard = QApplication::clipboard();
    QObject::connect(clipboard, &QClipboard::dataChanged,
            this, &ClipboardProxy::dataChanged);
    QObject::connect(clipboard, &QClipboard::selectionChanged,
            this, &ClipboardProxy::selectionChanged);
}

void ClipboardProxy::setDataText(const QString &text)
{
    QApplication::clipboard()->setText(text, QClipboard::Clipboard);
}

QString ClipboardProxy::dataText() const
{
    return QGuiApplication::clipboard()->text(QClipboard::Clipboard);
}

void ClipboardProxy::setSelectionText(const QString &text)
{
    QApplication::clipboard()->setText(text, QClipboard::Selection);
}

QString ClipboardProxy::selectionText() const
{
    return QApplication::clipboard()->text(QClipboard::Selection);
}
