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

#include <string.h>

#include "garil/garilparcel.h"

/**
 * SECTION:garilparcel
 * @title: Parcels
 * @short_description: Data structure to read from/write to the raw connection.
 *
 * GarilParcel provides APIs to encode/decode RIL requests and responses for
 * underlying transport.
 */

/**
 * GarilParcel:
 *
 * An opaque structure.
 */
struct _GarilParcel
{
  volatile gint ref_count;

  GByteArray *byte_array;
  goffset position;
  gboolean malformed;
};

/**
 * garil_parcel_new:
 * @array: A #GByteArray or %NULL.
 *
 * Create a new parcel to wrap either, when array is not %NULL, the passed
 * array, or a newly allocated #GByteArray.
 *
 * Returns: (transfer full): A newly allocated #GarilParcel, which should be
 *   freed with garil_parcel_unref().
 */
GarilParcel*
garil_parcel_new (GByteArray *array)
{
  GarilParcel *parcel = g_new0 (GarilParcel, 1);
  parcel->ref_count = 1;
  parcel->position = 0;
  parcel->malformed = FALSE;

  if (array != NULL)
    parcel->byte_array = g_byte_array_ref (array);
  else
    parcel->byte_array = g_byte_array_new ();

  return parcel;
}

/**
 * garil_parcel_ref:
 * @parcel: A #GarilParcel.
 *
 * Increment internal reference count of a #GarilParcel.
 *
 * Returns: The parcel passed in.
 */
GarilParcel*
garil_parcel_ref (GarilParcel *parcel)
{
  g_return_val_if_fail ((parcel != NULL), NULL);

  g_atomic_int_inc (&parcel->ref_count);

  return parcel;
}

/**
 * garil_parcel_unref:
 * @parcel: A #GarilParcel.
 *
 * Release a reference of a parcel. This may results in parcel being freed.
 */
void
garil_parcel_unref (GarilParcel *parcel)
{
  g_return_if_fail (parcel != NULL);

  if (g_atomic_int_dec_and_test (&parcel->ref_count)) {
    g_byte_array_unref (parcel->byte_array);
    g_free (parcel);
  }
}

/**
 * garil_parcel_get_size:
 * @parcel: A #GarilParcel.
 *
 * Returns the total amounts of data contained in the parcel.
 *
 * Returns: The size in bytes.
 */
gsize
garil_parcel_get_size (GarilParcel *parcel)
{
  g_return_val_if_fail ((parcel != NULL), 0);

  return parcel->byte_array->len;
}

/**
 * garil_parcel_get_available:
 * @parcel: A #GarilParcel.
 *
 * Returns the amount of data available to be read in the parcel.
 *
 * Returns: The size in bytes.
 */
gsize
garil_parcel_get_available (GarilParcel *parcel)
{
  g_return_val_if_fail ((parcel != NULL), 0);

  return parcel->byte_array->len - parcel->position;
}

/**
 * garil_parcel_get_position:
 * @parcel: A #GarilParcel.
 *
 * Returns the current position in the parcel data.
 *
 * Returns: The position.
 */
goffset
garil_parcel_get_position (GarilParcel *parcel)
{
  g_return_val_if_fail ((parcel != NULL), 0);

  return parcel->position;
}

/**
 * garil_parcel_is_malformed:
 * @parcel: A #GarilParcel.
 *
 * Return whether the parcel is marked malformed.
 *
 * Returns: %TRUE if the parce is marked malformed; %FALSE otherwise.
 */
gboolean
garil_parcel_is_malformed (GarilParcel *parcel)
{
  g_return_val_if_fail ((parcel != NULL), FALSE);

  return parcel->malformed;
}

static gboolean
ensure_available (GarilParcel *parcel,
                  gsize        size)
{
  if (garil_parcel_get_available (parcel) >= size)
    return TRUE;

  parcel->malformed = TRUE;
  return FALSE;
}

static inline gsize
pad_size (gsize size)
{
  return (size + 3) & ~3;
}

