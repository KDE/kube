#!/bin/sh
# Since gpg 2.1 we always use a socket in /run, which is isolated from the systems, so we always start a new instance.
# WARNING the flatpak socket dirs are not isolated between instances. Since we use the lockfile to avoid running multiple instances of kube this is ok,
# but we must avoid killing the original instance below.
gpg-agent --homedir ~/.gnupg --daemon --pinentry-program /app/bin/pinentry-qt
RETVAL=$?
kube --lockfile
# Concurrent runs of the flatpak are only protected by the lockfile,
# so no lingering around.
kill $(pidof sink_synchronizer)
## gpg-agent returns 2 if another agent was already started.
if [ $RETVAL -ne 0 ]; then
    gpg-connect-agent killagent /bye
    pkill gpg-agent
fi
