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

#pragma once

#if !defined (__GARIL_GARIL_H_INSIDE__) && !defined (LIBGARIL_COMPILATION)
#error "Only <garil/garil.h> can be included directly."
#endif

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include <garil/garilconnection.h>

G_BEGIN_DECLS

#define GARIL_TYPE_CLIENT  (garil_client_get_type ())

G_DECLARE_DERIVABLE_TYPE (GarilClient, garil_client, GARIL, CLIENT, GObject)

#define GARIL_CLIENT_PROP_CONNECTION "connection"

struct _GarilClientClass {
  GObjectClass parent_class;

  /*< private >*/

  /* Reserved slots for furture extension. */
  gpointer padding[12];
};

GarilClient* garil_client_new (GarilConnection *connection);

G_END_DECLS
