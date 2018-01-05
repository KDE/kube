/*
    Copyright (c) 2018 Christian Mollekopf <mollekopf@kolabsys.com>

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
#include <QtQuickTest/quicktest.h>
#include <sink/test.h>
#include <sink/store.h>

int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
    Sink::Test::initTest();

    auto resource = Sink::ApplicationDomain::DummyResource::create("account1");
    Sink::Store::create(resource).exec().waitForFinished();

    auto transportResource = Sink::ApplicationDomain::MailtransportResource::create("account1");
    Sink::Store::create(transportResource).exec().waitForFinished();

    auto identity = Sink::ApplicationDomain::Identity{};
    identity.setAccount("account1");
    identity.setAddress("identity@example.org");
    identity.setName("Identity Name");
    Sink::Store::create(identity).exec().waitForFinished();

    QTEST_ADD_GPU_BLACKLIST_SUPPORT
    QTEST_SET_MAIN_SOURCE_PATH
    return quick_test_main(argc, argv, "kubetest", 0);
}
