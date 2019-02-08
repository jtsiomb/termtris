#ifndef PIECES_H_
#define PIECES_H_

#define BLK(x, y)	((x) | ((y) << 4))

unsigned char pieces[][4][4] = {
	/* straight piece */
	{
		{BLK(0, 2), BLK(1, 2), BLK(2, 2), BLK(3, 2)},
		{BLK(1, 0), BLK(1, 1), BLK(1, 2), BLK(1, 3)},
		{BLK(0, 2), BLK(1, 2), BLK(2, 2), BLK(3, 2)},
		{BLK(1, 0), BLK(1, 1), BLK(1, 2), BLK(1, 3)}
	},
	/* box */
	{
		{BLK(1, 1), BLK(2, 1), BLK(1, 2), BLK(2, 2)},
		{BLK(1, 1), BLK(2, 1), BLK(1, 2), BLK(2, 2)},
		{BLK(1, 1), BLK(2, 1), BLK(1, 2), BLK(2, 2)},
		{BLK(1, 1), BLK(2, 1), BLK(1, 2), BLK(2, 2)}
	},
	/* J block */
	{
		{BLK(0, 1), BLK(1, 1), BLK(2, 1), BLK(2, 2)},
		{BLK(1, 0), BLK(1, 1), BLK(1, 2), BLK(0, 2)},
		{BLK(0, 0), BLK(0, 1), BLK(1, 1), BLK(2, 1)},
		{BLK(1, 0), BLK(2, 0), BLK(1, 1), BLK(1, 2)}
	},
	/* L block */
	{
		{BLK(0, 1), BLK(0, 2), BLK(1, 1), BLK(2, 1)},
		{BLK(0, 0), BLK(1, 0), BLK(1, 1), BLK(1, 2)},
		{BLK(0, 1), BLK(1, 1), BLK(2, 1), BLK(2, 0)},
		{BLK(1, 0), BLK(1, 1), BLK(1, 2), BLK(2, 2)}
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
	},
	/* Z block */
	{
		{BLK(0, 1), BLK(1, 1), BLK(1, 2), BLK(2, 2)},
		{BLK(0, 1), BLK(1, 1), BLK(1, 0), BLK(0, 2)},
		{BLK(0, 1), BLK(1, 1), BLK(1, 2), BLK(2, 2)},
		{BLK(0, 1), BLK(1, 1), BLK(1, 0), BLK(0, 2)}
	}
};


#endif	/* PIECES_H_ */
