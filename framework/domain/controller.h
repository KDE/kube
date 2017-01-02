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
#pragma once

#include <QObject>
#include <QVariant>
#include <Async/Async>

#define KUBE_CONTROLLER_PROPERTY(TYPE, NAME, LOWERCASENAME) \
    public: Q_PROPERTY(TYPE LOWERCASENAME MEMBER m##NAME NOTIFY LOWERCASENAME##Changed) \
    Q_SIGNALS: void LOWERCASENAME##Changed(); \
    private: TYPE m##NAME; \
    public: \
    struct NAME { \
        static constexpr const char *name = #LOWERCASENAME; \
        typedef TYPE Type; \
    }; \
    void set##NAME(const TYPE &value) { setProperty(NAME::name, QVariant::fromValue(value)); } \
    void clear##NAME() { setProperty(NAME::name, QVariant{}); } \
    TYPE get##NAME() const { return m##NAME; } \


#define KUBE_CONTROLLER_ACTION(NAME) \
    Q_PROPERTY (Kube::ControllerAction* NAME##Action READ NAME##Action CONSTANT) \
    private: QScopedPointer<Kube::ControllerAction> action_##NAME; \
    public: Kube::ControllerAction* NAME##Action() const { Q_ASSERT(action_##NAME); return action_##NAME.data(); } \
    private slots: void NAME(); \


namespace Kube {

class ControllerAction : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool enabled MEMBER mEnabled NOTIFY enabledChanged)
public:
    ControllerAction();
    template <typename Func>
    ControllerAction(const typename QtPrivate::FunctionPointer<Func>::Object *obj, Func slot)
    : ControllerAction()
    {
        QObject::connect(this, &ControllerAction::triggered, obj, slot);
    }

    ~ControllerAction() = default;

    Q_INVOKABLE void execute();
    void setEnabled(bool enabled) { setProperty("enabled", enabled); }

signals:
    void enabledChanged();
    void triggered();

private:
    bool mEnabled = true;
};

class Controller : public QObject {
    Q_OBJECT
public:
    Controller() = default;
    virtual ~Controller() = default;

public slots:
    void clear();

signals:
    void done();
    void error();

protected:
    void run(const KAsync::Job<void> &job);
};

}
