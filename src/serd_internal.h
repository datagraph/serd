/*
  Copyright 2011 David Robillard <http://drobilla.net>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
  AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
  THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SERD_INTERNAL_H
#define SERD_INTERNAL_H

#include <assert.h>
#include <stdlib.h>

#include "serd/serd.h"

/** A dynamic stack in memory. */
typedef struct {
	uint8_t* buf;       ///< Stack memory
	size_t   buf_size;  ///< Allocated size of buf (>= size)
	size_t   size;      ///< Conceptual size of stack in buf
} SerdStack;

/** An offset to start the stack at. Note 0 is reserved for NULL. */
#define SERD_STACK_BOTTOM sizeof(void*)

static inline SerdStack
serd_stack_new(size_t size)
{
	SerdStack stack;
	stack.buf       = malloc(size);
	stack.buf_size  = size;
	stack.size      = SERD_STACK_BOTTOM;
	return stack;
}

static inline bool
serd_stack_is_empty(SerdStack* stack)
{
	return stack->size <= SERD_STACK_BOTTOM;
}

static inline void
serd_stack_free(SerdStack* stack)
{
	free(stack->buf);
	stack->buf      = NULL;
	stack->buf_size = 0;
	stack->size     = 0;
}

static inline uint8_t*
serd_stack_push(SerdStack* stack, size_t n_bytes)
{
	const size_t new_size = stack->size + n_bytes;
	if (stack->buf_size < new_size) {
		stack->buf_size *= 2;
		stack->buf = realloc(stack->buf, stack->buf_size);
	}
	uint8_t* const ret = (stack->buf + stack->size);
	stack->size = new_size;
	return ret;
}

static inline void
serd_stack_pop(SerdStack* stack, size_t n_bytes)
{
	assert(stack->size >= n_bytes);
	stack->size -= n_bytes;
}

/** Return true if @a c lies within [min...max] (inclusive) */
static inline bool
in_range(const uint8_t c, const uint8_t min, const uint8_t max)
{
	return (c >= min && c <= max);
}

/** RFC2234: ALPHA := %x41-5A / %x61-7A  ; A-Z / a-z */
static inline bool
is_alpha(const uint8_t c)
{
	return in_range(c, 'A', 'Z') || in_range(c, 'a', 'z');
}

/** RFC2234: DIGIT ::= %x30-39  ; 0-9 */
static inline bool
is_digit(const uint8_t c)
{
	return in_range(c, '0', '9');
}

/** UTF-8 strlen.
 * @return Lengh of @a utf8 in characters.
 * @param utf8 A null-terminated UTF-8 string.
 * @param out_n_bytes (Output) Set to the size of @a utf8 in bytes.
 */
static inline size_t
serd_strlen(const uint8_t* utf8, size_t* out_n_bytes)
{
	size_t n_chars = 0;
	size_t i       = 0;
	for (; utf8[i]; ++i) {
		if ((utf8[i] & 0xC0) != 0x80) {
			// Does not start with `10', start of a new character
			++n_chars;
		}
	}
	if (out_n_bytes) {
		*out_n_bytes = i + 1;
	}
	return n_chars;
}

#endif  // SERD_INTERNAL_H
