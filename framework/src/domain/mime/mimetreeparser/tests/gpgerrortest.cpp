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

#include <objecttreeparser.h>

#include <gpgme.h>

#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QTest>
#include <QtGlobal>

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

        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile("openpgp-inline-charset-encrypted.mbox"));
        otp.print();
        otp.decryptParts();
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0];
        QVERIFY(bool(part));

        QVERIFY(part->text().startsWith("asdasd"));
        QCOMPARE(part->encryptions().size(), 1);
        auto enc = part->encryptions()[0];
        QCOMPARE(enc->error(),  MimeTreeParser::MessagePart::NoError);
        // QCOMPARE((int) enc->recipients().size(), 2);
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

        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile(mailFileName));
        otp.print();
        otp.decryptParts();
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QVERIFY(bool(part));

        QCOMPARE(part->encryptions().size(), 1);
        QVERIFY(part->text().isEmpty());
        auto enc = part->encryptions()[0];
        QCOMPARE(enc->error(), MimeTreeParser::MessagePart::NoKeyError);
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

        MimeTreeParser::ObjectTreeParser otp;
        otp.parseObjectTree(readMailFromFile(mailFileName));
        otp.print();
        otp.decryptParts();
        otp.print();
        auto partList = otp.collectContentParts();
        QCOMPARE(partList.size(), 1);
        auto part = partList[0].dynamicCast<MimeTreeParser::MessagePart>();
        QVERIFY(bool(part));

        QCOMPARE(part->encryptions().size(), 1);
        QCOMPARE(part->signatures().size(), 0);
        QVERIFY(part->text().isEmpty());
        auto enc = part->encryptions()[0];
        QCOMPARE(enc->error(), MimeTreeParser::MessagePart::NoKeyError);
        // QCOMPARE((int) enc->recipients().size(), 2);
    }

public Q_SLOTS:
    void init()
    {
        mResetGpgmeEngine = false;
        mModifiedEnv.clear();
        {

            gpgme_check_version(0);
            gpgme_ctx_t ctx = 0;
            gpgme_new(&ctx);
            gpgme_set_protocol(ctx, GPGME_PROTOCOL_OpenPGP);
            gpgme_engine_info_t info = gpgme_ctx_get_engine_info(ctx);
            mGpgmeEngine_fname = info->file_name;
            gpgme_release(ctx);
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
        qunsetenv(name);
    }

    void setEnv(const QByteArray &name, const QByteArray &value)
    {
        mModifiedEnv << name;
        qputenv(name, value);
    }

    void resetEnv()
    {
        foreach(const auto &i, mModifiedEnv) {
            if (mEnv.contains(i)) {
                qputenv(i, mEnv.value(i).toUtf8());
            } else {
                qunsetenv(i);
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
