## Kube

Kube is a personal information and collaboration application currently in its early
stages of development. It uses Sink for data access and synchronization, and
leverages the KDE PIM codebase where possible.

See docs/project.md for more information.

## License

GPLV2+

## Getting involved

www.kube.kde.org

## Extract translatable messages

$ svn checkout svn://anonsvn.kde.org/home/kde/trunk/l10n-kf5/scripts
$ mkdir po
$ mkdir enpo
$ export LUPDATE=lupdate-qt5
$ PATH=/path/to/scripts:$PATH bash /path/to/scripts/extract-messages.sh
