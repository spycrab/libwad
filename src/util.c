// Copyright 2019 spycrab0
// Licensed under GPLv3+
// Refer to the LICENSE file included.

#include "util.h"
#include "libwad.h"

#include <ctype.h>

const unsigned char NORMAL_COMMON_KEY[16] = {0xeb, 0xe4, 0x2a, 0x22, 0x5e, 0x85,
                                             0x93, 0xe4, 0x48, 0xd9, 0xc5, 0x45,
                                             0x73, 0x81, 0xaa, 0xf7};

const unsigned char KOREA_COMMON_KEY[16] = {0x63, 0xb8, 0x2b, 0xb4, 0xf4, 0x61,
                                            0x4e, 0x2e, 0x13, 0xf2, 0xfe, 0xfb,
                                            0xba, 0x4c, 0x9b, 0x7e};

const unsigned char DEBUG_COMMON_KEY[16] = {0x63, 0xb8, 0x2b, 0xb4, 0xf4, 0x61,
                                            0x4e, 0x2e, 0x13, 0xf2, 0xfe, 0xfb,
                                            0xba, 0x4c, 0x9b, 0x7e};

const unsigned char VWII_COMMON_KEY[16] = {0x30, 0xbf, 0xc7, 0x6e, 0x7c, 0x19,
                                           0xaf, 0xbb, 0x23, 0x16, 0x33, 0x30,
                                           0xce, 0xd7, 0xc2, 0x8d};

uint16_t be_int16v(uint16_t i) { return (i & 0xff) << 8 | (i & 0xff00) >> 8; }

void be_int16(uint16_t* i) { *i = be_int16v(*i); }

uint32_t be_int32v(uint32_t i)
{
  return (uint32_t)be_int16v(i & 0xffff) << 16 |
         be_int16v((i & 0xffff0000) >> 16);
}

void be_int32(uint32_t* i) { *i = be_int32v(*i); }

uint64_t be_int64v(uint64_t i)
{
  return (uint64_t)be_int32v(i & 0xffffffff) << 32 |
         be_int32v((i & 0xffffffff00000000) >> 32);
}

void be_int64(uint64_t* i) { *i = be_int64v(*i); }

uint32_t align32(uint32_t offset)
{
  if (offset % 64 == 0)
    return offset;

  return offset + 64 - (offset % 64);
}

uint64_t align64(uint64_t offset, uint64_t mod)
{
  if (offset % mod == 0)
    return offset;

  return offset + mod - (offset % mod);
}

static char s_filename[6 * 2 + 1] = {0};

static char nibble_to_alpha(char in)
{
  // Strip the upper nibble
  in &= 0x0f;

  return in <= 9 ? ('0' + in) : ('A' + in - 10);
}

const char* util_title_id_to_string(uint64_t id)
{
  int ptr = 0;

  for (int i = 4; i < 8; i++) {
    char c = (id >> (8 - i - 1) * 8) & 0xff;
    if (c > 0 && isalpha(c)) {
      s_filename[ptr++] = c;
    } else {
      s_filename[ptr++] = nibble_to_alpha((c & 0xf0) >> 4);
      s_filename[ptr++] = nibble_to_alpha(c & 0x0f);
    }
  }

  s_filename[ptr++] = nibble_to_alpha((char)(id >> (8 - 2 - 1) * 8));
  s_filename[ptr++] = nibble_to_alpha((char)(id >> (8 - 3 - 1) * 8));

  s_filename[ptr] = '\0';

  return s_filename;
}