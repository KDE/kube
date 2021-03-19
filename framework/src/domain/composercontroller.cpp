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
#include <QList>
#include <QDebug>
#include <QMimeDatabase>
#include <QUrlQuery>
#include <QFileInfo>
#include <QFile>
#include <QTemporaryFile>
#include <KEmailAddress>
#include <sink/store.h>
#include <sink/log.h>

#include "identitiesmodel.h"
#include "recepientautocompletionmodel.h"
#include "mime/mailtemplates.h"
#include "mime/mailcrypto.h"
#include "async.h"
#include "sinkutils.h"

std::vector<Crypto::Key> &operator+=(std::vector<Crypto::Key> &list, const std::vector<Crypto::Key> &add)
{
    list.insert(std::end(list), std::begin(add), std::end(add));
    return list;
}

class IdentitySelector : public Selector {
    Q_OBJECT
    Q_PROPERTY (QString currentAccountId WRITE setCurrentAccountId)

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

    void setCurrentAccountId(const QString &accountId)
    {
        for (int i = 0; i < model()->rowCount(); i++) {
            if (model()->index(i, 0).data(IdentitiesModel::AccountId).toString() == accountId) {
                setCurrentIndex(i);
                return;
            }
        }
    }

    void setCurrentIdentity(const QString &emailAddress)
    {
        for (int i = 0; i < model()->rowCount(); i++) {
            if (model()->index(i, 0).data(IdentitiesModel::Address).toString() == emailAddress) {
                setCurrentIndex(i);
                return;
            }
        }
    }

    QVector<QByteArray> getAllAddresses()
    {
        QVector<QByteArray> list;
        for (int i = 0; i < model()->rowCount(); i++) {
            list << model()->data(model()->index(i, 0), IdentitiesModel::Address).toString().toUtf8();
        }
        return list;
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

class AddresseeController : public Kube::ListPropertyController
{
    Q_OBJECT
    Q_PROPERTY(bool foundAllKeys READ foundAllKeys NOTIFY foundAllKeysChanged)
public:

    bool mFoundAllKeys = true;

    QSet<QByteArray> mMissingKeys;
    AddresseeController() : Kube::ListPropertyController{{"name", "keyFound", "key", "fetching"}}
    {
        QObject::connect(
            this, &Kube::ListPropertyController::added, this, [this](const QByteArray &id, const QVariantMap &map) {
                findKey(id, map.value("name").toString(), false);
            });

        QObject::connect(this, &Kube::ListPropertyController::removed, this, [this] (const QByteArray &id) {
            mMissingKeys.remove(id);
            setFoundAllKeys(mMissingKeys.isEmpty());
        });
    }

    bool foundAllKeys()
    {
        return mFoundAllKeys;
    }

    void setFoundAllKeys(bool found)
    {
        mFoundAllKeys = found;
        emit foundAllKeysChanged();
    }

    void findKey(const QByteArray &id, const QString &addressee, bool fetchRemote)
    {
        KMime::Types::Mailbox mb;
        mb.fromUnicodeString(addressee);

        if (mb.address().isEmpty()) {
            return;
        }

        SinkLog() << "Searching key for: " << mb.address();

        mMissingKeys << id;
        setFoundAllKeys(false);

        setValue(id, "fetching", fetchRemote);

        asyncRun<std::vector<Crypto::Key>>(this,
            [=] {
                return Crypto::findKeys({mb.address()}, false, fetchRemote);
            },
            [this, addressee, id](const std::vector<Crypto::Key> &keys) {
                setValue(id, "fetching", false);
                if (!keys.empty()) {
                    if (keys.size() > 1) {
                        SinkWarning() << "Found more than one key, encrypting to all of them.";
                    }
                    SinkLog() << "Found key: " << keys.front();
                    setValue(id, "keyFound", true);
                    setValue(id, "key", QVariant::fromValue(keys));
                    mMissingKeys.remove(id);
                    setFoundAllKeys(mMissingKeys.isEmpty());
                } else {
                    SinkWarning() << "Failed to find key for recipient.";
                }
            });
    }

    Q_INVOKABLE void fetchKeys(const QByteArray &id, const QString &addressee)
    {
        findKey(id, addressee, true);
    }

    void set(const QStringList &list)
    {
        for (const auto &email : list) {
            Kube::ListPropertyController::add({{"name", email}});
        }
    }

    Q_INVOKABLE virtual void add(const QVariantMap &map)
    {
        // Support adding multiple addresses separated by comma
        for (const auto &part : KEmailAddress::splitAddressList(map.value("name").toString())) {
            const auto address = part.trimmed();

            //Validation
            KMime::Types::Mailbox mb;
            mb.fromUnicodeString(address);
            if (mb.address().isEmpty()) {
                SinkTrace() << "Ignoring invalid address " << address;
                continue;
            }

            Kube::ListPropertyController::add({{"name", address}});
        }
    }
signals:
    void foundAllKeysChanged();
};

class AttachmentController : public Kube::ListPropertyController
{
public:

    AttachmentController()
        : Kube::ListPropertyController{{"name", "filename", "content", "mimetype", "description", "iconname", "url", "inline"}}
    {
        QObject::connect(this, &Kube::ListPropertyController::added, this, [this] (const QByteArray &id, const QVariantMap &map) {
            auto url = map.value("url").toUrl();
            setAttachmentProperties(id, url);
        });
    }

    void setAttachmentProperties(const QByteArray &id, const QUrl &url)
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
            QVariantMap map;
            map.insert("filename", fileInfo.fileName());
            map.insert("mimetype", mimeType.name().toLatin1());
            map.insert("filename", fileInfo.fileName().toLatin1());
            map.insert("inline", false);
            map.insert("iconname", mimeType.iconName());
            map.insert("url", url);
            map.insert("content", data);
            setValues(id, map);
        }
    }
};

