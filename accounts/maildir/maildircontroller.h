/*
 * Copyright (C) 2016 Michael Bohlender, <michael.bohlender@kdemail.net>
 * Copyright (c) 2016 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QUrl>

class MaildirController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QByteArray accountIdentifier READ accountIdentifier WRITE setAccountIdentifier NOTIFY accountIdentifierChanged)

    Q_PROPERTY(QString name MEMBER mName NOTIFY nameChanged)
    Q_PROPERTY(QString icon MEMBER mIcon NOTIFY iconChanged)
    Q_PROPERTY(QUrl path READ path WRITE setPath NOTIFY pathChanged)

public:
    explicit MaildirController(QObject *parent = Q_NULLPTR);

    QByteArray accountIdentifier() const;
    void setAccountIdentifier(const QByteArray &id);

    QUrl path() const;
    void setPath(const QUrl &path);

signals:
    void accountIdentifierChanged();
    void nameChanged();
    void iconChanged();
    void pathChanged();

public slots:
    void createAccount();
    void loadAccount(const QByteArray &id);
    void modifyAccount();
    void deleteAccount();

private:
    QByteArray mAccountIdentifier;
    QString mIcon;
    QString mName;
    QUrl mPath;
};

