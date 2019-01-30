# Cryptograghy

## OpenPGP

To get Open PGP support we use GpgME >= 1.8.0. GpgME has now include Gpgme++ and QGpgME, that were living before in kdepim repositories.

GpgME gives us directly PGP and SMime support and needs gpg-agent, gpg2, gpgsm and a pinentry to be installed to work correctly. The package names differ between the distributions on Fedora the are named gnupg2, gnupg2-smime and pinentry-qt (there are several different flavours for diffrent environments. I think the Qt flavour matches best).

gnupg using ~/.gnupg as default homedir to store every file it uses. In our test docker environment, we setup all relevat files and added keys without passphrase to test PGP/SMime support. We also incuded one key `With Passphrase "test" <passphrase@example.org>`, that has a phassphrase named `test`.

You can get all keys with secret key available via `gpg2 --list-secret-keys` all known keys with `gpg2 --list-keys`.

```
$ gpg2 --list-secret-keys
gpg: WARNING: unsafe permissions on homedir '/home/developer/.gnupg'
gpg: enabled debug flags: memstat
/home/developer/.gnupg/pubring.gpg
----------------------------------
sec   rsa2048 2009-11-13 [SC]
      1BA323932B3FAA826132C79E8D9860C58F246DE6
uid           [ultimate] unittest key (no password) <test@kolab.org>
ssb   rsa2048 2009-11-13 [E]

sec   rsa1024 2009-11-25 [SC]
      00949E2AF4A985AFB572FDD214B79E26050467AA
uid           [ultimate] kdetest <you@you.com>
ssb   rsa1024 2009-11-25 [E]

sec   rsa1024 2009-11-25 [SC]
      CA739AC832766152139B5C49FC4FAB94C727D4BB
uid           [ultimate] kde testing <bcc@bcc.org>
ssb   rsa1024 2009-11-25 [E]

sec   rsa2048 2016-11-22 [SC]
      4F7EE48F586A13D1397E91D270057E539B9DE64B
uid           [ultimate] With Passphrase "test" <passphrase@example.org>
ssb   rsa2048 2016-11-22 [E]
```

### Testing OpenPGP support

We have many testmails, that are signed and encrypted with keys without a passphrase.

All testmails can be found in the folder [github:cmollekopf/docker](https://github.com/cmollekopf/docker/tree/master/kube/testmails/cur).

* With `openpgp-` prefix are PGP/Mime messages [RFC 3156](https://tools.ietf.org/html/rfc3156).
* With `openpgp-inline-` prefix are PGP inline messages [RFC 4880](https://tools.ietf.org/html/rfc4880).
* With `smime-` prefix are SMIME messages [RFC 2633](https://tools.ietf.org/html/rfc2633).

To be able to also test gpg errors, we created some test mails, that triggers known issues:

* `openpgp-keymissing.mbox` this key is missing in the test environment, so gpg returns a KeyMissing error.
* `openpgp-wrong_passphrase.mbox`, here a pinentry popups and if you press three time `Cancel` in a row, a Passphrase error is returned. If you enter the correct passphrase `test` you the content of the mail. But than you need wither restart the testenvironment, kill `gpg-agent` or simply wait 1h before you see the passphrase error again.
* you can unistall the `gnupg-smime` package, or delete `/usr/bin/gpgsm` by hand to trigger a UnknownError for smime encrypted messages. If delete all `gnupgp2` related pacakges the application may react with segfaults.

We also have a unittest for gnupg errors in a lower level [kde:kube/framework/domain/mimetreeparser/tests/gpgerrortest.cpp](https://cgit.kde.org/kube.git/tree/framework/domain/mimetreeparser/tests/gpgerrortest.cpp?h=develop).
