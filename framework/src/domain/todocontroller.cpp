/*
 *  Copyright (C) 2017 Michael Bohldueer, <michael.bohldueer@kdemail.net>
 *  Copyright (C) 2018 Christian Mollekopf, <mollekopf@kolabsys.com>
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

#include "todocontroller.h"

#include <sink/applicationdomaintype.h>
#include <sink/store.h>
#include <sink/log.h>

#include <KCalendarCore/ICalFormat>
#include <KCalendarCore/Todo>
#include <QUuid>

using namespace Sink::ApplicationDomain;

TodoController::TodoController()
    : Kube::Controller(),
    action_save{new Kube::ControllerAction{this, &TodoController::save}}
{
    updateSaveAction();
}

void TodoController::save()
{
    using namespace Sink;
    using namespace Sink::ApplicationDomain;

    const auto calendar = getCalendar();
    if (!calendar) {
        qWarning() << "No calendar selected";
        return;
    }

    auto populateTodo = [this](KCalendarCore::Todo& todo) {
        todo.setSummary(getSummary());
        todo.setRelatedTo(getParentUid());
        todo.setDescription(getDescription());
        todo.setDtStart(getStart());
        todo.setDtDue(getDue());
        if (getComplete()) {
            todo.setCompleted(true);
        } else if (getDoing()) {
            todo.setCompleted(false);
            todo.setStatus(KCalendarCore::Incidence::StatusInProcess);
        } else {
            todo.setCompleted(false);
        }
    };

    if (auto e = mTodo.value<Sink::ApplicationDomain::Todo::Ptr>()) {
        Todo todo = *e;

        //Apply the changed properties on top of what's existing
        auto calcoreTodo = KCalendarCore::ICalFormat().readIncidence(todo.getIcal()).dynamicCast<KCalendarCore::Todo>();
        if(!calcoreTodo) {
            SinkWarning() << "Invalid ICal to process, ignoring...";
            return;
        }
        populateTodo(*calcoreTodo);

        todo.setIcal(KCalendarCore::ICalFormat().toICalString(calcoreTodo).toUtf8());
        todo.setCalendar(*calendar);

        auto job = Store::modify(todo)
            .then([&] (const KAsync::Error &error) {
                if (error) {
                    SinkWarning() << "Failed to save the todo: " << error;
                }
                emit done();
            });

        run(job);
    } else {
        Todo todo(calendar->resourceInstanceIdentifier());

        auto calcoreTodo = QSharedPointer<KCalendarCore::Todo>::create();
        calcoreTodo->setUid(QUuid::createUuid().toString());
        populateTodo(*calcoreTodo);

        todo.setIcal(KCalendarCore::ICalFormat().toICalString(calcoreTodo).toUtf8());
        todo.setCalendar(*calendar);

        auto job = Store::create(todo)
            .then([&] (const KAsync::Error &error) {
                if (error) {
                    SinkWarning() << "Failed to save the todo: " << error;
                }
                emit done();
            });

        run(job);
    }
}

void TodoController::updateSaveAction()
{
    saveAction()->setEnabled(!getSummary().isEmpty());
}

void TodoController::reload()
{
    loadTodo(mTodo);
}

void TodoController::loadTodo(const QVariant &variant)
{
    using namespace Sink;

    mTodo = variant;
    if (auto todo = variant.value<ApplicationDomain::Todo::Ptr>()) {
        setCalendar(ApplicationDomainType::Ptr::create(ApplicationDomainType::createEntity<ApplicationDomain::Calendar>(todo->resourceInstanceIdentifier(), todo->getCalendar())));
        setCalendarId(todo->getCalendar());

        auto icalTodo = KCalendarCore::ICalFormat().readIncidence(todo->getIcal()).dynamicCast<KCalendarCore::Todo>();
        if(!icalTodo) {
            SinkWarning() << "Invalid ICal to process, ignoring...";
            return;
        }
        setUid(icalTodo->uid().toUtf8());
        setParentUid(icalTodo->relatedTo().toUtf8());
        setSummary(icalTodo->summary());
        setDescription(icalTodo->description());
        setLocation(icalTodo->location());
        setStart(icalTodo->dtStart());
        setDue(icalTodo->dtDue());
        if (icalTodo->status() == KCalendarCore::Incidence::StatusCompleted) {
            setComplete(true);
            setDoing(false);
        } else if (icalTodo->status() == KCalendarCore::Incidence::StatusInProcess) {
            setComplete(false);
            setDoing(true);
        } else {
            setComplete(false);
            setDoing(false);
        }
    } else {
        qWarning() << "Not a todo" << variant;
    }
    setModified(false);
}

void TodoController::remove()
{
    if (auto c = mTodo.value<Sink::ApplicationDomain::Todo::Ptr>()) {
        run(Sink::Store::remove(*c));
    }
}

QVariant TodoController::getTodo() const
{
    return mTodo;
}
