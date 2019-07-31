// Copyright 2019 spycrab0
// Licensed under GPLv3+
// Refer to the LICENSE file included.

#include "ticket.h"

#include <mbedtls/aes.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "wad.h"

struct ticket_data {
  FILE* fh;
  char issuer[64];
  unsigned char title_key[16];
  char console_id[4];
  uint64_t title_id;
};

static struct ticket_data* ticket_parse(FILE* fh)
{
  struct ticket_data* data =
      (struct ticket_data*)malloc(sizeof(struct ticket_data));

  if (data == NULL) {
    g_error = LIBWAD_BAD_ALLOC;
    return NULL;
  }

  data->fh = NULL;

  unsigned char enc_title_key[16];

  // Skip signature fields
  fseek(fh, 0x0140, SEEK_CUR);

  fread(&data->issuer, sizeof(data->issuer), 1, fh);

  int use_debug_key =
      strncmp("Root-CA00000002-XS00000006", data->issuer, 64) == 0;

  fseek(fh, 0x3c + 0x03, SEEK_CUR);

  fread(enc_title_key, sizeof(enc_title_key), 1, fh);

  fseek(fh, 1, SEEK_CUR); // Unknown
  fseek(fh, 8, SEEK_CUR); // Ticket ID

  fread(&data->console_id, sizeof(data->console_id), 1, fh);

  fread(&data->title_id, sizeof(data->title_id), 1, fh);

  fseek(fh, 2 + 2 + 4 + 4 + 1, SEEK_CUR);

  uint8_t key_type;
  fread(&key_type, sizeof(key_type), 1, fh);

  const unsigned char* key;

  switch (key_type) {
  case 1:
    key = KOREA_COMMON_KEY;
    break;
  case 2:
    key = VWII_COMMON_KEY;
  case 0:
  default:
    // Dolphin ignores invalid key types, so do we
    key = NORMAL_COMMON_KEY;
  }

  if (use_debug_key)
    key = DEBUG_COMMON_KEY;

  // Decrypt title key

  mbedtls_aes_context ctx;

  mbedtls_aes_setkey_dec(&ctx, key, 128);

  unsigned char iv[16] = {0};
  memcpy(iv, &data->title_id, 8);

  int ret = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, 16, iv,
                                  &enc_title_key[0],
                                  (unsigned char*)&data->title_key[0]);

  if (ret != 0) {
    free(data);
    return NULL;
  }

  be_int64(&data->title_id);

  return data;
}

ticket_t ticket_from_wad(struct wad_data* wad)
{
  fseek(wad->fh, (long)wad_get_section_offset(wad, WAD_SECTION_TICKET),
        SEEK_SET);
  return ticket_parse(wad->fh);
}

ticket_t ticket_open(const char* filename)
{
  FILE* fh = fopen(filename, "rb");

  if (!fh) {
    g_error = LIBWAD_OPEN_FAILED;
    return NULL;
  }

  struct ticket_data* data = ticket_parse(fh);

  data->fh = fh;

  return (ticket_t)data;
}

void ticket_close(ticket_t handle)
{
  if (handle == NULL)
    return;

  FILE* fh = ((struct ticket_data*)handle)->fh;

  if (fh)
    fclose(fh);

  free(handle);
}

const char* ticket_get_issuer(ticket_t handle)
{
  return ((struct ticket_data*)handle)->issuer;
}

uint64_t ticket_get_title_id(ticket_t handle)
{
  return ((struct ticket_data*)handle)->title_id;
}

unsigned const char* ticket_get_title_key(ticket_t handle)
{
  return ((struct ticket_data*)handle)->title_key;
}
