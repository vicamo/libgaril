/* GARIL - Android RIL client library
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

#include <locale.h>
#include <glib.h>
#include <glib/gstdio.h>

#if defined (G_OS_UNIX)
# include <gio/gunixsocketaddress.h>
#endif

#include "garil/garil.h"

typedef struct {
  GMainLoop *loop;
  GIOStream *stream;
} TestContext1;

static GIOStream*
get_memory_stream (void)
{
  GInputStream *istream = g_memory_input_stream_new ();
  GOutputStream *ostream = g_memory_output_stream_new_resizable ();

  GIOStream *stream = g_simple_io_stream_new (istream, ostream);

  g_object_unref (istream);
  g_object_unref (ostream);

  return stream;
}

static void
test_new (GCancellable        *cancellable,
          GAsyncReadyCallback  callback)
{
  TestContext1 *context = g_new0 (TestContext1, 1);
  context->loop = g_main_loop_new (NULL, FALSE);
  context->stream = get_memory_stream ();

  garil_connection_new (context->stream,
                        GARIL_CONNECTION_FLAGS_DELAY_MESSAGE_PROCESSING,
                        cancellable, callback, context);
  if (cancellable != NULL)
    g_cancellable_cancel (cancellable);

  g_main_loop_run (context->loop);

  g_main_loop_unref (context->loop);
  g_object_unref (context->stream);
  g_free (context);
}

static void
check_stream_address_flags (GarilConnection *connection,
                            GIOStream       *expected_stream,
                            GSocketAddress  *expected_address)
{
  GIOStream *stream = garil_connection_get_stream (connection);
  g_assert_true (G_IS_IO_STREAM (stream));
  if (expected_stream != NULL)
    g_assert_true (stream == expected_stream);

  GSocketAddress *address = garil_connection_get_address (connection);
  if (expected_address != NULL)
    g_assert_true (address == expected_address);
  else
    g_assert_null (address);

  GarilConnectionFlags flags = garil_connection_get_flags (connection);
  g_assert_true (flags == GARIL_CONNECTION_FLAGS_DELAY_MESSAGE_PROCESSING);
}

static void
check_new (GAsyncResult    *res,
           GError         **expected_error,
           GIOStream       *expected_stream,
           GSocketAddress  *expected_address)
{
  GarilConnection *connection =
    garil_connection_new_finish (res, expected_error);
  if (expected_error != NULL) {
    g_assert_null (connection);
    g_assert_nonnull (*expected_error);
    return;
  }

  g_assert_true (GARIL_IS_CONNECTION (connection));

  check_stream_address_flags (connection, expected_stream, expected_address);

  g_object_unref (connection);
}

static void
on_test_new_ready_1 (GObject      *source_object G_GNUC_UNUSED,
                     GAsyncResult *res,
                     gpointer      user_data)
{
  TestContext1 *context = user_data;

  check_new (res, NULL, context->stream, NULL);

  g_main_loop_quit (context->loop);
}

/* Normal instantiation */
static void
test_new_1 (void)
{
  test_new (NULL, on_test_new_ready_1);
}

static void
on_test_new_ready_2 (GObject      *source_object G_GNUC_UNUSED,
                     GAsyncResult *res,
                     gpointer      user_data)
{
  TestContext1 *context = user_data;
  GError *error = NULL;

  check_new (res, &error, NULL, NULL);
  g_assert_true (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED));

  g_main_loop_quit (context->loop);

  g_error_free (error);
}

static void
test_new_2 (void)
{
  GCancellable *cancellable = g_cancellable_new ();

  test_new (cancellable, on_test_new_ready_2);

  g_object_unref (cancellable);
}

static void
test_new_sync_1 (void)
{
  GIOStream *stream = get_memory_stream ();

  GarilConnection *connection =
    garil_connection_new_sync (stream,
                               GARIL_CONNECTION_FLAGS_DELAY_MESSAGE_PROCESSING,
                               NULL, NULL);
  g_assert_true (GARIL_IS_CONNECTION (connection));

  check_stream_address_flags (connection, stream, NULL);

  g_object_unref (connection);
  g_object_unref (stream);
}