/**
 * garil_parcel_read:
 * @parcel: (not nullable): A #GarilParcel.
 * @buf: (array length=len) (out caller-allocates): Destination buffer.
 * @len: Length to be read out.
 *
 * Read arbitrary length of data out of the parcel. Do nothing if the parcel has
 * been marked malformed.
 */
void
garil_parcel_read (GarilParcel *parcel,
                   gpointer     buf,
                   gsize        len)
{
  g_return_if_fail ((parcel != NULL) && (buf != NULL));

  if (parcel->malformed)
    return;

  gconstpointer p = garil_parcel_read_inplace (parcel, len);
  if (p != NULL)
    memmove (buf, p, len);
}

/**
 * garil_parcel_read_dup:
 * @parcel: (not nullable): A #GarilParcel.
 * @len: Length to be read out.
 *
 * Read arbitrary length of data out of the parcel to a newly allocated buffer.
 * Do nothing if the parcel has been marked malformed.
 *
 * Returns: (transfer full) (array length=len): A newly allocated buffer or
 *   %NULL if malformed or zeroed len passed. Free with g_free().
 */
gpointer
garil_parcel_read_dup (GarilParcel *parcel,
                       gsize        len)
{
  g_return_val_if_fail ((parcel != NULL), NULL);

  if (parcel->malformed)
    return NULL;

  return g_memdup (garil_parcel_read_inplace (parcel, len), len);
}

/**
 * garil_parcel_read_inplace:
 * @parcel: (not nullable): A #GarilParcel.
 * @len: Length to be read out.
 *
 * Read arbitrary length of data out of the parcel with a pointer to internal
 * buffer. Do nothing if the parcel has been marked malformed.
 *
 * Returns: (transfer none) (array length=len): A pointer to internal buffer.
 *   It's owned by the parcel and should never be freed.
 */
gconstpointer
garil_parcel_read_inplace (GarilParcel *parcel,
                           gsize        len)
{
  g_return_val_if_fail ((parcel != NULL), NULL);

  if (parcel->malformed)
    return NULL;

  if (len > (G_MAXSIZE - 3)) {
    parcel->malformed = TRUE;
    return NULL;
  }

  const gsize padded_len = pad_size (len);
  if (!ensure_available (parcel, padded_len))
    return NULL;

  gpointer ret = parcel->byte_array->data + parcel->position;
  parcel->position += padded_len;
  return ret;
}

/**
 * garil_parcel_write:
 * @parcel: (not nullable): A #GarilParcel.
 * @buf: (array length=len): Source buffer.
 * @len: Length to be written.
 *
 * Write arbitrary length of data into the parcel. Do nothing if the parcel has
 * been marked malformed.
 */
void
garil_parcel_write (GarilParcel   *parcel,
                    gconstpointer  buf,
                    gsize          len)
{
  g_return_if_fail ((parcel != NULL) && ((buf != NULL) || !len));

  if (parcel->malformed || !len)
    return;

  gpointer p = garil_parcel_write_inplace (parcel, len);
  if (p != NULL) {
    memmove (p, buf, len);
    if (len != pad_size (len))
      memset (((guint8 *)p) + len, 0, pad_size (len) - len);
  }
}

/**
 * garil_parcel_write_inplace:
 * @parcel: (not nullable): A #GarilParcel.
 * @len: Length to be written.
 *
 * Write arbitrary length of data into the parcel with a pointer to internal
 * buffer. Do nothing if the parcel has been marked malformed.
 *
 * Returns: (transfer none) (array length=len): A pointer to internal buffer.
 *   It's owned by the parcel and should never be freed.
 */
gpointer
garil_parcel_write_inplace (GarilParcel *parcel,
                            gsize        len)
{
  g_return_val_if_fail ((parcel != NULL), NULL);

  if (parcel->malformed)
    return NULL;

  if (len > (G_MAXSIZE - 3)) {
    parcel->malformed = TRUE;
    return NULL;
  }

  const gsize padded_len = pad_size (len);
  g_byte_array_set_size (parcel->byte_array,
                         parcel->byte_array->len + padded_len);

  gpointer ret = parcel->byte_array->data + parcel->position;
  parcel->position += padded_len;
  return ret;
}

