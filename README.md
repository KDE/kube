## Kube

Kube is a personal information and communication application.
It uses Sink for data access and synchronization.

See docs/project.md for more information.

## License

GPLV2+

## Getting involved

* https://invent.kde.org/pim/kube
* https://kube-project.com

## Extract translatable messages

$ svn checkout svn://anonsvn.kde.org/home/kde/trunk/l10n-kf5/scripts
$ mkdir po
$ mkdir enpo
$ export LUPDATE=lupdate-qt5
$ PATH=/path/to/scripts:$PATH bash /path/to/scripts/extract-messages.sh
