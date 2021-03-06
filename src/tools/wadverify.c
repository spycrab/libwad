// Copyright 2019 spycrab0
// Licensed under GPLv3+
// Refer to the LICENSE file included.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libwad.h>

#define OPTPARSE_IMPLEMENTATION
#include <optparse.h>

void show_help(const char* program)
{
  printf("%s [options] (wadfile)\n\n"
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

  for (int c = optparse_long(&options, flags, NULL); c != -1;
       c = optparse_long(&options, flags, NULL)) {
    switch (c) {
    case 'h':
      show_help(argv[0]);
      return 0;
    case 'v':
      printf("wadverify from libwad version %s\n", libwad_get_version_string());
      return 0;
    case '?':
      fprintf(
          stderr,
          "Invalid arguments provided or parameter missing. See -h for help\n");
      return 1;
    }
  }

  const char* wad_path = optparse_arg(&options);

  if (wad_path == NULL) {
    show_help(argv[0]);
    return 1;
  }

  wad_t wad = wad_open(wad_path);

  if (wad == NULL) {
    fprintf(stderr, "Failed to open: %s\n", libwad_get_error_msg());
    return 1;
  }

  printf("Opened successfully\n");

  tmd_t tmd = wad_get_tmd(wad);

  uint16_t content_count = tmd_get_content_count(tmd);

  int error_content = 0;

  for (uint16_t i = 0; i < content_count; i++) {
    printf("Content %2hu...", i);

    unsigned char* data = data_extract_from_wad(wad, i, LIBWAD_VERIFY_HASH);

    if (data == NULL) {
      printf("Error: %s\n", libwad_get_error_msg());
      error_content = 1;
      continue;
    }

    printf("Ok\n");

    free(data);
  }

  wad_close(wad);

  if (error_content) {
    fprintf(stderr, "Failed to verify\n");
    return 1;
  }

  printf("Verified\n");
  return 0;
}