/**
 * garil_parcel_read_byte:
 * @parcel: (not nullable): A #GarilParcel.
 *
 * Read a byte value out of the parcel. Do nothing if the parcel has been
 * marked malformed.
 *
 * Returns: A byte value;
 */
guint8
garil_parcel_read_byte (GarilParcel *parcel)
{
  g_return_val_if_fail ((parcel != NULL), 0);

  if (parcel->malformed)
    return 0;

  return garil_parcel_read_int32 (parcel);
}

/**
 * garil_parcel_read_byte_array:
 * @parcel: (not nullable): A #GarilParcel.
 *
 * Read a byte array out of the parcel. Do nothing if the parcel has been
 * marked malformed.
 *
 * Returns: (transfer full): A newly allocated #GByteArray. It should be freed
 *   with g_byte_array_unref() or the like.
 */
GByteArray*
garil_parcel_read_byte_array (GarilParcel *parcel)
{
  g_return_val_if_fail ((parcel != NULL), NULL);

  if (parcel->malformed)
    return NULL;

  const gint32 len = garil_parcel_read_int32 (parcel);
  if (parcel->malformed || (len < 0)) {
    parcel->malformed = TRUE;
    return NULL;
  }

  gconstpointer p = garil_parcel_read_inplace (parcel, len);
  if (p == NULL)
    return NULL;

  GByteArray *byte_array = g_byte_array_new ();
  g_byte_array_append (byte_array, p, len);

  return byte_array;
}

/**
 * garil_parcel_write_byte:
 * @parcel: (not nullable): A #GarilParcel.
 * @value: A byte.
 *
 * Write a byte value into the parcel. Do nothing if the parcel has been
 * marked malformed.
 */
void
garil_parcel_write_byte (GarilParcel *parcel,
                         guint8       value)
{
  g_return_if_fail (parcel != NULL);

  if (parcel->malformed)
    return;

  garil_parcel_write_int32 (parcel, value);
}

/**
 * garil_parcel_write_byte_array:
 * @parcel: (not nullable): A #GarilParcel.
 * @byte_array: (not nullable): A #GByteArray.
 *
 * Write a byte array into the parcel. Do nothing if the parcel has been marked
 * malformed.
 */
void
garil_parcel_write_byte_array (GarilParcel      *parcel,
                               const GByteArray *byte_array)
{
  g_return_if_fail ((parcel != NULL) && (byte_array != NULL));

  garil_parcel_write_byte_array_buf (parcel, byte_array->data, byte_array->len);
}

/**
 * garil_parcel_write_byte_array_buf:
 * @parcel: (not nullable): A #GarilParcel.
 * @buf: (array length=len): Source buffer.
 * @len: Number of elements to be written.
 *
 * Write a byte array into the parcel. Do nothing if the parcel has been marked
 * malformed.
 */
void
garil_parcel_write_byte_array_buf (GarilParcel  *parcel,
                                   const guint8 *buf,
                                   gsize         len)
{
  g_return_if_fail ((parcel != NULL) && ((buf != NULL) || !len));

  if (parcel->malformed)
    return;

  garil_parcel_write_int32 (parcel, len);
  if (!len)
    return;

  garil_parcel_write (parcel, buf, len);
}

/**
 * garil_parcel_read_int32:
 * @parcel: (not nullable): A #GarilParcel.
 *
 * Read a #gint32 value out of the parcel. Do nothing if the parcel has been
 * marked malformed.
 *
 * Returns: A #gint32 value;
 */
gint32
garil_parcel_read_int32 (GarilParcel *parcel)
{
  g_return_val_if_fail ((parcel != NULL), 0);

  if (parcel->malformed)
    return 0;

  gint32 value  = 0;

  garil_parcel_read (parcel, &value, sizeof (gint32));
  value = GINT32_FROM_LE(value);

  return value;
}

