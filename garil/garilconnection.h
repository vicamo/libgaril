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

G_BEGIN_DECLS

#define GARIL_TYPE_CONNECTION  (garil_connection_get_type ())

G_DECLARE_FINAL_TYPE (GarilConnection, garil_connection, GARIL, CONNECTION,
                      GObject)

#define GARIL_CONNECTION_PROP_STREAM "stream"
#define GARIL_CONNECTION_PROP_ADDRESS "address"

typedef enum {
  GARIL_CONNECTION_FLAGS_NONE = 0,
  GARIL_CONNECTION_FLAGS_DELAY_MESSAGE_PROCESSING = (1 << 0),
} GarilConnectionFlags;

void garil_connection_new (GIOStream           *stream,
                           GCancellable        *cancellable,
                           GAsyncReadyCallback  callback,
                           gpointer             user_data);

GarilConnection* garil_connection_new_finish (GAsyncResult  *res,
                                              GError       **error);

GarilConnection* garil_connection_new_sync (GIOStream     *stream,
                                            GCancellable  *cancellable,
                                            GError       **error);

void garil_connection_new_for_address (GSocketAddress      *address,
                                       GCancellable        *cancellable,
                                       GAsyncReadyCallback  callback,
                                       gpointer             user_data);

GarilConnection* garil_connection_new_for_address_finish (GAsyncResult  *res,
                                                          GError       **error);

GarilConnection* garil_connection_new_for_address_sync (GSocketAddress  *address,
                                                        GCancellable    *cancellable,
                                                        GError         **error);

GIOStream* garil_connection_get_stream (GarilConnection *connection);

GSocketAddress* garil_connection_get_address (GarilConnection *connection);

G_END_DECLS
