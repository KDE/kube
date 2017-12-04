sinksh create account type kolabnow identifier kolabnowAccount name KolabnowAccount
sinksh create resource type sink.imap identifier kolabnowImap account kolabnowAccount server imaps://imap.kolabnow.com:993 username test1@kolab.org password Welcome2KolabSystems
sinksh create resource type sink.mailtransport identifier kolabnowSmtp account kolabnowAccount server smtps://smtp.kolabnow.com:587 username test1@kolab.org password Welcome2KolabSystems
sinksh create resource type sink.dav  identifier kolabnowDav account kolabnowAccount server https://apps.kolabnow.com/addressbooks/test1%40kolab.org  username test1@kolab.org  password Welcome2KolabSystems
sinksh create identity name "John Doe" address test1@kolab.org account kolabnowAccount