static void
test_new_sync_2 (void)
{
  GIOStream *stream = get_memory_stream ();

  GCancellable *cancellable = g_cancellable_new ();
  g_cancellable_cancel (cancellable);

  GError *error = NULL;
  GarilConnection *connection =
    garil_connection_new_sync (stream,
                               GARIL_CONNECTION_FLAGS_DELAY_MESSAGE_PROCESSING,
                               cancellable, &error);
  /*g_assert_null (connection);*/
  g_assert_nonnull (error);
  g_assert_true (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED));

  g_error_free (error);
  g_object_unref (cancellable);
  g_object_unref (stream);
}

typedef struct {
  GMainLoop *loop;
  GSocketAddress *address;
} TestContext2;

static void
test_new_for_address (GSocketAddress      *address,
                      GAsyncReadyCallback  callback,
                      GCancellable        *cancellable)
{
  TestContext2 *context = g_new0 (TestContext2, 1);
  context->loop = g_main_loop_new (NULL, FALSE);
  context->address = g_object_ref (address);

  garil_connection_new_for_address (address,
                                    GARIL_CONNECTION_FLAGS_DELAY_MESSAGE_PROCESSING,
                                    cancellable, callback, context);
  if (cancellable != NULL)
    g_cancellable_cancel (cancellable);

  g_main_loop_run (context->loop);

  g_main_loop_unref (context->loop);
  g_object_unref (context->address);
  g_free (context);
}

static void
check_new_for_address (GAsyncResult    *res,
                       GError         **expected_error,
                       GSocketAddress  *expected_address)
{
  GarilConnection *connection =
    garil_connection_new_for_address_finish (res, expected_error);
  if (expected_error != NULL) {
    g_assert_null (connection);
    g_assert_nonnull (*expected_error);
    return;
  }

  g_assert_true (GARIL_IS_CONNECTION (connection));

  check_stream_address_flags (connection, NULL, expected_address);

  g_object_unref (connection);
}

#if defined (G_OS_UNIX)
static GSocketService*
start_unix_sock_service (GSocketAddress **address)
{
  g_return_val_if_fail ((address != NULL && *address == NULL), NULL);

  gchar *path = g_build_path(G_DIR_SEPARATOR_S,
                             g_getenv("G_TEST_BUILDDIR"),
                             "test-connection.sock",
                             NULL);
  GFile *file = g_file_new_for_path (path);

  GFileType type = g_file_query_file_type (file, G_FILE_QUERY_INFO_NONE, NULL);
  if (type == G_FILE_TYPE_SPECIAL)
    g_assert_cmpint (g_unlink (path), ==, 0);

  g_object_unref (file);

  *address = g_unix_socket_address_new (path);
  g_free (path);

  GSocketService *service = g_socket_service_new ();
  GError *error = NULL;

  g_socket_listener_add_address (G_SOCKET_LISTENER (service),
                                 *address,
                                 G_SOCKET_TYPE_STREAM,
                                 G_SOCKET_PROTOCOL_DEFAULT,
                                 NULL, NULL, &error);
  if (error != NULL) {
    g_test_message ("Failed to create unix sock service: %s\n", error->message);
    g_assert_null (error);
  }

  g_socket_service_start (service);

  return service;
}
#endif /* G_OS_UNIX */

static void
on_test_new_for_address_ready_1 (GObject      *source_object G_GNUC_UNUSED,
                                 GAsyncResult *res,
                                 gpointer      user_data)
{
  TestContext2 *context = user_data;

  check_new_for_address (res, NULL, context->address);

  g_main_loop_quit (context->loop);
}

static void
test_new_for_address_1 (gconstpointer user_data)
{
  GSocketAddress *address = (GSocketAddress*)user_data;

  test_new_for_address (address, on_test_new_for_address_ready_1, NULL);
}

static void
on_test_new_for_address_ready_2 (GObject      *source_object G_GNUC_UNUSED,
                                 GAsyncResult *res,
                                 gpointer      user_data)
{
  TestContext2 *context = user_data;
  GError *error = NULL;

  check_new_for_address (res, &error, NULL);
  g_assert_true (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED));

  g_main_loop_quit (context->loop);

  g_error_free (error);
}

