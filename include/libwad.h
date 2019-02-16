// Copyright 2019 spycrab0
// Licensed under GPLv3+
// Refer to the LICENSE file included.

#ifndef LIBWAD_H
#define LIBWAD_H

#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
#define W_EXPORT __declspec(dllexport) extern
#else
#define W_EXPORT __attribute__((visibility("default")))
#endif

//! \file

//! Error codes used by the library
typedef enum {
  //! No error has occured thus far
  LIBWAD_NO_ERROR = 0,
  //! Failed to open a file for reading
  LIBWAD_OPEN_FAILED = 1,
  //! The provided file has a bad magic number at the beginning
  LIBWAD_BAD_MAGIC = 2,
  //! The provided wad has a bad ticket
  LIBWAD_BAD_TICKET = 3,
  //! The provided wad has a bad title metadata
  LIBWAD_BAD_TMD = 4,
  //! Error while reading or writing a file
  LIBWAD_IO_ERROR = 5,
  //! Failed to decrypt contents
  LIBWAD_DECRYPTION_FAILED = 6,
  //! Failed to allocate memory
  LIBWAD_BAD_ALLOC = 7,
  //! The given index exceeds the amount of elements
  LIBWAD_OUT_OF_RANGE = 8,
  //! The extracted data has a hash that does not match the one given in the
  //! metadata
  LIBWAD_HASH_MISMATCH = 9,
  //! The provided file has a bad certchain
  LIBWAD_BAD_CERTCHAIN = 10,
} libwad_error_t;

//@{
//! @name WAD

//! A handle representing a wad file
typedef void* wad_t;

#define WAD_BAD_SECTION 0xffffffff

//! Enum listing all sections of a wad
typedef enum {
  WAD_SECTION_HEADER = 0,
  WAD_SECTION_CERTCHAIN = 1,
  WAD_SECTION_TICKET = 2,
  WAD_SECTION_TMD = 3,
  WAD_SECTION_DATA = 4,
  WAD_SECTION_FOOTER = 5
} wad_section_t;

//! Opens a wad file for reading
/// @param path the path to the file to be opened
/// @returns A wad_t handle on success or NULL on error (See
/// libwad_get_error() for more details)
W_EXPORT wad_t wad_open(const char* path);

//! Closes a wad handle and frees its resources
/// @param handle the handle to be free'd
W_EXPORT void wad_close(wad_t handle);

//! Gets the offset of a given wad section
/// @returns offset of the section in bytes or WAD_BAD_SECTION on error
W_EXPORT uint64_t wad_get_section_offset(wad_t handle, wad_section_t type);

//! Gets the size of a given wad section
/// @returns size of the section in bytes or WAD_BAD_SECTION on error
W_EXPORT uint32_t wad_get_section_size(wad_t handle, wad_section_t type);

// @}

//! Get the last error
/// @returns An error code describing what went wrong
/// \remark Use libwad_get_error_msg() for a human readable version
W_EXPORT libwad_error_t libwad_get_error();

//! Get the last error as a string
/// @returns A human readable string describing the last error that occured
W_EXPORT const char* libwad_get_error_msg();

//! Get the library version
/// @returns A string describing the current release, build and branch
W_EXPORT const char* libwad_get_version_string();

//@{
//! @name Certificate chain

//! A handle representing a certificate chain file or certificate chain section
//! of a wad file
typedef void* certchain_t;

//! Struct containing all information stored in one of the certificate chain's
//! entries.
typedef struct {
  //! Value is one of the types listed in cert_signature_type_t
  uint32_t signature_type;
  //! Signature. Type and length depends on signature_type
  unsigned char* signature;
  //! Certificate issuer
  unsigned char issuer[64];
  //! Value is one of the types listed in cert_key_type_t
  uint32_t key_type;
  //! Child certificate that is signed by this cert
  unsigned char child_cert[64];
  //! Public key. Type and length depends on key_type
  unsigned char* public_key;
} cert_t;

