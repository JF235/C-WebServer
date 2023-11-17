
/**
 * `decode.c' - b64
 *
 * copyright (c) 2014 joseph werle
 * 
 * https://github.com/jwerle/b64.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "../includes/b64.h"

#ifdef b64_USE_CUSTOM_MALLOC
extern void* b64_malloc(size_t);
#endif

#ifdef b64_USE_CUSTOM_REALLOC
extern void* b64_realloc(void*, size_t);
#endif

char *
b64_decode (const char *src, size_t len) {
  return b64_decode_ex(src, len, NULL);
}

char *
b64_decode_ex (const char *src, size_t len, size_t *decsize) {
  int i = 0;
  int j = 0;
  int l = 0;
  size_t size = 0;
  b64_buffer_t decbuf;
  char buf[3];
  char tmp[4];

  // alloc
  if (b64_buf_malloc(&decbuf) == -1) { return NULL; }

  // parse until end of source
  while (len--) {
    // break if char is `=' or not base64 char
    if ('=' == src[j]) { break; }
    if (!(isalnum(src[j]) || '+' == src[j] || '/' == src[j])) { break; }

    // read up to 4 bytes at a time into `tmp'
    tmp[i++] = src[j++];

    // if 4 bytes read then decode into `buf'
    if (4 == i) {
      // translate values in `tmp' from table
      for (i = 0; i < 4; ++i) {
        // find translation char in `b64_table'
        for (l = 0; l < 64; ++l) {
          if (tmp[i] == b64_table[l]) {
            tmp[i] = l;
            break;
          }
        }
      }

      // decode
      buf[0] = (tmp[0] << 2) + ((tmp[1] & 0x30) >> 4);
      buf[1] = ((tmp[1] & 0xf) << 4) + ((tmp[2] & 0x3c) >> 2);
      buf[2] = ((tmp[2] & 0x3) << 6) + tmp[3];

      // write decoded buffer to `decbuf.ptr'
      if (b64_buf_realloc(&decbuf, size + 3) == -1)  return NULL;
      for (i = 0; i < 3; ++i) {
        ((char*)decbuf.ptr)[size++] = buf[i];
      }

      // reset
      i = 0;
    }
  }

  // remainder
  if (i > 0) {
    // fill `tmp' with `\0' at most 4 times
    for (j = i; j < 4; ++j) {
      tmp[j] = '\0';
    }

    // translate remainder
    for (j = 0; j < 4; ++j) {
        // find translation char in `b64_table'
        for (l = 0; l < 64; ++l) {
          if (tmp[j] == b64_table[l]) {
            tmp[j] = l;
            break;
          }
        }
    }

    // decode remainder
    buf[0] = (tmp[0] << 2) + ((tmp[1] & 0x30) >> 4);
    buf[1] = ((tmp[1] & 0xf) << 4) + ((tmp[2] & 0x3c) >> 2);
    buf[2] = ((tmp[2] & 0x3) << 6) + tmp[3];

    // write remainer decoded buffer to `decbuf.ptr'
    if (b64_buf_realloc(&decbuf, size + (i - 1)) == -1)  return NULL;
    for (j = 0; (j < i - 1); ++j) {
      ((char*)decbuf.ptr)[size++] = buf[j];
    }
  }

  // Make sure we have enough space to add '\0' character at end.
  if (b64_buf_realloc(&decbuf, size + 1) == -1)  return NULL;
  ((char*)decbuf.ptr)[size] = '\0';

  // Return back the size of decoded string if demanded.
  if (decsize != NULL) {
    *decsize = size;
  }

  return (char*) decbuf.ptr;
}

int b64_buf_malloc(b64_buffer_t * buf)
{
	buf->ptr = b64_malloc(B64_BUFFER_SIZE);
	if(!buf->ptr) return -1;

	buf->bufc = 1;

	return 0;
}

int b64_buf_realloc(b64_buffer_t* buf, size_t size)
{
	if (size > buf->bufc * B64_BUFFER_SIZE)
	{
		while (size > buf->bufc * B64_BUFFER_SIZE) buf->bufc++;
		buf->ptr = b64_realloc(buf->ptr, B64_BUFFER_SIZE * buf->bufc);
		if (!buf->ptr) return -1;
	}

	return 0;
}



char *
b64_encode (const char *src, size_t len) {
  int i = 0;
  int j = 0;
  b64_buffer_t encbuf;
  size_t size = 0;
  int buf[4];
  int tmp[3];

  // alloc
  if(b64_buf_malloc(&encbuf) == -1) { return NULL; }

  // parse until end of source
  while (len--) {
    // read up to 3 bytes at a time into `tmp'
    tmp[i++] = *(src++);

    // if 3 bytes read then encode into `buf'
    if (3 == i) {
      buf[0] = (tmp[0] & 0xfc) >> 2;
      buf[1] = ((tmp[0] & 0x03) << 4) + ((tmp[1] & 0xf0) >> 4);
      buf[2] = ((tmp[1] & 0x0f) << 2) + ((tmp[2] & 0xc0) >> 6);
      buf[3] = tmp[2] & 0x3f;

      // allocate 4 new byts for `enc` and
      // then translate each encoded buffer
      // part by index from the base 64 index table
      // into `encbuf.ptr' char array
      if (b64_buf_realloc(&encbuf, size + 4) == -1) return NULL;

      for (i = 0; i < 4; ++i) {
        encbuf.ptr[size++] = b64_table[buf[i]];
      }

      // reset index
      i = 0;
    }
  }

  // remainder
  if (i > 0) {
    // fill `tmp' with `\0' at most 3 times
    for (j = i; j < 3; ++j) {
      tmp[j] = '\0';
    }

    // perform same codec as above
    buf[0] = (tmp[0] & 0xfc) >> 2;
    buf[1] = ((tmp[0] & 0x03) << 4) + ((tmp[1] & 0xf0) >> 4);
    buf[2] = ((tmp[1] & 0x0f) << 2) + ((tmp[2] & 0xc0) >> 6);
    buf[3] = tmp[2] & 0x3f;

    // perform same write to `encbuf->ptr` with new allocation
    for (j = 0; (j < i + 1); ++j) {
      if (b64_buf_realloc(&encbuf, size + 1) == -1) return NULL;

      encbuf.ptr[size++] = b64_table[buf[j]];
    }

    // while there is still a remainder
    // append `=' to `encbuf.ptr'
    while ((i++ < 3)) {
      if (b64_buf_realloc(&encbuf, size + 1) == -1) return NULL;

      encbuf.ptr[size++] = '=';
    }
  }

  // Make sure we have enough space to add '\0' character at end.
  if (b64_buf_realloc(&encbuf, size + 1) == -1) return NULL;
  encbuf.ptr[size] = '\0';

  return encbuf.ptr;
}