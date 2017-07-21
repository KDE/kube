/*
  Copyright (C) 2017 Michael Bohlender, <bohlender@kolabsys.com>

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

pragma Singleton

import QtQuick 2.7

Item {
    //Colorscheme:
    //See https://community.kde.org/KDE_Visual_Design_Group/HIG/Color
    property string paperWhite: "#fcfcfc"
    property string abyssBlue: "#2980b9"
    property string charcoalGrey: "#31363b"
    property string coastalFog: "#7f8c8d"
    property string cardboardGrey: "#eff0f1"
    property string plasmaBlue: "#3daee9"
    property string alternateGrey: "#bdc3c7"
    property string nobleFir: "#27ae60" //Green
    property string bewareOrange: "#f67400"
    property string dangerRed: "#ed1515"
    property string darkCharcoalGrey: "#232629"

    //Colorusage:
    property string textColor: charcoalGrey
    property string disabledTextColor: coastalFog
    property string backgroundColor: cardboardGrey
    property string viewBackgroundColor: paperWhite
    property string highlightColor: plasmaBlue
    property string highlightedTextColor: paperWhite
    property string buttonColor: alternateGrey
    property string positiveColor: nobleFir
    property string warningColor: bewareOrange
    property string negativeColor: dangerRed
    property string statusbarColor: darkCharcoalGrey
    property string focusedButtonColor: abyssBlue

}

