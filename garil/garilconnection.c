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

#include "garil/garilconnection.h"

struct  _GarilConnection {
  gint dummy;
};

G_DEFINE_TYPE (GarilConnection, garil_connection, G_TYPE_OBJECT)

static void
garil_connection_class_init (GarilConnectionClass *klass G_GNUC_UNUSED)
{
}

static void
garil_connection_init (GarilConnection *connection G_GNUC_UNUSED)
{
  /* do nothing */
}
