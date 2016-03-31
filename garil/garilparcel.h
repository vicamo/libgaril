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

G_BEGIN_DECLS

typedef struct _GarilParcel GarilParcel;

GarilParcel *garil_parcel_new (GByteArray *array);
GarilParcel *garil_parcel_ref (GarilParcel *parcel);
void garil_parcel_unref (GarilParcel *parcel);

gsize garil_parcel_get_size (GarilParcel *parcel);
gsize garil_parcel_get_available (GarilParcel *parcel);
goffset garil_parcel_get_position (GarilParcel *parcel);
gboolean garil_parcel_is_malformed (GarilParcel *parcel);

void garil_parcel_read (GarilParcel *parcel,
                        gpointer     buf,
                        gsize        len);
gpointer garil_parcel_read_dup (GarilParcel *parcel,
                                gsize        len);
gconstpointer garil_parcel_read_inplace (GarilParcel *parcel,
                                         gsize        len);
void garil_parcel_write (GarilParcel   *parcel,
                         gconstpointer  buf,
                         gsize          len);
gpointer garil_parcel_write_inplace (GarilParcel *parcel,
                                     gsize        len);

guint8 garil_parcel_read_byte (GarilParcel *parcel);
GByteArray *garil_parcel_read_byte_array (GarilParcel *parcel);
void garil_parcel_write_byte (GarilParcel *parcel,
                              guint8       value);
void garil_parcel_write_byte_array (GarilParcel      *parcel,
                                    const GByteArray *byte_array);
void garil_parcel_write_byte_array_buf (GarilParcel  *parcel,
                                        const guint8 *buf,
                                        gsize         len);

gint32 garil_parcel_read_int32 (GarilParcel *parcel);
GArray *garil_parcel_read_int32_array (GarilParcel *parcel);
void garil_parcel_write_int32 (GarilParcel *parcel,
                               gint32       value);
void garil_parcel_write_int32_array (GarilParcel  *parcel,
                                     const GArray *array);
void garil_parcel_write_int32_array_buf (GarilParcel  *parcel,
                                         const gint32 *buf,
                                         gsize         len);

gchar *garil_parcel_read_string16 (GarilParcel *parcel);
gchar **garil_parcel_read_string16_array (GarilParcel *parcel,
                                          gsize       *len);
void garil_parcel_write_string16 (GarilParcel *parcel,
                                  const gchar *utf8_str);
void garil_parcel_write_string16_array (GarilParcel         *parcel,
                                        const gchar * const *array,
                                        gsize                len);

G_END_DECLS
