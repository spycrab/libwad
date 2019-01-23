// Copyright 2019 spycrab0
// Licensed under GPLv3+
// Refer to the LICENSE file included.

#ifndef UTIL_H
#define UTIL_H

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

// Common keys
extern const unsigned char NORMAL_COMMON_KEY[16];
extern const unsigned char KOREA_COMMON_KEY[16];
extern const unsigned char DEBUG_COMMON_KEY[16];
extern const unsigned char VWII_COMMON_KEY[16];

void be_int16(uint16_t* i);
void be_int32(uint32_t* i);
void be_int64(uint64_t* i);

uint32_t align32(uint32_t offset);
uint64_t align64(uint64_t offset, uint64_t mod);

#endif