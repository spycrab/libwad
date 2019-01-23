// Copyright 2019 spycrab0
// Licensed under GPLv3+
// Refer to the LICENSE file included.

#ifndef TICKET_H
#define TICKET_H

#include "libwad.h"

#include <stdio.h>

struct wad_data;

ticket_t ticket_from_wad(struct wad_data* wad);

#endif
