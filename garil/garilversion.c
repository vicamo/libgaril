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

gint
garil_get_major_version (void)
{
  return GARIL_MAJOR_VERSION;
}

gint
garil_get_minor_version (void)
{
  return GARIL_MINOR_VERSION;
}

gint
garil_get_micro_version (void)
{
  return GARIL_MICRO_VERSION;
}

gint
garil_get_interface_age (void)
{
  return GARIL_INTERFACE_AGE;
}

gint
garil_get_binary_age (void)
{
  return GARIL_BINARY_AGE;
}

const gchar*
garil_get_version_string (void)
{
  return GARIL_VERSION;
}

const gchar*
garil_get_api_version_string (void)
{
  return GARIL_API_VERSION;
}
