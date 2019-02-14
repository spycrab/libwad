// Copyright 2019 spycrab0
// Licensed under GPLv3+
// Refer to the LICENSE file included.

#include "data.h"

#include <memory.h>
#include <stdlib.h>

#include <mbedtls/aes.h>
#include <mbedtls/sha1.h>

#include "util.h"
#include "wad.h"

unsigned char* data_extract_from_wad(wad_t handle, uint16_t index,
                                     data_verify_t verify)
{
  struct wad_data* wad = (struct wad_data*)handle;
  tmd_t tmd = wad_get_tmd(wad);
  tmd_t ticket = wad_get_ticket(wad);

  fseek(wad->fh, (long)wad_get_section_offset(wad, WAD_SECTION_DATA), SEEK_SET);

  return data_extract((data_t)wad->fh, tmd, ticket, index, verify);
}

unsigned char* data_extract(data_t handle, wad_t tmd, tmd_t ticket,
                            uint16_t index, data_verify_t verify)
{
  tmd_content_t* content = tmd_get_content(tmd, index);

  if (content == NULL) {
    g_error = LIBWAD_OUT_OF_RANGE;
    return NULL;
  }

  unsigned char* enc_buffer =
      (unsigned char*)malloc(align64(content->size, 16));
  unsigned char* buffer = (unsigned char*)malloc(align64(content->size, 16));

  if (enc_buffer == NULL || buffer == NULL) {
    g_error = LIBWAD_BAD_ALLOC;

    free(enc_buffer);
    free(buffer);

    return NULL;
  }

  FILE* fh = (FILE*)handle;

  // Skip over the previous entries
  for (uint16_t i = 0; i < index; i++)
    fseek(fh, (long)align64(tmd_get_content(tmd, i)->size, 64), SEEK_CUR);

  if (fread(enc_buffer, align64(content->size, 16), 1, fh) != 1) {
    g_error = LIBWAD_IO_ERROR;
    free(enc_buffer);
    free(buffer);
    return 0;
  }

  unsigned const char* key = ticket_get_title_key(ticket);

  // Decrypt title key

  mbedtls_aes_context ctx;

  mbedtls_aes_setkey_dec(&ctx, key, 128);

  unsigned char iv[16] = {0};

  uint16_t x = content->index;

  be_int16(&x);

  memcpy(iv, &x, sizeof(x));

  int ret = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT,
                                  align64(content->size, 16), iv,
                                  &enc_buffer[0], &buffer[0]);

  free(enc_buffer);

  if (ret != 0) {
    g_error = LIBWAD_DECRYPTION_FAILED;
    free(buffer);
    return NULL;
  }

  if (verify == LIBWAD_DONT_VERIFY_HASH)
    return buffer;

  unsigned char hash[20];
  mbedtls_sha1_ret(buffer, content->size, hash);

  if (memcmp(hash, content->hash, 20) != 0) {
    g_error = LIBWAD_HASH_MISMATCH;
    free(buffer);
    return NULL;
  }

  return buffer;
}

data_t data_open(const char* filename)
{
  FILE* fh = fopen(filename, "rb");

  if (fh != NULL) {
    g_error = LIBWAD_OPEN_FAILED;
    return NULL;
  }

  return fh;
}

void data_close(data_t handle)
{
  if (handle == NULL)
    return;

  fclose((FILE*)handle);
}