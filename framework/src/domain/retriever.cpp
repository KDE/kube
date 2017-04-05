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


#include "retriever.h"

Retriever::Retriever(QObject *parent) : QObject(parent)
{
}

QAbstractItemModel* Retriever::model() const
{
    return mModel;
}

void Retriever::setModel(QAbstractItemModel* model)
{
    mValue = QVariant();
    mModel = model;
    connect(model, &QAbstractItemModel::rowsInserted, this, &Retriever::onRowsInserted);
    connect(model, &QAbstractItemModel::modelReset, this, &Retriever::onModelReset);
    if (model->rowCount(QModelIndex())) {
        mValue = model->index(0, 0, QModelIndex()).data(mModel->roleNames().key(mPropertyName.toLatin1()));
        emit valueChanged();
    }
}

void Retriever::onRowsInserted(const QModelIndex &parent, int first, int last)
{
    if (!mValue.isValid()) {
        mValue = mModel->index(0, 0, QModelIndex()).data(mModel->roleNames().key(mPropertyName.toLatin1()));
        emit valueChanged();
    }
}

void Retriever::onModelReset()
{
    mValue = QVariant();
}
