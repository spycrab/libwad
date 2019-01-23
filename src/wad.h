// Copyright 2019 spycrab0
// Licensed under GPLv3+
// Refer to the LICENSE file included.

#include "libwad.h"

#include <stdio.h>

extern int g_error;

struct wad_data {
  FILE* fh;
  uint32_t type;
  uint32_t certchain_size;
  uint32_t ticket_size;
  uint32_t tmd_size;
  uint32_t data_size;
  uint32_t footer_size;

  certchain_t certchain;
  ticket_t ticket;
  tmd_t tmd;
};
