# - Try to find the IsoCodes libraries
# Once done this will define
#
#  ISOCODES_FOUND - system has ISOCODES
#  ISOCODES_INCLUDE_DIR - the ISOCODES include directory
#  ISOCODES_LIBRARIES - ISOCODES library
#
# Copyright (c) 2012 CSSlayer <wengxt@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(ISOCODES_INCLUDE_DIR AND ISOCODES_LIBRARIES)
    # Already in cache, be silent
    set(ISOCODES_FIND_QUIETLY TRUE)
endif(ISOCODES_INCLUDE_DIR AND ISOCODES_LIBRARIES)

find_package(PkgConfig)
pkg_check_modules(PC_ISOCODES iso-codes)

set(IsoCodes_PREFIX ${PC_ISOCODES_PREFIX})

