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
#include "kube_export.h"
#include <QObject>
#include <QVariant>
#include <QAbstractItemModel>
#include <QStandardItemModel>
#include <QQmlParserStatus>
#include <KAsync/Async>

#define KUBE_CONTROLLER_PROPERTY(TYPE, NAME, LOWERCASENAME) \
    public: Q_PROPERTY(TYPE LOWERCASENAME READ get##NAME WRITE setInternal##NAME NOTIFY LOWERCASENAME##Changed) \
    Q_SIGNALS: void LOWERCASENAME##Changed(); \
    private: TYPE m##NAME; \
    public: \
    struct NAME { \
        static constexpr const char *name = #LOWERCASENAME; \
        typedef TYPE Type; \
    }; \
    void set##NAME(const TYPE &value) { setProperty(NAME::name, QVariant::fromValue(value)); } \
    void setInternal##NAME(const TYPE &value) { if (m##NAME != value) {m##NAME = value; emit LOWERCASENAME##Changed(); propertyChanged(NAME::name);} } \
    void clear##NAME() { setProperty(NAME::name, QVariant{}); } \
    TYPE get##NAME() const { return m##NAME; } \


#define KUBE_CONTROLLER_ACTION(NAME) \
    Q_PROPERTY (Kube::ControllerAction* NAME##Action READ NAME##Action CONSTANT) \
    private: QScopedPointer<Kube::ControllerAction> action_##NAME; \
    public: Kube::ControllerAction* NAME##Action() const { Q_ASSERT(action_##NAME); return action_##NAME.data(); } \
    private slots: void NAME(); \

#define KUBE_CONTROLLER_LISTCONTROLLER(NAME) \
    Q_PROPERTY (Kube::ListPropertyController* NAME READ NAME##Controller CONSTANT) \
    private: QScopedPointer<Kube::ListPropertyController> controller_##NAME; \
    public: Kube::ListPropertyController* NAME##Controller() const { Q_ASSERT(controller_##NAME); return controller_##NAME.data(); } \


namespace Kube {

class ControllerState : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool enabled MEMBER mEnabled NOTIFY enabledChanged)
public:
    ControllerState();
    ~ControllerState() = default;

    void setEnabled(bool enabled) { setProperty("enabled", enabled); }

signals:
    void enabledChanged();

private:
    bool mEnabled = false;
};

class KUBE_EXPORT ControllerAction : public ControllerState {
    Q_OBJECT
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

signals:
    void triggered();
};

class Controller : public QObject, public QQmlParserStatus {
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(bool modified READ modified WRITE setModified NOTIFY modifiedChanged)

public:
    Controller() = default;
    virtual ~Controller() = default;

    virtual void init();

    void classBegin() override;
    void componentComplete() override;
    bool modified() const;
    void setModified(bool);
    void propertyChanged(const QByteArray &);

public slots:
    virtual void clear();

signals:
    void done();
    void error();
    void cleared();
    void modifiedChanged();

protected:
    void run(const KAsync::Job<void> &job);

private:
    bool mModified{false};
};

class KUBE_EXPORT ListPropertyController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* model READ model CONSTANT)
    Q_PROPERTY(bool empty READ empty NOTIFY emptyChanged)

public:
    ListPropertyController(const QStringList &roles);
    Q_INVOKABLE virtual void add(const QVariantMap &value);
    Q_INVOKABLE virtual void remove(const QByteArray &id);
    Q_INVOKABLE void clear();

    QAbstractItemModel *model();

    void setValue(const QByteArray &id, const QString &key, const QVariant &);
    void setValues(const QByteArray &id, const QVariantMap &values);
    QVariant value(const QByteArray &id, const QString &key);
    void traverse(const std::function<void(const QVariantMap &)> &f);

    QByteArray findByProperty(const QByteArray &key, const QVariant &) const;

    template<typename T>
    QList<T> getList(const QString &property)
    {
        QList<T> list;
        traverse([&] (const QVariantMap &map) {
            list << map[property].value<T>();
        });
        return list;
    }

    bool empty() const;

Q_SIGNALS:
    void added(QByteArray, QVariantMap);
    void removed(QByteArray);
    void emptyChanged();

protected:
    QScopedPointer<QStandardItemModel> mModel;

private:
    QHash<QString, int> mRoles;
};


}
