Softfonts notes
===============

DEC terminals from VT220 and up support custom character set downloads into the
DRCS buffer (Dynamically Redefinable Character Set), with the DECDLD command.
There are three different downloadable character formats for each generation of
DEC terminals, VT2x0, VT3x0, and VT4x0/VT5x0 (considering only 80x24 modes,
otherwise there are more variations still).

EGA and VGA PC graphics cards support custom characters to be uploaded into
video RAM, either directly, or with a video BIOS call (int 10h/ax=1110h).


DEC downloadable character definition
-------------------------------------
The following are the maximum character matrix sizes for each generation of DEC
terminals in 80x24 mode:

  - VT2x0: 8x10
  - VT3x0: 15x12 with the displayed character being stretched 3x vertically.
  - VT4x0/VT5x0: 10x16

Character bitmaps are converted to sixels, and loaded with the DECDLD command.
The DECDLD on a VT200-series starts with `ESC P`, followed by a number of
parameters:

  font no;first char;erase mode;matrix size/width;columns;text/full {

followed by the desired character set "name" (which is usually " @", followed by
the sixels for a series of characters, and closed by the terminator `ESC \`.

VT300 and above adds a few more parameters before the curly brace:

  matrix height;charset size

  - *Font number* is either 0 or 1, but is completely ignored either way.
  - *First char* can be a number from 1 to 94, corresponding to characters
    '!' (21h) to '~' (7eh) for charset size of 94, or 0 to 95, corresponding to
    "space" (20h) up to "delete" (7fh) for charset size of 96 (not supported on
    VT200-series).
  - *erase mode* is 0 to erase all characters of the specified
    font/size/rendition while downloading, 1 to erase only
    the characters which re being replaced, and 2 to erase all characters.
  - *matrix size* on the VT200-series is 0 for default (7x10), 2 for 5x10, 3 for
    6x10 and 4 for 7x10. On VT300-series and up, it can be the same, or define
    the matrix width (like 15 for the full width of the VT3x0 fonts in 80x24
    mode).
  - *columns* is 0 (default) or 1 if this is the 80-column version of the font,
    or 2 if it's intended for the 132-column mode. Beyond the VT200-series,
    there are more values to also specify the rows, while the previous ones are
    considered to specify 24-row modes. 11 is for 80x36, 12 for 132x36, 21 for
    80x48, and 22 for 132x48.
  - *text/full* define wether the downloaded characters are meant for text (0 or
    1), and the terminal needs to leave empty space around them and center them
    accordingly, or for graphics (2), in which case the whole matrix is available,
    and nearby characters are adjacent with no gap. The VT200-series in 80x24
    mode does not support "full" to be specified here.
  - *matrix height* can be any value from 1 to 12 on the VT300-series or 16 on
    the VT400 and above.
  - *charset size* is 0 for 94-character fonts (21h to 7eh), or 1 for
    96-character fonts (20h to 7fh).

### Sixel data

Bitmaps are encoded as sixels. The bitmap is split into horizontal bands each 6
scanlines tall. Each column of each band is encoded into a sixel, a printable
character corresponding to the bit pattern of that column, and those sixels are
transmitted one after the other. After the whole band of 6-pixel columns is
transmitted, a `/` is used as a separator, and the next 6-tall band is encoded
and transmitted in the same way, until the whole height of the bitmap is
exhausted.

The pixel-column sixel encoding is done by first padding any missing high order
bits with 0 if there are fewer than 6 scanlines in it (this happens in the last
band, if the bitmap height is not a multiple of 6). Pixels at the bottom of the
column correspond to the high order bits of the bit pattern, and the top of the
column to low order bits. The value of the bit pattern is adjusted into the
printable range by adding 63 to it, so `000000` corresponds to `?`, and `111111`
corresponds to `~`.

Here's a table for easy conversion between bit patterns (octal) and characters:

|    00  ?     |     20  O     |     40  _     |     60  o    |
|    01  @     |     21  P     |     41  `     |     61  p    |
|    02  A     |     22  Q     |     42  a     |     62  q    |
|    03  B     |     23  R     |     43  b     |     63  r    |
|    04  C     |     24  S     |     44  c     |     64  s    |
|    05  D     |     25  T     |     45  d     |     65  t    |
|    06  E     |     26  U     |     46  e     |     66  u    |
|    07  F     |     27  V     |     47  f     |     67  v    |
|    10  G     |     30  W     |     50  g     |     70  w    |
|    11  H     |     31  X     |     51  h     |     71  x    |
|    12  I     |     32  Y     |     52  i     |     72  y    |
|    13  J     |     33  Z     |     53  j     |     73  z    |
|    14  K     |     34  [     |     54  k     |     74  {    |
|    15  L     |     35  \     |     55  l     |     75  |    |
|    16  M     |     36  ]     |     56  m     |     76  }    |
|    17  N     |     37  ^     |     57  n     |     77  ~    |

