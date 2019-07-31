// Copyright 2019 spycrab0
// Licensed under GPLv3+
// Refer to the LICENSE file included.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libwad.h>

#define OPTPARSE_IMPLEMENTATION
#include <optparse.h>

// Taken from util.c
// Not the prettiest sight but I don't want to expose util.h
static uint16_t be_int16v(uint16_t i)
{
  return (i & 0xff) << 8 | (i & 0xff00) >> 8;
}

static void be_int16(uint16_t* i) { *i = be_int16v(*i); }

static uint32_t be_int32v(uint32_t i)
{
  return (uint32_t)be_int16v(i & 0xffff) << 16 |
         be_int16v((i & 0xffff0000) >> 16);
}

static uint32_t align32(uint32_t offset)
{
  if (offset % 64 == 0)
    return offset;

  return offset + 64 - (offset % 64);
}

static void be_int32(uint32_t* i) { *i = be_int32v(*i); }
// End of util.c stuff

uint32_t get_file_size(const char* path)
{
  FILE* fh = fopen(path, "rb");
  fseek(fh, 0, SEEK_END);
  uint32_t size = (uint32_t)ftell(fh);
  fclose(fh);

  return size;
}

void pad_file(FILE* fh)
{
  uint32_t offset = ftell(fh);

  for (uint64_t i = 0; i < align32(offset) - offset; i++)
    fwrite("\0", 1, 1, fh);
}

void append_file(FILE* dst, const char* path)
{
  FILE* src = fopen(path, "rb");

  for (int i = fgetc(src); i != EOF; i = fgetc(src))
    fputc(i, dst);
}

void show_help(const char* program)
{
  printf("%s [options]\n\nOptions:\n\n"
         "-c, --certchain CERT\tCertchain path (required)\n"
         "-d, --data DATA\t\tData path (required)\n"
         "-f, --footer FOOTER\tFooter path (required)\n"
         "-h, --help\t\tDisplay this list\n"
         "-o, --output OUTPUT\tOutput path (required)\n"
         "-q, --quiet\t\tQuiet\n"
         "-t, --ticket TICKET\tTicket path (required)\n"
         "-m, --tmd TMD\t\tTitle metadata path (required)\n"
         "-v, --version\t\tDisplay version\n\n",
         program);
}

int main(int argc, char** argv)
{
  struct optparse options;

  optparse_init(&options, argv);

  int quiet = 0;

  const char* out_path = NULL;
  const char* certchain_path = NULL;
  const char* ticket_path = NULL;
  const char* tmd_path = NULL;
  const char* data_path = NULL;
  const char* footer_path = NULL;

  struct optparse_long flags[] = {
      {"footer", 'f', OPTPARSE_OPTIONAL}, {"certchain", 'c', OPTPARSE_REQUIRED},
      {"data", 'd', OPTPARSE_REQUIRED},   {"help", 'h', OPTPARSE_NONE},
      {"tmd", 'm', OPTPARSE_REQUIRED},    {"output", 'o', OPTPARSE_REQUIRED},
      {"quiet", 'q', OPTPARSE_NONE},      {"ticket", 't', OPTPARSE_REQUIRED},
      {"version", 'v', OPTPARSE_NONE},    {0}};

  for (int c = optparse_long(&options, flags, NULL); c != -1;
       c = optparse_long(&options, flags, NULL)) {
    switch (c) {
    case 'h':
      show_help(argv[0]);
      return 0;
    case 'c':
      certchain_path = options.optarg;
      break;
    case 'd':
      data_path = options.optarg;
      break;
    case 'f':
      footer_path = options.optarg;
      break;
    case 'm':
      tmd_path = options.optarg;
      break;
    case 'o':
      out_path = options.optarg;
      break;
    case 't':
      ticket_path = options.optarg;
      break;
    case 'v':
      printf("wadextract from libwad version %s\n",
             libwad_get_version_string());
      return 0;
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

  if (out_path == NULL) {
    show_help(argv[0]);
    return 1;
  }

  if (certchain_path == NULL || ticket_path == NULL || tmd_path == NULL ||
      data_path == NULL) {
    fprintf(stderr, "Error: Missing path to a required section (certificate "
                    "chain, ticket, tmd, data). See -h for help.\n");
    return 1;
  }

  FILE* fh = fopen(out_path, "wb");

  if (fh == NULL) {
    fprintf(stderr, "Error: Failed to open '%s' for writing\n", out_path);
    return 1;
  }

  uint32_t certchain_size = get_file_size(certchain_path);
  uint32_t ticket_size = get_file_size(ticket_path);
  uint32_t tmd_size = get_file_size(tmd_path);
  uint32_t data_size = get_file_size(data_path);
  uint32_t footer_size = footer_path != NULL ? get_file_size(footer_path) : 0;

  if (!quiet)
    printf("Writing header...\n");

  uint32_t magic = 0x20;

  be_int32(&magic);
  be_int32(&ticket_size);
  be_int32(&certchain_size);
  be_int32(&tmd_size);
  be_int32(&data_size);
  be_int32(&footer_size);

  // Write header
  fwrite(&magic, sizeof(uint32_t), 1, fh);
  fwrite("Is\0\0", 4, 1, fh); // Is the type important here?
  fwrite(&certchain_size, sizeof(uint32_t), 1, fh);
  fwrite("\0\0\0\0", 4, 1, fh); // Padding
  fwrite(&ticket_size, sizeof(uint32_t), 1, fh);
  fwrite(&tmd_size, sizeof(uint32_t), 1, fh);
  fwrite(&data_size, sizeof(uint32_t), 1, fh);
  fwrite(&footer_size, sizeof(uint32_t), 1, fh);

  pad_file(fh);

  // Certchain
  if (!quiet)
    printf("Writing certificate chain...\n");

  append_file(fh, certchain_path);
  pad_file(fh);

  // Ticket
  if (!quiet)
    printf("Writing ticket...\n");

  append_file(fh, ticket_path);
  pad_file(fh);

  // TMD
  if (!quiet)
    printf("Writing TMD...\n");

  append_file(fh, tmd_path);
  pad_file(fh);

  // Data
  if (!quiet)
    printf("Writing data...\n");

  append_file(fh, data_path);
  pad_file(fh);

  if (footer_path) {
    if (!quiet)
      printf("Writing footer...\n");

    // Footer
    append_file(fh, footer_path);
    pad_file(fh);
  }

  fclose(fh);

  if (!quiet)
    printf("'%s' created successfully.", out_path);

  return 0;
}
