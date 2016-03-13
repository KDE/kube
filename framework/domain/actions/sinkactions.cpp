/*
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
#include <actions/context.h>
#include <actions/actionhandler.h>

#include <sink/store.h>

using namespace Kube;

static ActionHandlerHelper markAsReadHandler("org.kde.kube.actions.mark-as-read",
    [](Context *context) -> bool {
        return context->property("mail").isValid();
    },
    [](Context *context) {
        auto mail = context->property("mail").value<Sink::ApplicationDomain::Mail::Ptr>();
        if (!mail) {
            qWarning() << "Failed to get the mail mail: " << context->property("mail");
            return;
        }
        mail->setProperty("unread", false);
        qDebug() << "Mark as read " << mail->identifier();
        Sink::Store::modify(*mail).exec();
    }
);

static ActionHandlerHelper deleteHandler("org.kde.kube.actions.delete",
    [](Context *context) -> bool {
        return context->property("mail").isValid();
    },
    [](Context *context) {
        auto mail = context->property("mail").value<Sink::ApplicationDomain::Mail::Ptr>();
        if (!mail) {
            qWarning() << "Failed to get the mail mail: " << context->property("mail");
            return;
        }
        mail->setProperty("unread", false);
        qDebug() << "Remove " << mail->identifier();
        Sink::Store::remove(*mail).exec();
    }
);

static ActionHandlerHelper synchronizeHandler("org.kde.kube.actions.synchronize",
    [](Context *context) -> bool {
        return true;
    },
    [](Context *context) {
        if (auto folder = context->property("folder").value<Sink::ApplicationDomain::Folder::Ptr>()) {
            qDebug() << "Synchronizing resource " << folder->resourceInstanceIdentifier();
            Sink::Store::synchronize(Sink::Query::ResourceFilter(folder->resourceInstanceIdentifier())).exec();
        } else {
            qDebug() << "Synchronizing all";
            Sink::Store::synchronize(Sink::Query()).exec();
        }
    }
);

// static ActionHandlerHelper saveAsDraft("org.kde.kube.actions.save-as-draft",
//     [](Context *context) -> bool {
//         return context->property("mail").isValid();
//     },
//     [](Context *context) {
//         Sink::Query query;
//         query += Sink::Query::RequestedProperties(QByteArrayList() << "name")
//         //FIXME do something like specialuse?
//         query += Sink::Query::PropertyFilter("name", "Drafts");
//         // query += Sink::Query::PropertyContainsFilter("specialuser", "drafts");
//         query += Sink::Query::PropertyFilter("drafts", true);
//         //TODO Use drafts folder of that specific account
//         Sink::Store::fetchAll<Sink::ApplicationDomain::Folder>(query)
//             .then<void, QList<Sink::ApplicationDomain::Folder>>([](const QList<Sink::ApplicationDomain::Folder> folders) {
//                 if (folders.isEmpty()) {
//                     return KAsync::start([]() {
//                         //If message is already existing, modify, otherwise create
//                     });
//                 }
//             });
//         //TODO
//         // * Find drafts folder
//         // * Store KMime::Message on disk for use in blob property
//         // * Check if message is already existing and either create or update
//         // * 
//         // auto mail = context->property("mail").value<Sink::ApplicationDomain::Mail::Ptr>();
//         // if (!mail) {
//         //     qWarning() << "Failed to get the mail mail: " << context->property("mail");
//         //     return;
//         // }
//         // mail->setProperty("unread", false);
//         // qDebug() << "Mark as read " << mail->identifier();
//         // Sink::Store::modify(*mail).exec();
//     }
// );
