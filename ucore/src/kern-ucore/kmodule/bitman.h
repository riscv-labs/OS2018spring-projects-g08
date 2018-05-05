#ifndef H_BIT_MAN
#define H_BIT_MAN

#define PICK_BITS(v, offset, cnt) (((v) >> (offset)) & ((1ULL << (cnt)) - 1))
#define SET_BITS(v, offset, cnt, val) ((v) & ~(((1ULL << (cnt)) - 1) << (offset)) | (((val) & ((1ULL << (cnt)) - 1)) << (offset)))

#endif