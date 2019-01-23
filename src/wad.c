// Copyright 2019 spycrab0
// Licensed under GPLv3+
// Refer to the LICENSE file included.

#include "wad.h"

#include <stdio.h>
#include <stdlib.h>

#include "certchain.h"
#include "ticket.h"
#include "tmd.h"
#include "util.h"

#include "version.h"

int g_error = 0;

wad_t wad_open(const char* filename)
{
  FILE* fh = fopen(filename, "rb");

  if (!fh) {
    g_error = LIBWAD_OPEN_FAILED;
    return NULL;
  }

  struct wad_data* wad = (struct wad_data*)malloc(sizeof(struct wad_data));

  wad->certchain = NULL;
  wad->ticket = NULL;
  wad->tmd = NULL;

  wad->fh = fh;

  // Parse header
  uint32_t header_magic;

  fread(&header_magic, sizeof(header_magic), 1, fh);

  be_int32(&header_magic);

  if (header_magic != 0x20) {
    g_error = LIBWAD_BAD_MAGIC;
    wad_close(wad);
    return NULL;
  }

  fread(&wad->type, sizeof(wad->type), 1, fh);
  be_int32(&wad->type);

  fread(&wad->certchain_size, sizeof(wad->certchain_size), 1, fh);
  be_int32(&wad->certchain_size);

  fseek(fh, 4, SEEK_CUR);

  fread(&wad->ticket_size, sizeof(wad->ticket_size), 1, fh);
  be_int32(&wad->ticket_size);

  fread(&wad->tmd_size, sizeof(wad->tmd_size), 1, fh);
  be_int32(&wad->tmd_size);

  fread(&wad->data_size, sizeof(wad->data_size), 1, fh);
  be_int32(&wad->data_size);

  fread(&wad->footer_size, sizeof(wad->footer_size), 1, fh);
  be_int32(&wad->footer_size);

  wad->certchain = certchain_from_wad(wad);

  if (wad->certchain == NULL) {
    g_error = LIBWAD_BAD_CERTCHAIN;
    wad_close(wad);
    return NULL;
  }

  wad->ticket = ticket_from_wad(wad);

  if (wad->ticket == NULL) {
    g_error = LIBWAD_BAD_TICKET;
    wad_close(wad);
    return NULL;
  }

  wad->tmd = tmd_from_wad(wad);

  if (wad->tmd == NULL) {
    g_error = LIBWAD_BAD_TMD;
    wad_close(wad);
    return NULL;
  }

  if (ferror(wad->fh) != 0) {
    g_error = LIBWAD_IO_ERROR;
    wad_close(wad);
    return NULL;
  }

  return wad;
}

void wad_close(wad_t handle)
{
  if (handle == NULL)
    return;

  struct wad_data* wad = (struct wad_data*)handle;

  fclose(wad->fh);
  certchain_close(wad->certchain);
  ticket_close(wad->ticket);
  tmd_close(wad->tmd);
  free(wad);

  handle = NULL;
}

const char* libwad_get_error_msg()
{
  switch (g_error) {
  case LIBWAD_NO_ERROR:
    return "No error";
  case LIBWAD_OPEN_FAILED:
    return "Failed to open wad";
  case LIBWAD_BAD_MAGIC:
    return "Bad magic - file might not be in wad format";
  case LIBWAD_BAD_TICKET:
    return "Failed to parse ticket";
  case LIBWAD_BAD_TMD:
    return "Failed to parse title metadata";
  case LIBWAD_IO_ERROR:
    return "I/O Error";
  case LIBWAD_DECRYPTION_FAILED:
    return "Decryption failed";
  case LIBWAD_BAD_ALLOC:
    return "Failed to allocate memory";
  case LIBWAD_OUT_OF_RANGE:
    return "Index out of range";
  case LIBWAD_HASH_MISMATCH:
    return "Hashes do not match";
  case LIBWAD_BAD_CERTCHAIN:
    return "Bad certchain";
  default:
    return "Unknown error";
  }
}

libwad_error_t libwad_get_error() { return g_error; }

uint64_t wad_get_section_offset(wad_t handle, wad_section_t type)
{
  if (type > WAD_SECTION_FOOTER)
    return WAD_BAD_SECTION;

  uint64_t offset = 0;

  for (int i = type - 1; i >= 0; i--)
    offset += align32(wad_get_section_size(handle, i));

  return offset;
}

uint32_t wad_get_section_size(wad_t handle, wad_section_t type)
{
  if (type > WAD_SECTION_FOOTER)
    return WAD_BAD_SECTION;

  struct wad_data* data = (struct wad_data*)handle;

  switch (type) {
  case WAD_SECTION_HEADER:
    return 0x20;
  case WAD_SECTION_CERTCHAIN:
    return data->certchain_size;
  case WAD_SECTION_TICKET:
    return data->ticket_size;
  case WAD_SECTION_TMD:
    return data->tmd_size;
  case WAD_SECTION_DATA:
    return data->data_size;
  case WAD_SECTION_FOOTER:
    return data->footer_size;
  }
}

const char* libwad_get_version_string() { return VERSION_STRING; }

tmd_t wad_get_tmd(wad_t handle)
{
  return (tmd_t)((struct wad_data*)handle)->tmd;
}

ticket_t wad_get_ticket(wad_t handle)
{
  return (ticket_t)((struct wad_data*)handle)->ticket;
}

certchain_t wad_get_certchain(wad_t handle)
{
  return (ticket_t)((struct wad_data*)handle)->certchain;
}