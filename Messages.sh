#!/bin/sh

# call xgettext on all source files.
$EXTRACT_TR_STRINGS `find . -name \*.cpp -o -name \*.h -o -name \*.qml` -o $podir/kube_qt.pot
