
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

#include "webengineprofile.h"

#include <QWebEngineUrlRequestInterceptor>
#include <QDebug>
#include <QDesktopServices>

class WebUrlRequestInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
public:
    WebUrlRequestInterceptor(QObject *p = Q_NULLPTR) : QWebEngineUrlRequestInterceptor{p}
    {}

    void interceptRequest(QWebEngineUrlRequestInfo &info) Q_DECL_OVERRIDE
    {
        qDebug() << info.requestMethod() << info.requestUrl() << info.resourceType() << info.navigationType();
        const bool isNavigationRequest = info.resourceType() == QWebEngineUrlRequestInfo::ResourceTypeMainFrame;
        if (isNavigationRequest) {
            QDesktopServices::openUrl(info.requestUrl());
            info.block(true);
        }
        //TODO handle mailto to open a composer
    }
};

WebEngineProfile::WebEngineProfile(QObject *parent)
    : QQuickWebEngineProfile(parent)
{
    setRequestInterceptor(new WebUrlRequestInterceptor(this));
}

#include "webengineprofile.moc"