ComposerController::ComposerController()
    : Kube::Controller(),
    controller_to{new AddresseeController},
    controller_cc{new AddresseeController},
    controller_bcc{new AddresseeController},
    controller_attachments{new AttachmentController},
    action_send{new Kube::ControllerAction{this, &ComposerController::send}},
    action_saveAsDraft{new Kube::ControllerAction{this, &ComposerController::saveAsDraft}},
    mRecipientCompleter{new RecipientCompleter},
    mIdentitySelector{new IdentitySelector{*this}}
{
    QObject::connect(this, &ComposerController::identityChanged, &ComposerController::findPersonalKey);
}

void ComposerController::findPersonalKey()
{
    auto identity = getIdentity();
    SinkLog() << "Looking for personal key for: " << identity.address();
    asyncRun<std::vector<Crypto::Key>>(this, [=] {
            return Crypto::findKeys({identity.address()}, true);
        },
        [this](const std::vector<Crypto::Key> &keys) {
            if (keys.empty()) {
                SinkWarning() << "Failed to find a personal key.";
            } else if (keys.size() > 1) {
                SinkWarning() << "Found multiple keys, using all of them.";
            }
            setPersonalKeys(QVariant::fromValue(keys));
            setFoundPersonalKeys(!keys.empty());
        });
}