//! Types of signatures used by cert_t
typedef enum {
  CERT_SIGNATURE_TYPE_RSA_4096 = 0x10000,
  CERT_SIGNATURE_TYPE_RSA_2048 = 0x10001,
  CERT_SIGNATURE_TYPE_ECC = 0x10002
} cert_signature_type_t;

//! Types of keys used by cert_t
typedef enum {
  CERT_KEY_TYPE_RSA_4096 = 0,
  CERT_KEY_TYPE_RSA_2048 = 1,
  CERT_KEY_TYPE_ECC_B233 = 2
} cert_key_type_t;

//! Opens a certificate chain file for reading
/// @param filename path to the file to be opened
/// @returns A certchain_t handle on success or NULL on error (See
/// libwad_get_error() for more details)
W_EXPORT certchain_t certchain_open(const char* filename);

//! Closes a certchain handle and frees its resources
/// @param handle The handle to be closed
W_EXPORT void certchain_close(certchain_t certchain);

//! Get certchain section of a wad file
W_EXPORT certchain_t wad_get_certchain(wad_t handle);

//! Get the amount of certificates stored in this certificate chain
W_EXPORT size_t certchain_get_cert_count(certchain_t handle);

//! Get single certificate out of the certchain
W_EXPORT cert_t* certchain_get_cert(certchain_t handle, size_t index);

//! Get length for a key of a given type
W_EXPORT size_t certchain_get_private_key_length(cert_key_type_t type);

//! Get length for a signature of a given type
W_EXPORT size_t certchain_get_signature_key_length(cert_signature_type_t type);

//@}

//@{
//! @name Title Metadata

//! A handle representing a tmd file or tmd section of a wad file
typedef void* tmd_t;

//! Enum describing all types of titles
typedef enum {
  //! Title is a part of the system
  LIBWAD_TYPE_SYSTEM = 0x00001,
  //! Title is a game
  LIBWAD_TYPE_GAME = 0x10000,
  //! Title provides a channel
  LIBWAD_TYPE_CHANNEL = 0x10001,
  //! Title provides a system channel
  LIBWAD_TYPE_SYSTEM_CHANNEL = 0x10002,
  //! Title provides a game and a channel
  LIBWAD_TYPE_GAME_WITH_CHANNEL = 0x10004,
  //! Title provides downloadable content
  LIBWAD_TYPE_DLC = 0x10005,
  //! Title provides a hidden channel
  LIBWAD_TYPE_HIDDEN_CHANNEL = 0x10008,
} tmd_type_t;

//! Enum describing all title regions
typedef enum {
  //! NTSC (Japan)
  LIBWAD_REGION_NTSC_J = 0,
  //! NTSC (USA)
  LIBWAD_REGION_NTSC_U = 1,
  //! PAL (Europe)
  LIBWAD_REGION_PAL = 2,
  //! International
  LIBWAD_REGION_INTERNATIONAL = 3,
  //! NTSC (Korea)
  LIBWAD_REGION_NTSC_K = 4
} tmd_region_t;

//! Struct containing all information stored about a given piece of content in
//! the tmd
typedef struct {
  uint32_t id;
  uint16_t index;
  uint16_t type;
  //! Size of the contents
  uint64_t size;
  //! SHA-1 hash of the decrypted contents
  char hash[20];
} tmd_content_t;

//! Opens a title metadata file for reading
/// @param filename path to the file to be opened
/// @returns A tmd_t handle on success or NULL on error (See
/// libwad_get_error() for more details)
W_EXPORT tmd_t tmd_open(const char* filename);

//! Closes a tmd handle and frees its resources
/// @param handle The handle to be closed
W_EXPORT void tmd_close(tmd_t handle);

//! Gets the title metadata section of a given wad file
/// @param handle The handle of the wad file
W_EXPORT tmd_t wad_get_tmd(wad_t handle);

