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

class MailsController : public Kube::ListPropertyController
{
public:

    MailsController()
        : Kube::ListPropertyController{{"email"}}
    {
    }

    void set(const QStringList &list)
    {
        for (const auto &email: list) {
            add({{"email", email}});
        }
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
    Query query;
    query.containsFilter<SinkResource::Capabilities>(ResourceCapabilities::Contact::storage);
    auto job = Store::fetchAll<SinkResource>(query)
        .then([=](const QList<SinkResource::Ptr> &resources) {
            if (!resources.isEmpty()) {
                auto resourceId = resources[0]->identifier();
                KContacts::Addressee addressee;
                addressee.setGivenName(getFirstName());
                addressee.setFamilyName(getLastName());
                addressee.setFormattedName(getFirstName() + " " + getLastName());
                KContacts::VCardConverter converter;
                const auto vcard = converter.createVCard(addressee, KContacts::VCardConverter::v3_0);

                Contact contact(resourceId);
                contact.setVcard(vcard);

                return Store::create(contact);
            }
            SinkWarning() << "Failed to find a resource for the contact";
            return KAsync::error<void>(0, "Failed to find a contact resource.");
        })
        .then([&] (const KAsync::Error &error) {
            SinkLog() << "Failed to save the contact: " << error;
            emit done();
        });
    run(job);
}

void ContactController::updateSaveAction()
{
    saveAction()->setEnabled(!getFirstName().isEmpty());
}

void ContactController::loadContact(const QVariant &contact)
{
    if (auto c = contact.value<Sink::ApplicationDomain::Contact::Ptr>()) {
        const auto &vcard = c->getVcard();
        KContacts::VCardConverter converter;
        const auto addressee = converter.parseVCard(vcard);

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

QVariant ContactController::contact() const
{
    return QVariant{};
}
