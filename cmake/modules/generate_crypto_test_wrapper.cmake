# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2013, Sandro Knauß <mail@sandroknauss.de>
# Copyright (c) 2018, Christian Mollekopf <mollekopf@kolabsys.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (UNIX)

file(WRITE "${_filename}"
"#!/usr/bin/env sh
# created by cmake, don't edit, changes will be lost

# don't mess with a gpg-agent already running on the system
unset GPG_AGENT_INFO

${_library_path_variable}=${_ld_library_path}\${${_library_path_variable}:+:\$${_library_path_variable}} GNUPGHOME=${_gnupghome} gpg-agent --daemon \"${_executable}\" \"$@\"
_result=$?
GNUPGHOME=${_gnupghome} gpg-connect-agent killagent /bye
exit \$_result
")

# make it executable
# since this is only executed on UNIX, it is safe to call chmod
exec_program(chmod ARGS ug+x \"${_filename}\" OUTPUT_VARIABLE _dummy )

else (UNIX)

file(TO_NATIVE_PATH "${_ld_library_path}" win_path)
file(TO_NATIVE_PATH "${_gnupghome}" win_gnupghome)

file(WRITE "${_filename}"
"
set PATH=${win_path};$ENV{PATH}
set GNUPGHOME=${win_gnupghome};$ENV{GNUPGHOME}
gpg-agent --daemon \"${_executable}\" %*
")

endif (UNIX)
