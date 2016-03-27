/* libgaril - Android RIL client library
 * Copyright (C) 2016 You-Sheng Yang
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#if defined (HAVE_CONFIG_H)
#include "config.h"
#endif

#include "garil/garilversion.h"

/**
 * garil_get_major_version:
 *
 * Get the major version number of the Garil library.
 *
 * This function is in the library, so it represents the Garil library your code
 * is running against. Contrast with #GARIL_MAJOR_VERSION macro, which
 * represents the major version of the Garil headers you have included when
 * compiling your code.
 *
 * Returns: the major version number of the Garil library
 */
gint
garil_get_major_version (void)
{
  return GARIL_MAJOR_VERSION;
}

/**
 * garil_get_minor_version:
 *
 * Get the minor version number of the Garil library.
 *
 * This function is in the library, so it represents the Garil library your code
 * is running against. Contrast with #GARIL_MINOR_VERSION macro, which
 * represents the minor version of the Garil headers you have included when
 * compiling your code.
 *
 * Returns: the minor version number of the Garil library
 */
gint
garil_get_minor_version (void)
{
  return GARIL_MINOR_VERSION;
}

/**
 * garil_get_micro_version:
 *
 * Get the micro version number of the Garil library.
 *
 * This function is in the library, so it represents the Garil library your code
 * is running against. Contrast with #GARIL_MICRO_VERSION macro, which
 * represents the micro version of the Garil headers you have included when
 * compiling your code.
 *
 * Returns: the micro version number of the Garil library
 */
gint
garil_get_micro_version (void)
{
  return GARIL_MICRO_VERSION;
}

/**
 * garil_get_interface_age:
 *
 * Returns the interface age as passed to `libtool` when building the Garil
 * library the process is running against.
 *
 * This function is in the library, so it represents the Garil library your code
 * is running against. Contrast with #GARIL_INTERFACE_AGE macro, which
 * represents the interface age of the Garil headers you have included when
 * compiling your code.
 *
 * Returns: the interface age of the Garil library
 */
gint
garil_get_interface_age (void)
{
  return GARIL_INTERFACE_AGE;
}

/**
 * garil_get_binary_age:
 *
 * Returns the binary age as passed to `libtool` when building the Garil
 * library the process is running against.
 *
 * This function is in the library, so it represents the Garil library your code
 * is running against. Contrast with #GARIL_BINARY_AGE macro, which represents
 * the binary age of the Garil headers you have included when compiling your
 * code.
 *
 * Returns: the binary age of the Garil library
 */
gint
garil_get_binary_age (void)
{
  return GARIL_BINARY_AGE;
}

/**
 * garil_get_version_string:
 *
 * Get the version string of the Garil library.
 *
 * This function is in the library, so it represents the Garil library your code
 * is running against. Contrast with #GARIL_VERSION macro, which represents the
 * version string of the Garil headers you have included when compiling your
 * code.
 *
 * Returns: the version string of the Garil library
 */
const gchar*
garil_get_version_string (void)
{
  return GARIL_VERSION;
}

/**
 * garil_get_api_version_string:
 *
 * Get the API version string of the Garil library.
 *
 * This function is in the library, so it represents the Garil library your code
 * is running against. Contrast with #GARIL_API_VERSION macro, which represents
 * the API version string of the Garil headers you have included when
 * compiling your code.
 *
 * Returns: the API version string of the Garil library
 */
const gchar*
garil_get_api_version_string (void)
{
  return GARIL_API_VERSION;
}