static void
test_new_for_address_2 (gconstpointer user_data)
{
  GSocketAddress *address = (GSocketAddress *)user_data;
  GCancellable *cancellable = g_cancellable_new ();

  test_new_for_address (address, on_test_new_for_address_ready_2, cancellable);

  g_object_unref (cancellable);
}

static void
on_test_new_for_address_ready_3 (GObject      *source_object G_GNUC_UNUSED,
                                 GAsyncResult *res,
                                 gpointer      user_data)
{
  TestContext2 *context = user_data;
  GError *error = NULL;

  check_new_for_address (res, &error, NULL);
  g_assert_true (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND));

  g_main_loop_quit (context->loop);

  g_error_free (error);
}

static void
test_new_for_address_3 (void)
{
#if defined (G_OS_UNIX)
  GSocketAddress *address = g_unix_socket_address_new ("/nonexist");

  test_new_for_address (address, on_test_new_for_address_ready_3, NULL);

  g_object_unref (address);
#else
  g_test_skip ("Skipped on non-Unix systems");
#endif
}

static void
test_new_for_address_sync_1 (gconstpointer user_data)
{
  GSocketAddress *address = (GSocketAddress*)user_data;

  GarilConnection *connection =
    garil_connection_new_for_address_sync (address,
                                           GARIL_CONNECTION_FLAGS_DELAY_MESSAGE_PROCESSING,
                                           NULL, NULL);
  g_assert_true (GARIL_IS_CONNECTION (connection));

  check_stream_address_flags (connection, NULL, address);

  g_object_unref (connection);
}

static void
test_new_for_address_sync_2 (gconstpointer user_data)
{
  GSocketAddress *address = (GSocketAddress*)user_data;

  GCancellable *cancellable = g_cancellable_new ();
  g_cancellable_cancel (cancellable);

  GError *error = NULL;
  GarilConnection *connection =
    garil_connection_new_for_address_sync (address,
                                           GARIL_CONNECTION_FLAGS_DELAY_MESSAGE_PROCESSING,
                                           cancellable, &error);
  g_assert_null (connection);
  g_assert_nonnull (error);
  g_assert_true (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED));

  g_error_free (error);
  g_object_unref (cancellable);
}

int
main (int   argc,
      char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base (PACKAGE_BUGREPORT);

#if defined (G_OS_UNIX)
  GSocketAddress *unix_sock_address = NULL;
  GSocketService *unix_sock_service =
    start_unix_sock_service (&unix_sock_address);
#endif

  /* garil_connection_new */

  g_test_add_func ("/GarilConnection/garil_connection_new/1", test_new_1);
  g_test_add_func ("/GarilConnection/garil_connection_new/2", test_new_2);

  /* garil_connection_new_sync */

  g_test_add_func ("/GarilConnection/garil_connection_new_sync/1",
                   test_new_sync_1);
  g_test_add_func ("/GarilConnection/garil_connection_new_sync/2",
                   test_new_sync_2);

  /* garil_connection_new_for_address */

#if defined (G_OS_UNIX)
  g_test_add_data_func ("/GarilConnection/garil_connection_new_for_address/unix/1",
                        unix_sock_address,
                        test_new_for_address_1);
  g_test_add_data_func ("/GarilConnection/garil_connection_new_for_address/unix/2",
                        unix_sock_address,
                        test_new_for_address_2);
  g_test_add_func ("/GarilConnection/garil_connection_new_for_address/unix/3",
                   test_new_for_address_3);
#endif /* G_OS_UNIX */

  /* garil_connection_new_for_address_sync */

#if defined (G_OS_UNIX)
  g_test_add_data_func ("/GarilConnection/garil_connection_new_for_address_sync/unix/1",
                        unix_sock_address,
                        test_new_for_address_sync_1);
  g_test_add_data_func ("/GarilConnection/garil_connection_new_for_address_sync/unix/2",
                        unix_sock_address,
                        test_new_for_address_sync_2);
#endif /* G_OS_UNIX */

  int ret = g_test_run ();

#if defined (G_OS_UNIX)
  g_socket_service_stop (unix_sock_service);
  g_socket_listener_close (G_SOCKET_LISTENER (unix_sock_service));

  g_object_unref (unix_sock_address);
  g_object_unref (unix_sock_service);
#endif

  return ret;
}
