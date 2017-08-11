/*
    Copyright (c) 2016 Michael Bohlender <michael.bohlender@kdemail.net>
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


#include "composercontroller.h"
#include <settings/settings.h>
#include <KMime/Message>
#include <QVariant>
#include <QSortFilterProxyModel>
#include <QList>
#include <QDebug>
#include <QMimeDatabase>
#include <QUrlQuery>
#include <QFileInfo>
#include <QFile>
#include <sink/store.h>
#include <sink/log.h>

#include "identitiesmodel.h"
#include "recepientautocompletionmodel.h"
#include "mime/mailtemplates.h"

class IdentitySelector : public Selector {
public:
    IdentitySelector(ComposerController &controller) : Selector(new IdentitiesModel), mController(controller)
    {
    }

    void setCurrent(const QModelIndex &index) Q_DECL_OVERRIDE
    {
        if (index.isValid()) {
            auto currentAccountId = index.data(IdentitiesModel::AccountId).toByteArray();

            KMime::Types::Mailbox mb;
            mb.setName(index.data(IdentitiesModel::Username).toString());
            mb.setAddress(index.data(IdentitiesModel::Address).toString().toUtf8());
            SinkLog() << "Setting current identity: " << mb.prettyAddress() << "Account: " << currentAccountId;

            mController.setIdentity(mb);
            mController.setAccountId(currentAccountId);
        } else {
            SinkWarning() << "No valid identity for index: " << index;
            mController.clearIdentity();
            mController.clearAccountId();
        }

    }
private:
    ComposerController &mController;
};

class RecipientCompleter : public Completer {
public:
    RecipientCompleter() : Completer(new RecipientAutocompletionModel)
    {
    }

    void setSearchString(const QString &s) {
        static_cast<RecipientAutocompletionModel*>(model())->setFilter(s);
        Completer::setSearchString(s);
    }
};


ComposerController::ComposerController()
    : Kube::Controller(),
    action_send{new Kube::ControllerAction{this, &ComposerController::send}},
    action_saveAsDraft{new Kube::ControllerAction{this, &ComposerController::saveAsDraft}},
    mRecipientCompleter{new RecipientCompleter},
    mIdentitySelector{new IdentitySelector{*this}},
    mToModel{new QStringListModel},
    mCcModel{new QStringListModel},
    mBccModel{new QStringListModel},
    mAttachmentModel{new QStandardItemModel}
{
    mAttachmentModel->setItemRoleNames({{NameRole, "name"},
                                        {FilenameRole, "filename"},
                                        {ContentRole, "content"},
                                        {MimeTypeRole, "mimetype"},
                                        {DescriptionRole, "description"},
                                        {IconNameRole, "iconName"},
                                        {UrlRole, "url"},
                                        {InlineRole, "inline"}});
    updateSaveAsDraftAction();
    // mSendAction->monitorProperty<To>();
    // mSendAction->monitorProperty<Send>([] (const QString &) -> bool{
    //     //validate
    // });
    // registerAction<ControllerAction>(&ComposerController::send);
    // actionDepends<ControllerAction, To, Subject>();
    // TODO do in constructor

    QObject::connect(this, &ComposerController::subjectChanged, &ComposerController::updateSendAction);
    QObject::connect(this, &ComposerController::accountIdChanged, &ComposerController::updateSendAction);
    QObject::connect(this, &ComposerController::subjectChanged, &ComposerController::updateSaveAsDraftAction);
    QObject::connect(this, &ComposerController::accountIdChanged, &ComposerController::updateSaveAsDraftAction);
    updateSendAction();
}

void ComposerController::clear()
{
    Controller::clear();
    //Reapply account and identity from selection
    mIdentitySelector->reapplyCurrentIndex();
    mToModel->setStringList({});
    mCcModel->setStringList({});
    mBccModel->setStringList({});
}

QAbstractItemModel *ComposerController::toModel() const
{
    return mToModel.data();
}

void ComposerController::addTo(const QString &s)
{
    auto list = mToModel->stringList();
    list.append(s);
    mToModel->setStringList(list);
    updateSendAction();
}

void ComposerController::removeTo(const QString &s)
{
    auto list = mToModel->stringList();
    list.removeAll(s);
    mToModel->setStringList(list);
    updateSendAction();
}

QAbstractItemModel *ComposerController::ccModel() const
{
    return mCcModel.data();
}

void ComposerController::addCc(const QString &s)
{
    auto list = mCcModel->stringList();
    list.append(s);
    mCcModel->setStringList(list);
    updateSendAction();
}

void ComposerController::removeCc(const QString &s)
{
    auto list = mCcModel->stringList();
    list.removeAll(s);
    mCcModel->setStringList(list);
    updateSendAction();
}

QAbstractItemModel *ComposerController::bccModel() const
{
    return mBccModel.data();
}

void ComposerController::addBcc(const QString &s)
{
    auto list = mBccModel->stringList();
    list.append(s);
    mBccModel->setStringList(list);
    updateSendAction();
}

void ComposerController::removeBcc(const QString &s)
{
    auto list = mBccModel->stringList();
    list.removeAll(s);
    mBccModel->setStringList(list);
    updateSendAction();
}

QAbstractItemModel *ComposerController::attachmentModel() const
{
    return mAttachmentModel.data();
}

void ComposerController::addAttachment(const QUrl &url)
{
    QMimeDatabase db;
    auto mimeType = db.mimeTypeForUrl(url);
    if (mimeType.name() == QLatin1String("inode/directory")) {
        qWarning() << "Can't deal with directories yet.";
    } else {
        if (!url.isLocalFile()) {
            qWarning() << "Cannot attach remote file: " << url;
            return;
        }

        QFileInfo fileInfo(url.toLocalFile());
        if (!fileInfo.exists()) {
            qWarning() << "The file doesn't exist: " << url;
        }

        QFile file{fileInfo.filePath()};
        file.open(QIODevice::ReadOnly);
        const auto data = file.readAll();
        auto item = new QStandardItem;
        item->setData(fileInfo.fileName(), NameRole);
        item->setData(mimeType.name().toLatin1(), MimeTypeRole);
        item->setData(fileInfo.fileName(), FilenameRole);
        item->setData(false, InlineRole);
        item->setData(mimeType.iconName(), IconNameRole);
        item->setData(url, UrlRole);
        item->setData(data, ContentRole);
        mAttachmentModel->appendRow(item);
    }
}

void ComposerController::removeAttachment(const QUrl &url)
{
    auto root = mAttachmentModel->invisibleRootItem();
    if (root->hasChildren()) {
        for (int row = 0; row < root->rowCount(); row++) {
            auto item = root->child(row, 0);
            const auto url = item->data(UrlRole).toUrl();
            if (url == item->data(UrlRole).toUrl()) {
                root->removeRow(row);
                return;
            }
        }
    }
}

Completer *ComposerController::recipientCompleter() const
{
    return mRecipientCompleter.data();
}

Selector *ComposerController::identitySelector() const
{
    return mIdentitySelector.data();
}

static void applyAddresses(const KMime::Types::Mailbox::List &list, std::function<void(const QByteArray &, const QByteArray &)> callback)
{
    for (const auto &to : list) {
        callback(to.address(), to.name().toUtf8());
    }
}

static void applyAddresses(const QStringList &list, std::function<void(const QByteArray &, const QByteArray &)> callback)
{
    KMime::Types::Mailbox::List mailboxes;
    for (const auto &s : list) {
        KMime::Types::Mailbox mb;
        mb.fromUnicodeString(s);
        mailboxes << mb;
    }
    applyAddresses(mailboxes, callback);
}

static QStringList getStringListFromAddresses(const KMime::Types::Mailbox::List &s)
{
    QStringList list;
    applyAddresses(s, [&](const QByteArray &addrSpec, const QByteArray &displayName) {
        if (displayName.isEmpty()) {
            list << QString{addrSpec};
        } else {
            list << QString("%1 <%2>").arg(QString{displayName}).arg(QString{addrSpec});
        }
    });
    return list;
}

void ComposerController::addAttachmentPart(KMime::Content *partToAttach)
{
    auto item = new QStandardItem;

    if (partToAttach->contentType()->mimeType() == "multipart/digest" ||
            partToAttach->contentType()->mimeType() == "message/rfc822") {
        // if it is a digest or a full message, use the encodedContent() of the attachment,
        // which already has the proper headers
        item->setData(partToAttach->encodedContent(), ContentRole);
    } else {
        item->setData(partToAttach->decodedContent(), ContentRole);
    }
    item->setData(partToAttach->contentType()->mimeType(), MimeTypeRole);

    QMimeDatabase db;
    auto mimeType = db.mimeTypeForName(partToAttach->contentType()->mimeType());
    item->setData(mimeType.iconName(), IconNameRole);

    if (partToAttach->contentDescription(false)) {
        item->setData(partToAttach->contentDescription()->asUnicodeString(), DescriptionRole);
    }
    QString name;
    QString filename;
    if (partToAttach->contentType(false)) {
        if (partToAttach->contentType()->hasParameter(QStringLiteral("name"))) {
            name = partToAttach->contentType()->parameter(QStringLiteral("name"));
        }
    }
    if (partToAttach->contentDisposition(false)) {
        filename = partToAttach->contentDisposition()->filename();
        item->setData(partToAttach->contentDisposition()->disposition() == KMime::Headers::CDinline, InlineRole);
    }

    if (name.isEmpty() && !filename.isEmpty()) {
        name = filename;
    }
    if (filename.isEmpty() && !name.isEmpty()) {
        filename = name;
    }

    if (!filename.isEmpty()) {
        item->setData(filename, FilenameRole);
    }
    if (!name.isEmpty()) {
        item->setData(name, NameRole);
    }

    mAttachmentModel->appendRow(item);
}

void ComposerController::setMessage(const KMime::Message::Ptr &msg)
{
    mToModel->setStringList(getStringListFromAddresses(msg->to(true)->mailboxes()));
    mCcModel->setStringList(getStringListFromAddresses(msg->cc(true)->mailboxes()));
    mBccModel->setStringList(getStringListFromAddresses(msg->bcc(true)->mailboxes()));

    setSubject(msg->subject(true)->asUnicodeString());
    setBody(MailTemplates::plaintextContent(msg));

    //TODO use ObjecTreeParser to get encrypted attachments as well
    foreach (const auto &att, msg->attachments()) {
        addAttachmentPart(att);
    }

    setExistingMessage(msg);
}

void ComposerController::loadMessage(const QVariant &message, bool loadAsDraft)
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    Query query(*message.value<Mail::Ptr>());
    query.request<Mail::MimeMessage>();
    Store::fetchOne<Mail>(query).then([this, loadAsDraft](const Mail &mail) {
        setExistingMail(mail);

        const auto mailData = KMime::CRLFtoLF(mail.getMimeMessage());
        if (!mailData.isEmpty()) {
            KMime::Message::Ptr mail(new KMime::Message);
            mail->setContent(mailData);
            mail->parse();
            if (loadAsDraft) {
                setMessage(mail);
            } else {
                MailTemplates::reply(mail, [this] (const KMime::Message::Ptr &reply) {
                    //We assume reply
                    setMessage(reply);
                });
            }
        } else {
            qWarning() << "Retrieved empty message";
        }
    }).exec();
}

void ComposerController::recordForAutocompletion(const QByteArray &addrSpec, const QByteArray &displayName)
{
    if (auto model = static_cast<RecipientAutocompletionModel*>(recipientCompleter()->model())) {
        model->addEntry(addrSpec, displayName);
    }
}

KMime::Message::Ptr ComposerController::assembleMessage()
{
    applyAddresses(mToModel->stringList(), [&](const QByteArray &addrSpec, const QByteArray &displayName) {
        recordForAutocompletion(addrSpec, displayName);
    });
    applyAddresses(mCcModel->stringList(), [&](const QByteArray &addrSpec, const QByteArray &displayName) {
        recordForAutocompletion(addrSpec, displayName);
    });
    applyAddresses(mBccModel->stringList(), [&](const QByteArray &addrSpec, const QByteArray &displayName) {
        recordForAutocompletion(addrSpec, displayName);
    });

    QList<Attachment> attachments;
    auto root = mAttachmentModel->invisibleRootItem();
    if (root->hasChildren()) {
        for (int row = 0; row < root->rowCount(); row++) {
            auto item = root->child(row, 0);
            attachments << Attachment{
                item->data(NameRole).toString(),
                item->data(FilenameRole).toString(),
                item->data(MimeTypeRole).toByteArray(),
                item->data(InlineRole).toBool(),
                item->data(ContentRole).toByteArray()
            };
        }
    }
    return MailTemplates::createMessage(mExistingMessage, mToModel->stringList(), mCcModel->stringList(), mBccModel->stringList(), getIdentity(), getSubject(), getBody(), attachments);
}

void ComposerController::updateSendAction()
{
    auto enabled = !mToModel->stringList().isEmpty() && !getSubject().isEmpty() && !getAccountId().isEmpty();
    sendAction()->setEnabled(enabled);
}

void ComposerController::send()
{
    // verify<To, Subject>()
    // && verify<Subject>();
    auto message = assembleMessage();

    auto accountId = getAccountId();
    //SinkLog() << "Sending a mail: " << *this;
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    Q_ASSERT(!accountId.isEmpty());
    Query query;
    query.containsFilter<SinkResource::Capabilities>(ResourceCapabilities::Mail::transport);
    query.filter<SinkResource::Account>(accountId);
    auto job = Store::fetchAll<SinkResource>(query)
        .then([=](const QList<SinkResource::Ptr> &resources) {
            if (!resources.isEmpty()) {
                auto resourceId = resources[0]->identifier();
                SinkLog() << "Sending message via resource: " << resourceId;
                Mail mail(resourceId);
                mail.setMimeMessage(message->encodedContent(true));
                return Store::create(mail)
                    .then<void>([=] {
                        //Trigger a sync, but don't wait for it.
                        Store::synchronize(Sink::SyncScope{}.resourceFilter(resourceId)).exec();
                    });
            }
            SinkWarning() << "Failed to find a mailtransport resource";
            return KAsync::error<void>(0, "Failed to find a MailTransport resource.");
        })
        .then([&] (const KAsync::Error &error) {
            SinkLog() << "Message was sent: ";
            emit done();
        });
    run(job);
}

void ComposerController::updateSaveAsDraftAction()
{
    bool enabled = !getAccountId().isEmpty();
    sendAction()->setEnabled(enabled);
}

void ComposerController::saveAsDraft()
{
    SinkLog() << "Save as draft";
    const auto accountId = getAccountId();
    auto existingMail = getExistingMail();

    auto message = assembleMessage();
    //FIXME this is something for the validation
    if (!message) {
        SinkWarning() << "Failed to get the mail: ";
        return;
    }

    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    auto job = [&] {
        if (existingMail.identifier().isEmpty()) {
            SinkLog() << "Creating a new draft" << existingMail.identifier();
            Query query;
            query.containsFilter<SinkResource::Capabilities>(ResourceCapabilities::Mail::drafts);
            query.filter<SinkResource::Account>(accountId);
            return Store::fetchOne<SinkResource>(query)
                .then([=](const SinkResource &resource) {
                    Mail mail(resource.identifier());
                    mail.setDraft(true);
                    mail.setMimeMessage(message->encodedContent(true));
                    return Store::create(mail);
                })
                .onError([] (const KAsync::Error &error) {
                    SinkWarning() << "Error while creating draft: " << error.errorMessage;
                });
        } else {
            SinkLog() << "Modifying an existing mail" << existingMail.identifier();
            existingMail.setDraft(true);
            existingMail.setMimeMessage(message->encodedContent(true));
            return Store::modify(existingMail);
        }
    }();
    job = job.then([&] (const KAsync::Error &) {
        emit done();
    });
    run(job);
}
