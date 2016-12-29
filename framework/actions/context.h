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

#include <QObject>

#define KUBE_CONTEXT_PROPERTY(TYPE, NAME, LOWERCASENAME) \
    public: Q_PROPERTY(TYPE LOWERCASENAME MEMBER m##NAME NOTIFY LOWERCASENAME##Changed) \
    struct NAME { \
        static constexpr const char *name = #LOWERCASENAME; \
        typedef TYPE Type; \
    }; \
    void set##NAME(const TYPE &value) { setProperty(NAME::name, QVariant::fromValue(value)); } \
    TYPE get##NAME() const { return m##NAME; } \
    Q_SIGNALS: void LOWERCASENAME##Changed(); \
    private: TYPE m##NAME;


namespace Kube {

class Context : public QObject {
    Q_OBJECT
public:
    Context(QObject *parent = 0);
    virtual ~Context(){};
    virtual void clear();
};

}

QDebug operator<<(QDebug dbg, const Kube::Context &);

Q_DECLARE_METATYPE(Kube::Context*);

