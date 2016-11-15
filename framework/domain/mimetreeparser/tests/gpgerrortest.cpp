/*
    Copyright (c) 2016 Sandro Knauß <knauss@kolabsystems.com>

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

#include "interface.h"
#include "interface_p.h"

#include <QGpgME/Protocol>
#include <gpgme++/context.h>
#include <gpgme++/engineinfo.h>
#include <gpgme.h>

#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QTest>

QByteArray readMailFromFile(const QString &mailFile)
{
    QFile file(QLatin1String(MAIL_DATA_DIR) + QLatin1Char('/') + mailFile);
    file.open(QIODevice::ReadOnly);
    Q_ASSERT(file.isOpen());
    return file.readAll();
}

void killAgent(const QString& dir)
{
    QProcess proc;
    proc.setProgram(QStringLiteral("gpg-connect-agent"));
    QStringList arguments;
    arguments << "-S " << dir + "/S.gpg-agent";
    proc.start();
    proc.waitForStarted();
    proc.write("KILLAGENT\n");
    proc.write("BYE\n");
    proc.closeWriteChannel();
    proc.waitForFinished();
}

class GpgErrorTest : public QObject
{
    Q_OBJECT

private slots:

    void testGpgConfiguredCorrectly()
    {
        setEnv("GNUPGHOME", GNUPGHOME);

        Parser parser(readMailFromFile("openpgp-inline-charset-encrypted.mbox"));

        auto contentPartList = parser.collectContentParts();
        QCOMPARE(contentPartList.size(), 1);
        auto contentPart = contentPartList[0];
        QCOMPARE(contentPart->availableContents(),  QVector<QByteArray>() << "plaintext");
        auto contentList = contentPart->content("plaintext");
        QVERIFY(contentList[0]->content().startsWith("asdasd"));
        QCOMPARE(contentList[0]->encryptions().size(), 1);
        QCOMPARE(contentList[0]->signatures().size(), 1);
    }

    void testNoGPGInstalled_data()
    {
        QTest::addColumn<QString>("mailFileName");

        QTest::newRow("openpgp-inline-charset-encrypted") << "openpgp-inline-charset-encrypted.mbox";
        QTest::newRow("openpgp-encrypted-attachment-and-non-encrypted-attachment") << "openpgp-encrypted-attachment-and-non-encrypted-attachment.mbox";
        QTest::newRow("smime-encrypted") << "smime-encrypted.mbox";
    }

    void testNoGPGInstalled()
    {
        QFETCH(QString, mailFileName);

        setEnv("PATH", "/nonexististing");
        setGpgMEfname("/nonexisting/gpg", "");

        Parser parser(readMailFromFile(mailFileName));
        auto contentPartList = parser.collectContentParts();

        QCOMPARE(contentPartList.size(), 1);
        auto contentPart = contentPartList[0];
        QCOMPARE(contentPart->availableContents(),  QVector<QByteArray>() << "plaintext");
        auto contentList = contentPart->content("plaintext");
        QCOMPARE(contentList[0]->encryptions().size(), 1);
        QCOMPARE(contentList[0]->signatures().size(), 0);
        QVERIFY(contentList[0]->content().isEmpty());
    }

    void testGpgIncorrectGPGHOME_data()
    {
        QTest::addColumn<QString>("mailFileName");

        QTest::newRow("openpgp-inline-charset-encrypted") << "openpgp-inline-charset-encrypted.mbox";
        QTest::newRow("openpgp-encrypted-attachment-and-non-encrypted-attachment") << "openpgp-encrypted-attachment-and-non-encrypted-attachment.mbox";
        QTest::newRow("smime-encrypted") << "smime-encrypted.mbox";
    }

    void testGpgIncorrectGPGHOME()
    {
        QFETCH(QString, mailFileName);
        setEnv("GNUPGHOME", QByteArray(GNUPGHOME) + QByteArray("noexist"));

        Parser parser(readMailFromFile(mailFileName));

        auto contentPartList = parser.collectContentParts();
        QCOMPARE(contentPartList.size(), 1);
        auto contentPart = contentPartList[0];
        QCOMPARE(contentPart->availableContents(),  QVector<QByteArray>() << "plaintext");
        auto contentList = contentPart->content("plaintext");
        QCOMPARE(contentList[0]->encryptions().size(), 1);
        QCOMPARE(contentList[0]->signatures().size(), 0);
        QVERIFY(contentList[0]->content().isEmpty());
    }

public Q_SLOTS:
    void init()
    {
        mResetGpgmeEngine = false;
        mModifiedEnv.clear();
        {
            QGpgME::openpgp();      // We need to intialize it, otherwise ctx will be a nullpointer
            const GpgME::Context *ctx = GpgME::Context::createForProtocol(GpgME::Protocol::OpenPGP);
            const auto engineinfo = ctx->engineInfo();
            mGpgmeEngine_fname = engineinfo.fileName();
        }
        mEnv = QProcessEnvironment::systemEnvironment();
        unsetEnv("GNUPGHOME");
    }

    void cleanup()
    {
        QCoreApplication::sendPostedEvents();

        const QString &gnupghome = qgetenv("GNUPGHOME");
        if (!gnupghome.isEmpty()) {
            killAgent(gnupghome);
        }

        resetGpgMfname();
        resetEnv();
    }
private:
    void unsetEnv(const QByteArray &name) 
    {
        mModifiedEnv << name;
        unsetenv(name);
    }

    void setEnv(const QByteArray &name, const QByteArray &value)
    {
        mModifiedEnv << name;
        setenv(name, value , 1);
    }

    void resetEnv()
    {
        foreach(const auto &i, mModifiedEnv) {
            if (mEnv.contains(i)) {
                setenv(i, mEnv.value(i).toUtf8(), 1);
            } else {
                unsetenv(i);
            }
        }
    }

    void resetGpgMfname()
    {
        if (mResetGpgmeEngine) {
            gpgme_set_engine_info (GPGME_PROTOCOL_OpenPGP, mGpgmeEngine_fname, NULL);
        }
    }

    void setGpgMEfname(const QByteArray &fname, const QByteArray &homedir)
    {
        mResetGpgmeEngine = true;
        gpgme_set_engine_info (GPGME_PROTOCOL_OpenPGP, fname, homedir);
    }

    QSet<QByteArray> mModifiedEnv;
    QProcessEnvironment mEnv;
    bool mResetGpgmeEngine;
    QByteArray mGpgmeEngine_fname;
};

QTEST_GUILESS_MAIN(GpgErrorTest)
#include "gpgerrortest.moc"
