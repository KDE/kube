sinksh create account type kolabnow identifier kolabnowAccount name KolabnowAccount
sinksh create resource type sink.imap identifier kolabnowImap account kolabnowAccount server imaps://imap.kolabnow.com:993 username test1@kolab.org
sinksh create resource type sink.mailtransport identifier kolabnowSmtp account kolabnowAccount server smtps://smtp.kolabnow.com:587 username test1@kolab.org
sinksh create resource type sink.carddav  identifier kolabnowCarddav account kolabnowAccount server https://apps.kolabnow.com username test1@kolab.org
sinksh create resource type sink.caldav  identifier kolabnowCaldav account kolabnowAccount server https://apps.kolabnow.com username test1@kolab.org
sinksh create identity name "John Doe" address test1@kolab.org account kolabnowAccount