//! Get the title id from a tmd_t
/// \remark Use util_title_id_to_string() to get a human readable version of
/// this id
W_EXPORT uint64_t tmd_get_title_id(tmd_t handle);

//! Get the amount of contents the title has
W_EXPORT uint16_t tmd_get_content_count(tmd_t handle);

//! Returns the minimum IOS version required
W_EXPORT uint64_t tmd_get_ios_version(tmd_t handle);

//! Returns the version of the title
W_EXPORT uint32_t tmd_get_title_version(tmd_t handle);

//! Gets the id representing the region of the title
/// @returns An id representing the region
/// \remark Use tmd_get_title_region_string(tmd_t) to get a human readable
/// string
W_EXPORT tmd_region_t tmd_get_title_region(tmd_t handle);

//! Get a human readable string representing the title region
W_EXPORT const char* tmd_get_title_region_string(tmd_t handle);

//! Get the type of the title
/// @returns An id describing the title type
/// \remark Use tmd_get_title_type_string(tmd_t) to get a human readable string
W_EXPORT tmd_type_t tmd_get_title_type(tmd_t handle);

//! Get a human readable string representing the title type
W_EXPORT const char* tmd_get_title_type_string(tmd_t handle);

//! Get the metadata of content
/// @param index index of content to get
W_EXPORT tmd_content_t* tmd_get_content(tmd_t handle, uint16_t index);

//@}

//@{
//! @name Ticket

//! A handle representing a ticket file or tmd section of a ticket file
typedef void* ticket_t;

//! Opens a ticket file for reading
/// @param path Path to the file to be opened
/// @returns A ticket_t handle on success or NULL on error (See
/// libwad_get_error() for more details)
W_EXPORT ticket_t ticket_open(const char* path);

//! Closes a ticket handle and frees its resources
// @param handle the handle to be free'd
W_EXPORT void ticket_close(ticket_t data);

//! Gets the ticket section of a given wad file
// @param handle the handle of the wad file
W_EXPORT ticket_t wad_get_ticket(wad_t handle);

//! Get the ticket issuer
/// @returns the key used to encrypt the contents
W_EXPORT const char* ticket_get_issuer(ticket_t handle);

//! Get the title id from a ticket_t
/// \remark You can use util_title_id_to_string(uint64_t) to get a human
/// readable version of this id
W_EXPORT uint64_t ticket_get_title_id(ticket_t handle);

//! Get the title key
// @returns the key used to encrypt the contents (always 16 bytes in length)
W_EXPORT unsigned const char* ticket_get_title_key(ticket_t handle);
/// @}

//@{
//! @name Data / Contents

//! A handle representing a data file or data section of a wad file
typedef void* data_t;

//! Whether or not the validity of the extracted data should be verified
typedef enum {
  //! Don't verify the hash
  LIBWAD_DONT_VERIFY_HASH = 0,
  //! Verify the hash of the extracted data
  LIBWAD_VERIFY_HASH = 1
} data_verify_t;

//! Opens a data file for reading
/// @param path path to the file to be opened
/// @returns a data_t handle on success or NULL on error (See
/// libwad_get_error() for more details)
W_EXPORT data_t data_open(const char* path);

//! Closes a data handle and frees its resources
// @param handle the handle to be free'd
W_EXPORT void data_close(data_t handle);

//! Extracts given content from a wad
/// \warning You have to take care of freeing the returned buffer yourself
W_EXPORT unsigned char* data_extract_from_wad(wad_t handle, uint16_t index,
                                              data_verify_t verify);

//! Extracts given content from a file
/// \warning You have to take care of freeing the returned buffer yourself
W_EXPORT unsigned char* data_extract(data_t handle, wad_t tmd, tmd_t ticket,
                                     uint16_t index, data_verify_t verify);

//@}

//@{
//! @name Utilities

//! Get a human readable version of a title id
W_EXPORT const char* util_title_id_to_string(uint64_t title_id);
// @}

#undef W_EXPORT

#endif