/**
 * garil_parcel_read_int32_array:
 * @parcel: (not nullable): A #GarilParcel.
 *
 * Read a #gint32 array out of the parcel. Do nothing if the parcel has been
 * marked malformed.
 *
 * Returns: (transfer full) (element-type gint32): A newly allocated #GArray. It
 *   should be freed with g_array_unref() or g_array_free();
 */
GArray*
garil_parcel_read_int32_array (GarilParcel *parcel)
{
  g_return_val_if_fail ((parcel != NULL), NULL);

  if (parcel->malformed)
    return NULL;

  const gint32 len = garil_parcel_read_int32 (parcel);
  if (parcel->malformed || (len < 0)) {
    parcel->malformed = TRUE;
    return NULL;
  }

  const gint32 *p = garil_parcel_read_inplace (parcel, len * sizeof (gint32));
  if (p == NULL)
    return NULL;

  GArray *array = g_array_sized_new (FALSE, FALSE, sizeof (gint32), len);
#if (G_BYTE_ORDER == G_LITTLE_ENDIAN)
  g_array_append_vals (array, p, len);
#else
  /* g_array_sized_new() only preallocates the space, but an explicit
   * g_array_set_size() call is still required. */
  g_array_set_size (array, len);
  for (gint32 i = 0, *q = (gint32 *) array->data; i < len; i++, p++, q++) {
    *q = GINT32_FROM_LE (*p);
  }
#endif

  return array;
}

/**
 * garil_parcel_write_int32:
 * @parcel: (not nullable): A #GarilParcel.
 * @value: A #gint32.
 *
 * Write a #gint32 value into the parcel. Do nothing if the parcel has been
 * marked malformed.
 */
void
garil_parcel_write_int32 (GarilParcel *parcel,
                          gint32       value)
{
  g_return_if_fail (parcel != NULL);

  if (parcel->malformed)
    return;

  gint32 *p = garil_parcel_write_inplace (parcel, sizeof (gint32));
  if (p == NULL)
    return;

  *p = GINT32_TO_LE (value);
}

/**
 * garil_parcel_write_int32_array:
 * @parcel: (not nullable): A #GarilParcel.
 * @array: (not nullable) (element-type gint32): A #GArray of #gint32.
 *
 * Write a #gint32 array into the parcel. Do nothing if the parcel has been
 * marked malformed.
 */
void
garil_parcel_write_int32_array (GarilParcel  *parcel,
                                const GArray *array)
{
  g_return_if_fail ((parcel != NULL) && (array != NULL));

  garil_parcel_write_int32_array_buf (parcel, (const gint32 *) array->data,
                                      array->len);
}

/**
 * garil_parcel_write_int32_array_buf:
 * @parcel: (not nullable): A #GarilParcel.
 * @buf: (array length=len): Source buffer.
 * @len: Number of elements to be written.
 *
 * Write a #gint32 buffer into the parcel. Do nothing if the parcel has been
 * marked malformed.
 */
void
garil_parcel_write_int32_array_buf (GarilParcel  *parcel,
                                    const gint32 *buf,
                                    gsize         len)
{
  g_return_if_fail ((parcel != NULL) && ((buf != NULL) || !len));

  if (parcel->malformed)
    return;

  garil_parcel_write_int32 (parcel, len);
  if (!len)
    return;

  gint32 *p = garil_parcel_write_inplace (parcel, len * sizeof (gint32));
  if (p == NULL)
    return;

#if (G_BYTE_ORDER == G_LITTLE_ENDIAN)
  memmove (p, buf, len * sizeof (gint32));
#else
  for (gsize i = 0; i < len; i++, p++, buf++) {
    *p = GINT32_TO_LE (*buf);
  }
#endif
}

/**
 * garil_parcel_read_string16:
 * @parcel: (not nullable): A #GarilParcel.
 *
 * Read a utf-16 encoded string out of the parcel. Do nothing if the parcel has
 * been marked malformed.
 *
 * Returns: (transfer full): A string, which should be freed with g_free().
 */
