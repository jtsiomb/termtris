Termtris
========
Termtris is a tetris game for ANSI/VT220-compatible terminals.

By default it opens `/dev/tty`. If you want to use it with a serial terminal,
specify the correct device file with the `-t` option. For example:
`./termtris -t /dev/ttyS0`

License
-------
Copyright (C) 2019 John Tsiombikas <nuclear@member.fsf.org>

This program is free software. Feel free to use, modify, and/or redistribute it
under the terms of the GNU General Public License version 3, or at your option
any later version published by the Free Software Foundation. See COPYING for
details.
