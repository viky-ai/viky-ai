/**
 *  Header for define generic type
 *
 *  Copyright (c) 2014 Pertimm by Brice Ruzand
 *  Dev : April 2014
 *  Version 1.0
 */
#ifndef _LPCGENTYPE_ALIVE_H_
#define _LPCGENTYPE_ALIVE_H_

// Deprecated warning
#if    __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)
#define OG_DEPRECATED_FOR(f) __attribute__((__deprecated__("Use '" #f "' instead")))
#elif    __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
#define OG_DEPRECATED_FOR(f) __attribute__((__deprecated__))
#else
#define OG_DEPRECATED_FOR(f)
#endif

/**
 * Function status type.
 *
 * Values: 0 (ok), -1 (error).
 *
 * @code
 * og_status status = function("Say hello !!");
 * if(status == -1) throw ERROR;
 * ...
 * @endcode
 *
 * @ingroup generic
 */
typedef int og_status;

/**
 * Bitfield type.
 *
 * @code
 * og_bitfield flags = 0x100 + 0x10 + 0x1;
 * @endcode
 *
 * @ingroup generic
 */
typedef unsigned og_bitfield;

/**
 * 64 bits Bitfield type (use for matrices).
 *
 * @code
 * og_bitfield_long flags = 0x100 + 0x10 + 0x1;
 * @endcode
 *
 * @ingroup generic
 */
typedef unsigned long long og_bitfield_long;

/**
 * Number of bits (not bytes !!!) that can be used in og_bitfield_long (at least 64 bits)
 */
#define DOgBitfieldLongBitSize (sizeof(og_bitfield_long) * 8)

/**
 * Boolean type, with additional error status.
 *
 * Values: 1 (true), 0 (false), -1 (error).
 *
 * @code
 * og_bool is_valid = TRUE;
 * if(is_valid) ...
 * @endcode
 *
 * @ingroup generic
 */
typedef int og_bool;

/**
 * Immutable String type.
 * @warning Instance of og_ssil_string type are <b>read only</b>.
 *
 * @code
 * og_string text = "Hello World";
 * @endcode
 *
 * @struct og_string
 * @ingroup generic
 */
typedef unsigned char const *og_string;

/**
 * Destination string buffer type.
 * Instance of this type are read/write to use it as a buffer.
 *
 * @code
 * og_char_buffer buffer[10];
 * @endcode
 *
 * @struct og_char_buffer
 * @ingroup generic
 */
typedef unsigned char og_char_buffer;

#endif /* _LPCGENTYPE_ALIVE_H_ */
