// Copyright 2019 spycrab0
// Licensed under GPLv3+
// Refer to the LICENSE file included.

#include <ctype.h>
#include <libwad.h>
#include <stdio.h>
#include <stdlib.h>

#define OPTPARSE_IMPLEMENTATION
#include <optparse.h>

#include "info.h"

int main(int argc, char** argv)
{
  struct optparse options;

  optparse_init(&options, argv);

  const char* flags = "hvtmcap:";

  int display_ticket = 0, display_tmd = 0, display_certchain = 0;

  for (int i = optparse(&options, flags); i != -1;
       i = optparse(&options, flags)) {
    switch (i) {
    case 'h':
      printf("Options:\n\n"
             "-a (default):\tDisplay all information\n"
             "-c:\tDisplay certificate chain information\n"
             "-h:\tDisplay this list\n"
             "-m:\tDisplay TMD information\n"
             "-t:\tDisplay ticket information\n"
             "-v:\tDisplay version\n");
      return 0;
    case 't':
      display_ticket = 1;
      break;
    case 'm':
      display_tmd = 1;
      break;
    case 'c':
      display_certchain = 1;
      break;
    case 'v':
      printf("wadinfo from libwad version %s\n", libwad_get_version_string());
      return 0;
    case '?':
      printf(
          "Invalid arguments provided or parameter missing. See -h for help\n");
      return 1;
    }
  }

  if (!display_certchain && !display_ticket && !display_tmd) {
    display_certchain = display_ticket = display_tmd = 1;
  }

  const char* wad_path = optparse_arg(&options);

  if (wad_path == NULL) {
    fprintf(stderr, "%s [options] (wadfile)\n", argv[0]);
    return 1;
  }

  wad_t wad = wad_open(wad_path);

  if (wad == NULL) {
    fprintf(stderr, "Failed to parse wad '%s': %s\n", wad_path,
            libwad_get_error_msg());
    return 1;
  }

  if (display_certchain) {
    printf("\n\n--- Certificate chain ---\n\n");
    if (!info_print_certchain(wad_get_certchain(wad)))
      return 1;
  }

  if (display_ticket) {
    printf("\n\n--- Ticket ---\n\n");
    if (!info_print_ticket(wad_get_ticket(wad)))
      return 1;
  }

  if (display_tmd) {
    printf("\n\n--- Title metadata ---\n\n");
    if (!info_print_tmd(wad_get_tmd(wad)))
      return 1;
  }

  wad_close(wad);

  return 0;
}
