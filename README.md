# libwad

[![Build Status](https://travis-ci.org/spycrab/libwad.svg?branch=master)](https://travis-ci.org/spycrab/libwad)  

libwad is a C library for handling the wad file format used by Nintendo to
distribute content for their Wii console.  
  
libwad also provides a suite of tools for performing various tasks surronding the format.

## Requirements
- [CMake](https://cmake.org/)
- [Doxygen](http://www.doxygen.nl) (*optional; Needed for documentation only*)
- A C Compiler with C99 support

## Installation

If you don't want CMake to create the library or tools install target. Set ``ENABLE_INSTALL`` to ``OFF``.
## Documentation

Documentation can be built via the target ``docs`` if CMake detects a Doxygen install.  
If you don't wish to have documentation created, set the CMake option ``ENABLE_DOCS`` to ``OFF``.

## Tools
libwad provides various tools for handling wad files and parts of them.  
If you don't wish to build these, set the CMake option ``ENABLE_TOOLS`` to ``OFF``.

### wadinfo / certinfo / tmdinfo / ticketinfo

Tool for displaying information stored in a WAD or single section files.

### wadextract

Tool for extracting data from wads.

### wadverify

Tool for verifying the validity of wads.

### wadglue

Tool for combining separate sections of a wad into one file

## License

libwad is licensed under the GNU General Public License v3 or any later
version at your option. See [LICENSE](LICENSE).

libwad depends on libraries which are subject to different licenses.
See [THIRDPARTY.md](THIRDPARTY.md) for a complete list.
