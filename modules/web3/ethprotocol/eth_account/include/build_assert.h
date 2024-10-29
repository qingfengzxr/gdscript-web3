//
// Created by wyatt on 24-9-14.
//

#ifndef BUILD_ASSERT_H
#define BUILD_ASSERT_H
/* CC0 (Public domain) - see LICENSE file for details */

/**
 * BUILD_ASSERT - assert a build-time dependency.
 * @cond: the compile-time condition which must be true.
 *
 * Your compile will fail if the condition isn't true, or can't be evaluated
 * by the compiler.  This can only be used within a function.
 *
 * Example:
 *	#include <stddef.h>
 *	...
 *	static char *foo_to_char(struct foo *foo)
 *	{
 *		// This code needs string to be at start of foo.
 *		BUILD_ASSERT(offsetof(struct foo, string) == 0);
 *		return (char *)foo;
 *	}
 */
#define BUILD_ASSERT(cond)                   \
	do {                                     \
		(void)sizeof(char[1 - 2 * !(cond)]); \
	} while (0)

/**
 * BUILD_ASSERT_OR_ZERO - assert a build-time dependency, as an expression.
 * @cond: the compile-time condition which must be true.
 *
 * Your compile will fail if the condition isn't true, or can't be evaluated
 * by the compiler.  This can be used in an expression: its value is "0".
 *
 * Example:
 *	#define foo_to_char(foo)					\
 *		 ((char *)(foo)						\
 *		  + BUILD_ASSERT_OR_ZERO(offsetof(struct foo, string) == 0))
 */
#define BUILD_ASSERT_OR_ZERO(cond) \
	(sizeof(char[1 - 2 * !(cond)]) - 1)

#ifdef _WIN32
#define ETHC_EXPORT __declspec(dllexport)
#elif __GNUC__
#define ETHC_EXPORT __attribute__((visibility("default")))
#else
#define ETHC_EXPORT
#endif

#endif //BUILD_ASSERT_H
