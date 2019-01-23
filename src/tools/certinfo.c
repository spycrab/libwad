// Copyright 2019 spycrab0
// Licensed under GPLv3+
// Refer to the LICENSE file included.

#include "info.h"

#include <stdio.h>

#include <libwad.h>

#define OPTPARSE_IMPLEMENTATION
#include <optparse.h>

int main(int argc, char** argv)
{
  struct optparse options;
  optparse_init(&options, argv);

  const char* flags = "hv";

  for (char c = optparse(&options, flags); c != -1;
       c = optparse(&options, flags)) {
    switch (c) {
    case 'h':
      printf("Options:\n\n"
             "-h:\tDisplay this list\n"
             "-v:\tDisplay version\n");
      return 0;
    case 'v':
      printf("certinfo from libwad version %s\n", libwad_get_version_string());
      return 0;
    case '?':
      printf(
          "Invalid arguments provided or parameter missing. See -h for help\n");
      return 1;
    }
  }

  const char* certchain_path = optparse_arg(&options);

  if (certchain_path == NULL) {
    fprintf(stderr, "%s [options] (certfile)\n", argv[0]);
    return 1;
  }

  certchain_t certchain = certchain_open(certchain_path);

  if (certchain == NULL) {
    fprintf(stderr, "Failed to parse certchain '%s': %s\n", certchain_path,
            libwad_get_error_msg());
    return 1;
  }

  if (!info_print_certchain(certchain))
    return 1;

  return 0;
}