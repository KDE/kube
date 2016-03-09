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
#include "mailtransport.h"

#include <QByteArray>
#include <QList>
#include <QDebug>

extern "C" {

#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

struct upload_status {
    int offset;
    const char *data;
};

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
    struct upload_status *upload_ctx = (struct upload_status *)userp;
    const char *data;

    if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
        return 0;
    }

    data = &upload_ctx->data[upload_ctx->offset];
    if(data) {
        size_t len = strlen(data);
        if (len > size * nmemb) {
            len = size * nmemb;
        }
        fprintf(stderr, "read n bytes: %d\n",len);
        memcpy(ptr, data, len);
        upload_ctx->offset += len;
        return len;
    }

    return 0;
}


void sendMessageCurl(const char *to[], int numTos, const char *cc[], int numCcs, const char *msg, bool useTls, const char* from, const char *username, const char *password, const char *server, bool verifyPeer)
{
    //For ssl use "smtps://mainserver.example.net
    const char* cacert = 0; // = "/path/to/certificate.pem";

    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx;

    upload_ctx.offset = 0;
    upload_ctx.data = msg;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);

        curl_easy_setopt(curl, CURLOPT_URL, server);

        if (useTls) {
            curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_TRY);
        }

        if (!verifyPeer) {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        }
        if (cacert) {
            curl_easy_setopt(curl, CURLOPT_CAINFO, cacert);
        }

        if (from) {
            curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from);
        }

        for (int i = 0; i < numTos; i++) {
            recipients = curl_slist_append(recipients, to[i]);
        }
        for (int i = 0; i < numCcs; i++) {
            recipients = curl_slist_append(recipients, cc[i]);
        }
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        /* We're using a callback function to specify the payload (the headers and
        * body of the message). You could just use the CURLOPT_READDATA option to
        * specify a FILE pointer to read from. */
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        /* Since the traffic will be encrypted, it is very useful to turn on debug
        * information within libcurl to see what is happening during the transfer.
        */
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }
        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }
}

};

void MailTransport::sendMessage(const KMime::Message::Ptr &message, const QByteArray &server, const QByteArray &username, const QByteArray &password, const QByteArray &cacert)
{
    QByteArray msg = message->encodedContent();
    qWarning() << "Sending message " << msg;

    QByteArray from(message->from(true)->mailboxes().isEmpty() ? QByteArray() : message->from(true)->mailboxes().first().address());
    QList<QByteArray> toList;
    for (const auto &mb : message->to(true)->mailboxes()) {
        toList << mb.address();
    }
    QList<QByteArray> ccList;
    for (const auto &mb : message->cc(true)->mailboxes()) {
        ccList << mb.address();
    }
    bool useTls = true;
    bool verifyPeer = false;

    const int numTos = toList.size();
    const char* to[numTos];
    for (int i = 0; i < numTos; i++) {
        to[i] = toList.at(i);
    }

    const int numCcs = ccList.size();
    const char* cc[numCcs];
    for (int i = 0; i < numCcs; i++) {
        cc[i] = ccList.at(i);
    }

    sendMessageCurl(to, numTos, cc, numCcs, msg, useTls, from.isEmpty() ? nullptr : from, username, password, server, verifyPeer);
    qWarning() << "Message sent";
}
