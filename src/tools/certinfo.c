// Copyright 2019 spycrab0
// Licensed under GPLv3+
// Refer to the LICENSE file included.

#include "info.h"

#include <stdio.h>

#include <libwad.h>

#define OPTPARSE_IMPLEMENTATION
#include <optparse.h>

void show_help(const char* program)
{
  printf("%s [options] (certfile)\n\n"
         "Options:\n\n"
         "-h, --help\t\tShow this message\n"
         "-v, --version\t\tDisplay version\n\n",
         program);
}

int main(int argc, char** argv)
{
  struct optparse options;
  optparse_init(&options, argv);

  struct optparse_long flags[] = {
      {"help", 'h', OPTPARSE_NONE}, {"version", 'v', OPTPARSE_NONE}, {0}};

  for (char c = optparse_long(&options, flags, NULL); c != -1;
       c = optparse_long(&options, flags, NULL)) {
    switch (c) {
    case 'h':
      show_help(argv[0]);
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
    show_help(argv[0]);
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
