// Copyright 2019 spycrab0
// Licensed under GPLv3+
// Refer to the LICENSE file included.

#include "tmd.h"

#include <stdlib.h>

#include "util.h"
#include "wad.h"

struct tmd_data {
  FILE* fh;
  uint64_t ios_version;
  uint64_t title_id;
  uint32_t title_type;
  uint16_t group_id; // TODO: Introduce enum for this
  uint16_t region;
  uint16_t title_version;
  uint16_t content_count;
  tmd_content_t* contents;
};

static tmd_t tmd_parse(FILE* fh)
{
  struct tmd_data* data = (struct tmd_data*)malloc(sizeof(struct tmd_data));

  if (data == NULL) {
    g_error = LIBWAD_BAD_ALLOC;
    return NULL;
  }

  data->fh = NULL;

  fseek(fh, 0x184, SEEK_CUR);

  fread(&data->ios_version, sizeof(data->ios_version), 1, fh);
  be_int64(&data->ios_version);

  fread(&data->title_id, sizeof(data->title_id), 1, fh);
  be_int64(&data->title_id);

  fread(&data->title_type, sizeof(data->title_type), 1, fh);
  be_int32(&data->title_type);

  fread(&data->group_id, sizeof(data->group_id), 1, fh);
  be_int16(&data->group_id);

  fseek(fh, sizeof(uint16_t), SEEK_CUR); // Skip zero field

  fread(&data->region, sizeof(data->region), 1, fh);
  be_int16(&data->region);

  // Skip ratings, reserved, ipc mask, reserved, access rights
  fseek(fh,
        sizeof(char[16]) + sizeof(char[12]) + sizeof(char[12]) +
            sizeof(char[18]) + sizeof(uint32_t),
        SEEK_CUR);

  fread(&data->title_version, sizeof(data->title_version), 1, fh);
  be_int16(&data->title_version);

  fread(&data->content_count, sizeof(data->content_count), 1, fh);
  be_int16(&data->content_count);

  // Skip boot index and padding
  fseek(fh, sizeof(uint16_t) + sizeof(uint16_t), SEEK_CUR);

  data->contents =
      (tmd_content_t*)malloc(sizeof(tmd_content_t) * data->content_count);

  if (data->contents == NULL) {
    g_error = LIBWAD_BAD_ALLOC;
    data_close(data);
    return NULL;
  }

  for (uint32_t i = 0; i < data->content_count; i++) {
    tmd_content_t* c = &(data->contents[i]);
    fread(&c->id, sizeof(c->id), 1, fh);
    be_int32(&c->id);

    fread(&c->index, sizeof(c->index), 1, fh);
    be_int16(&c->index);

    fread(&c->type, sizeof(c->type), 1, fh);
    be_int16(&c->type);

    fread(&c->size, sizeof(c->size), 1, fh);
    be_int64(&c->size);

    fread(&c->hash, sizeof(c->hash), 1, fh);
  }

  return data;
}

tmd_t tmd_from_wad(struct wad_data* wad)
{
  fseek(wad->fh, (long)wad_get_section_offset(wad, WAD_SECTION_TMD), SEEK_SET);

  return tmd_parse(wad->fh);
}

ticket_t tmd_open(const char* filename)
{
  FILE* fh = fopen(filename, "rb");
  struct tmd_data* data = tmd_parse(fh);

  data->fh = fh;

  return (ticket_t)data;
}

uint16_t tmd_get_content_count(tmd_t handle)
{
  return ((struct tmd_data*)handle)->content_count;
}

tmd_type_t tmd_get_title_type(tmd_t handle)
{
  return ((struct tmd_data*)handle)->title_type;
}

const char* tmd_get_title_type_string(tmd_t handle)
{
  tmd_type_t type = tmd_get_title_type(handle);

  switch (type) {
  case LIBWAD_TYPE_SYSTEM:
    return "System";
  case LIBWAD_TYPE_CHANNEL:
    return "Channel";
  case LIBWAD_TYPE_SYSTEM_CHANNEL:
    return "System Channel";
  case LIBWAD_TYPE_GAME_WITH_CHANNEL:
    return "Game with Channel";
  case LIBWAD_TYPE_DLC:
    return "DLC";
  case LIBWAD_TYPE_HIDDEN_CHANNEL:
    return "Hidden Channel";
  default:
    return "Unknown Type";
  }
}

uint64_t tmd_get_title_id(tmd_t handle)
{
  return ((struct tmd_data*)handle)->title_id;
}

uint32_t tmd_get_title_version(tmd_t handle)
{
  return ((struct tmd_data*)handle)->title_version;
}

tmd_region_t tmd_get_title_region(tmd_t handle)
{
  return ((struct tmd_data*)handle)->region;
}

const char* tmd_get_title_region_string(tmd_t handle)
{
  tmd_region_t region = tmd_get_title_region(handle);

  switch (region) {
  case LIBWAD_REGION_NTSC_J:
    return "NTSC-J";
  case LIBWAD_REGION_NTSC_U:
    return "NTSC-U";
  case LIBWAD_REGION_NTSC_K:
    return "NTSC-K";
  case LIBWAD_REGION_INTERNATIONAL:
    return "International";
  default:
    return "Unknown Region";
  }
}

uint64_t tmd_get_ios_version(tmd_t handle)
{
  return ((struct tmd_data*)handle)->ios_version;
}

void tmd_close(tmd_t handle)
{
  if (handle == NULL)
    return;

  FILE* fh = ((struct tmd_data*)handle)->fh;

  if (fh != NULL)
    fclose(fh);

  free(((struct tmd_data*)handle)->contents);
  free(handle);
}

tmd_content_t* tmd_get_content(tmd_t handle, uint16_t index)
{
  if (index >= tmd_get_content_count(handle))
    return NULL;

  return &((struct tmd_data*)handle)->contents[index];
}
