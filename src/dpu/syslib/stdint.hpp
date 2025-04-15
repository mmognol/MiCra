/* Copyright 2020 UPMEM. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
*/

#ifndef _DPUSYSCORE_STDINT_H_
#define _DPUSYSCORE_STDINT_H_
 
/**
 * @file stdint.h
 * @brief Provides abstraction over machine types.
*/
 
/* Exact integer types */
 
/* Signed */
 
using int8_t = signed char;
using int16_t = short int;
using int32_t = int;
using int64_t = long long int;

/* Unsigned */

using uint8_t = unsigned char;
using uint16_t = unsigned short int;
using uint32_t = unsigned int;
using uint64_t = unsigned long int;

/* Small types */
 
/* Signed */
 
using int_least8_t = signed char;
using int_least16_t = short int;
using int_least32_t = int;
using int_least64_t = long int;

/* Unsigned */

using uint_least8_t = unsigned char;
using uint_least16_t = unsigned short int;
using uint_least32_t = unsigned int;
using uint_least64_t = unsigned long int;

/* Fast types */

/* Signed */

using int_fast8_t = signed char;
using int_fast16_t = int;
using int_fast32_t = int;
using int_fast64_t = long int;

/* Unsigned */

using uint_fast8_t = unsigned char;
using uint_fast16_t = unsigned int;
using uint_fast32_t = unsigned int;
using uint_fast64_t = unsigned long int;

/* Types for void* pointers */

/**
 * @brief A signed value which can contain a pointer value.
 */
using intptr_t = int;
 /**
 * @brief An unsigned value which can contain a pointer value.
 */
using uintptr_t = unsigned int;

/* Greatest-width integer types */

/**
 * @brief A signed value which can contain all signed values.
 */
using intmax_t = long long int;
/**
 * @brief An unsigned value which can contain all unsigned values.
 */
using uintmax_t = unsigned long long int;

#include "limits.hpp"

#endif /* _DPUSYSCORE_STDINT_H_ */
