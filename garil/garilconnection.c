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
#include "garil/garilenumtypes.h"

/**
 * SECTION:garilconnection
 * @title: RIL Connections
 * @short_description: Raw RIL connection API
 *
 * GarilConnection wraps raw RIL traffic with GIO asynchronous APIs.
 */

/**
 * GarilConnection:
 *
 * An opaque structure.
 */
struct _GarilConnection {
  /*< private >*/
  GObject parent_instance;

  volatile gint atom_flags;

  GMutex init_lock;
  GError *init_error;
  GIOStream *stream;
  GSocketAddress *address;
  GarilConnectionFlags flags;
};

static void initable_iface_init (GInitableIface *initable_iface);
static void async_initable_iface_init (GAsyncInitableIface *async_initable_iface);

G_DEFINE_TYPE_WITH_CODE (GarilConnection, garil_connection, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE,
                                                initable_iface_init)
                         G_IMPLEMENT_INTERFACE (G_TYPE_ASYNC_INITABLE,
                                                async_initable_iface_init)
                        );

/* Flags for GarilConnection::atom_flags */
enum
{
  FLAG_INITIALIZED = (1 << 0),
};

enum
{
  PROP_0,
  PROP_STREAM,
  PROP_ADDRESS,
  PROP_FLAGS,
  N_PROPERTIES
};

static GParamSpec *props[N_PROPERTIES] = { NULL, };

