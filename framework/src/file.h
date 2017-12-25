/*
    Copyright (c) 2017 Christian Mollekopf <mollekopf@kolabsys.com>

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

#include <QObject>
#include <QString>

namespace Kube {

class File : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString path MEMBER mPath)
    Q_PROPERTY(QString defaultContent MEMBER mDefaultContent)
    Q_PROPERTY(QString data READ data CONSTANT)
public:
    QString data();
    Q_INVOKABLE QString read(const QString &path);
private:
    QString mPath;
    QString mDefaultContent;
};

}
