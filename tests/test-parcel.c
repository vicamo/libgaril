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
#include <string.h>

#include <glib.h>

#include "garil/garil.h"

typedef struct {
  const guint8 *data;
  gsize len;
} Bytes;

typedef struct {
  const guint32 *data;
  gsize len;
} Bytes32;

#define EMPTY_BYTES { .data = NULL, .len = 0, }

typedef struct {
  Bytes input;
  GarilParcel *parcel;
} FixtureMalformed;

static gsize
pad_size (gsize size)
{
  return (size + 3) & ~3;
}

static void
check_malformed_fixture (FixtureMalformed *fixture)
{
  g_assert_cmpint (garil_parcel_get_size (fixture->parcel), ==,
                   fixture->input.len);
  g_assert_cmpint (garil_parcel_get_available (fixture->parcel), ==,
                   fixture->input.len);
  g_assert_cmpint (garil_parcel_get_position (fixture->parcel), ==, 0);
  g_assert_true (garil_parcel_is_malformed (fixture->parcel));
}

/****************************** garil_parcel_new ******************************/

static void
test_new_valid__basic (gconstpointer user_data)
{
  GByteArray *byte_array = (GByteArray *) user_data;

  GarilParcel *parcel = garil_parcel_new (byte_array);
  g_assert_nonnull (parcel);
  if (byte_array != NULL)
    g_assert_cmpint (garil_parcel_get_size (parcel), ==, byte_array->len);
  else
    g_assert_cmpint (garil_parcel_get_size (parcel), ==, 0);
  g_assert_cmpint (garil_parcel_get_available (parcel), ==, 0);
  g_assert_cmpint (garil_parcel_get_position (parcel), ==, 0);
  g_assert_false (garil_parcel_is_malformed (parcel));

  garil_parcel_unref (parcel);
}

/***************************** garil_parcel_read ******************************/

static void
test_read__basic (void)
{
  static const guint8 data[] = {
    0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };

  static const guint8 signature[] = {
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7
  };

  static const gsize positions[] = { 0, 4, 8, 12, 16, 24, 32 };

  guint8 buf [sizeof (signature)];

  GByteArray *byte_array =
    g_byte_array_new_take (g_memdup (data, sizeof (data)), sizeof (data));

  GarilParcel *parcel = garil_parcel_new (byte_array);

  for (gint i = 0; i < G_N_ELEMENTS (positions); i++) {
    memcpy (buf, signature, sizeof (buf));

    garil_parcel_read (parcel, buf, i);

    if (positions[i]) {
      g_assert_cmpmem (buf, i, &data[positions[i - 1]], i);
      if (i < sizeof (buf)) {
        g_assert_cmpmem (&buf[i], sizeof (buf) - i,
                         &signature[i], sizeof (signature) - i);
      }
    } else {
      g_assert_cmpmem (buf, sizeof (buf), signature, sizeof (signature));
    }
    g_assert_cmpint (garil_parcel_get_size (parcel), ==, sizeof (data));
    g_assert_cmpint (garil_parcel_get_available (parcel), ==,
                     sizeof (data) - positions[i]);
    g_assert_cmpint (garil_parcel_get_position (parcel), ==, positions[i]);
    g_assert_false (garil_parcel_is_malformed (parcel));
  }

  {
    const gsize len =
      sizeof (data) - positions[G_N_ELEMENTS (positions) - 1] + 1;
    g_assert_cmpint (len, <=, sizeof (buf));

    memcpy (buf, signature, sizeof (buf));

    garil_parcel_read (parcel, buf, len);

    g_assert_cmpmem (buf, sizeof (buf), signature, sizeof (signature));
    g_assert_cmpint (garil_parcel_get_size (parcel), ==, sizeof (data));
    g_assert_cmpint (garil_parcel_get_available (parcel), ==,
                     sizeof (data) - positions[G_N_ELEMENTS (positions) - 1]);
    g_assert_cmpint (garil_parcel_get_position (parcel), ==,
                     positions[G_N_ELEMENTS (positions) - 1]);
    g_assert_true (garil_parcel_is_malformed (parcel));
  }

  garil_parcel_unref (parcel);
  g_byte_array_unref (byte_array);
}

/* Byte array that is valid for:
 * - garil_parcel_read
 * - garil_parcel_read_dup
 * - garil_parcel_read_inplace
 * - garil_parcel_write
 * - garil_parcel_write_inplace
 * - garil_parcel_read_byte
 * - garil_parcel_read_byte_array
 * - garil_parcel_write_byte
 * - garil_parcel_write_byte_array
 * - garil_parcel_write_byte_array_buf
 * - garil_parcel_read_int32
 * - garil_parcel_read_int32_array
 * - garil_parcel_write_int32
 * - garil_parcel_write_int32_array
 * - garil_parcel_write_int32_array_buf
 * - garil_parcel_read_string16
 * - garil_parcel_write_string16
 * - garil_parcel_write_string16_array
 */
static const guint8 bytes_read__malformed_data[] = {
  0x01, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00
};

static const Bytes bytes_read__malformed = {
  .data = bytes_read__malformed_data,
  .len = sizeof (bytes_read__malformed_data),
};

static void
test_read__malformed (FixtureMalformed *fixture,
                      gconstpointer     user_data G_GNUC_UNUSED)
{
  guint8 buf;

  garil_parcel_read (fixture->parcel, &buf, sizeof (buf));

  check_malformed_fixture (fixture);
}

/*************************** garil_parcel_read_dup ****************************/

static void
test_read_dup__basic (void)
{
  static const guint8 data[] = {
    0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };

  static const gsize positions[] = { 0, 4, 8, 12, 16, 24, 32 };

  GByteArray *byte_array =
    g_byte_array_new_take (g_memdup (data, sizeof (data)), sizeof (data));

  GarilParcel *parcel = garil_parcel_new (byte_array);

  for (gint i = 0; i < G_N_ELEMENTS (positions); i++) {
    guint8 *p = garil_parcel_read_dup (parcel, i);

    if (i) {
      g_assert_nonnull (p);
      g_assert_cmpmem (p, i, &data[positions[i - 1]], i);
      g_free (p);
    } else
      g_assert_null (p);
    g_assert_cmpint (garil_parcel_get_size (parcel), ==, sizeof (data));
    g_assert_cmpint (garil_parcel_get_available (parcel), ==,
                     sizeof (data) - positions[i]);
    g_assert_cmpint (garil_parcel_get_position (parcel), ==, positions[i]);
    g_assert_false (garil_parcel_is_malformed (parcel));
  }

  {
    const gsize len =
      sizeof (data) - positions[G_N_ELEMENTS (positions) - 1] + 1;

    guint8 *p = garil_parcel_read_dup (parcel, len);

    g_assert_null (p);
    g_assert_cmpint (garil_parcel_get_size (parcel), ==, sizeof (data));
    g_assert_cmpint (garil_parcel_get_available (parcel), ==,
                     sizeof (data) - positions[G_N_ELEMENTS (positions) - 1]);
    g_assert_cmpint (garil_parcel_get_position (parcel), ==,
                     positions[G_N_ELEMENTS (positions) - 1]);
    g_assert_true (garil_parcel_is_malformed (parcel));
  }

  garil_parcel_unref (parcel);
  g_byte_array_unref (byte_array);
}

#define bytes_read_dup__malformed bytes_read__malformed

static void
test_read_dup__malformed (FixtureMalformed *fixture,
                          gconstpointer     user_data G_GNUC_UNUSED)
{
  gpointer p = garil_parcel_read_dup (fixture->parcel, sizeof (guint8));
  g_assert_null (p);

  check_malformed_fixture (fixture);
}

/************************* garil_parcel_read_inplace **************************/

static void
test_read_inplace__basic (void)
{
  static const guint8 data[] = {
    0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };

  static const gsize positions[] = { 0, 4, 8, 12, 16, 24, 32 };

  GByteArray *byte_array =
    g_byte_array_new_take (g_memdup (data, sizeof (data)), sizeof (data));

  GarilParcel *parcel = garil_parcel_new (byte_array);

  for (gint i = 0; i < G_N_ELEMENTS (positions); i++) {
    const guint8 *p = garil_parcel_read_inplace (parcel, i);

    g_assert (p == (byte_array->data + positions[i > 0 ? i - 1 : 0]));
    g_assert_cmpint (garil_parcel_get_size (parcel), ==, sizeof (data));
    g_assert_cmpint (garil_parcel_get_available (parcel), ==,
                     sizeof (data) - positions[i]);
    g_assert_cmpint (garil_parcel_get_position (parcel), ==, positions[i]);
    g_assert_false (garil_parcel_is_malformed (parcel));
  }

  {
    const gsize len =
      sizeof (data) - positions[G_N_ELEMENTS (positions) - 1] + 1;

    const gchar *p = garil_parcel_read_inplace (parcel, len);

    g_assert_null (p);
    g_assert_cmpint (garil_parcel_get_size (parcel), ==, sizeof (data));
    g_assert_cmpint (garil_parcel_get_available (parcel), ==,
                     sizeof (data) - positions[G_N_ELEMENTS (positions) - 1]);
    g_assert_cmpint (garil_parcel_get_position (parcel), ==,
                     positions[G_N_ELEMENTS (positions) - 1]);
    g_assert_true (garil_parcel_is_malformed (parcel));
  }

  garil_parcel_unref (parcel);
  g_byte_array_unref (byte_array);
}

#define bytes_read_inplace__malformed bytes_read__malformed

static void
test_read_inplace__malformed (FixtureMalformed *fixture,
                              gconstpointer     user_data G_GNUC_UNUSED)
{
  gconstpointer p =
    garil_parcel_read_inplace (fixture->parcel, sizeof (guint8));
  g_assert_null (p);

  check_malformed_fixture (fixture);
}

/***************************** garil_parcel_write *****************************/

static void
test_write__basic (void)
{
  static const guint8 signature[] = {
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7
  };
  g_assert_cmpint (pad_size (sizeof (signature)), ==, sizeof (signature));

  static guint8 zeros[sizeof (signature)];
  memset (zeros, 0, sizeof (zeros));

  GByteArray *byte_array = g_byte_array_new ();
  GarilParcel *parcel = garil_parcel_new (byte_array);

  gsize total_size = 0;
  for (gsize i = 0; i <= sizeof (signature); i++) {
    garil_parcel_write (parcel, signature, i);

    const gsize padded = pad_size (i);
    total_size += padded;
    g_assert_cmpmem (byte_array->data + total_size - padded, i, signature, i);
    if (i != padded) {
      g_assert_cmpmem (byte_array->data + total_size - padded + i, padded - i,
                       zeros, padded - i);
    }
    g_assert_cmpint (garil_parcel_get_size (parcel), ==, total_size);
    g_assert_cmpint (garil_parcel_get_available (parcel), ==, 0);
    g_assert_cmpint (garil_parcel_get_position (parcel), ==, total_size);
    g_assert_false (garil_parcel_is_malformed (parcel));
  }

  garil_parcel_unref (parcel);
  g_byte_array_unref (byte_array);
}

