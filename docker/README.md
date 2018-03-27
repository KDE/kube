This is a container to build and run kube in.

It contains cyrus imap which provides both an imap server as well as cal/cardDAV,
which is required for some tests of sink.

# Building
Use the build.sh script to build the container. To rebuild from scratch add the --no-cache option to the docker command.

#Using
The container starts cyrus imap including the caldav and carddav modules.

Logs for the services are available from /var/log/messages.

The caldav server is available at: http://localhost/dav/calendars/users/doe
The carddav server is available at: http://localhost/dav/addressbooks/user/doe
