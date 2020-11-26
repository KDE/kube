#!/bin/sh
# Since gpg 2.1 we always use a socket in /run, which is isolated from the systems, so we always start a new instance.
gpg-agent --homedir ~/.gnupg --daemon --pinentry-program /app/bin/pinentry-qt
kube --lockfile
# Concurrent runs of the flatpak are only protected by the lockfile,
# so no lingering around.
kill $(pidof sink_synchronizer)
gpg-connect-agent killagent /bye
pkill gpg-agent
