/*
  Copyright (c) 2014-2016 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __MIMETREEPARSER_VERIFYOPAQUEBODYPARTMEMENTO_H__
#define __MIMETREEPARSER_VERIFYOPAQUEBODYPARTMEMENTO_H__

#include "cryptobodypartmemento.h"
#include <gpgme++/verificationresult.h>
#include <gpgme++/decryptionresult.h>
#include <gpgme++/key.h>

#include <QString>
#include <QPointer>

#include "bodypart.h"

namespace QGpgME
{
class VerifyOpaqueJob;
class KeyListJob;
}

class QStringList;

namespace MimeTreeParser
{

class VerifyOpaqueBodyPartMemento
    : public CryptoBodyPartMemento
{
    Q_OBJECT
public:
    VerifyOpaqueBodyPartMemento(QGpgME::VerifyOpaqueJob *job,
                                QGpgME::KeyListJob *klj,
                                const QByteArray &signature);
    ~VerifyOpaqueBodyPartMemento();

    bool start() Q_DECL_OVERRIDE;
    void exec() Q_DECL_OVERRIDE;

    const QByteArray &plainText() const
    {
        return m_plainText;
    }
    const GpgME::VerificationResult &verifyResult() const
    {
        return m_vr;
    }
    const GpgME::Key &signingKey() const
    {
        return m_key;
    }

private Q_SLOTS:
    void slotResult(const GpgME::VerificationResult &vr,
                    const QByteArray &plainText);
    void slotKeyListJobDone();
    void slotNextKey(const GpgME::Key &);

private:
    void saveResult(const GpgME::VerificationResult &,
                    const QByteArray &);
    bool canStartKeyListJob() const;
    QStringList keyListPattern() const;
    bool startKeyListJob();
private:
    // input:
    const QByteArray m_signature;
    QPointer<QGpgME::VerifyOpaqueJob> m_job;
    QPointer<QGpgME::KeyListJob> m_keylistjob;
    // output:
    GpgME::VerificationResult m_vr;
    QByteArray m_plainText;
    GpgME::Key m_key;
};

}

#endif // __MIMETREEPARSER_VERIFYOPAQUEBODYPARTMEMENTO_H__
