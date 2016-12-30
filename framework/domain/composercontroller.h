/*
    Copyright (c) 2016 Michael Bohlender <michael.bohlender@kdemail.net>

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
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QQmlEngine>
#include <QAbstractItemModel>
#include <sink/applicationdomaintype.h>

#include <actions/context.h>
#include <actions/action.h>

namespace KMime {
class Message;
}

class ComposerContext : public Kube::Context {
    Q_OBJECT
    KUBE_CONTEXT_PROPERTY(QString, To, to)
    KUBE_CONTEXT_PROPERTY(QString, Cc, cc)
    KUBE_CONTEXT_PROPERTY(QString, Bcc, bcc)
    KUBE_CONTEXT_PROPERTY(QString, From, from)
    KUBE_CONTEXT_PROPERTY(QString, Subject, subject)
    KUBE_CONTEXT_PROPERTY(QString, Body, body)
};

class Completer : public QObject {
    Q_OBJECT
    Q_PROPERTY (QAbstractItemModel* model READ model CONSTANT)
    Q_PROPERTY (QString searchString WRITE setSearchString READ searchString)

public:
    Completer(QAbstractItemModel *model) : mModel{model}
    {
        QQmlEngine::setObjectOwnership(mModel, QQmlEngine::CppOwnership);
    }
    QAbstractItemModel *model() { return mModel; }
    virtual void setSearchString(const QString &s) { mSearchString = s; }
    QString searchString() const { return mSearchString; }

private:
    QAbstractItemModel *mModel = nullptr;
    QString mSearchString;
};

/**
 * Exposes a model and maintains a current index selection.
 */
class Selector : public QObject {
    Q_OBJECT
    Q_PROPERTY (int currentIndex READ currentIndex WRITE setCurrentIndex)
    Q_PROPERTY (QAbstractItemModel* model READ model CONSTANT)

public:
    Selector(QAbstractItemModel *model) : mModel{model}
    {
        QQmlEngine::setObjectOwnership(mModel, QQmlEngine::CppOwnership);
    }

    virtual QAbstractItemModel *model() { return mModel; }

    void setCurrentIndex(int i) {
        mCurrentIndex = i;
        Q_ASSERT(mModel);
        setCurrent(mModel->index(mCurrentIndex, 0));
    }

    int currentIndex() { return mCurrentIndex; }

    virtual void setCurrent(const QModelIndex &) = 0;
private:
    QAbstractItemModel *mModel = nullptr;
    int mCurrentIndex = 0;
};

class ComposerController : public QObject
{
    Q_OBJECT
    Q_PROPERTY (Kube::Context* mailContext READ mailContext CONSTANT)

    Q_PROPERTY (Completer* recipientCompleter READ recipientCompleter CONSTANT)
    Q_PROPERTY (Selector* identitySelector READ identitySelector CONSTANT)

    Q_PROPERTY (Kube::Action* sendAction READ sendAction)
    Q_PROPERTY (Kube::Action* saveAsDraftAction READ saveAsDraftAction)

public:
    explicit ComposerController(QObject *parent = Q_NULLPTR);

    Kube::Context* mailContext();

    Completer *recipientCompleter() const;
    Selector *identitySelector() const;

    Q_INVOKABLE void loadMessage(const QVariant &draft, bool loadAsDraft);

    Kube::Action* sendAction();
    Kube::Action* saveAsDraftAction();

public slots:
    void clear();

signals:
    void done();

private:
    Kube::ActionHandler *messageHandler();
    void recordForAutocompletion(const QByteArray &addrSpec, const QByteArray &displayName);
    void setMessage(const QSharedPointer<KMime::Message> &msg);

    ComposerContext mContext;
};
