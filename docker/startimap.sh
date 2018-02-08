#!/bin/bash
sudo saslauthd -a pam &
sudo /usr/libexec/cyrus-imapd/master -d
#Give the imap server some time to start
sleep 1
