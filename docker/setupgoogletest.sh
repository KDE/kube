#The account password is Welcome2Kube
sinksh create account type gmail identifier googleAccount name Google
sinksh create resource type sink.imap identifier googleImap account googleAccount server imaps://imap.gmail.com:993 username kubeprojecttest@gmail.com
sinksh create resource type sink.mailtransport identifier googleSmtp account googleAccount server smtps://smtp.gmail.com:587 username kubeprojecttest@gmail.com
sinksh create resource type sink.carddav  identifier googleCarddav account googleAccount server https://www.googleapis.com/carddav/v1/principals/kubeprojecttest@gmail.com/ username kubeprojecttest@gmail.com
sinksh create resource type sink.caldav  identifier googleCaldav account googleAccount server https://www.google.com/calendar/dav/kubeprojecttest@gmail.com/ username kubeprojecttest@gmail.com
sinksh create identity name "John Doe" address kubeprojecttest@gmail.com account googleAccount