#define bytes_write__malformed bytes_read__malformed

static void
test_write__malformed (FixtureMalformed *fixture,
                       gconstpointer     user_data G_GNUC_UNUSED)
{
  guint8 value = 0xff;

  garil_parcel_write (fixture->parcel, &value, sizeof (value));

  check_malformed_fixture (fixture);
}

/************************* garil_parcel_write_inplace *************************/

static void
test_write_inplace__basic (void)
{
  GByteArray *byte_array = g_byte_array_new ();
  GarilParcel *parcel = garil_parcel_new (byte_array);

  gsize total_size = 0;
  for (gsize i = 0; i <= 8; i++) {
    guint8 *p = garil_parcel_write_inplace (parcel, i);

    const gsize padded = pad_size (i);
    total_size += padded;
    g_assert (p == (byte_array->data + total_size - padded));
    g_assert_cmpint (garil_parcel_get_size (parcel), ==, total_size);
    g_assert_cmpint (garil_parcel_get_available (parcel), ==, 0);
    g_assert_cmpint (garil_parcel_get_position (parcel), ==, total_size);
    g_assert_false (garil_parcel_is_malformed (parcel));
  }

  garil_parcel_unref (parcel);
  g_byte_array_unref (byte_array);
}

#define bytes_write_inplace__malformed bytes_read__malformed

static void
test_write_inplace__malformed (FixtureMalformed *fixture,
                               gconstpointer     user_data G_GNUC_UNUSED)
{
  gpointer p = garil_parcel_write_inplace (fixture->parcel, sizeof (guint8));
  g_assert_null (p);

  check_malformed_fixture (fixture);
}

/*************************** garil_parcel_read_byte ***************************/

typedef struct {
  Bytes input;
  gboolean malformed;
  guint8 expected;
} TestDataReadByte;

#define DEFINE_DATA(n, m, e, ...) \
  const guint8 testdata_read_byte_ ## n ## _input[] = { __VA_ARGS__ }; \
  const TestDataReadByte testdata_read_byte_ ## n = { \
    .input = { \
      .data = testdata_read_byte_ ## n ## _input, \
      .len = G_N_ELEMENTS (testdata_read_byte_ ## n ## _input), \
    }, \
    .malformed = m, \
    .expected = e, \
  };

#define DEFINE_VALID(n, e, ...) DEFINE_DATA (n, FALSE, e, __VA_ARGS__)
#define DEFINE_INVALID(n, ...)  DEFINE_DATA (n, TRUE, 0, __VA_ARGS__)

DEFINE_VALID (1, 0x00, 0x00, 0x00, 0x00, 0x00)
DEFINE_VALID (2, 0x01, 0x01, 0x00, 0x00, 0x00)
DEFINE_VALID (3, 0xff, 0xff, 0x00, 0x00, 0x00)
DEFINE_VALID (4, 0x01, 0x01, 0x02, 0x03, 0x04)
DEFINE_VALID (5, 0xff, 0xff, 0xff, 0xff, 0xff)
DEFINE_VALID (6, 0x01, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00)

const TestDataReadByte testdata_read_byte_7 = {
  .input = EMPTY_BYTES,
  .malformed = TRUE,
  .expected = 0
};

DEFINE_INVALID (8, 0x01)
DEFINE_INVALID (9, 0x01, 0x02)
DEFINE_INVALID (10, 0x01, 0x02, 0x3)

#undef DEFINE_DATA
#undef DEFINE_VALID
#undef DEFINE_INVALID

static void
test_read_byte__basic (gconstpointer user_data)
{
  const TestDataReadByte *data = user_data;

  GByteArray *byte_array =
    g_byte_array_new_take (g_memdup (data->input.data, data->input.len),
                           data->input.len);
  GarilParcel *parcel = garil_parcel_new (byte_array);

  guint8 value = garil_parcel_read_byte (parcel);
  g_assert_cmpint (value, ==, data->expected);
  g_assert_cmpint (garil_parcel_get_size (parcel), ==, data->input.len);
  g_assert (garil_parcel_is_malformed (parcel) == data->malformed);
  if (data->malformed) {
    g_assert_cmpint (garil_parcel_get_available (parcel), ==, data->input.len);
    g_assert_cmpint (garil_parcel_get_position (parcel), ==, 0);
  } else {
    g_assert_cmpint (garil_parcel_get_available (parcel), ==,
                     data->input.len - sizeof (gint32));
    g_assert_cmpint (garil_parcel_get_position (parcel), ==, sizeof (gint32));
  }

  garil_parcel_unref (parcel);
  g_byte_array_unref (byte_array);
}

#define bytes_read_byte__malformed bytes_read__malformed

static void
test_read_byte__malformed (FixtureMalformed *fixture,
                           gconstpointer     user_data G_GNUC_UNUSED)
{
  guint8 value = garil_parcel_read_byte (fixture->parcel);
  g_assert_cmpint (value, ==, 0);

  check_malformed_fixture (fixture);
}

/************************ garil_parcel_read_byte_array ************************/

typedef struct {
  Bytes input;
  Bytes expected;
  gsize position;
} TestDataReadByteArray;

#define DEFINE_EXPECTED(n, ...) \
  const guint8 testdata_read_byte_array_ ## n ## _expected[] = { __VA_ARGS__ };
