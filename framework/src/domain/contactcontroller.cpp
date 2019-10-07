/*
 *  Copyright (C) 2017 Michael Bohlender, <michael.bohlender@kdemail.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "contactcontroller.h"

#include <sink/applicationdomaintype.h>
#include <sink/store.h>
#include <sink/log.h>
#include <KContacts/VCardConverter>

using namespace Sink::ApplicationDomain;

class MailsController : public Kube::ListPropertyController
{
public:

    MailsController()
        : Kube::ListPropertyController{{"email", "isMain"}}
    {
    }

    void set(const QStringList &list)
    {
        for (const auto &email: list) {
            add({{"email", email}, {"isMain", false}});
        }
    }

    QList<QString> get()
    {
        return getList<QString>("email");
    }
};

class PhonesController : public Kube::ListPropertyController
{
public:

    PhonesController()
        : Kube::ListPropertyController{{"number"}}
    {
    }

    void set(const QStringList &list)
    {
        for (const auto &number: list) {
            add({{"number", number}});
        }
    }

    QList<QString> get()
    {
        return getList<QString>("number");
    }
};

ContactController::ContactController()
    : Kube::Controller(),
    controller_mails{new MailsController},
    controller_phones(new PhonesController),
    action_save{new Kube::ControllerAction{this, &ContactController::save}}
{
    updateSaveAction();
}

void ContactController::save()
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    const auto addressbook = getAddressbook();
    if (!addressbook) {
        qWarning() << "No addressbook selected";
        return;
    }

    auto populateAddressee = [this] (KContacts::Addressee &addressee) {
        addressee.setGivenName(getFirstName());
        addressee.setFamilyName(getLastName());
        addressee.setFormattedName(getFirstName() + " " + getLastName());
        addressee.setEmails(static_cast<MailsController*>(mailsController())->get());
        //TODO phone numbers, addresses, ...
    };

    if (auto c = mContact.value<Sink::ApplicationDomain::Contact::Ptr>()) {
        Contact contact = *c;

        //Apply the changed properties on top of what's existing
        KContacts::Addressee addressee = KContacts::VCardConverter{}.parseVCard(contact.getVcard());
        populateAddressee(addressee);

        contact.setVcard(KContacts::VCardConverter{}.createVCard(addressee, KContacts::VCardConverter::v3_0));
        contact.setAddressbook(*addressbook);

        auto job = Store::modify(contact)
            .then([&] (const KAsync::Error &error) {
                if (error) {
                    SinkWarning() << "Failed to save the contact: " << error;
                }
                emit done();
            });

        run(job);

    } else {
        Contact contact(addressbook->resourceInstanceIdentifier());

        KContacts::Addressee addressee;
        populateAddressee(addressee);

        contact.setVcard(KContacts::VCardConverter{}.createVCard(addressee, KContacts::VCardConverter::v3_0));
        contact.setAddressbook(*addressbook);

        auto job = Store::create(contact)
            .then([&] (const KAsync::Error &error) {
                if (error) {
                    SinkWarning() << "Failed to save the contact: " << error;
                }
                emit done();
            });

        run(job);
    }
}

void ContactController::updateSaveAction()
{
    saveAction()->setEnabled(!getFirstName().isEmpty());
}

void ContactController::loadContact(const QVariant &variant)
{
    using namespace Sink;

    mContact = variant;
    if (auto c = variant.value<ApplicationDomain::Contact::Ptr>()) {

        setAddressbook(ApplicationDomainType::Ptr::create(ApplicationDomainType::createEntity<ApplicationDomain::Addressbook>(c->resourceInstanceIdentifier(), c->getAddressbook())));
        const auto addressee = KContacts::VCardConverter{}.parseVCard(c->getVcard());

        setName(c->getFn());
        setFirstName(addressee.givenName());
        setLastName(addressee.familyName());

        static_cast<MailsController*>(mailsController())->set(addressee.emails());

        QStringList numbers;
        for (const auto &n : addressee.phoneNumbers()) {
            numbers << n.number();
        }
        static_cast<PhonesController*>(phonesController())->set(numbers);

        for(const auto &a :addressee.addresses()) {
            setStreet(a.street());
            setCity(a.locality());
            setCountry(a.country());
            break;
        }
        setCompany(addressee.organization());
        setJobTitle(addressee.role());
        setImageData(addressee.photo().rawData());
    }
}

void ContactController::loadByEmail(const QString &email)
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    static_cast<MailsController*>(mailsController())->set({email});

    //TODO query for contact by email address
    //We probably can't atm., perhaps just load all and filter in memory for the time being?
    // Query query;
    // query.request<Contact::Uid>();
    // query.request<Contact::VCard>();
    // query.request<Contact::Emails>();
    // query.filter<Contact::Email>(icalEvent->uid().toUtf8());
    // Store::fetchAll<Event>(query).then([this](const QList<Contact::Ptr> &contacts) {
    //     if (events.isEmpty()) {
    //         setState(InvitationState::Unknown);
    //         populateFromEvent(*icalEvent);
    //         setStart(icalEvent->dtStart());
    //         setEnd(icalEvent->dtEnd());
    //         setUid(icalEvent->uid().toUtf8());
    //         return KAsync::null();
    //     }

}

void ContactController::remove()
{
    if (auto c = mContact.value<Sink::ApplicationDomain::Contact::Ptr>()) {
        run(Sink::Store::remove(*c));
    }
}

QVariant ContactController::contact() const
{
    return mContact;
}

QString ContactController::emailAddress() const
{
    return {};
}
