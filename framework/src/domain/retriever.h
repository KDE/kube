/*
    Copyright (c) 2016 Christian Mollekopf <mollekopf@kolabsystems.com>

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
#include <QObject>
#include <QAbstractItemModel>
#include <QVariant>

/**
 * A wrapper for a QAbstractItemModel to retrieve a value from a single index via property binding
 * 
 * Assign a model that retrieves the index, set the property your interested in, and propery-bind "value".
 */
class KUBE_EXPORT Retriever : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* model READ model WRITE setModel)
    Q_PROPERTY(QString propertyName MEMBER mPropertyName)
    Q_PROPERTY(QVariant value MEMBER mValue NOTIFY valueChanged)

public:
    explicit Retriever(QObject *parent = Q_NULLPTR);

    QAbstractItemModel* model() const;
    void setModel(QAbstractItemModel* model);

signals:
    void valueChanged();

private slots:
    void onRowsInserted(const QModelIndex &parent, int first, int last);
    void onModelReset();

private:
    QString mPropertyName;
    QVariant mValue;
    QAbstractItemModel *mModel;
};