gchar*
garil_parcel_read_string16 (GarilParcel *parcel)
{
  g_return_val_if_fail ((parcel != NULL), NULL);

  if (parcel->malformed)
    return NULL;

  const gint32 len = garil_parcel_read_int32 (parcel);
  if (parcel->malformed || (len < 0))
    return NULL;

  /* Byte order for each gunichar2 is the native one. */
  const gunichar2 *utf16_str =
    garil_parcel_read_inplace (parcel, (len + 1) * sizeof (gunichar2));
  if (utf16_str == NULL)
    return NULL;

  gchar *utf8_str = g_utf16_to_utf8 (utf16_str, len, NULL, NULL, NULL);
  if (utf8_str == NULL)
    parcel->malformed = TRUE;

  return utf8_str;
}

/**
 * garil_parcel_read_string16_array:
 * @parcel: (not nullable): A #GarilParcel.
 * @len: (not nullable) (out): Number of elements in the @array.
 *
 * Read an array of utf-16 encoded strings out of the parcel. Do nothing if the
 * parcel has been marked malformed.
 *
 * The elements in @array may be NULL. Actual array length is specified in @len.
 *
 * Returns: (transfer full) (array length=len): A string array. It should be
 *   freed with g_free().
 */
gchar**
garil_parcel_read_string16_array (GarilParcel *parcel,
                                  gsize       *len)
{
  g_return_val_if_fail (((parcel != NULL) && (len != NULL)), NULL);

  if (parcel->malformed)
    return NULL;

  gint32 ilen = garil_parcel_read_int32 (parcel);
  if (parcel->malformed || (ilen < 0)) {
    parcel->malformed = TRUE;
    return NULL;
  }

  gchar **array = NULL;

  if (ilen) {
    array = g_new0 (gchar *, ilen);
    for (gint i = 0; i < ilen; i++) {
      array[i] = garil_parcel_read_string16 (parcel);
      if (parcel->malformed) {
        for (gint j = 0; j < i; j++)
          g_free (array[j]);

        g_free (array);
        array = NULL;
        break;
      }
    }
  }

  if (!parcel->malformed)
    *len = ilen;

  return array;
}

/**
 * garil_parcel_write_string16:
 * @parcel: (not nullable): A #GarilParcel.
 * @utf8_str: A string.
 *
 * Write a utf-16 encoded string into the parcel. Do nothing if the parcel has
 * been marked malformed.
 */
void
garil_parcel_write_string16 (GarilParcel *parcel,
                             const gchar *utf8_str)
{
  g_return_if_fail (parcel != NULL);

  if (parcel->malformed)
    return;

  if (utf8_str == NULL) {
    garil_parcel_write_int32 (parcel, -1);
    return;
  }

  glong len = 0;
  gunichar2 *utf16_str = g_utf8_to_utf16 (utf8_str, -1, NULL, &len, NULL);
  if (utf16_str == NULL)
    return;

  garil_parcel_write_int32 (parcel, len);
  /* Byte order for each gunichar2 is the native one. */
  garil_parcel_write (parcel, utf16_str, (len + 1) * sizeof (gunichar2));
}

/**
 * garil_parcel_write_string16_array:
 * @parcel: (not nullable): A #GarilParcel.
 * @array: (array length=len): An string array.
 * @len: Number of elements in the @array.
 *
 * Write an array of utf-8 encoded strings into the parcel. Do nothing if the
 * parcel has been marked malformed.
 *
 * The elements in @array may be NULL. Actual array length should be specified
 * in @len.
 */
void
garil_parcel_write_string16_array (GarilParcel         *parcel,
                                   const gchar * const *array,
                                   gsize                len)
{
  g_return_if_fail ((parcel != NULL) && ((array != NULL) || !len));

  if (parcel->malformed)
    return;

  garil_parcel_write_int32 (parcel, len);

  for (guint i = 0; i < len; i++)
    garil_parcel_write_string16 (parcel, array[i]);
}
