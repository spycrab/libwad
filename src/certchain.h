// Copyright 2019 spycrab0
// Licensed under GPLv3+
// Refer to the LICENSE file included.

#ifndef CERTCHAIN_H
#define CERTCHAIN_H

#include "libwad.h"

#include <stdio.h>

struct link {
  cert_t data;
  struct link* next;
};

struct certchain_data {
  FILE* fh;
  size_t cert_count;
  struct link* chain;
};

certchain_t certchain_from_wad(wad_t handle);

#endif
