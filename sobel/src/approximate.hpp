#ifndef __APPROXIMATE_HPP__
#define __APPROXIMATE_HPP__

#define _LONG_  3
#define _INT_   2
#define _SHORT_ 1
#define _BYTE_  0

#define TYPE _BYTE_

#define FLIT_BYTE 64 //size of a flit in byte
#define N_FLIT 5 //number of payload flits

#define N_LEVEL (N_FLIT - 1)
#define CACHE_LINE_BYTE (N_FLIT * FLIT_BYTE)

const int K_BITS_TABLE[][N_LEVEL] = {
  {6, 4, 3, 1},
  {12, 9, 6, 3},
  {25, 19, 12, 6},
  {51, 38, 25, 12}
};

const int * K_LEVELS = K_BITS_TABLE[TYPE];

const int UNIT_SIZES[] = {1, 2, 4, 8};

#define UNIT_BYTE UNIT_SIZES[TYPE] //size of the approximable data unit in byte
#define UNIT_BIT (UNIT_BYTE * 8)

#endif
