#include <QTest>
#include <QDebug>
#include <QSignalSpy>

#include <actions/action.h>
#include <actions/context.h>
#include <actions/actionhandler.h>

#include <sink/log.h>

SINK_DEBUG_AREA("actiontest")

class HandlerContext : public Kube::Context {
    Q_OBJECT
    KUBE_CONTEXT_PROPERTY(QString, Property1, property1)
    KUBE_CONTEXT_PROPERTY(QString, Property2, property2)
};

class HandlerContextWrapper : public Kube::ContextWrapper {
    using Kube::ContextWrapper::ContextWrapper;
    KUBE_CONTEXTWRAPPER_PROPERTY(QString, Property1, property1)
    KUBE_CONTEXTWRAPPER_PROPERTY(QString, Property2, property2)
};



class Handler : public Kube::ActionHandlerBase<HandlerContextWrapper>
{
public:
    Handler() : Kube::ActionHandlerBase<HandlerContextWrapper>{"org.kde.kube.test.action1"}
    {}

    //TODO default implementation checks that all defined properties are available in the context
    // bool isReady() override {
    //     auto accountId = context->property("accountId").value<QByteArray>();
    //     return !accountId.isEmpty();
    // }

    KAsync::Job<void> execute(HandlerContextWrapper &context)
    {
        SinkLog() << "Executing action1";
        SinkLog() << context;
        executions.append(context.context);
        return KAsync::null<void>();
    }
    mutable QList<Kube::Context> executions;
};

class Context1 : public Kube::ContextWrapper {
    using Kube::ContextWrapper::ContextWrapper;
    KUBE_CONTEXTWRAPPER_PROPERTY(QString, Property1, property1)
    KUBE_CONTEXTWRAPPER_PROPERTY(QByteArray, Property2, property2)
};

class Context2 : public Kube::ContextWrapper {
    using Kube::ContextWrapper::ContextWrapper;
    KUBE_CONTEXTWRAPPER_PROPERTY(QByteArray, Property2, property2)
};


class ActionTest : public QObject
{
    Q_OBJECT
private slots:

    void initTestCase()
    {
    }

    void testActionExecution()
    {
        Handler actionHandler;

        HandlerContext context;
        //Kube::Context context;
        HandlerContextWrapper{context}.setProperty1(QString("property1"));
        context.setProperty("property2", QVariant::fromValue(QString("property2")));
        auto future = Kube::Action("org.kde.kube.test.action1", context).executeWithResult();

        QTRY_VERIFY(future.isDone());
        QVERIFY(!future.error());

        QCOMPARE(actionHandler.executions.size(), 1);
        QCOMPARE(actionHandler.executions.first().availableProperties().size(), 2);
    }

    void testContextCasting()
    {
        Kube::Context c;

        Context1 context1{c};
        context1.setProperty1("property1");
        context1.setProperty2("property2");

        auto context2 = Context2{c};
        QCOMPARE(context2.getProperty2(), QByteArray("property2"));
    }

};

QTEST_GUILESS_MAIN(ActionTest)
#include "actiontest.moc"