#define DEFINE_VALID(n, p, ...) \
  const guint8 testdata_read_byte_array_ ## n ## _input[] = { __VA_ARGS__ }; \
  const TestDataReadByteArray testdata_read_byte_array_ ## n = { \
    .input = { \
      .data = testdata_read_byte_array_ ## n ## _input, \
      .len = G_N_ELEMENTS (testdata_read_byte_array_ ## n ## _input), \
    }, \
    .expected = { \
      .data = testdata_read_byte_array_ ## n ## _expected, \
      .len = G_N_ELEMENTS (testdata_read_byte_array_ ## n ## _expected), \
    }, \
    .position = p, \
  };
#define DEFINE_INVALID(n, p, ...) \
  const guint8 testdata_read_byte_array_ ## n ## _input[] = { __VA_ARGS__ }; \
  const TestDataReadByteArray testdata_read_byte_array_ ## n = { \
    .input = { \
      .data = testdata_read_byte_array_ ## n ## _input, \
      .len = G_N_ELEMENTS (testdata_read_byte_array_ ## n ## _input), \
    }, \
    .expected = EMPTY_BYTES, \
    .position = p, \
  };

DEFINE_EXPECTED (1)
DEFINE_VALID (1, 4,
              0x00, 0x00, 0x00, 0x00)
DEFINE_EXPECTED (2, 0x02)
DEFINE_VALID (2, 8,
              0x01, 0x00, 0x00, 0x00, 0x02, 0x03, 0x04, 0x05)
DEFINE_EXPECTED (3, 0x02, 0x03)
DEFINE_VALID (3, 8,
              0x02, 0x00, 0x00, 0x00, 0x02, 0x03, 0x04, 0x05)
DEFINE_EXPECTED (4, 0x02, 0x03, 0x04)
DEFINE_VALID (4, 8,
              0x03, 0x00, 0x00, 0x00, 0x02, 0x03, 0x04, 0x05)
DEFINE_EXPECTED (5, 0x02, 0x03, 0x04, 0x05)
DEFINE_VALID (5, 8,
              0x04, 0x00, 0x00, 0x00, 0x02, 0x03, 0x04, 0x05)
DEFINE_EXPECTED (6, 0x02, 0x03, 0x04, 0x05, 0x06)
DEFINE_VALID (6, 12,
              0x05, 0x00, 0x00, 0x00, 0x02, 0x03, 0x04, 0x05,
              0x06, 0x07, 0x08, 0x09)
DEFINE_EXPECTED (7, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07)
DEFINE_VALID (7, 12,
              0x06, 0x00, 0x00, 0x00, 0x02, 0x03, 0x04, 0x05,
              0x06, 0x07, 0x08, 0x09)
DEFINE_EXPECTED (8, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08)
DEFINE_VALID (8, 12,
              0x07, 0x00, 0x00, 0x00, 0x02, 0x03, 0x04, 0x05,
              0x06, 0x07, 0x08, 0x09)
DEFINE_EXPECTED (9, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09)
DEFINE_VALID (9, 12,
              0x08, 0x00, 0x00, 0x00, 0x02, 0x03, 0x04, 0x05,
              0x06, 0x07, 0x08, 0x09)
DEFINE_EXPECTED (10, 0x02)
DEFINE_VALID (10, 8,
              0x01, 0x00, 0x00, 0x00, 0x02, 0x03, 0x04, 0x05,
              0x06, 0x07, 0x08, 0x09)

const TestDataReadByteArray testdata_read_byte_array_11 = {
  .input = EMPTY_BYTES,
  .expected = EMPTY_BYTES,
  .position = 0,
};

DEFINE_INVALID (12, 0, 0x01)
DEFINE_INVALID (13, 0, 0x01, 0x00)
DEFINE_INVALID (14, 0, 0x01, 0x00, 0x00)
DEFINE_INVALID (15, 4, 0x01, 0x00, 0x00, 0x00)
DEFINE_INVALID (16, 4, 0x01, 0x00, 0x00, 0x00, 0x02)
DEFINE_INVALID (17, 4, 0x01, 0x00, 0x00, 0x00, 0x02, 0x03)
DEFINE_INVALID (18, 4, 0x01, 0x00, 0x00, 0x00, 0x02, 0x03, 0x04)
DEFINE_INVALID (19, 4, 0x05, 0x00, 0x00, 0x00, 0x02, 0x03, 0x04, 0x05,
                0x06)
DEFINE_INVALID (20, 4, 0x05, 0x00, 0x00, 0x00, 0x02, 0x03, 0x04, 0x05,
                0x06, 0x07)
DEFINE_INVALID (21, 4, 0x05, 0x00, 0x00, 0x00, 0x02, 0x03, 0x04, 0x05,
                0x06, 0x07, 0x08)
DEFINE_INVALID (22, 4, 0xff, 0xff, 0xff, 0xff)
DEFINE_INVALID (23, 4, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00)

#undef DEFINE_EXPECTED
#undef DEFINE_VALID
#undef DEFINE_INVALID

static void
test_read_byte_array__basic (gconstpointer user_data)
{
  const TestDataReadByteArray *data = user_data;

  GByteArray *byte_array =
    g_byte_array_new_take (g_memdup (data->input.data, data->input.len),
                           data->input.len);
  GarilParcel *parcel = garil_parcel_new (byte_array);

  GByteArray *result = garil_parcel_read_byte_array (parcel);
  g_assert_cmpint (garil_parcel_get_size (parcel), ==, data->input.len);
  g_assert_cmpint (garil_parcel_get_available (parcel), ==,
                   data->input.len - data->position);
  g_assert_cmpint (garil_parcel_get_position (parcel), ==, data->position);
  if (data->expected.data != NULL) {
    g_assert_nonnull (result);
    g_assert_cmpmem (result->data, result->len,
                     data->expected.data, data->expected.len);
    g_assert_false (garil_parcel_is_malformed (parcel));

    g_byte_array_unref (result);
  } else {
    g_assert_null (result);
    g_assert_true (garil_parcel_is_malformed (parcel));
  }

  garil_parcel_unref (parcel);
  g_byte_array_unref (byte_array);
}

#define bytes_read_byte_array__malformed bytes_read__malformed

static void
test_read_byte_array__malformed (FixtureMalformed *fixture,
                                 gconstpointer     user_data G_GNUC_UNUSED)
{
  GByteArray *array = garil_parcel_read_byte_array (fixture->parcel);
  g_assert_null (array);

  check_malformed_fixture (fixture);
}

/************************** garil_parcel_write_byte ***************************/

typedef struct {
  guint8 input;
  Bytes expected;
} TestDataWriteByte;

#define DEFINE_VALID(n, i) \
  const guint8 testdata_write_byte_ ## n ## _expected[] = { i, 0x0, 0x0, 0x0 }; \
  const TestDataWriteByte testdata_write_byte_ ## n = { \
    .input = i, \
    .expected = { \
      .data = testdata_write_byte_ ## n ## _expected, \
      .len = G_N_ELEMENTS (testdata_write_byte_ ## n ## _expected), \
    }, \
  };

DEFINE_VALID (1, 0);
DEFINE_VALID (2, 1);
DEFINE_VALID (3, G_MAXUINT8);

#undef DEFINE_VALID

static void
test_write_byte__basic (gconstpointer user_data)
{
  const TestDataWriteByte *data = user_data;

  GByteArray *byte_array = g_byte_array_new ();
  GarilParcel *parcel = garil_parcel_new (byte_array);

  garil_parcel_write_byte (parcel, data->input);
  g_assert_cmpint (garil_parcel_get_size (parcel), ==, data->expected.len);
  g_assert_cmpint (garil_parcel_get_available (parcel), ==, 0);
  g_assert_cmpint (garil_parcel_get_position (parcel), ==, data->expected.len);
  g_assert_cmpmem (byte_array->data, byte_array->len,
                   data->expected.data, data->expected.len);
  g_assert_false (garil_parcel_is_malformed (parcel));

  garil_parcel_unref (parcel);
  g_byte_array_unref (byte_array);
}

#define bytes_write_byte__malformed bytes_read__malformed

static void
test_write_byte__malformed (FixtureMalformed *fixture,
                            gconstpointer     user_data G_GNUC_UNUSED)
{
  garil_parcel_write_byte (fixture->parcel, 0);

  check_malformed_fixture (fixture);
}

/*********************** garil_parcel_write_byte_array ************************/

typedef struct {
  Bytes input;
  Bytes expected;
} TestDataWriteByteArray;

#define DEFINE_EXPECTED(n, ...) \
  const guint8 testdata_write_byte_array_ ## n ## _expected[] = { __VA_ARGS__ };
#define DEFINE_VALID(n, ...) \
  const guint8 testdata_write_byte_array_ ## n ## _input[] = { __VA_ARGS__ }; \
  const TestDataWriteByteArray testdata_write_byte_array_ ## n = { \
    .input = { \
      .data = testdata_write_byte_array_ ## n ## _input, \
      .len = G_N_ELEMENTS (testdata_write_byte_array_ ## n ## _input), \
    }, \
    .expected = { \
      .data = testdata_write_byte_array_ ## n ## _expected, \
      .len = G_N_ELEMENTS (testdata_write_byte_array_ ## n ## _expected), \
    }, \
  };

DEFINE_EXPECTED (1, 0x00, 0x00, 0x00, 0x00)
const TestDataWriteByteArray testdata_write_byte_array_1 = {
  .input = EMPTY_BYTES,
  .expected = {
    .data = testdata_write_byte_array_1_expected,
    .len = G_N_ELEMENTS (testdata_write_byte_array_1_expected),
  },
};

DEFINE_EXPECTED (2, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00)
DEFINE_VALID (2, 0x01)
DEFINE_EXPECTED (3, 0x02, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00)
DEFINE_VALID (3, 0x01, 0x02)
DEFINE_EXPECTED (4, 0x03, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x00)
DEFINE_VALID (4, 0x01, 0x02, 0x03)
DEFINE_EXPECTED (5, 0x04, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04)
DEFINE_VALID (5, 0x01, 0x02, 0x03, 0x04)
DEFINE_EXPECTED (6, 0x05, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04,
                 0x05, 0x00, 0x00, 0x00)
DEFINE_VALID (6, 0x01, 0x02, 0x03, 0x04, 0x05)
DEFINE_EXPECTED (7, 0x06, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04,
                 0x05, 0x06, 0x00, 0x00)
DEFINE_VALID (7, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06)
DEFINE_EXPECTED (8, 0x07, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04,
                 0x05, 0x06, 0x07, 0x00)
DEFINE_VALID (8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07)
DEFINE_EXPECTED (9, 0x08, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04,
                 0x05, 0x06, 0x07, 0x08)
DEFINE_VALID (9, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08)

#undef DEFINE_EXPECTED
#undef DEFINE_VALID

static void
test_write_byte_array__basic (gconstpointer user_data)
{
  const TestDataWriteByteArray *data = user_data;

  GByteArray *byte_array = g_byte_array_new ();
  GarilParcel *parcel = garil_parcel_new (byte_array);

  GByteArray *array = g_byte_array_new ();
  if (data->input.data != NULL)
    g_byte_array_append (array, data->input.data, data->input.len);

  garil_parcel_write_byte_array (parcel, array);
  g_byte_array_unref (array);

  g_assert_cmpint (garil_parcel_get_size (parcel), ==, data->expected.len);
  g_assert_cmpint (garil_parcel_get_available (parcel), ==, 0);
  g_assert_cmpint (garil_parcel_get_position (parcel), ==, data->expected.len);
  g_assert_cmpmem (byte_array->data, byte_array->len,
                   data->expected.data, data->expected.len);
  g_assert_false (garil_parcel_is_malformed (parcel));

  garil_parcel_unref (parcel);
  g_byte_array_unref (byte_array);
}

#define bytes_write_byte_array__malformed bytes_read__malformed

static void
test_write_byte_array__malformed (FixtureMalformed *fixture,
                                  gconstpointer     user_data G_GNUC_UNUSED)
{
  static const guint8 data[] = { 0x00 };

  GByteArray *array = g_byte_array_new ();
  g_byte_array_append (array, data, sizeof (data));

  garil_parcel_write_byte_array (fixture->parcel, array);

  g_byte_array_unref (array);

  check_malformed_fixture (fixture);
}

/********************* garil_parcel_write_byte_array_buf **********************/

typedef struct {
  Bytes input;
  Bytes expected;
} TestDataWriteByteArrayBuf;

#define DEFINE_EXPECTED(n, ...) \
  const guint8 testdata_write_byte_array_buf_ ## n ## _expected[] = { __VA_ARGS__ };
#define DEFINE_VALID(n, ...) \
  const guint8 testdata_write_byte_array_buf_ ## n ## _input[] = { __VA_ARGS__ }; \
  const TestDataWriteByteArrayBuf testdata_write_byte_array_buf_ ## n = { \
    .input = { \
      .data = testdata_write_byte_array_buf_ ## n ## _input, \
      .len = G_N_ELEMENTS (testdata_write_byte_array_buf_ ## n ## _input), \
    }, \
    .expected = { \
      .data = testdata_write_byte_array_buf_ ## n ## _expected, \
      .len = G_N_ELEMENTS (testdata_write_byte_array_buf_ ## n ## _expected), \
    }, \
  };

DEFINE_EXPECTED (1, 0x00, 0x00, 0x00, 0x00)
DEFINE_VALID (1)
DEFINE_EXPECTED (2, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00)
DEFINE_VALID (2, 0x01)
DEFINE_EXPECTED (3, 0x02, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00)
DEFINE_VALID (3, 0x01, 0x02)
DEFINE_EXPECTED (4, 0x03, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x00)
DEFINE_VALID (4, 0x01, 0x02, 0x03)
DEFINE_EXPECTED (5, 0x04, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04)
DEFINE_VALID (5, 0x01, 0x02, 0x03, 0x04)
DEFINE_EXPECTED (6, 0x05, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04,
                 0x05, 0x00, 0x00, 0x00)
DEFINE_VALID (6, 0x01, 0x02, 0x03, 0x04, 0x05)
DEFINE_EXPECTED (7, 0x06, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04,
                 0x05, 0x06, 0x00, 0x00)
DEFINE_VALID (7, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06)
DEFINE_EXPECTED (8, 0x07, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04,
                 0x05, 0x06, 0x07, 0x00)
DEFINE_VALID (8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07)
DEFINE_EXPECTED (9, 0x08, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04,
                 0x05, 0x06, 0x07, 0x08)
DEFINE_VALID (9, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08)

#undef DEFINE_EXPECTED
#undef DEFINE_VALID

static void
test_write_byte_array_buf__basic (gconstpointer user_data)
{
  const TestDataWriteByteArrayBuf *data = user_data;

  GByteArray *byte_array = g_byte_array_new ();
  GarilParcel *parcel = garil_parcel_new (byte_array);

  garil_parcel_write_byte_array_buf (parcel, data->input.data, data->input.len);

  g_assert_cmpint (garil_parcel_get_size (parcel), ==, data->expected.len);
  g_assert_cmpint (garil_parcel_get_available (parcel), ==, 0);
  g_assert_cmpint (garil_parcel_get_position (parcel), ==, data->expected.len);
  g_assert_cmpmem (byte_array->data, byte_array->len,
                   data->expected.data, data->expected.len);
  g_assert_false (garil_parcel_is_malformed (parcel));

  garil_parcel_unref (parcel);
  g_byte_array_unref (byte_array);
}

#define bytes_write_byte_array_buf__malformed bytes_read__malformed

static void
test_write_byte_array_buf__malformed (FixtureMalformed *fixture,
                                      gconstpointer     user_data G_GNUC_UNUSED)
{
  static const guint8 data[] = { 0x00 };

  garil_parcel_write_byte_array_buf (fixture->parcel, data, sizeof (data));

  check_malformed_fixture (fixture);
}

/************************** garil_parcel_read_int32 ***************************/

typedef struct {
  Bytes input;
  gboolean malformed;
  gint32 expected;
} TestDataReadInt32;

#define DEFINE_DATA(n, m, e, ...) \
  const guint8 testdata_read_int32_ ## n ## _input[] = { __VA_ARGS__ }; \
  const TestDataReadInt32 testdata_read_int32_ ## n = { \
    .input = { \
      .data = testdata_read_int32_ ## n ## _input, \
      .len = G_N_ELEMENTS (testdata_read_int32_ ## n ## _input), \
    }, \
    .malformed = m, \
    .expected = e, \
  };

#define DEFINE_VALID(n, e, ...) DEFINE_DATA (n, FALSE, e, __VA_ARGS__)
#define DEFINE_INVALID(n, ...)  DEFINE_DATA (n, TRUE, 0, __VA_ARGS__)

DEFINE_VALID (1, 0x00, 0x00, 0x00, 0x00, 0x00)
DEFINE_VALID (2, 0x01, 0x01, 0x00, 0x00, 0x00)
DEFINE_VALID (3, 0x0100, 0x00, 0x01, 0x00, 0x00)
DEFINE_VALID (4, 0x010000, 0x00, 0x00, 0x01, 0x00)
DEFINE_VALID (5, 0x01000000, 0x00, 0x00, 0x00, 0x01)
DEFINE_VALID (6, G_MAXINT32, 0xff, 0xff, 0xff, 0x7f)
DEFINE_VALID (7, -1, 0xff, 0xff, 0xff, 0xff)
DEFINE_VALID (8, -2, 0xfe, 0xff, 0xff, 0xff)
DEFINE_VALID (9, G_MININT32, 0x00, 0x00, 0x00, 0x80)
DEFINE_VALID (10, 0x01, 0x01, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff)

const TestDataReadInt32 testdata_read_int32_11 = {
  .input = EMPTY_BYTES,
  .malformed = TRUE,
  .expected = 0,
};

DEFINE_INVALID (12, 0x01)
DEFINE_INVALID (13, 0x01, 0x00)
DEFINE_INVALID (14, 0x01, 0x00, 0x00)

#undef DEFINE_DATA
#undef DEFINE_VALID
#undef DEFINE_INVALID

static void
test_read_int32__basic (gconstpointer user_data)
{
  const TestDataReadInt32 *data = user_data;

  GByteArray *byte_array =
    g_byte_array_new_take (g_memdup (data->input.data, data->input.len),
                           data->input.len);
  GarilParcel *parcel = garil_parcel_new (byte_array);

  gint32 value = garil_parcel_read_int32 (parcel);
  g_assert_cmpint (value, ==, data->expected);
  g_assert_cmpint (garil_parcel_get_size (parcel), ==, data->input.len);
  g_assert (garil_parcel_is_malformed (parcel) == data->malformed);
  if (data->malformed) {
    g_assert_cmpint (garil_parcel_get_available (parcel), ==, data->input.len);
    g_assert_cmpint (garil_parcel_get_position (parcel), ==, 0);
  } else {
    g_assert_cmpint (garil_parcel_get_available (parcel), ==,
                     data->input.len - sizeof (gint32));
    g_assert_cmpint (garil_parcel_get_position (parcel), ==, sizeof (gint32));
  }

  garil_parcel_unref (parcel);
  g_byte_array_unref (byte_array);
}

#define bytes_read_int32__malformed bytes_read__malformed

static void
test_read_int32__malformed (FixtureMalformed *fixture,
                            gconstpointer     user_data G_GNUC_UNUSED)
{
  gint32 value = garil_parcel_read_int32 (fixture->parcel);
  g_assert_cmpint (value, ==, 0);

  check_malformed_fixture (fixture);
}

/*********************** garil_parcel_read_int32_array ************************/

typedef struct {
  Bytes input;
  Bytes32 expected;
  gsize position;
} TestDataReadInt32Array;

#define DEFINE_EXPECTED(n, ...) \
  const gint32 testdata_read_int32_array_ ## n ## _expected[] = { __VA_ARGS__ };
#define DEFINE_VALID(n, ...) \
  const guint8 testdata_read_int32_array_ ## n ## _input[] = { __VA_ARGS__ }; \
  const TestDataReadInt32Array testdata_read_int32_array_ ## n = { \
    .input = { \
      .data = testdata_read_int32_array_ ## n ## _input, \
      .len = G_N_ELEMENTS (testdata_read_int32_array_ ## n ## _input), \
    }, \
    .expected = { \
      .data = testdata_read_int32_array_ ## n ## _expected, \
      .len = G_N_ELEMENTS (testdata_read_int32_array_ ## n ## _expected), \
    }, \
    .position = (sizeof (testdata_read_int32_array_ ## n ## _expected) + sizeof (gint32)), \
  };
#define DEFINE_INVALID(n, p, ...) \
  const guint8 testdata_read_int32_array_ ## n ## _input[] = { __VA_ARGS__ }; \
  const TestDataReadInt32Array testdata_read_int32_array_ ## n = { \
    .input = { \
      .data = testdata_read_int32_array_ ## n ## _input, \
      .len = G_N_ELEMENTS (testdata_read_int32_array_ ## n ## _input), \
    }, \
    .expected = EMPTY_BYTES, \
    .position = p, \
  };

DEFINE_EXPECTED (1)
DEFINE_VALID (1, 0x00, 0x00, 0x00, 0x00)
DEFINE_EXPECTED (2, 0x02)
DEFINE_VALID (2, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00)
DEFINE_EXPECTED (3, 0x02, 0x03)
DEFINE_VALID (3, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
              0x03, 0x00, 0x00, 0x00)
DEFINE_EXPECTED (4, 0x02)
DEFINE_VALID (4, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
              0x03, 0x00, 0x00, 0x00)

const TestDataReadInt32Array testdata_read_int32_array_5 = {
  .input = EMPTY_BYTES,
  .expected = EMPTY_BYTES,
  .position = 0,
};

DEFINE_INVALID (6, 0, 0x01)
DEFINE_INVALID (7, 0, 0x01, 0x00)
DEFINE_INVALID (8, 0, 0x01, 0x00, 0x00)
DEFINE_INVALID (9, 4, 0x01, 0x00, 0x00, 0x00)
DEFINE_INVALID (10, 4, 0x01, 0x00, 0x00, 0x00, 0x02)
DEFINE_INVALID (11, 4, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00)
DEFINE_INVALID (12, 4, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00)
DEFINE_INVALID (13, 4, 0xff, 0xff, 0xff, 0xff)
DEFINE_INVALID (14, 4, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00)

#undef DEFINE_EXPECTED
#undef DEFINE_VALID
#undef DEFINE_INVALID

static void
test_read_int32_array__basic (gconstpointer user_data)
{
  const TestDataReadInt32Array *data = user_data;

  GByteArray *byte_array =
    g_byte_array_new_take (g_memdup (data->input.data, data->input.len),
                           data->input.len);
  GarilParcel *parcel = garil_parcel_new (byte_array);

  GArray *array = garil_parcel_read_int32_array (parcel);
  g_assert_cmpint (garil_parcel_get_size (parcel), ==, data->input.len);
  g_assert_cmpint (garil_parcel_get_available (parcel), ==,
                   data->input.len - data->position);
  g_assert_cmpint (garil_parcel_get_position (parcel), ==, data->position);
  if (data->expected.data != NULL) {
    g_assert_nonnull (array);
    g_assert_cmpmem (array->data, array->len * sizeof (gint32),
                     data->expected.data, data->expected.len * sizeof (gint32));
    g_assert_false (garil_parcel_is_malformed (parcel));

    g_array_unref (array);
  } else {
    g_assert_null (array);
    g_assert_true (garil_parcel_is_malformed (parcel));
  }

  garil_parcel_unref (parcel);
  g_byte_array_unref (byte_array);
}

#define bytes_read_int32_array__malformed bytes_read__malformed

static void
test_read_int32_array__malformed (FixtureMalformed *fixture,
                                  gconstpointer     user_data G_GNUC_UNUSED)
{
  GArray *array = garil_parcel_read_int32_array (fixture->parcel);
  g_assert_null (array);

  check_malformed_fixture (fixture);
}

/************************** garil_parcel_write_int32 **************************/

typedef struct {
  gint32 input;
  Bytes expected;
} TestDataWriteInt32;

#define DEFINE_VALID(n, i, ...) \
  const guint8 testdata_write_int32_ ## n ## _expected[] = { __VA_ARGS__ }; \
  const TestDataWriteInt32 testdata_write_int32_ ## n = { \
    .input = i, \
    .expected = { \
      .data = testdata_write_int32_ ## n ## _expected, \
      .len = G_N_ELEMENTS (testdata_write_int32_ ## n ## _expected), \
    }, \
  };

DEFINE_VALID (1, 0, 0x00, 0x00, 0x00, 0x00);
DEFINE_VALID (2, 1, 0x01, 0x00, 0x00, 0x00);
DEFINE_VALID (3, G_MAXINT32, 0xff, 0xff, 0xff, 0x7f);
DEFINE_VALID (4, -1, 0xff, 0xff, 0xff, 0xff);
DEFINE_VALID (5, G_MININT32, 0x00, 0x00, 0x00, 0x80);

#undef DEFINE_VALID

static void
test_write_int32__basic (gconstpointer user_data)
{
  const TestDataWriteInt32 *data = user_data;

  GByteArray *byte_array = g_byte_array_new ();
  GarilParcel *parcel = garil_parcel_new (byte_array);

  garil_parcel_write_int32 (parcel, data->input);
  g_assert_cmpint (garil_parcel_get_size (parcel), ==, data->expected.len);
  g_assert_cmpint (garil_parcel_get_available (parcel), ==, 0);
  g_assert_cmpint (garil_parcel_get_position (parcel), ==, data->expected.len);
  g_assert_cmpmem (byte_array->data, byte_array->len,
                   data->expected.data, data->expected.len);
  g_assert_false (garil_parcel_is_malformed (parcel));

  garil_parcel_unref (parcel);
  g_byte_array_unref (byte_array);
}

#define bytes_write_int32__malformed bytes_read__malformed

static void
test_write_int32__malformed (FixtureMalformed *fixture,
                             gconstpointer     user_data G_GNUC_UNUSED)
{
  garil_parcel_write_int32 (fixture->parcel, 0);

  check_malformed_fixture (fixture);
}

/*********************** garil_parcel_write_int32_array ***********************/

typedef struct {
  Bytes32 input;
  Bytes expected;
} TestDataWriteInt32Array;

#define DEFINE_EXPECTED(n, ...) \
  const guint8 testdata_write_int32_array_ ## n ## _expected[] = { __VA_ARGS__ };
#define DEFINE_VALID(n, ...) \
  const guint32 testdata_write_int32_array_ ## n ## _input[] = { __VA_ARGS__ }; \
  const TestDataWriteInt32Array testdata_write_int32_array_ ## n = { \
    .input = { \
      .data = testdata_write_int32_array_ ## n ## _input, \
      .len = G_N_ELEMENTS (testdata_write_int32_array_ ## n ## _input), \
    }, \
    .expected = { \
      .data = testdata_write_int32_array_ ## n ## _expected, \
      .len = G_N_ELEMENTS (testdata_write_int32_array_ ## n ## _expected), \
    }, \
  };

DEFINE_EXPECTED (1, 0x00, 0x00, 0x00, 0x00)
const TestDataWriteInt32Array testdata_write_int32_array_1 = {
  .input = EMPTY_BYTES,
  .expected = {
    .data = testdata_write_int32_array_1_expected,
    .len = G_N_ELEMENTS (testdata_write_int32_array_1_expected),
  },
};

DEFINE_EXPECTED (2, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00)
DEFINE_VALID (2, 0x01)
DEFINE_EXPECTED (3, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                 0x02, 0x00, 0x00, 0x00)
DEFINE_VALID (3, 0x01, 0x02)

#undef DEFINE_EXPECTED
#undef DEFINE_VALID

static void
test_write_int32_array__basic (gconstpointer user_data)
{
  const TestDataWriteInt32Array *data = user_data;

  GByteArray *byte_array = g_byte_array_new ();
  GarilParcel *parcel = garil_parcel_new (byte_array);

  GArray *array = g_array_new (FALSE, FALSE, sizeof (gint32));
  if (data->input.data != NULL) {
    g_array_append_vals (array, data->input.data, data->input.len);
  }

  garil_parcel_write_int32_array (parcel, array);
  g_array_unref (array);

  g_assert_cmpint (garil_parcel_get_size (parcel), ==, data->expected.len);
  g_assert_cmpint (garil_parcel_get_available (parcel), ==, 0);
  g_assert_cmpint (garil_parcel_get_position (parcel), ==, data->expected.len);
  g_assert_cmpmem (byte_array->data, byte_array->len,
                   data->expected.data, data->expected.len);
  g_assert_false (garil_parcel_is_malformed (parcel));

  garil_parcel_unref (parcel);
  g_byte_array_unref (byte_array);
}

#define bytes_write_int32_array__malformed bytes_read__malformed

static void
test_write_int32_array__malformed (FixtureMalformed *fixture,
                                   gconstpointer     user_data G_GNUC_UNUSED)
{
  static const gint32 data[] = { 0x00 };

  GArray *array = g_array_new (FALSE, FALSE, sizeof (gint32));
  g_array_append_vals (array, data, G_N_ELEMENTS (data));

  garil_parcel_write_int32_array (fixture->parcel, array);

  g_array_unref (array);

  check_malformed_fixture (fixture);
}

/********************* garil_parcel_write_int32_array_buf *********************/

typedef struct {
  Bytes32 input;
  Bytes expected;
} TestDataWriteInt32ArrayBuf;

#define DEFINE_EXPECTED(n, ...) \
  const guint8 testdata_write_int32_array_buf_ ## n ## _expected[] = { __VA_ARGS__ };
#define DEFINE_VALID(n, ...) \
  const guint32 testdata_write_int32_array_buf_ ## n ## _input[] = { __VA_ARGS__ }; \
  const TestDataWriteInt32ArrayBuf testdata_write_int32_array_buf_ ## n = { \
    .input = { \
      .data = testdata_write_int32_array_buf_ ## n ## _input, \
      .len = G_N_ELEMENTS (testdata_write_int32_array_buf_ ## n ## _input), \
    }, \
    .expected = { \
      .data = testdata_write_int32_array_buf_ ## n ## _expected, \
      .len = G_N_ELEMENTS (testdata_write_int32_array_buf_ ## n ## _expected), \
    }, \
  };

DEFINE_EXPECTED (1, 0x00, 0x00, 0x00, 0x00)
DEFINE_VALID (1)
DEFINE_EXPECTED (2, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00)
DEFINE_VALID(2, 0x01)
DEFINE_EXPECTED (3, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                 0x02, 0x00, 0x00, 0x00)
DEFINE_VALID(3, 0x01, 0x02)

#undef DEFINE_EXPECTED
#undef DEFINE_VALID

static void
test_write_int32_array_buf__basic (gconstpointer user_data)
{
  const TestDataWriteInt32ArrayBuf *data = user_data;

  GByteArray *byte_array = g_byte_array_new ();
  GarilParcel *parcel = garil_parcel_new (byte_array);

  garil_parcel_write_int32_array_buf (parcel, data->input.data, data->input.len);

  g_assert_cmpint (garil_parcel_get_size (parcel), ==, data->expected.len);
  g_assert_cmpint (garil_parcel_get_available (parcel), ==, 0);
  g_assert_cmpint (garil_parcel_get_position (parcel), ==, data->expected.len);
  g_assert_cmpmem (byte_array->data, byte_array->len,
                   data->expected.data, data->expected.len);
  g_assert_false (garil_parcel_is_malformed (parcel));

  garil_parcel_unref (parcel);
  g_byte_array_unref (byte_array);
}

#define bytes_write_int32_array_buf__malformed bytes_read__malformed

static void
test_write_int32_array_buf__malformed (FixtureMalformed *fixture,
                                       gconstpointer     user_data G_GNUC_UNUSED)
{
  static const gint32 data[] = { 0x00 };

  garil_parcel_write_int32_array_buf (fixture->parcel,
                                      data, G_N_ELEMENTS (data));

  check_malformed_fixture (fixture);
}

/************************* garil_parcel_read_string16 *************************/

typedef struct {
  Bytes input;
  const gchar *expected;
  gsize position;
  gboolean malformed;
} TestDataReadString16;

#define DEFINE_VALID(n, e, p, ...) \
  const guint8 testdata_read_string16_ ## n ## _input[] = { __VA_ARGS__ }; \
  const TestDataReadString16 testdata_read_string16_ ## n = { \
    .input = { \
      .data = testdata_read_string16_ ## n ## _input, \
      .len = G_N_ELEMENTS (testdata_read_string16_ ## n ## _input), \
    }, \
    .expected = e, \
    .position = p, \
    .malformed = FALSE, \
  };
#define DEFINE_INVALID(n, p, ...) \
  const guint8 testdata_read_string16_ ## n ## _input[] = { __VA_ARGS__ }; \
  const TestDataReadString16 testdata_read_string16_ ## n = { \
    .input = { \
      .data = testdata_read_string16_ ## n ## _input, \
      .len = G_N_ELEMENTS (testdata_read_string16_ ## n ## _input), \
    }, \
    .expected = NULL, \
    .position = p, \
    .malformed = TRUE, \
  };

DEFINE_VALID (1, NULL, 4,
              0xff, 0xff, 0xff, 0xff)
DEFINE_VALID (2, "", 8,
              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
DEFINE_VALID (3, "a", 8,
              0x01, 0x00, 0x00, 0x00, 0x61, 0x00, 0x00, 0x00)
DEFINE_VALID (4, "ab", 12,
              0x02, 0x00, 0x00, 0x00, 0x61, 0x00, 0x62, 0x00,
              0x00, 0x00, 0x00, 0x00)
/* Full width Latin small latter a */
DEFINE_VALID (5, "\xef\xbd\x81", 8,
              0x01, 0x00, 0x00, 0x00, 0x41, 0xff, 0x00, 0x00)
/* Full width Latin small latter a and b */
DEFINE_VALID (6, "\xef\xbd\x81\xef\xbd\x82", 12,
              0x02, 0x00, 0x00, 0x00, 0x41, 0xff, 0x42, 0xff,
              0x00, 0x00, 0x00, 0x00)
DEFINE_INVALID (7, 0)
DEFINE_INVALID (8, 0, 0x01)
DEFINE_INVALID (9, 0, 0x01, 0x00)
DEFINE_INVALID (10, 0, 0x01, 0x00, 0x00)
DEFINE_INVALID (11, 4, 0x01, 0x00, 0x00, 0x00)
DEFINE_INVALID (12, 4, 0x01, 0x00, 0x00, 0x00, 0x61)
DEFINE_INVALID (13, 4, 0x01, 0x00, 0x00, 0x00, 0x61, 0x00)
DEFINE_INVALID (14, 4, 0x01, 0x00, 0x00, 0x00, 0x61, 0x00, 0x00)
DEFINE_VALID (15, "", 8,
              0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
/* U+D800 - U+DFFF: UTF-16 surrogates */
DEFINE_INVALID (16, 8, 0x01, 0x00, 0x00, 0x00, 0x00, 0xd8, 0x00, 0x00)

#undef DEFINE_VALID
#undef DEFINE_INVALID

static void
test_read_string16__basic (gconstpointer user_data)
{
  const TestDataReadString16 *data = user_data;

  GByteArray *byte_array = g_byte_array_new ();
  if (data->input.data != NULL)
    g_byte_array_append (byte_array, data->input.data, data->input.len);

  GarilParcel *parcel = garil_parcel_new (byte_array);

  gchar *result = garil_parcel_read_string16 (parcel);
  g_assert_cmpint (garil_parcel_get_size (parcel), ==, data->input.len);
  g_assert_cmpint (garil_parcel_get_position (parcel), ==, data->position);
  g_assert_cmpint (garil_parcel_get_available (parcel), ==,
                   data->input.len - data->position);
  g_assert (garil_parcel_is_malformed (parcel) == data->malformed);
  if (data->expected != NULL) {
    g_assert_nonnull (result);
    g_assert_cmpstr (result, ==, data->expected);
    g_free (result);
  } else
    g_assert_null (result);

  garil_parcel_unref (parcel);
  g_byte_array_unref (byte_array);
}

#define bytes_read_string16__malformed bytes_read__malformed

static void
test_read_string16__malformed (FixtureMalformed *fixture,
                               gconstpointer     user_data G_GNUC_UNUSED)
{
  gchar *p = garil_parcel_read_string16 (fixture->parcel);
  g_assert_null (p);

  check_malformed_fixture (fixture);
}

/********************** garil_parcel_read_string16_array **********************/

typedef struct {
  Bytes input;
  struct {
    const gchar **data;
    gsize len;
  } expected;
  gsize position;
  gboolean malformed;
} TestDataReadString16Array;

#define DEFINE_EXPECTED(n, ...) \
  const gchar *testdata_read_string16_array_ ## n ## _expected[] = { __VA_ARGS__ };
#define DEFINE_VALID(n, p, ...) \
  const guint8 testdata_read_string16_array_ ## n ## _input[] = { __VA_ARGS__ }; \
  const TestDataReadString16Array testdata_read_string16_array_ ## n = { \
    .input = { \
      .data = testdata_read_string16_array_ ## n ## _input, \
      .len = G_N_ELEMENTS (testdata_read_string16_array_ ## n ## _input), \
    }, \
    .expected = { \
      .data = testdata_read_string16_array_ ## n ## _expected, \
      .len = G_N_ELEMENTS (testdata_read_string16_array_ ## n ## _expected), \
    }, \
    .position = p, \
    .malformed = FALSE, \
  };
#define DEFINE_INVALID(n, p, ...) \
  const guint8 testdata_read_string16_array_ ## n ## _input[] = { __VA_ARGS__ }; \
  const TestDataReadString16Array testdata_read_string16_array_ ## n = { \
    .input = { \
      .data = testdata_read_string16_array_ ## n ## _input, \
      .len = G_N_ELEMENTS (testdata_read_string16_array_ ## n ## _input), \
    }, \
    .expected = EMPTY_BYTES, \
    .position = p, \
    .malformed = TRUE, \
  };

const TestDataReadString16Array testdata_read_string16_array_1 = {
  .input = EMPTY_BYTES,
  .expected = EMPTY_BYTES,
  .position = 0,
  .malformed = TRUE,
};

const guint8 testdata_read_string16_array_2_input[] = {
  0x00, 0x00, 0x00, 0x00
};
const TestDataReadString16Array testdata_read_string16_array_2 = {
  .input = {
    .data = testdata_read_string16_array_2_input,
    .len = G_N_ELEMENTS (testdata_read_string16_array_2_input),
  },
  .expected = EMPTY_BYTES,
  .position = 4,
  .malformed = FALSE,
};

DEFINE_EXPECTED (3, NULL)
DEFINE_VALID (3, 8,
              0x01, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff)
DEFINE_EXPECTED (4, NULL, NULL)
DEFINE_VALID (4, 12,
              0x02, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
              0xff, 0xff, 0xff, 0xff)
DEFINE_EXPECTED (5, NULL, "")
DEFINE_VALID (5, 16,
              0x02, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
DEFINE_EXPECTED (6, NULL, "a")
DEFINE_VALID (6, 16,
              0x02, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
              0x01, 0x00, 0x00, 0x00, 0x61, 0x00, 0x00, 0x00)
DEFINE_EXPECTED (7, NULL, "ab")
DEFINE_VALID (7, 20,
              0x02, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
              0x02, 0x00, 0x00, 0x00, 0x61, 0x00, 0x62, 0x00,
              0x00, 0x00, 0x00, 0x00)
DEFINE_EXPECTED (8, "")
DEFINE_VALID (8, 12,
              0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00)
DEFINE_EXPECTED (9, "", NULL)
DEFINE_VALID (9, 16,
              0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff)
DEFINE_EXPECTED (10, "", "")
DEFINE_VALID (10, 20,
              0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00)
DEFINE_EXPECTED (11, "", "a")
DEFINE_VALID (11, 20,
              0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
              0x61, 0x00, 0x00, 0x00)
DEFINE_EXPECTED (12, "", "ab")
DEFINE_VALID (12, 24,
              0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
              0x61, 0x00, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00)
DEFINE_EXPECTED (13, "a")
DEFINE_VALID (13, 12,
              0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
              0x61, 0x00, 0x00, 0x00)
DEFINE_EXPECTED (14, "a", NULL)
DEFINE_VALID (14, 16,
              0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
              0x61, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff)
DEFINE_EXPECTED (15, "a", "")
DEFINE_VALID (15, 20,
              0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
              0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00)
DEFINE_EXPECTED (16, "a", "a")
DEFINE_VALID (16, 20,
              0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
              0x61, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
              0x61, 0x00, 0x00, 0x00)
DEFINE_EXPECTED (17, "a", "ab")
DEFINE_VALID (17, 24,
              0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
              0x61, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
              0x61, 0x00, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00)
DEFINE_EXPECTED (18, "ab")
DEFINE_VALID (18, 16,
              0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
              0x61, 0x00, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00)
DEFINE_EXPECTED (19, "ab", NULL)
DEFINE_VALID (19, 20,
              0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
              0x61, 0x00, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00,
              0xff, 0xff, 0xff, 0xff)
DEFINE_EXPECTED (20, "ab", "")
DEFINE_VALID (20, 24,
              0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
              0x61, 0x00, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
DEFINE_EXPECTED (21, "ab", "a")
DEFINE_VALID (21, 24,
              0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
              0x61, 0x00, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x01, 0x00, 0x00, 0x00, 0x61, 0x00, 0x00, 0x00)
DEFINE_EXPECTED (22, "ab", "ab")
DEFINE_VALID (22, 28,
              0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
              0x61, 0x00, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00,
              0x02, 0x00, 0x00, 0x00, 0x61, 0x00, 0x62, 0x00,
              0x00, 0x00, 0x00, 0x00)

DEFINE_INVALID (23, 0, 0x01)
DEFINE_INVALID (24, 0, 0x01, 0x00)
DEFINE_INVALID (25, 0, 0x01, 0x00, 0x00)
DEFINE_INVALID (26, 4, 0x01, 0x00, 0x00, 0x00)
DEFINE_INVALID (27, 4, 0x01, 0x00, 0x00, 0x00, 0x01)
DEFINE_INVALID (28, 4, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00)
DEFINE_INVALID (29, 4, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00)
DEFINE_INVALID (30, 8, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00)
DEFINE_INVALID (31, 8, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x61)
DEFINE_INVALID (32, 8, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x61, 0x00)
DEFINE_INVALID (33, 8, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x61, 0x00, 0x00)
DEFINE_INVALID (34, 12, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x61, 0x00, 0x00, 0x00)
DEFINE_INVALID (35, 12, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x61, 0x00, 0x00, 0x00, 0x01)
DEFINE_INVALID (36, 12, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x61, 0x00, 0x00, 0x00, 0x01, 0x00)
DEFINE_INVALID (37, 12, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x61, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00)
DEFINE_INVALID (38, 16, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x61, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00)
DEFINE_INVALID (39, 16, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x61, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x61)
DEFINE_INVALID (40, 16, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x61, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x61, 0x00)
DEFINE_INVALID (41, 16, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x61, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x61, 0x00, 0x00)

#undef DEFINE_EXPECTED
#undef DEFINE_VALID
#undef DEFINE_INVALID

static void
test_read_string16_array__basic (gconstpointer user_data)
{
  const TestDataReadString16Array *data = user_data;

  GByteArray *byte_array = g_byte_array_new ();
  if (data->input.data != NULL)
    g_byte_array_append (byte_array, data->input.data, data->input.len);

  GarilParcel *parcel = garil_parcel_new (byte_array);

  const gsize mark = 0xfe;
  gsize len = mark;
  gchar **p = garil_parcel_read_string16_array (parcel, &len);
  g_assert_cmpint (garil_parcel_get_size (parcel), ==, data->input.len);
  g_assert_cmpint (garil_parcel_get_position (parcel), ==, data->position);
  g_assert_cmpint (garil_parcel_get_available (parcel), ==,
                   data->input.len - data->position);
  g_assert (garil_parcel_is_malformed (parcel) == data->malformed);
  if (data->malformed) {
    g_assert_null (p);
    g_assert_cmpint (len, ==, mark);
  } else {
    g_assert_cmpint (len, ==, data->expected.len);
    if (!len)
      g_assert_null (p);
    else {
      g_assert_nonnull (p);
      for (gsize i = 0; i < len; i++) {
        if (data->expected.data[i] == NULL)
          g_assert_null (p[i]);
        else {
          g_assert_nonnull (p[i]);
          g_assert_cmpstr (p[i], ==, data->expected.data[i]);

          g_free (p[i]);
        }
      }

      g_free (p);
    }
  }

  garil_parcel_unref (parcel);
  g_byte_array_unref (byte_array);
}

static const guint8 bytes_read_string16_array__malformed_data[] = {
  0x00, 0x00, 0x00, 0x00
};

static const Bytes bytes_read_string16_array__malformed = {
  .data = bytes_read_string16_array__malformed_data,
  .len = sizeof (bytes_read_string16_array__malformed_data),
};

static void
test_read_string16_array__malformed (FixtureMalformed *fixture,
                                     gconstpointer     user_data G_GNUC_UNUSED)
{
  const gsize mark = 0xfe;
  gsize len = mark;
  gchar **p = garil_parcel_read_string16_array (fixture->parcel, &len);
  g_assert_null (p);
  g_assert_cmpint (len, ==, mark);

  check_malformed_fixture (fixture);
}

/************************ garil_parcel_write_string16 *************************/

typedef struct {
  const gchar *input;
  Bytes expected;
} TestDataWriteString16;

#define DEFINE_VALID(n, i, ...) \
  const guint8 testdata_write_string16_ ## n ## _expected[] = { __VA_ARGS__ }; \
  const TestDataWriteString16 testdata_write_string16_ ## n = { \
    .input = i, \
    .expected = { \
      .data = testdata_write_string16_ ## n ## _expected, \
      .len = G_N_ELEMENTS (testdata_write_string16_ ## n ## _expected), \
    }, \
  };

DEFINE_VALID (1, NULL, 0xff, 0xff, 0xff, 0xff)
DEFINE_VALID (2, "", 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
DEFINE_VALID (3, "a", 0x01, 0x00, 0x00, 0x00, 0x61, 0x00, 0x00, 0x00)
DEFINE_VALID (4, "ab", 0x02, 0x00, 0x00, 0x00, 0x61, 0x00, 0x62, 0x00,
              0x00, 0x00, 0x00, 0x00)
DEFINE_VALID (5, "\xef\xbd\x81", 0x01, 0x00, 0x00, 0x00, 0x41, 0xff, 0x00, 0x00)
DEFINE_VALID (6, "\xef\xbd\x81\xef\xbd\x82", 0x02, 0x00, 0x00, 0x00,
              0x41, 0xff, 0x42, 0xff, 0x00, 0x00, 0x00, 0x00)

#undef DEFINE_VALID

static void
test_write_string16__basic (gconstpointer user_data)
{
  const TestDataWriteString16 *data = user_data;

  GByteArray *byte_array = g_byte_array_new ();
  GarilParcel *parcel = garil_parcel_new (byte_array);

  garil_parcel_write_string16 (parcel, data->input);
  g_assert_cmpint (garil_parcel_get_size (parcel), ==, data->expected.len);
  g_assert_cmpint (garil_parcel_get_position (parcel), ==, data->expected.len);
  g_assert_cmpint (garil_parcel_get_available (parcel), ==, 0);
  g_assert_false (garil_parcel_is_malformed (parcel));
  g_assert_cmpmem (byte_array->data, byte_array->len,
                   data->expected.data, data->expected.len);

  garil_parcel_unref (parcel);
  g_byte_array_unref (byte_array);
}

#define bytes_write_string16__malformed bytes_read__malformed

static void
test_write_string16__malformed (FixtureMalformed *fixture,
                                gconstpointer     user_data G_GNUC_UNUSED)
{
  garil_parcel_write_string16 (fixture->parcel, NULL);

  check_malformed_fixture (fixture);
}

/********************* garil_parcel_write_string16_array **********************/

typedef struct {
  struct {
    const gchar **data;
    gsize len;
  } input;
  Bytes expected;
} TestDataWriteString16Array;

#define DEFINE_EXPECTED(n, ...) \
  const guint8 testdata_write_string16_array_ ## n ## _expected[] = { __VA_ARGS__ };
#define DEFINE_VALID(n, ...) \
  const gchar *testdata_write_string16_array_ ## n ## _input[] = { __VA_ARGS__ }; \
  const TestDataWriteString16Array testdata_write_string16_array_ ## n = { \
    .input = { \
      .data = testdata_write_string16_array_ ## n ## _input, \
      .len = G_N_ELEMENTS (testdata_write_string16_array_ ## n ## _input), \
    }, \
    .expected = { \
      .data = testdata_write_string16_array_ ## n ## _expected, \
      .len = G_N_ELEMENTS (testdata_write_string16_array_ ## n ## _expected), \
    }, \
  };

DEFINE_EXPECTED (1, 0x00, 0x00, 0x00, 0x00)
const TestDataWriteString16Array testdata_write_string16_array_1 = {
  .input = EMPTY_BYTES,
  .expected = {
    .data = testdata_write_string16_array_1_expected,
    .len = G_N_ELEMENTS (testdata_write_string16_array_1_expected),
  },
};

DEFINE_EXPECTED (2, 0x01, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff)
DEFINE_VALID (2, NULL)
DEFINE_EXPECTED (3, 0x02, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
                 0xff, 0xff, 0xff, 0xff)
DEFINE_VALID (3, NULL, NULL)
DEFINE_EXPECTED (4, 0x02, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
DEFINE_VALID (4, NULL, "")
DEFINE_EXPECTED (5, 0x02, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
                 0x01, 0x00, 0x00, 0x00, 0x61, 0x00, 0x00, 0x00)
DEFINE_VALID (5, NULL, "a")
DEFINE_EXPECTED (6, 0x02, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
                 0x02, 0x00, 0x00, 0x00, 0x61, 0x00, 0x62, 0x00,
                 0x00, 0x00, 0x00, 0x00)
DEFINE_VALID (6, NULL, "ab")
DEFINE_EXPECTED (7, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                 0x00, 0x00, 0x00, 0x00)
DEFINE_VALID (7, "")
DEFINE_EXPECTED (8, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff)
DEFINE_VALID (8, "", NULL)
DEFINE_EXPECTED (9, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                 0x00, 0x00, 0x00, 0x00)
DEFINE_VALID (9, "", "")
DEFINE_EXPECTED (10, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                 0x61, 0x00, 0x00, 0x00)
DEFINE_VALID (10, "", "a")
DEFINE_EXPECTED (11, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                 0x61, 0x00, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00)
DEFINE_VALID (11, "", "ab")
DEFINE_EXPECTED (12, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                 0x61, 0x00, 0x00, 0x00)
DEFINE_VALID (12, "a")
DEFINE_EXPECTED (13, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                 0x61, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff)
DEFINE_VALID (13, "a", NULL)
DEFINE_EXPECTED (14, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                 0x00, 0x00, 0x00, 0x00)
DEFINE_VALID (14, "a", "")
DEFINE_EXPECTED (15, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                 0x61, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                 0x61, 0x00, 0x00, 0x00)
DEFINE_VALID (15, "a", "a")
DEFINE_EXPECTED (16, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                 0x61, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                 0x61, 0x00, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00)
DEFINE_VALID (16, "a", "ab")
DEFINE_EXPECTED (17, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                 0x61, 0x00, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00)
DEFINE_VALID (17, "ab")
DEFINE_EXPECTED (18, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                 0x61, 0x00, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00,
                 0xff, 0xff, 0xff, 0xff)
DEFINE_VALID (18, "ab", NULL)
DEFINE_EXPECTED (19, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                 0x61, 0x00, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00,
                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
DEFINE_VALID (19, "ab", "")
DEFINE_EXPECTED (20, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                 0x61, 0x00, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00,
                 0x01, 0x00, 0x00, 0x00, 0x61, 0x00, 0x00, 0x00)
DEFINE_VALID (20, "ab", "a")
DEFINE_EXPECTED (21, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                 0x61, 0x00, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00,
                 0x02, 0x00, 0x00, 0x00, 0x61, 0x00, 0x62, 0x00,
                 0x00, 0x00, 0x00, 0x00)
DEFINE_VALID (21, "ab", "ab")

#undef DEFINE_INPUT
#undef DEFINE_VALID

static void
test_write_string16_array__basic (gconstpointer user_data)
{
  const TestDataWriteString16Array *data = user_data;

  GByteArray *byte_array = g_byte_array_new ();
  GarilParcel *parcel = garil_parcel_new (byte_array);

  garil_parcel_write_string16_array (parcel, data->input.data, data->input.len);
  g_assert_cmpint (garil_parcel_get_size (parcel), ==, data->expected.len);
  g_assert_cmpint (garil_parcel_get_position (parcel), ==, data->expected.len);
  g_assert_cmpint (garil_parcel_get_available (parcel), ==, 0);
  g_assert_false (garil_parcel_is_malformed (parcel));
  g_assert_cmpmem (byte_array->data, byte_array->len,
                   data->expected.data, data->expected.len);

  garil_parcel_unref (parcel);
  g_byte_array_unref (byte_array);
}

#define bytes_write_string16_array__malformed bytes_read__malformed

static void
test_write_string16_array__malformed (FixtureMalformed *fixture,
                                      gconstpointer     user_data G_GNUC_UNUSED)
{
  garil_parcel_write_string16_array (fixture->parcel, NULL, 0);

  check_malformed_fixture (fixture);
}

/************************************ main ************************************/

static void
fixture_setup_malformed (FixtureMalformed *fixture,
                         gconstpointer     user_data G_GNUC_UNUSED)
{
  g_assert_nonnull (fixture);

  const Bytes *data = user_data;
  fixture->input.data = data->data;
  fixture->input.len = data->len;

  GByteArray *byte_array = g_byte_array_new ();
  g_byte_array_append (byte_array, fixture->input.data, fixture->input.len);

  fixture->parcel = garil_parcel_new (byte_array);
  g_byte_array_unref (byte_array);

  gconstpointer p =
    garil_parcel_read_inplace (fixture->parcel, fixture->input.len + 1);
  g_assert_null (p);
  g_assert_true (garil_parcel_is_malformed (fixture->parcel));
}

static void
fixture_teardown_malformed (FixtureMalformed *fixture,
                            gconstpointer     user_data G_GNUC_UNUSED)
{
  garil_parcel_unref (fixture->parcel);
}

int
main (int   argc,
      char *argv[])
{
  setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);
  g_test_bug_base (PACKAGE_BUGREPORT);

  g_test_add_data_func ("/GarilParcel/garil_parcel_new/1",
                        NULL, test_new_valid__basic);
  g_test_add_data_func_full ("/GarilParcel/garil_parcel_new/2",
                             g_byte_array_new (),
                             test_new_valid__basic,
                             (GDestroyNotify) g_byte_array_unref);
  g_test_add_data_func_full ("/GarilParcel/garil_parcel_new/3",
                             g_byte_array_sized_new (128),
                             test_new_valid__basic,
                             (GDestroyNotify) g_byte_array_unref);

#define ADD_FUNC(name, n, sub) \
  g_test_add_func ("/GarilParcel/garil_parcel_" #name "/" #n, \
                   test_ ## name ## __ ## sub);

#define ADD_DATA_FUNC(name, n, sub) \
  g_test_add_data_func ("/GarilParcel/garil_parcel_" #name "/" #n, \
                        &testdata_ ## name ## _ ## n, \
                        test_ ## name ## __ ## sub);

#define ADD_MALFORMED(name, n) \
  g_test_add ("/GarilParcel/garil_parcel_" #name "/" #n, \
              FixtureMalformed, \
              &bytes_ ## name ## __malformed, \
              fixture_setup_malformed, \
              test_ ## name ## __malformed, \
              fixture_teardown_malformed);

  ADD_FUNC (read, 1, basic)
  ADD_MALFORMED (read, 2)

  ADD_FUNC (read_dup, 1, basic)
  ADD_MALFORMED (read_dup, 2)

  ADD_FUNC (read_inplace, 1, basic)
  ADD_MALFORMED (read_inplace, 2)

  ADD_FUNC (write, 1, basic)
  ADD_MALFORMED (write, 2)

  ADD_FUNC (write_inplace, 1, basic)
  ADD_MALFORMED (write_inplace, 2)

  ADD_DATA_FUNC (read_byte, 1, basic)
  ADD_DATA_FUNC (read_byte, 2, basic)
  ADD_DATA_FUNC (read_byte, 3, basic)
  ADD_DATA_FUNC (read_byte, 4, basic)
  ADD_DATA_FUNC (read_byte, 5, basic)
  ADD_DATA_FUNC (read_byte, 6, basic)
  ADD_DATA_FUNC (read_byte, 7, basic)
  ADD_DATA_FUNC (read_byte, 8, basic)
  ADD_DATA_FUNC (read_byte, 9, basic)
  ADD_DATA_FUNC (read_byte, 10, basic)
  ADD_MALFORMED (read_byte, 11)

  ADD_DATA_FUNC (read_byte_array, 1, basic)
  ADD_DATA_FUNC (read_byte_array, 2, basic)
  ADD_DATA_FUNC (read_byte_array, 3, basic)
  ADD_DATA_FUNC (read_byte_array, 4, basic)
  ADD_DATA_FUNC (read_byte_array, 5, basic)
  ADD_DATA_FUNC (read_byte_array, 6, basic)
  ADD_DATA_FUNC (read_byte_array, 7, basic)
  ADD_DATA_FUNC (read_byte_array, 8, basic)
  ADD_DATA_FUNC (read_byte_array, 9, basic)
  ADD_DATA_FUNC (read_byte_array, 10, basic)
  ADD_DATA_FUNC (read_byte_array, 11, basic)
  ADD_DATA_FUNC (read_byte_array, 12, basic)
  ADD_DATA_FUNC (read_byte_array, 13, basic)
  ADD_DATA_FUNC (read_byte_array, 14, basic)
  ADD_DATA_FUNC (read_byte_array, 15, basic)
  ADD_DATA_FUNC (read_byte_array, 16, basic)
  ADD_DATA_FUNC (read_byte_array, 17, basic)
  ADD_DATA_FUNC (read_byte_array, 18, basic)
  ADD_DATA_FUNC (read_byte_array, 19, basic)
  ADD_DATA_FUNC (read_byte_array, 20, basic)
  ADD_DATA_FUNC (read_byte_array, 21, basic)
  ADD_DATA_FUNC (read_byte_array, 22, basic)
  ADD_DATA_FUNC (read_byte_array, 23, basic)
  ADD_MALFORMED (read_byte_array, 24)

  ADD_DATA_FUNC (write_byte, 1, basic)
  ADD_DATA_FUNC (write_byte, 2, basic)
  ADD_DATA_FUNC (write_byte, 3, basic)
  ADD_MALFORMED (write_byte, 4)

  ADD_DATA_FUNC (write_byte_array, 1, basic)
  ADD_DATA_FUNC (write_byte_array, 2, basic)
  ADD_DATA_FUNC (write_byte_array, 3, basic)
  ADD_DATA_FUNC (write_byte_array, 4, basic)
  ADD_DATA_FUNC (write_byte_array, 5, basic)
  ADD_DATA_FUNC (write_byte_array, 6, basic)
  ADD_DATA_FUNC (write_byte_array, 7, basic)
  ADD_DATA_FUNC (write_byte_array, 8, basic)
  ADD_DATA_FUNC (write_byte_array, 9, basic)
  ADD_MALFORMED (write_byte_array, 10)

  ADD_DATA_FUNC (write_byte_array_buf, 1, basic)
  ADD_DATA_FUNC (write_byte_array_buf, 2, basic)
  ADD_DATA_FUNC (write_byte_array_buf, 3, basic)
  ADD_DATA_FUNC (write_byte_array_buf, 4, basic)
  ADD_DATA_FUNC (write_byte_array_buf, 5, basic)
  ADD_DATA_FUNC (write_byte_array_buf, 6, basic)
  ADD_DATA_FUNC (write_byte_array_buf, 7, basic)
  ADD_DATA_FUNC (write_byte_array_buf, 8, basic)
  ADD_DATA_FUNC (write_byte_array_buf, 9, basic)
  ADD_MALFORMED (write_byte_array_buf, 10)

  ADD_DATA_FUNC (read_int32, 1, basic)
  ADD_DATA_FUNC (read_int32, 2, basic)
  ADD_DATA_FUNC (read_int32, 3, basic)
  ADD_DATA_FUNC (read_int32, 4, basic)
  ADD_DATA_FUNC (read_int32, 5, basic)
  ADD_DATA_FUNC (read_int32, 6, basic)
  ADD_DATA_FUNC (read_int32, 7, basic)
  ADD_DATA_FUNC (read_int32, 8, basic)
  ADD_DATA_FUNC (read_int32, 9, basic)
  ADD_DATA_FUNC (read_int32, 10, basic)
  ADD_DATA_FUNC (read_int32, 11, basic)
  ADD_DATA_FUNC (read_int32, 12, basic)
  ADD_DATA_FUNC (read_int32, 13, basic)
  ADD_DATA_FUNC (read_int32, 14, basic)
  ADD_MALFORMED (read_int32, 15)

  ADD_DATA_FUNC (read_int32_array, 1, basic)
  ADD_DATA_FUNC (read_int32_array, 2, basic)
  ADD_DATA_FUNC (read_int32_array, 3, basic)
  ADD_DATA_FUNC (read_int32_array, 4, basic)
  ADD_DATA_FUNC (read_int32_array, 5, basic)
  ADD_DATA_FUNC (read_int32_array, 6, basic)
  ADD_DATA_FUNC (read_int32_array, 7, basic)
  ADD_DATA_FUNC (read_int32_array, 8, basic)
  ADD_DATA_FUNC (read_int32_array, 9, basic)
  ADD_DATA_FUNC (read_int32_array, 10, basic)
  ADD_DATA_FUNC (read_int32_array, 11, basic)
  ADD_DATA_FUNC (read_int32_array, 12, basic)
  ADD_DATA_FUNC (read_int32_array, 13, basic)
  ADD_DATA_FUNC (read_int32_array, 14, basic)
  ADD_MALFORMED (read_int32_array, 15)

  ADD_DATA_FUNC (write_int32, 1, basic)
  ADD_DATA_FUNC (write_int32, 2, basic)
  ADD_DATA_FUNC (write_int32, 3, basic)
  ADD_DATA_FUNC (write_int32, 4, basic)
  ADD_DATA_FUNC (write_int32, 5, basic)
  ADD_MALFORMED (write_int32, 6)

  ADD_DATA_FUNC (write_int32_array, 1, basic)
  ADD_DATA_FUNC (write_int32_array, 2, basic)
  ADD_DATA_FUNC (write_int32_array, 3, basic)
  ADD_MALFORMED (write_int32_array, 4)

  ADD_DATA_FUNC (write_int32_array_buf, 1, basic)
  ADD_DATA_FUNC (write_int32_array_buf, 2, basic)
  ADD_DATA_FUNC (write_int32_array_buf, 3, basic)
  ADD_MALFORMED (write_int32_array_buf, 4)

  ADD_DATA_FUNC (read_string16, 1, basic)
  ADD_DATA_FUNC (read_string16, 2, basic)
  ADD_DATA_FUNC (read_string16, 3, basic)
  ADD_DATA_FUNC (read_string16, 4, basic)
  ADD_DATA_FUNC (read_string16, 5, basic)
  ADD_DATA_FUNC (read_string16, 6, basic)
  ADD_DATA_FUNC (read_string16, 7, basic)
  ADD_DATA_FUNC (read_string16, 8, basic)
  ADD_DATA_FUNC (read_string16, 9, basic)
  ADD_DATA_FUNC (read_string16, 10, basic)
  ADD_DATA_FUNC (read_string16, 11, basic)
  ADD_DATA_FUNC (read_string16, 12, basic)
  ADD_DATA_FUNC (read_string16, 13, basic)
  ADD_DATA_FUNC (read_string16, 14, basic)
  ADD_DATA_FUNC (read_string16, 15, basic)
  ADD_DATA_FUNC (read_string16, 16, basic)
  ADD_MALFORMED (read_string16, 17)

  ADD_DATA_FUNC (read_string16_array, 1, basic)
  ADD_DATA_FUNC (read_string16_array, 2, basic)
  ADD_DATA_FUNC (read_string16_array, 3, basic)
  ADD_DATA_FUNC (read_string16_array, 4, basic)
  ADD_DATA_FUNC (read_string16_array, 5, basic)
  ADD_DATA_FUNC (read_string16_array, 6, basic)
  ADD_DATA_FUNC (read_string16_array, 7, basic)
  ADD_DATA_FUNC (read_string16_array, 8, basic)
  ADD_DATA_FUNC (read_string16_array, 9, basic)
  ADD_DATA_FUNC (read_string16_array, 10, basic)
  ADD_DATA_FUNC (read_string16_array, 11, basic)
  ADD_DATA_FUNC (read_string16_array, 12, basic)
  ADD_DATA_FUNC (read_string16_array, 13, basic)
  ADD_DATA_FUNC (read_string16_array, 14, basic)
  ADD_DATA_FUNC (read_string16_array, 15, basic)
  ADD_DATA_FUNC (read_string16_array, 16, basic)
  ADD_DATA_FUNC (read_string16_array, 17, basic)
  ADD_DATA_FUNC (read_string16_array, 18, basic)
  ADD_DATA_FUNC (read_string16_array, 19, basic)
  ADD_DATA_FUNC (read_string16_array, 20, basic)
  ADD_DATA_FUNC (read_string16_array, 21, basic)
  ADD_DATA_FUNC (read_string16_array, 22, basic)
  ADD_DATA_FUNC (read_string16_array, 23, basic)
  ADD_DATA_FUNC (read_string16_array, 24, basic)
  ADD_DATA_FUNC (read_string16_array, 25, basic)
  ADD_DATA_FUNC (read_string16_array, 26, basic)
  ADD_DATA_FUNC (read_string16_array, 27, basic)
  ADD_DATA_FUNC (read_string16_array, 28, basic)
  ADD_DATA_FUNC (read_string16_array, 29, basic)
  ADD_DATA_FUNC (read_string16_array, 30, basic)
  ADD_DATA_FUNC (read_string16_array, 31, basic)
  ADD_DATA_FUNC (read_string16_array, 32, basic)
  ADD_DATA_FUNC (read_string16_array, 33, basic)
  ADD_DATA_FUNC (read_string16_array, 34, basic)
  ADD_DATA_FUNC (read_string16_array, 35, basic)
  ADD_DATA_FUNC (read_string16_array, 36, basic)
  ADD_DATA_FUNC (read_string16_array, 37, basic)
  ADD_DATA_FUNC (read_string16_array, 38, basic)
  ADD_DATA_FUNC (read_string16_array, 39, basic)
  ADD_DATA_FUNC (read_string16_array, 40, basic)
  ADD_DATA_FUNC (read_string16_array, 41, basic)
  ADD_MALFORMED (read_string16_array, 42)

  ADD_DATA_FUNC (write_string16, 1, basic)
  ADD_DATA_FUNC (write_string16, 2, basic)
  ADD_DATA_FUNC (write_string16, 3, basic)
  ADD_DATA_FUNC (write_string16, 4, basic)
  ADD_DATA_FUNC (write_string16, 5, basic)
  ADD_DATA_FUNC (write_string16, 6, basic)
  ADD_MALFORMED (write_string16, 7)

  ADD_DATA_FUNC (write_string16_array, 1, basic)
  ADD_DATA_FUNC (write_string16_array, 2, basic)
  ADD_DATA_FUNC (write_string16_array, 3, basic)
  ADD_DATA_FUNC (write_string16_array, 4, basic)
  ADD_DATA_FUNC (write_string16_array, 5, basic)
  ADD_DATA_FUNC (write_string16_array, 6, basic)
  ADD_DATA_FUNC (write_string16_array, 7, basic)
  ADD_DATA_FUNC (write_string16_array, 8, basic)
  ADD_DATA_FUNC (write_string16_array, 9, basic)
  ADD_DATA_FUNC (write_string16_array, 10, basic)
  ADD_DATA_FUNC (write_string16_array, 11, basic)
  ADD_DATA_FUNC (write_string16_array, 12, basic)
  ADD_DATA_FUNC (write_string16_array, 13, basic)
  ADD_DATA_FUNC (write_string16_array, 14, basic)
  ADD_DATA_FUNC (write_string16_array, 15, basic)
  ADD_DATA_FUNC (write_string16_array, 16, basic)
  ADD_DATA_FUNC (write_string16_array, 17, basic)
  ADD_DATA_FUNC (write_string16_array, 18, basic)
  ADD_DATA_FUNC (write_string16_array, 19, basic)
  ADD_DATA_FUNC (write_string16_array, 20, basic)
  ADD_DATA_FUNC (write_string16_array, 21, basic)
  ADD_MALFORMED (write_string16_array, 22)

  return g_test_run ();
}
