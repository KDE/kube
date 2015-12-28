/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#ifndef CRYPTOBODYPARTMEMENTO_H
#define CRYPTOBODYPARTMEMENTO_H

#include <gpgme++/error.h>

#include <QObject>
#include <QString>

#include "interfaces/bodypart.h"
#include "messageviewer/viewer.h"

namespace MessageViewer
{

class CryptoBodyPartMemento
    : public QObject,
      public Interface::BodyPartMemento
{
    Q_OBJECT
public:
    CryptoBodyPartMemento();
    ~CryptoBodyPartMemento();

    virtual bool start() = 0;
    virtual void exec() = 0;
    bool isRunning() const;

    const QString &auditLogAsHtml() const
    {
        return m_auditLog;
    }
    GpgME::Error auditLogError() const
    {
        return m_auditLogError;
    }

    void detach() Q_DECL_OVERRIDE;

Q_SIGNALS:
    void update(MessageViewer::Viewer::UpdateMode);

protected Q_SLOTS:
    void notify()
    {
        Q_EMIT update(Viewer::Force);
    }

protected:
    void setAuditLog(const GpgME::Error &err, const QString &log);
    void setRunning(bool running);

private:
    bool m_running;
    QString m_auditLog;
    GpgME::Error m_auditLogError;
};
}
#endif // CRYPTOBODYPARTMEMENTO_H
