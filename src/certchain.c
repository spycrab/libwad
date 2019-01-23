// Copyright 2019 spycrab0
// Licensed under GPLv3+
// Refer to the LICENSE file included.

#include "certchain.h"

#include <stdlib.h>

#include "util.h"
#include "wad.h"

certchain_t certchain_parse(FILE* fh, size_t end)
{
  struct certchain_data* data = malloc(sizeof(struct certchain_data));
  data->fh = NULL;
  data->cert_count = 0;
  data->chain = NULL;

  if (ftell(fh) >= end)
    return NULL;

  data->chain = malloc(sizeof(struct link));

  struct link* current = data->chain;
  while (ftell(fh) < end) {
    current->next = NULL;
    cert_t* cert = &current->data;
    cert->signature = NULL;
    cert->public_key = NULL;

    fread(&cert->signature_type, sizeof(cert->signature_type), 1, fh);
    be_int32(&cert->signature_type);

    uint32_t signature_size =
        certchain_get_signature_key_length(cert->signature_type);

    if (signature_size == 0) {
      printf("Bad signature type: %x\n", cert->signature_type);
      certchain_close((certchain_t)data);
      return NULL;
    }

    cert->signature = malloc(signature_size);

    fread(cert->signature, signature_size, 1, fh);

    fseek(fh, 0x3c, SEEK_CUR);

    fread(cert->issuer, sizeof(cert->issuer), 1, fh);

    fread(&cert->key_type, sizeof(cert->key_type), 1, fh);
    be_int32(&cert->key_type);

    uint32_t key_size = certchain_get_private_key_length(cert->key_type);

    if (key_size == 0) {
      printf("Bad key type: %x\n", cert->key_type);
      certchain_close((certchain_t)data);
      return NULL;
    }

    fread(&cert->child_cert, sizeof(cert->child_cert), 1, fh);

    cert->public_key = malloc(key_size);

    fread(cert->public_key, key_size, 1, fh);

    fseek(fh, align32(ftell(fh)), SEEK_SET);

    data->cert_count++;

    if (ftell(fh) < end) {
      current->next = malloc(sizeof(struct link));
      current = current->next;
    }
  }

  return data;
}

certchain_t certchain_from_wad(wad_t handle)
{
  struct wad_data* wad = (struct wad_data*)handle;

  fseek(wad->fh, wad_get_section_offset(wad, WAD_SECTION_CERTCHAIN), SEEK_SET);
  
  return certchain_parse(wad->fh, align32(0x20) + align32(wad->certchain_size));
}

certchain_t certchain_open(const char* filename)
{
  FILE* fh = fopen(filename, "rb");

  fseek(fh, 0, SEEK_END);
  size_t end = ftell(fh);

  fseek(fh, 0, SEEK_SET);

  struct certchain_data* data =
      (struct certchain_data*)certchain_parse(fh, end);
  data->fh = fh;

  return data;
}

void certchain_close(certchain_t handle)
{
  if (handle == NULL)
    return;

  struct certchain_data* data = (struct certchain_data*)handle;

  if (data->fh != NULL)
    fclose(data->fh);

  struct link* current = data->chain;

  while (current != NULL) {
    // Free cert_t data
    free(current->data.signature);
    free(current->data.public_key);

    struct link* next = current->next;

    free(current);
    current = next;
  }

  free(handle);
}

size_t certchain_get_cert_count(certchain_t handle)
{
  return ((struct certchain_data*)handle)->cert_count;
}

cert_t* certchain_get_cert(certchain_t handle, size_t index)
{
  struct certchain_data* data = (struct certchain_data*)handle;

  if (data->cert_count <= index) {
    g_error = LIBWAD_OUT_OF_RANGE;
    return NULL;
  }

  for (struct link* l = data->chain; l != NULL; l = l->next) {
    if (index-- == 0)
      return &l->data;
  }

  return NULL;
}

//! Get length for a key of a given type
size_t certchain_get_private_key_length(cert_key_type_t type)
{
  switch (type) {
  case CERT_KEY_TYPE_RSA_4096:
    return 0x200 + 0x4 + 0x38;
  case CERT_KEY_TYPE_RSA_2048:
    return 0x100 + 0x4 + 0x38;
  case CERT_KEY_TYPE_ECC_B233:
    return 60 + 0 + 60;
  default:
    return 0;
  }
}

//! Get length for a signature of a given type
size_t certchain_get_signature_key_length(cert_signature_type_t type)
{
  switch (type) {
  case CERT_SIGNATURE_TYPE_RSA_4096:
    return 0x200;
  case CERT_SIGNATURE_TYPE_RSA_2048:
    return 0x100;
  case CERT_SIGNATURE_TYPE_ECC:
    return 0x40;
    break;
  default:
    return 0;
  }
}