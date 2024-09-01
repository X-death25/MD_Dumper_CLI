# - Try to find the freetype library
# Once done this defines
#
#  LIBUSB1_FOUND - system has libusb
#  LIBUSB1_INCLUDE_DIRS - the libusb include directory
#  LIBUSB1_LIBRARIES - Link these to use libusb

# Copyright (c) 2006, 2008  Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (LIBUSB1_INCLUDE_DIRS AND LIBUSB1_LIBRARIES)

  # in cache already
  set(LIBUSB1_FOUND TRUE)

else (LIBUSB1_INCLUDE_DIRS AND LIBUSB1_LIBRARIES)
  IF (NOT WIN32)
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    find_package(PkgConfig)
    pkg_check_modules(PC_LIBUSB libusb-1.0)
  ENDIF(NOT WIN32)

  FIND_PATH(LIBUSB1_INCLUDE_DIRS libusb.h
    NAMES
    libusb.h
    PATHS
    ${PC_LIBUSB1_INCLUDE_DIRS}
    ${PC_LIBUSB1_INCLUDEDIR}
    ${_dirs}
    HINTS
    "${LIBUSB1_ROOT_DIR}"
    PATH_SUFFIXES
    include/libusb-1.0
    include
    libusb-1.0)

  FIND_LIBRARY(LIBUSB1_LIBRARIES NAMES libusb-1.0
    NAMES
    libusb-1.0
    usb-1.0
    PATHS
    ${PC_LIBUSB1_LIBRARY_DIRS}
    ${PC_LIBUSB1_LIBDIR}
    ${_dirs}
    HINTS
    "${LIBUSB1_ROOT_DIR}"
    PATH_SUFFIXES
    ${_lib_suffixes})

  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBUSB1 DEFAULT_MSG LIBUSB1_LIBRARIES LIBUSB1_INCLUDE_DIRS)

  MARK_AS_ADVANCED(LIBUSB1_INCLUDE_DIRS LIBUSB1_LIBRARIES)

endif (LIBUSB1_INCLUDE_DIRS AND LIBUSB1_LIBRARIES)
