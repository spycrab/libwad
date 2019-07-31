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
         "-f, --from INDEX\tStart extracting at entry\n"
         "-h, --help\t\tShow this message\n"
         "-i, --ignore-hashes\tIgnore content hashes\n"
         "-k, --keep-going\tKeep going despite errors\n"
         "-n, --entry INDEX\tExtract given entry only\n"
         "-o, --output NAME\tOutput path\n"
         "-q, --quiet\t\tQuiet\n"
         "-s, --sections\t\tExtract sections instead of contents\n"
         "-t, --to INDEX\t\tStop extracting at entry\n"
         "-v, --version\t\tDisplay version\n\n",
         program);
}

int main(int argc, char** argv)
{
  struct optparse options;

  optparse_init(&options, argv);

  uint16_t from = 0, to = 0;
  int quiet = 0, keep_going = 0, verify_hash = 0, sections = 0;
  const char* out_path = NULL;

  struct optparse_long flags[] = {{"from", 'f', OPTPARSE_OPTIONAL},
                                  {"help", 'h', OPTPARSE_NONE},
                                  {"ignore-hashes", 'i', OPTPARSE_NONE},
                                  {"keep-going", 'k', OPTPARSE_NONE},
                                  {"entry", 'n', OPTPARSE_OPTIONAL},
                                  {"output", 'o', OPTPARSE_REQUIRED},
                                  {"quiet", 'q', OPTPARSE_NONE},
                                  {"sections", 's', OPTPARSE_NONE},
                                  {"to", 't', OPTPARSE_OPTIONAL},
                                  {"version", 'v', OPTPARSE_NONE},
                                  {0}};

  for (int c = optparse_long(&options, flags, NULL); c != -1;
       c = optparse_long(&options, flags, NULL)) {
    switch (c) {
    case 'h':
      show_help(argv[0]);
      return 0;
    case 's':
      sections = 1;
      break;
    case 'n':
      from = atoi(options.optarg);
      to = from + 1;
      break;
    case 'k':
      keep_going = 1;
    case 'i':
      verify_hash = 0;
      break;
    case 'v':
      printf("wadextract from libwad version %s\n",
             libwad_get_version_string());
      return 0;
    case 'f':
      from = atoi(optparse_arg(&options));
      break;
    case 't':
      to = atoi(optparse_arg(&options)) + 1;
      break;
    case 'o':
      printf("Foo\n");
      out_path = options.optarg;
      break;
    case 'q':
      quiet = 1;
      break;
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
    fprintf(stderr, "Failed to open '%s': %s", wad_path,
            libwad_get_error_msg());
    return 1;
  }

  tmd_t tmd = wad_get_tmd(wad);

  const char* title_id = util_title_id_to_string(tmd_get_title_id(tmd));
  uint16_t count = tmd_get_content_count(tmd);

  if (sections) {
    uint64_t offsets[WAD_SECTION_FOOTER + 1];
    uint32_t sizes[WAD_SECTION_FOOTER + 1];

    const char* section_names[] = {"header", "certchain", "ticket",
                                   "tmd",    "data",      "footer"};

    for (int i = 0; i <= WAD_SECTION_FOOTER; i++) {
      offsets[i] = wad_get_section_offset(wad, i);
      sizes[i] = wad_get_section_size(wad, i);
    }

    wad_close(wad);

    FILE* wad_fh = fopen(wad_path, "rb");

    for (int i = 0; i <= WAD_SECTION_FOOTER; i++) {
      printf("Extracting %s...", section_names[i]);

      if (sizes[i] == 0) {
        printf("Empty\n");
        continue;
      }

      char filename[256];

      snprintf(filename, sizeof(filename), "%s-%s.bin",
               out_path == NULL ? title_id : out_path, section_names[i]);

      FILE* fh = fopen((const char*)filename, "wb");

      if (fh == NULL) {
        fclose(wad_fh);
        printf("Error: Failed to open file for writing '%s'\n", filename);

        if (keep_going)
          continue;
        else
          return 1;
      }

      char* buffer = (char*)malloc(sizes[i]);

      fseek(wad_fh, (long)offsets[i], SEEK_SET);
      fread(buffer, sizes[i], 1, wad_fh);
      fwrite(buffer, sizes[i], 1, fh);

      free(buffer);

      fclose(fh);

      printf("Ok\n");
    }

    fclose(wad_fh);
    return 0;
  }

  if (to == 0)
    to = count;

  if (to <= from) {
    fprintf(stderr, "Bad range given\n");
    return 1;
  }

  if (from >= count || to > count) {
    fprintf(stderr, "Limits exceed number of contents %d\n", count);
    return 1;
  }

  for (uint16_t i = from; i < to; i++) {

    if (!quiet)
      printf("Extracting content %2hu...", i);

    unsigned char* data = data_extract_from_wad(
        wad, i, verify_hash ? LIBWAD_VERIFY_HASH : LIBWAD_DONT_VERIFY_HASH);

    if (data == NULL) {
      if (keep_going) {
        printf("Error: %s\n", libwad_get_error_msg());
        continue;
      }

      fprintf(stderr, "Failed to extract entry %hu: %s\n", i,
              libwad_get_error_msg());
      return 1;
    }

    char filename[256];

    if (from + 1 == to)
      snprintf(filename, sizeof(filename), "%s%s",
               out_path == NULL ? title_id : out_path,
               out_path == NULL ? ".bin" : "");
    else
      snprintf(filename, sizeof(filename), "%s-%04hu.bin",
               out_path == NULL ? title_id : out_path, i);

    FILE* fh = fopen(filename, "wb");

    uint64_t size = tmd_get_content(tmd, i)->size;

    if (fwrite(data, (long)size, 1, fh) != 1) {
      if (keep_going) {
        printf("Error: Failed to write\n");
        free(data);
        continue;
      }

      free(data);
      fprintf(stderr, "Failed to write entry %hu\n", i);
      return 1;
    }

    free(data);

    if (!quiet)
      printf("Ok\n");

    fclose(fh);
  }

  printf("\nDone.\n");

  return 0;
}
