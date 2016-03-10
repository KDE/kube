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

#include "colorpalette.h"

ColorPalette::ColorPalette(QObject *parent) : QObject(parent), m_background("#fcfcfc"), m_selected("#3daee9"), m_read("#232629"), m_border("#232629")
{

}

QString ColorPalette::background() const
{
    return m_background;
}

QString ColorPalette::read() const
{
    return m_read;
}

QString ColorPalette::selected() const
{
    return m_selected;
}

QString ColorPalette::border() const
{
    return m_border;
}

