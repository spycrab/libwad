// Copyright 2019 spycrab0
// Licensed under GPLv3+
// Refer to the LICENSE file included.

#include "info.h"

#include <inttypes.h>
#include <stdio.h>

int info_print_certchain(certchain_t handle)
{
  size_t count = certchain_get_cert_count(handle);

  printf("Number of certificates: %zu\n\n", count);

  for (size_t i = 0; i < count; i++) {
    cert_t* c = certchain_get_cert(handle, i);

    if (c == NULL) {
      printf("Error while getting certificate %zu: %s\n", i,
             libwad_get_error_msg());

      return 0;
    }

    printf("Issuer: %s\n", c->issuer);
    printf("Child: %s\n", c->child_cert);

    printf("\n");
  }

  return 1;
}

int info_print_ticket(ticket_t handle)
{
  uint64_t id = ticket_get_title_id(handle);

  printf("Title ID:\t%016" PRIx64 " (%s)\n", id, util_title_id_to_string(id));
  printf("Title Key:\t");

  unsigned const char* key = ticket_get_title_key(handle);

  for (uint16_t x = 0; x < 16; x++)
    printf("%02hhx", key[x]);

  printf("\n");

  return 1;
}

int info_print_tmd(tmd_t handle)
{
  printf("IOS Version:\t%" PRIu64 " (%016" PRIx64 ")\n",
         tmd_get_ios_version(handle) & 0xff, tmd_get_ios_version(handle));
  printf("Title Version:\t%d\n", tmd_get_title_version(handle));
  printf("Title Type:\t%s (%d)\n", tmd_get_title_type_string(handle),
         tmd_get_title_type(handle));
  printf("Title Region:\t%s (%d)\n\n", tmd_get_title_region_string(handle),
         tmd_get_title_region(handle));

  printf("Content count:\t%" PRIu16 "\n", tmd_get_content_count(handle));

  printf("\nContents:\n\n");

  for (uint16_t i = 0; i < tmd_get_content_count(handle); i++) {
    tmd_content_t* c = tmd_get_content(handle, i);

    printf("ID: %" PRIu32 "\n", c->id);
    printf("Index: %" PRIu16 "\n", c->index);
    printf("Type: %04" PRIx16 "\n", c->type);
    printf("Size: %" PRIu64 "\n", c->size);
    printf("Hash: ");

    for (uint16_t x = 0; x < 20; x++)
      printf("%02hhx", c->hash[x]);

    printf("\n\n");
  }

  return 1;
}