void ComposerController::clear()
{
    Controller::clear();
    //Reapply account and identity from selection
    mIdentitySelector->reapplyCurrentIndex();
    //FIXME implement in Controller::clear instead
    toController()->clear();
    ccController()->clear();
    bccController()->clear();
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

static QStringList getStringListFromAddresses(const KMime::Types::Mailbox::List &mailboxes)
{
    QStringList list;
    for (const auto &mb : mailboxes) {
        list << mb.prettyAddress(KMime::Types::Mailbox::QuoteWhenNecessary);
    }
    return list;
}

void ComposerController::addAttachmentPart(KMime::Content *partToAttach)
{
    QVariantMap map;
    // May need special care for the multipart/digest MIME type
    map.insert("content", partToAttach->decodedContent());
    map.insert("mimetype", partToAttach->contentType()->mimeType());

    QMimeDatabase db;
    auto mimeType = db.mimeTypeForName(partToAttach->contentType()->mimeType());
    map.insert("iconname", mimeType.iconName());

    if (partToAttach->contentDescription(false)) {
        map.insert("description", partToAttach->contentDescription()->asUnicodeString());
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
        map.insert("inline", partToAttach->contentDisposition()->disposition() == KMime::Headers::CDinline);
    }

    if (name.isEmpty() && !filename.isEmpty()) {
        name = filename;
    }
    if (filename.isEmpty() && !name.isEmpty()) {
        filename = name;
    }

    if (!filename.isEmpty()) {
        map.insert("filename", filename);
    }
    if (!name.isEmpty()) {
        map.insert("name", name);
    }
    attachmentsController()->add(map);
}

void ComposerController::setMessage(const KMime::Message::Ptr &msg)
{
    static_cast<AddresseeController*>(toController())->set(getStringListFromAddresses(msg->to(true)->mailboxes()));
    static_cast<AddresseeController*>(ccController())->set(getStringListFromAddresses(msg->cc(true)->mailboxes()));
    static_cast<AddresseeController*>(bccController())->set(getStringListFromAddresses(msg->bcc(true)->mailboxes()));

    setSubject(msg->subject(true)->asUnicodeString());
    bool isHtml = false;
    const auto body = MailTemplates::body(msg, isHtml);
    setHtmlBody(isHtml);
    setBody(body);

    //TODO use ObjecTreeParser to get encrypted attachments as well
    foreach (const auto &att, msg->attachments()) {
        addAttachmentPart(att);
    }

    setExistingMessage(msg);
    emit messageLoaded(body);
}

void ComposerController::loadDraft(const QVariant &message) {
    clear();
    loadMessage(message, [this] (const KMime::Message::Ptr &mail) {
        setEncrypt(KMime::isEncrypted(mail.data()));
        setSign(KMime::isSigned(mail.data()));
        mRemoveDraft = true;
        setMessage(mail);
    });
}

void ComposerController::selectIdentityFromMailboxes(const KMime::Types::Mailbox::List &mailboxes, const QVector<QString> &meStrings)
{
    for (const auto &mb : mailboxes) {
        const auto address = mb.addrSpec().asString();
        if (meStrings.contains(address)) {
            static_cast<IdentitySelector*>(mIdentitySelector.data())->setCurrentIdentity(address);
            return;
        }
    }
}

void ComposerController::loadReply(const QVariant &message) {
    clear();
    auto guard = QPointer<QObject>{this};
    loadMessage(message, [this, guard] (const KMime::Message::Ptr &mail) {
        Q_ASSERT(guard);
        //Find all personal email addresses to exclude from reply
        KMime::Types::AddrSpecList me;
        QVector<QString> meStrings;
        auto list = static_cast<IdentitySelector*>(mIdentitySelector.data())->getAllAddresses();
        for (const auto &a : list) {
            KMime::Types::Mailbox mb;
            mb.setAddress(a);
            me << mb.addrSpec();
            meStrings << a;
        }

        selectIdentityFromMailboxes(mail->to()->mailboxes() + mail->cc()->mailboxes() + mail->bcc()->mailboxes(), meStrings);

        setEncrypt(KMime::isEncrypted(mail.data()));
        setSign(KMime::isSigned(mail.data()));
        MailTemplates::reply(mail, [this, guard] (const auto &msg) {
            Q_ASSERT(guard);
            setMessage(msg);
        }, me);
    });
}

void ComposerController::loadForward(const QVariant &message) {
    clear();
    loadMessage(message, [this] (const KMime::Message::Ptr &mail) {
        setEncrypt(KMime::isEncrypted(mail.data()));
        setSign(KMime::isSigned(mail.data()));
        MailTemplates::forward(mail, [this] (const auto &msg) {
            setMessage(msg);
        });
    });
}

void ComposerController::loadMessage(const QVariant &message, std::function<void(const KMime::Message::Ptr&)> callback)
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    auto msg = message.value<Mail::Ptr>();
    Q_ASSERT(msg);
    Query query(*msg);
    query.request<Mail::MimeMessage>();
    query.request<Mail::Draft>();
    setLoading(true);
    Store::fetchOne<Mail>(query).then([this, callback](const Mail &mail) {
        setExistingMail(mail);
        setLoading(false);

        const auto mailData = KMime::CRLFtoLF(mail.getMimeMessage());
        if (!mailData.isEmpty()) {
            KMime::Message::Ptr mail(new KMime::Message);
            mail->setContent(mailData);
            mail->parse();
            callback(mail);
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

std::vector<Crypto::Key> ComposerController::getRecipientKeys()
{
    std::vector<Crypto::Key> keys;
    {
        const auto list = toController()->getList<std::vector<Crypto::Key>>("key");
        for (const auto &l: list) {
            keys.insert(std::end(keys), std::begin(l), std::end(l));
        }
    }
    {
        const auto list = ccController()->getList<std::vector<Crypto::Key>>("key");
        for (const auto &l: list) {
            keys.insert(std::end(keys), std::begin(l), std::end(l));
        }
    }
    {
        const auto list = bccController()->getList<std::vector<Crypto::Key>>("key");
        for (const auto &l: list) {
            keys.insert(std::end(keys), std::begin(l), std::end(l));
        }
    }
    return keys;
}

KMime::Message::Ptr ComposerController::assembleMessage()
{
    auto toAddresses = toController()->getList<QString>("name");
    auto ccAddresses = ccController()->getList<QString>("name");
    auto bccAddresses = bccController()->getList<QString>("name");
    applyAddresses(toAddresses + ccAddresses + bccAddresses, [&](const QByteArray &addrSpec, const QByteArray &displayName) {
        recordForAutocompletion(addrSpec, displayName);
    });

    QList<Attachment> attachments;
    attachmentsController()->traverse([&](const QVariantMap &value) {
        attachments << Attachment{
            value["name"].toString(),
            value["filename"].toString(),
            value["mimetype"].toByteArray(),
            value["inline"].toBool(),
            value["content"].toByteArray()
        };
    });

    Crypto::Key attachedKey;
    std::vector<Crypto::Key> signingKeys;
    if (getSign()) {
        signingKeys = getPersonalKeys().value<std::vector<Crypto::Key>>();
        Q_ASSERT(!signingKeys.empty());
        attachedKey = signingKeys[0];
    }
    std::vector<Crypto::Key> encryptionKeys;
    if (getEncrypt()) {
        //Encrypt to self so we can read the sent message
        auto personalKeys = getPersonalKeys().value<std::vector<Crypto::Key>>();

        attachedKey = personalKeys[0];

        encryptionKeys += personalKeys;
        encryptionKeys += getRecipientKeys();
    }

    return MailTemplates::createMessage(mExistingMessage, toAddresses, ccAddresses, bccAddresses, getIdentity(), getSubject(), getBody(), getHtmlBody(), attachments, signingKeys, encryptionKeys, attachedKey);
}

void ComposerController::send()
{
    auto message = assembleMessage();
    if (!message) {
        SinkWarning() << "Failed to assemble the message.";
        return;
    }

    auto accountId = getAccountId();
    Q_ASSERT(!accountId.isEmpty());
    if (accountId.isEmpty()) {
        SinkWarning() << "No account id.";
        return;
    }
    auto job = SinkUtils::sendMail(message->encodedContent(true), accountId.toUtf8())
        .then([&] (const KAsync::Error &error) {
            if (!error) {
                if (mRemoveDraft) {
                    SinkLog() << "Removing draft message.";
                    Sink::Store::remove(getExistingMail()).exec();
                }
            }
            emit done();
        });

    run(job);
}

void ComposerController::saveAsDraft()
{
    SinkLog() << "Save as draft";
    const auto accountId = getAccountId();
    Q_ASSERT(!accountId.isEmpty());
    if (accountId.isEmpty()) {
        SinkWarning() << "No account id.";
        return;
    }
    auto existingMail = getExistingMail();

    auto message = assembleMessage();
    if (!message) {
        SinkWarning() << "Failed to assemble the message.";
        return;
    }

    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    auto job = [&] {
        if (existingMail.identifier().isEmpty() || !existingMail.getDraft()) {
            SinkLog() << "Creating a new draft" << existingMail.identifier() << "in account" << accountId;
            Query query;
            query.containsFilter<SinkResource::Capabilities>(ResourceCapabilities::Mail::drafts);
            query.filter<SinkResource::Account>(accountId.toLatin1());
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

#include "composercontroller.moc"
