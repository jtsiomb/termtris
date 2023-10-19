/*
Termtris - a tetris game for ANSI/VT100 terminals
Copyright (C) 2019-2022  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef PIECES_H_
#define PIECES_H_

#define BLK(x, y)	((x) | ((y) << 4))
#define BLKX(c)		((unsigned char)(c) & 0xf)
#define BLKY(c)		((unsigned char)(c) >> 4)

#define NUM_PIECES	7

static unsigned char pieces[NUM_PIECES][4][4] = {
	/* L block */
	{
		{BLK(0, 1), BLK(0, 2), BLK(1, 1), BLK(2, 1)},
		{BLK(0, 0), BLK(1, 0), BLK(1, 1), BLK(1, 2)},
		{BLK(0, 1), BLK(1, 1), BLK(2, 1), BLK(2, 0)},
		{BLK(1, 0), BLK(1, 1), BLK(1, 2), BLK(2, 2)}
	},
	/* J block */
	{
		{BLK(0, 1), BLK(1, 1), BLK(2, 1), BLK(2, 2)},
		{BLK(1, 0), BLK(1, 1), BLK(1, 2), BLK(0, 2)},
		{BLK(0, 0), BLK(0, 1), BLK(1, 1), BLK(2, 1)},
		{BLK(1, 0), BLK(2, 0), BLK(1, 1), BLK(1, 2)}
	},
	/* I block */
	{
		{BLK(0, 2), BLK(1, 2), BLK(2, 2), BLK(3, 2)},
		{BLK(1, 0), BLK(1, 1), BLK(1, 2), BLK(1, 3)},
		{BLK(0, 2), BLK(1, 2), BLK(2, 2), BLK(3, 2)},
		{BLK(1, 0), BLK(1, 1), BLK(1, 2), BLK(1, 3)}
	},
	/* O block */
	{
		{BLK(1, 1), BLK(2, 1), BLK(1, 2), BLK(2, 2)},
		{BLK(1, 1), BLK(2, 1), BLK(1, 2), BLK(2, 2)},
		{BLK(1, 1), BLK(2, 1), BLK(1, 2), BLK(2, 2)},
		{BLK(1, 1), BLK(2, 1), BLK(1, 2), BLK(2, 2)}
	},
	/* Z block */
	{
		{BLK(0, 1), BLK(1, 1), BLK(1, 2), BLK(2, 2)},
		{BLK(0, 1), BLK(1, 1), BLK(1, 0), BLK(0, 2)},
		{BLK(0, 1), BLK(1, 1), BLK(1, 2), BLK(2, 2)},
		{BLK(0, 1), BLK(1, 1), BLK(1, 0), BLK(0, 2)}
	},
	/* S block */
	{
		{BLK(1, 1), BLK(2, 1), BLK(0, 2), BLK(1, 2)},
		{BLK(0, 0), BLK(0, 1), BLK(1, 1), BLK(1, 2)},
		{BLK(1, 1), BLK(2, 1), BLK(0, 2), BLK(1, 2)},
		{BLK(0, 0), BLK(0, 1), BLK(1, 1), BLK(1, 2)}
	},
	/* T block */
	{
		{BLK(0, 1), BLK(1, 1), BLK(2, 1), BLK(1, 2)},
		{BLK(1, 0), BLK(1, 1), BLK(1, 2), BLK(0, 1)},
		{BLK(0, 1), BLK(1, 1), BLK(2, 1), BLK(1, 0)},
		{BLK(1, 0), BLK(1, 1), BLK(1, 2), BLK(2, 1)}
	}
};

static int piece_spawnpos[NUM_PIECES][2] = {
	{-1, -2}, {-1, -2}, {-2, -2}, {-1, -2}, {-1, -2}, {-1, -2}, {-1, -2}
};


#endif	/* PIECES_H_ */
