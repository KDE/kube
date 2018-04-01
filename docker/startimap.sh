#!/bin/bash
sudo saslauthd -a shadow &
sudo /usr/libexec/cyrus-imapd/master -d
#Give the imap server some time to start
sleep 1