static void
set_property (GObject      *object,
              guint         prop_id,
              const GValue *value,
              GParamSpec   *pspec)
{
  GarilConnection *connection = GARIL_CONNECTION (object);

  switch (prop_id) {
    case PROP_STREAM:
      connection->stream = g_value_dup_object (value);
      break;
    case PROP_ADDRESS:
      connection->address = g_value_dup_object (value);
      break;
    case PROP_FLAGS:
      connection->flags = g_value_get_flags (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
get_property (GObject    *object,
              guint       prop_id,
              GValue     *value,
              GParamSpec *pspec)
{
  GarilConnection *connection = GARIL_CONNECTION (object);

  switch (prop_id) {
    case PROP_STREAM:
      g_value_set_object (value, garil_connection_get_stream (connection));
      break;
    case PROP_ADDRESS:
      g_value_set_object (value, garil_connection_get_address (connection));
      break;
    case PROP_FLAGS:
      g_value_set_flags (value, garil_connection_get_flags (connection));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
finalize (GObject *object)
{
  GarilConnection *connection = GARIL_CONNECTION (object);

  if (connection->stream != NULL) {
    g_object_unref (connection->stream);
    connection->stream = NULL;
  }

  if (connection->address != NULL) {
    g_object_unref (connection->address);
    connection->address = NULL;
  }

  if (connection->init_error != NULL) {
    g_error_free (connection->init_error);
    connection->init_error = NULL;
  }

  g_mutex_clear (&connection->init_lock);

  G_OBJECT_CLASS (garil_connection_parent_class)->finalize (object);
}

static void
garil_connection_class_init (GarilConnectionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  /* virtual methods */

  object_class->set_property = set_property;
  object_class->get_property = get_property;
  object_class->finalize = finalize;

  /* properties */

  /**
   * GarilConnection:stream:
   *
   * The underlying #GIOStream used for I/O.
   */
  props[PROP_STREAM] =
    g_param_spec_object (GARIL_CONNECTION_PROP_STREAM,
                         "IO Stream", "The underlying streams used for I/O",
                         G_TYPE_IO_STREAM,
                         G_PARAM_CONSTRUCT_ONLY | \
                           G_PARAM_READWRITE | \
                           G_PARAM_STATIC_STRINGS);

  /**
   * GarilConnection:address:
   *
   * A #GSocketAddress specifying endpoint that should be used to establish the
   * connection. %NULL if the connection was instantiated with a specific stream
   * instead.
   */
  props[PROP_ADDRESS] =
    g_param_spec_object (GARIL_CONNECTION_PROP_ADDRESS,
                         "Address",
                         "Socket address specifying potential socket endpoints",
                         G_TYPE_SOCKET_ADDRESS,
                         G_PARAM_CONSTRUCT_ONLY | \
                           G_PARAM_READWRITE | \
                           G_PARAM_STATIC_STRINGS);

  /**
   * GarilConnection:flags:
   *
   * Flags from the #GarilConnectionFlags enumeration.
   */
  props[PROP_FLAGS] =
    g_param_spec_flags (GARIL_CONNECTION_PROP_FLAGS,
                        "Flags", "Flags",
                        GARIL_TYPE_CONNECTION_FLAGS,
                        GARIL_CONNECTION_FLAGS_NONE,
                         G_PARAM_CONSTRUCT_ONLY | \
                           G_PARAM_READWRITE | \
                           G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPERTIES, props);
}

static void
garil_connection_init (GarilConnection *connection)
{
  g_mutex_init (&connection->init_lock);
}

static gboolean
initable_init (GInitable     *initable,
               GCancellable  *cancellable,
               GError       **error)
{
  GarilConnection *connection = GARIL_CONNECTION (initable);
  gboolean ret = FALSE;

  g_mutex_lock (&connection->init_lock);

  if (g_atomic_int_get (&connection->atom_flags) & FLAG_INITIALIZED) {
    ret = (connection->init_error == NULL);
    goto out;
  }

  g_assert (connection->init_error == NULL);

  if ((cancellable != NULL) && g_cancellable_is_cancelled (cancellable)) {
    g_cancellable_set_error_if_cancelled (cancellable, &connection->init_error);
    goto out;
  }

  if (connection->address != NULL) {
    g_assert (connection->stream == NULL);

    GSocketClient *socket_client = g_socket_client_new ();
    GSocketConnection *socket_connection;

    socket_connection =
      g_socket_client_connect (socket_client,
                               G_SOCKET_CONNECTABLE (connection->address),
                               cancellable, &connection->init_error);
    g_object_unref (socket_client);

    if (socket_connection == NULL)
      goto out;

    connection->stream = G_IO_STREAM (socket_connection);
  } else if (connection->stream != NULL) {
    /* nothing to do */
  } else {
    g_assert_not_reached ();
  }

  ret = TRUE;

out:
  if (!ret) {
    g_assert (connection->init_error != NULL);
    g_propagate_error (error, g_error_copy (connection->init_error));
  }

  g_atomic_int_or (&connection->atom_flags, FLAG_INITIALIZED);
  g_mutex_unlock (&connection->init_lock);

  return ret;
}

static void
initable_iface_init (GInitableIface *initable_iface)
{
  initable_iface->init = initable_init;
}

static void
async_initable_iface_init (GAsyncInitableIface *async_initable_iface G_GNUC_UNUSED)
{
  /* use default */
}

/**
 * garil_connection_new:
 * @stream: A #GIOStream.
 * @flags: Flags describing how to make the connection.
 * @cancellable: (nullable): A #GCancellable or %NULL.
 * @callback: A #GAsyncReadyCallback to call when the request is satisfied.
 * @user_data: (nullable): The data to pass to the @callback.
 *
 * Asynchronously sets up a RIL connection for sending requests to and receiving
 * unsolicited responses from the endpoint represented by stream.
 *
 * When the operation is finished, callback will be invoked. You can then call
 * #garil_connection_new_finish() to get the result of the operation.
 *
 * This is a asynchronously fallable constructor. See
 * #garil_connection_new_sync() for the synchronous version.
 */
void
garil_connection_new (GIOStream            *stream,
                      GarilConnectionFlags  flags,
                      GCancellable         *cancellable,
                      GAsyncReadyCallback   callback,
                      gpointer              user_data)
{
  g_return_if_fail (G_IS_IO_STREAM (stream));

  g_async_initable_new_async (GARIL_TYPE_CONNECTION,
                              G_PRIORITY_DEFAULT,
                              cancellable, callback, user_data,
                              GARIL_CONNECTION_PROP_STREAM, stream,
                              GARIL_CONNECTION_PROP_FLAGS, flags,
                              NULL);
}

/**
 * garil_connection_new_finish:
 * @res: A #GAsyncResult obtained from the #GAsyncReadyCallback passed to
 *   #garil_connection_new().
 * @error: (out) (nullable): Return location for error or %NULL.
 *
 * Finishes an operation started with #garil_connection_new().
 *
 * Returns: (transfer full): A #GarilConnection or %NULL if error is set. Free
 *   with #g_object_unref()
 */
GarilConnection*
garil_connection_new_finish (GAsyncResult  *res,
                             GError       **error)
{
  GObject *object;
  GObject *source;

  g_return_val_if_fail (G_IS_ASYNC_RESULT (res), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  source = g_async_result_get_source_object (res);
  g_assert (source != NULL);

  object = g_async_initable_new_finish (G_ASYNC_INITABLE (source), res, error);

  g_object_unref (source);

  if (object != NULL)
    return GARIL_CONNECTION (object);

  return NULL;
}

/**
 * garil_connection_new_sync:
 * @stream: A #GIOStream.
 * @flags: Flags describing how to make the connection.
 * @cancellable: (nullable): A #GCancellable or %NULL.
 * @error: (out) (nullable): Return location for error or %NULL.
 *
 * Synchronously sets up a RIL connection for sending requests to and receiving
 * unsolicited responses from the endpoint represented by stream.
 *
 * This is a synchronously fallable constructor. See #garil_connection_new()
 * for the asynchronous version.
 *
 * Returns: (transfer full): A #GarilConnection or %NULL if error is set. Free
 *   with #g_object_unref().
 */
GarilConnection*
garil_connection_new_sync (GIOStream            *stream,
                           GarilConnectionFlags  flags,
                           GCancellable          *cancellable,
                           GError               **error)
{
  g_return_val_if_fail (G_IS_IO_STREAM (stream), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  return g_initable_new (GARIL_TYPE_CONNECTION, cancellable, error,
                         GARIL_CONNECTION_PROP_STREAM, stream,
                         GARIL_CONNECTION_PROP_FLAGS, flags,
                         NULL);
}

/**
 * garil_connection_new_for_address:
 * @address: A #GSocketAddress.
 * @flags: Flags describing how to make the connection.
 * @cancellable: (nullable): A #GCancellable or %NULL.
 * @callback: A #GAsyncReadyCallback to call when the request is satisfied.
 * @user_data: (nullable): The data to pass to the @callback.
 *
 * Asynchronously sets up a RIL connection for sending requests to and receiving
 * unsolicited responses from the endpoint specified by address.
 *
 * When the operation is finished, callback will be invoked. You can then call
 * #garil_connection_new_for_address_finish() to get the result of the
 * operation.
 *
 * This is a asynchronously fallable constructor. See
 * #garil_connection_new_for_address_sync() for the synchronous version.
 */
void
garil_connection_new_for_address (GSocketAddress       *address,
                                  GarilConnectionFlags  flags,
                                  GCancellable         *cancellable,
                                  GAsyncReadyCallback   callback,
                                  gpointer              user_data)
{
  g_return_if_fail (G_IS_SOCKET_ADDRESS (address));

  g_async_initable_new_async (GARIL_TYPE_CONNECTION,
                              G_PRIORITY_DEFAULT,
                              cancellable, callback, user_data,
                              GARIL_CONNECTION_PROP_ADDRESS, address,
                              GARIL_CONNECTION_PROP_FLAGS, flags,
                              NULL);
}

/**
 * garil_connection_new_for_address_finish:
 * @res: A #GAsyncResult obtained from the #GAsyncReadyCallback passed to
 *   #garil_connection_new_for_address().
 * @error: (out) (nullable): Return location for error or %NULL.
 *
 * Finishes an operation started with #garil_connection_new_for_address().
 *
 * Returns: (transfer full): A #GarilConnection or %NULL if error is set. Free
 *   with #g_object_unref().
 */
GarilConnection*
garil_connection_new_for_address_finish (GAsyncResult  *res,
                                         GError       **error)
{
  return garil_connection_new_finish (res, error);
}

/**
 * garil_connection_new_for_address_sync:
 * @address: A #GSocketAddress.
 * @flags: Flags describing how to make the connection.
 * @cancellable: (nullable): A #GCancellable or %NULL.
 * @error: (out) (nullable): Return location for error or %NULL.
 *
 * Synchronously sets up a RIL connection for sending requests to and receiving
 * unsolicited responses from the endpoint specified by address.
 *
 * This is a synchronously fallable constructor. See
 * #garil_connection_new_for_address() for the asynchronous version.
 *
 * Returns: (transfer full): A #GarilConnection or %NULL if error is set. Free
 *   with #g_object_unref().
 */
GarilConnection*
garil_connection_new_for_address_sync (GSocketAddress        *address,
                                       GarilConnectionFlags   flags,
                                       GCancellable          *cancellable,
                                       GError               **error)
{
  g_return_val_if_fail (G_IS_SOCKET_ADDRESS (address), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  return g_initable_new (GARIL_TYPE_CONNECTION, cancellable, error,
                         GARIL_CONNECTION_PROP_ADDRESS, address,
                         GARIL_CONNECTION_PROP_FLAGS, flags,
                         NULL);
}

/**
 * garil_connection_get_stream:
 * @connection: A #GarilConnection.
 *
 * Get the underlying stream used for I/O.
 *
 * Returns: (transfer none): The stream used for I/O.
 */
GIOStream*
garil_connection_get_stream (GarilConnection *connection)
{
  g_return_val_if_fail (GARIL_IS_CONNECTION (connection), NULL);

  return connection->stream;
}

/**
 * garil_connection_get_address:
 * @connection: A #GarilConnection.
 *
 * Get the socket address specifying the endpoint. %NULL if the connection was
 * not instantiated from #garil_connection_new_for_address().
 *
 * Returns: (transfer none): The socket address specifying the endpoint.
 */
GSocketAddress*
garil_connection_get_address (GarilConnection *connection)
{
  g_return_val_if_fail (GARIL_IS_CONNECTION (connection), NULL);

  return connection->address;
}

/**
 * garil_connection_get_flags:
 * @connection: A #GarilConnection.
 *
 * Get the flags describing how to create the connection.
 *
 * Returns: The #GarilConnectionFlags passed to the constructor.
 */
GarilConnectionFlags
garil_connection_get_flags (GarilConnection *connection)
{
  g_return_val_if_fail (GARIL_IS_CONNECTION (connection),
                        GARIL_CONNECTION_FLAGS_NONE);

  return connection->flags;
}
