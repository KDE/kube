#!/bin/sh

# call xgettext on all source files.
$EXTRACT_TR_STRINGS `find . -name \*.cpp -o -name \*.h -name \*.qml` -o $podir/kube.pot
