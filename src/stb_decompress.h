#pragma once

#include "prelude.h"

// from stb.h - public domain
// see: https://github.com/nothings/stb/blob/master/deprecated/stb.h#L10388

using stb_uchar = unsigned char;
using stb_uint = unsigned int;

thread_local static unsigned char *stb__barrier;
thread_local static unsigned char *stb__barrier2;
thread_local static unsigned char *stb__barrier3;
thread_local static unsigned char *stb__barrier4;

static stb_uchar *stb__dout;
static void stb__match(stb_uchar *data, stb_uint length) {
  // INVERSE of memmove... write each byte before copying the next...
  assert(stb__dout + length <= stb__barrier);
  if (stb__dout + length > stb__barrier) {
    stb__dout += length;
    return;
  }
  if (data < stb__barrier4) {
    stb__dout = stb__barrier + 1;
    return;
  }
  while (length--)
    *stb__dout++ = *data++;
}

static void stb__lit(stb_uchar *data, stb_uint length) {
  assert(stb__dout + length <= stb__barrier);
  if (stb__dout + length > stb__barrier) {
    stb__dout += length;
    return;
  }
  if (data < stb__barrier2) {
    stb__dout = stb__barrier + 1;
    return;
  }
  memcpy(stb__dout, data, length);
  stb__dout += length;
}

#define stb__in2(x) ((i[x] << 8) + i[(x) + 1])
#define stb__in3(x) ((i[x] << 16) + stb__in2((x) + 1))
#define stb__in4(x) ((i[x] << 24) + stb__in3((x) + 1))

static stb_uchar *stb_decompress_token(stb_uchar *i) {
  if (*i >= 0x20) { // use fewer if's for cases that expand small
    if (*i >= 0x80)
      stb__match(stb__dout - i[1] - 1, i[0] - 0x80 + 1), i += 2;
    else if (*i >= 0x40)
      stb__match(stb__dout - (stb__in2(0) - 0x4000 + 1), i[2] + 1), i += 3;
    else /* *i >= 0x20 */
      stb__lit(i + 1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
  } else { // more ifs for cases that expand large, since overhead is amortized
    if (*i >= 0x18)
      stb__match(stb__dout - (stb__in3(0) - 0x180000 + 1), i[3] + 1), i += 4;
    else if (*i >= 0x10)
      stb__match(stb__dout - (stb__in3(0) - 0x100000 + 1), stb__in2(3) + 1),
          i += 5;
    else if (*i >= 0x08)
      stb__lit(i + 2, stb__in2(0) - 0x0800 + 1),
          i += 2 + (stb__in2(0) - 0x0800 + 1);
    else if (*i == 0x07)
      stb__lit(i + 3, stb__in2(1) + 1), i += 3 + (stb__in2(1) + 1);
    else if (*i == 0x06)
      stb__match(stb__dout - (stb__in3(1) + 1), i[4] + 1), i += 5;
    else if (*i == 0x04)
      stb__match(stb__dout - (stb__in3(1) + 1), stb__in2(4) + 1), i += 6;
  }
  return i;
}

static stb_uint stb_decompress_length(stb_uchar *input) {
  return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}

static stb_uint stb_adler32(stb_uint adler32, stb_uchar *buffer,
                            stb_uint buflen) {
  const unsigned long ADLER_MOD = 65521;
  unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
  unsigned long blocklen, i;

  blocklen = buflen % 5552;
  while (buflen) {
    for (i = 0; i + 7 < blocklen; i += 8) {
      s1 += buffer[0], s2 += s1;
      s1 += buffer[1], s2 += s1;
      s1 += buffer[2], s2 += s1;
      s1 += buffer[3], s2 += s1;
      s1 += buffer[4], s2 += s1;
      s1 += buffer[5], s2 += s1;
      s1 += buffer[6], s2 += s1;
      s1 += buffer[7], s2 += s1;

      buffer += 8;
    }

    for (; i < blocklen; ++i)
      s1 += *buffer++, s2 += s1;

    s1 %= ADLER_MOD, s2 %= ADLER_MOD;
    buflen -= blocklen;
    blocklen = 5552;
  }
  return (s2 << 16) + s1;
}

static stb_uint stb_decompress(stb_uchar *output, stb_uchar *i,
                               stb_uint length) {
  stb_uint olen;
  if (stb__in4(0) != 0x57bC0000)
    return 0;
  if (stb__in4(4) != 0)
    return 0; // error! stream is > 4GB
  olen = stb_decompress_length(i);
  stb__barrier2 = i;
  stb__barrier3 = i + length;
  stb__barrier = output + olen;
  stb__barrier4 = output;
  i += 16;

  stb__dout = output;
  while (1) {
    stb_uchar *old_i = i;
    i = stb_decompress_token(i);
    if (i == old_i) {
      if (*i == 0x05 && i[1] == 0xfa) {
        assert(stb__dout == output + olen);
        if (stb__dout != output + olen)
          return 0;
        if (stb_adler32(1, output, olen) != (stb_uint)stb__in4(2))
          return 0;
        return olen;
      } else {
        assert(0); /* NOTREACHED */
        return 0;
      }
    }
    assert(stb__dout <= output + olen);
    if (stb__dout > output + olen)
      return 0;
  }
}

// end stb.h

inline String stb_decompress_data(const unsigned int *data,
                                  const unsigned int size) {
  u32 len = stb_decompress_length((u8 *)data);
  char *buf = (char *)mem_alloc(len + 1);
  stb_decompress((u8 *)buf, (u8 *)data, size);

  buf[len] = '\0';

  String contents;
  contents.data = buf;
  contents.len = len;
  return contents;
}