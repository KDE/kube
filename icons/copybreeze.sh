#!/usr/bin/env python2

import sh
import subprocess
import os
from shutil import copyfile, copy2
from os import path

wantedIcons = [
    "application-menu.svg",
    "dialog-cancel.svg",
    "dialog-ok.svg",
    "document-decrypt.svg",
    "document-edit.svg",
    "document-encrypt.svg",
    "document-save.svg",
    "edit-delete.svg",
    "edit-find.svg",
    "edit-undo.svg",
    "error.svg",
    "folder.svg",
    "im-user.svg",
    "mail-mark-important.svg",
    "mail-mark-unread-new.svg",
    "mail-reply-sender.svg",
    "mail-folder-outbox.svg",
    "network-disconnect.svg",
    "view-refresh.svg",
    "go-down.svg",
    "go-previous.svg",
    "mail-message.svg"
]

def ensure_dir(file_path):
    directory = os.path.dirname(file_path)
    if not os.path.exists(directory):
        os.makedirs(directory)

def copyFile(rootDir, dir, file):
    print("Copy file " + root + ", " + dir + ", " + file)
    reldir = dir.replace(path.join(rootDir, "icons"), "")
    src = os.path.join(dir, file)
    if os.path.islink(src):
        # We're dealing with a symlink
        linkto = os.readlink(src)
        targetRelpath = path.join(os.path.dirname(src), linkto)
        targetReldir = os.path.dirname(targetRelpath)

        # First recursively copy target
        copyFile(rootDir, targetReldir, targetRelpath.replace(targetReldir + "/", ""))

        #Create symlinks for normal and dark version
        dst = "./breeze/icons" + path.join(reldir, file)
        if not os.path.exists(dst):
            ensure_dir(dst)
            os.symlink(linkto, dst)

        invertedDst = "./breeze/icons" + path.join(reldir, file.replace(".svg", "-inverted.svg"))
        if not os.path.exists(invertedDst):
            ensure_dir(invertedDst)
            os.symlink(linkto.replace(".svg", "-inverted.svg"), invertedDst)
    else:
        # A regular icon, just copy normal and dark version
        dst = "./breeze/icons" + path.join(reldir, file)
        if not os.path.exists(dst):
            print("Copying: " + path.join(dir, file) + " to " + dst)
            ensure_dir(dst)
            copy2(src, dst)

        invertedDst = "./breeze/icons" + path.join(reldir, file.replace(".svg", "-inverted.svg"))
        if not os.path.exists(invertedDst):
            print("Copying: " + src.replace("icons", "icons-dark") + " to " + invertedDst)
            ensure_dir(invertedDst)
            copy2(src.replace("icons", "icons-dark"), invertedDst)

dir="upstreamBreeze"
if not os.path.exists(dir):
    sh.git.clone("--depth", "1", "git://anongit.kde.org/breeze-icons.git", dir)

dirToWalk = dir + "/icons"
for root, dirs, files in os.walk(dirToWalk):
    for file in files:
        if any(file == s for s in wantedIcons):
            copyFile(dir, root, file)
