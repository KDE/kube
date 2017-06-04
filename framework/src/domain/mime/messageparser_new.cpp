/*
    Copyright (c) 2016 Sandro Knauß <knauss@kolabsys.com>

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

#include "messageparser.h"
#include "mimetreeparser/otp/objecttreeparser.h"
#include "mimetreeparser/otp/messagepart.h"
#include "htmlutils.h"

#include <QDebug>
#include <QMimeDatabase>
#include <QTextDocument>

class NewModelPrivate
{
public:
    NewModelPrivate(NewModel *q_ptr, const std::shared_ptr<MimeTreeParser::ObjectTreeParser> &parser);
    ~NewModelPrivate();

    void createTree();
    NewModel *q;
    QVector<MimeTreeParser::MessagePartPtr> mParts;
    std::shared_ptr<MimeTreeParser::ObjectTreeParser> mParser;
};

NewModelPrivate::NewModelPrivate(NewModel *q_ptr, const std::shared_ptr<MimeTreeParser::ObjectTreeParser> &parser)
    : q(q_ptr)
    , mParser(parser)
{
    mParts = mParser->collectContentParts();
    qWarning() << "Collected content parts: " << mParts.size();
}

NewModelPrivate::~NewModelPrivate()
{
}

NewModel::NewModel(std::shared_ptr<MimeTreeParser::ObjectTreeParser> parser)
    : d(std::unique_ptr<NewModelPrivate>(new NewModelPrivate(this, parser)))
{
}

NewModel::~NewModel()
{
}

QHash<int, QByteArray> NewModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TypeRole] = "type";
    roles[ContentRole] = "content";
    roles[IsEmbededRole] = "embeded";
    roles[IsEncryptedRole] = "encrypted";
    roles[IsSignedRole] = "signed";
    roles[SecurityLevelRole] = "securityLevel";
    roles[EncryptionErrorType] = "errorType";
    roles[EncryptionErrorString] = "errorString";
    roles[IsErrorRole] = "error";
    return roles;
}

QModelIndex NewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0) {
        return QModelIndex();
    }
    if (row < d->mParts.size()) {
        return createIndex(row, column, d->mParts.at(row).data());
    }
    return QModelIndex();
}

QVariant NewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        switch (role) {
        case Qt::DisplayRole:
            return QString("root");
        case IsEmbededRole:
            return false;
        }
        return QVariant();
    }

    if (index.internalPointer()) {
        const auto messagePart = static_cast<MimeTreeParser::MessagePart*>(index.internalPointer());
        Q_ASSERT(messagePart);
        switch(role) {
            case Qt::DisplayRole:
                return QStringLiteral("Content%1");
            case TypeRole: {
                if (messagePart->error()) {
                    return "error";
                }
                //For simple html we don't need a browser
                auto complexHtml = [&] {
                    if (messagePart->isHtml()) {
                        const auto text = messagePart->text();
                        if (text.contains("<!DOCTYPE html PUBLIC")) {
                            return true;
                        }
                        //Media queries are too advanced
                        if (text.contains("@media")) {
                            return true;
                        }
                        if (text.contains("<style")) {
                            return true;
                        }
                        return false;
                    } else {
                        return false;
                    }
                }();
                if (complexHtml) {
                    return "html";
                }
                return "plain";
            }
            case IsEmbededRole:
                return false;
            case IsErrorRole:
                return messagePart->error();
            case ContentRole: {
                auto errorText = messagePart->partMetaData()->errorText;
                if (!errorText.isEmpty()) {
                    return errorText;
                }
                const auto text = messagePart->isHtml() ? messagePart->htmlContent() : messagePart->text();
                if (messagePart->isHtml()) {
                    return d->mParser->resolveCidLinks(text);
                } else { //We assume plain
                    //We alwas do richtext (so we get highlighted links and stuff).
                    return HtmlUtils::linkify(Qt::convertFromPlainText(text));
                }
                return text;
            }
            case IsEncryptedRole:
                return messagePart->encryptions().size() > 0;
            case IsSignedRole:
                return messagePart->signatures().size() > 0;
            case EncryptionErrorType:
                return messagePart->error();
            case EncryptionErrorString:
                return messagePart->errorString();
        }
    }
    return QVariant();
}

QModelIndex NewModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int NewModel::rowCount(const QModelIndex &parent) const
{
    return d->mParts.count();
}

int NewModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}
