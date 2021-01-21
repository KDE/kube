#!/bin/bash

echo "cm user.doe" | cyradm --auth PLAIN -u cyrus -w admin localhost

echo "sam user.doe.* cyrus c;
dm user.doe.*;
cm user.doe.test;
cm user.doe.Drafts;
cm user.doe.Trash;
sam user.doe cyrus c;
" | cyradm --auth PLAIN -u cyrus -w admin localhost

echo "sam user.doe.* cyrus c;
subscribe INBOX.test;
subscribe INBOX.Drafts;
subscribe INBOX.Trash;
" | cyradm --auth PLAIN -u doe -w doe localhost

MSG=$(cat <<EOF
Return-Path: <nepomuk-bounces@kde.org>
Received: from compute4.internal (compute4.nyi.mail.srv.osa [10.202.2.44])
	 by slots3a1p1 (Cyrus git2.5+0-git-fastmail-8998) with LMTPA;
	 Mon, 11 Mar 2013 14:28:42 -0400
X-Sieve: CMU Sieve 2.4
X-Spam-score: 0.0
X-Spam-hits: BAYES_00 -1.9, RCVD_IN_DNSWL_MED -2.3, RP_MATCHES_RCVD -0.704,
  LANGUAGES unknown, BAYES_USED global, SA_VERSION 3.3.1
X-Spam-source: IP='46.4.96.248', Host='postbox.kde.org', Country='unk', FromHeader='org',
  MailFrom='org'
X-Spam-charsets: plain='us-ascii'
X-Resolved-to: chrigi_1@fastmail.fm
X-Delivered-to: chrigi_1@fastmail.fm
X-Mail-from: nepomuk-bounces@kde.org
Received: from mx4.nyi.mail.srv.osa ([10.202.2.203])
  by compute4.internal (LMTPProxy); Mon, 11 Mar 2013 14:28:42 -0400
Received: from postbox.kde.org (postbox.kde.org [46.4.96.248])
	by mx4.messagingengine.com (Postfix) with ESMTP id 1C9D2440F88
	for <chrigi_1@fastmail.fm>; Mon, 11 Mar 2013 14:28:42 -0400 (EDT)
Received: from postbox.kde.org (localhost [IPv6:::1])
	by postbox.kde.org (Postfix) with ESMTP id 00FFEB3732B;
	Mon, 11 Mar 2013 18:28:40 +0000 (UTC)
DKIM-Signature: v=1; a=rsa-sha256; c=simple/simple; d=kde.org; s=default;
	t=1363026520; bh=cOdvyBAJJ8ho64q0H7rxkl+cB2y6TiyVOX0fO3yZ64U=;
	h=Date:From:To:Message-ID:In-Reply-To:References:MIME-Version:
	 Subject:List-Id:List-Unsubscribe:List-Archive:List-Post:List-Help:
	 List-Subscribe:Content-Type:Content-Transfer-Encoding:Sender; b=dv
	dJAFu+6JCuNun5WIuP4ysfKpLh0DeuhEEfy2cQavUGMICJ27k7tI73x6gN37V5Q/evJ
	NDFna3/IhNBsAQeLiXs28HKxzcVhbnq5jdFR6fbyo6k1fOKt5vTT1GTDZ+3zIGPD1CU
	ioDBGxPb/Ds6gee90tjadOj6o+Oc+2ZSq94=
X-Original-To: nepomuk@kde.org
X-Remote-Delivered-To: nepomuk@localhost.kde.org
Received: from build.kde.org (build.kde.org [IPv6:2a01:4f8:160:9363::5])
 by postbox.kde.org (Postfix) with ESMTP id 4491CB3732B
 for <nepomuk@kde.org>; Mon, 11 Mar 2013 18:28:27 +0000 (UTC)
Received: from localhost ([127.0.0.1]) by build.kde.org with esmtp (Exim 4.72)
 (envelope-from <null@kde.org>) id 1UF7SV-0000gs-11
 for nepomuk@kde.org; Mon, 11 Mar 2013 18:28:27 +0000
Date: Mon, 11 Mar 2013 18:28:27 +0000 (UTC)
From: KDE CI System  <null@kde.org>
To: nepomuk@kde.org
Message-ID:
MIME-Version: 1.0
X-Jenkins-Job: nepomuk-core_stable
X-Jenkins-Result: UNSTABLE
X-Scanned-By: MIMEDefang 2.71 on 46.4.96.248
Subject: [Nepomuk] Jenkins build is still unstable: nepomuk-core_stable #158
X-BeenThere: nepomuk@kde.org
X-Mailman-Version: 2.1.14
Precedence: list
List-Id: The Semantic KDE <nepomuk.kde.org>
List-Unsubscribe: <https://mail.kde.org/mailman/options/nepomuk>,
 <mailto:nepomuk-request@kde.org?subject=unsubscribe>
List-Archive: <http://mail.kde.org/pipermail/nepomuk>
List-Post: <mailto:nepomuk@kde.org>
List-Help: <mailto:nepomuk-request@kde.org?subject=help>
List-Subscribe: <https://mail.kde.org/mailman/listinfo/nepomuk>,
 <mailto:nepomuk-request@kde.org?subject=subscribe>
Content-Type: text/plain; charset="us-ascii"
Content-Transfer-Encoding: 7bit
Errors-To: nepomuk-bounces@kde.org
Sender: nepomuk-bounces@kde.org
X-Truedomain: NotChecked

See <http://build.kde.org/job/nepomuk-core_stable/changes>

_______________________________________________
Nepomuk mailing list
Nepomuk@kde.org
https://mail.kde.org/mailman/listinfo/nepomuk
EOF
)

#Create a bunch of test messages in the test folder
FOLDERPATH=/var/spool/imap/d/user/doe/test
for i in `seq 1 5000`;
do
   sudo echo "$MSG" | sed "s/Message-ID:/Message-ID: <$i>/" > $FOLDERPATH/$i.
done

sudo chown -R cyrus:mail $FOLDERPATH
sudo reconstruct "user.doe.test"

sinksh create account type generic identifier perfAccount name perfAccount
sinksh create resource type sink.imap identifier perfImap account perfAccount server imaps://localhost:993 username doe
sinksh create resource type sink.mailtransport identifier perfSmtp account perfAccount server smtps://localhost:587 username doe
sinksh create resource type sink.carddav  identifier perfCarddav account perfAccount server https://localhost username doe
sinksh create resource type sink.caldav  identifier perfCaldav account perfAccount server https://localhost username doe
sinksh create identity name "John Doe" address doe@kolab.org account perfAccount
