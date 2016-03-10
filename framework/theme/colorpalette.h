/*
  Copyright (C) 2016 Michael Bohlender <michael.bohlender@kdemail.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <QObject>
#include <QString>

class ColorPalette : public QObject
{
    Q_OBJECT

    Q_PROPERTY (QString background READ background NOTIFY themeChanged)
    Q_PROPERTY (QString selected READ background NOTIFY themeChanged)
    Q_PROPERTY (QString read READ read NOTIFY themeChanged)
    Q_PROPERTY (QString border READ border NOTIFY themeChanged)

public:
    explicit ColorPalette(QObject *parent = Q_NULLPTR);

    QString background() const;
    QString selected() const;
    QString read() const;
    QString border() const;

signals:
    void themeChanged();

private:
    QString m_background;
    QString m_selected;
    QString m_read;
    QString m_border;
};
