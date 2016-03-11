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
#pragma once

#include <QObject>

class MaildirSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QByteArray identifier READ identifier WRITE setIdentifier)
    Q_PROPERTY(QByteArray accountIdentifier READ accountIdentifier WRITE setAccountIdentifier)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)

public:
    MaildirSettings(QObject *parent = 0);

    void setIdentifier(const QByteArray &);
    QByteArray identifier() const;

    void setAccountIdentifier(const QByteArray &);
    QByteArray accountIdentifier() const;

    void setPath(const QString &);
    QString path() const;

    Q_INVOKABLE void save();
    Q_INVOKABLE void remove();

signals:
    void pathChanged();

private:
    QByteArray mIdentifier;
    QByteArray mAccountIdentifier;
    QString mPath;
};
