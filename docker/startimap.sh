#!/bin/bash
sudo saslauthd -a pam &
sudo /usr/libexec/cyrus-imapd/master -d
