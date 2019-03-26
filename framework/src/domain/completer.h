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
#pragma once

#include "kube_export.h"
#include <QAbstractItemModel>
#include <QString>
#include <QQmlEngine>

class KUBE_EXPORT Completer : public QObject {
    Q_OBJECT
    Q_PROPERTY (QAbstractItemModel* model READ model CONSTANT)
    Q_PROPERTY (QString searchString WRITE setSearchString READ searchString)

public:
    Completer(QAbstractItemModel *model);
    QAbstractItemModel *model() { return mModel; }
    virtual void setSearchString(const QString &s) { mSearchString = s; }
    QString searchString() const { return mSearchString; }

private:
    QAbstractItemModel *mModel = nullptr;
    QString mSearchString;
};

