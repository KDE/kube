#!/usr/bin/env bash
gpg2 --batch --generate-key keyconfig
gpg2 --export-secret-key -a test1@kolab.org > private-key
gpg2 --export -a test1@kolab.org > public-key
