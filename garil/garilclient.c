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

#include "garil/garilclient.h"

/**
 * SECTION:garilclient
 * @title: Client Context
 * @short_description: RIL client API
 *
 * GarilClient provides an convenient interface to perform complex tasks.
 */

typedef struct  _GarilClientPrivate {
  GarilConnection *connection;
} GarilClientPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GarilClient, garil_client, G_TYPE_OBJECT)

#define GARIL_CLIENT_GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GARIL_TYPE_CLIENT, GarilClientPrivate))

enum
{
  PROP_0,
  PROP_CONNECTION,
  N_PROPERTIES
};

static GParamSpec *props[N_PROPERTIES] = { NULL, };

static void
set_property (GObject      *object,
              guint         prop_id,
              const GValue *value,
              GParamSpec   *pspec)
{
  GarilClient *client = GARIL_CLIENT (object);
  GarilClientPrivate *priv = GARIL_CLIENT_GET_PRIVATE (client);

  switch (prop_id) {
    case PROP_CONNECTION:
      priv->connection = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
get_property (GObject    *object,
              guint       prop_id,
              GValue     *value G_GNUC_UNUSED,
              GParamSpec *pspec)
{
  GarilClient *client = GARIL_CLIENT (object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
finalize (GObject *object)
{
  GarilClient *client = GARIL_CLIENT (object);
  GarilClientPrivate *priv = GARIL_CLIENT_GET_PRIVATE (client);

  g_object_unref (priv->connection);
  priv->connection = NULL;
}

static void
garil_client_class_init (GarilClientClass *client_class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (client_class);

  /* virtual methods */

  object_class->set_property = set_property;
  object_class->get_property = get_property;
  object_class->finalize = finalize;

  /* properties */

  /**
   * GarilClient:connection:
   *
   * The underlying #GarilConnection object for this client.
   */
  props[PROP_CONNECTION] =
    g_param_spec_object (GARIL_CLIENT_PROP_CONNECTION,
                         "RIL connection", "",
                         GARIL_TYPE_CONNECTION,
                         G_PARAM_CONSTRUCT_ONLY | \
                           G_PARAM_WRITABLE | \
                           G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPERTIES, props);
}

static void
garil_client_init (GarilClient *client G_GNUC_UNUSED)
{
  /* do nothing */
}

/**
 * garil_client_new:
 * @connection: A #GarilConnection.
 *
 * Create a #GarilClient.
 *
 * Returns: (transfer full): A #GarilClient or %NULL if connection is invalid.
 */
GarilClient*
garil_client_new (GarilConnection *connection)
{
  g_return_val_if_fail (GARIL_IS_CONNECTION (connection), NULL);

  return g_object_new (GARIL_TYPE_CLIENT,
                       GARIL_CLIENT_PROP_CONNECTION, connection,
                       NULL);
}
