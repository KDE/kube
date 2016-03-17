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
#include <QValidator>

class MaildirSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QByteArray accountIdentifier READ accountIdentifier WRITE setAccountIdentifier)
    Q_PROPERTY(QUrl path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QValidator* pathValidator READ pathValidator CONSTANT)
    Q_PROPERTY(QString icon MEMBER mIcon NOTIFY changed)
    Q_PROPERTY(QString accountName MEMBER mName NOTIFY changed)

public:
    MaildirSettings(QObject *parent = 0);

    void setAccountIdentifier(const QByteArray &);
    QByteArray accountIdentifier() const;

    void setPath(const QUrl &);
    QUrl path() const;
    QValidator *pathValidator() const;

    Q_INVOKABLE void save();
    Q_INVOKABLE void remove();

signals:
    void pathChanged();
    void changed();

private:
    QByteArray mIdentifier;
    QByteArray mAccountIdentifier;
    QString mPath;
    QString mIcon;
    QString mName;
};
