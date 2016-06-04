#ifndef NOJS_UTILS_H
#define NOJS_UTILS_H

#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>

#define DISALLOW_COPY_AND_ASSIGN(TypeName)                                    \
  void operator=(const TypeName&) = delete;                                   \
  void operator=(TypeName&&) = delete;                                        \
  TypeName(const TypeName&) = delete;                                         \
  TypeName(TypeName&&) = delete

// Windows 8+ does not like abort() in Release mode
#ifdef _WIN32
#define ABORT() raise(SIGABRT)
#else
#define ABORT() abort()
#endif

#if defined(NDEBUG)
# define ASSERT(expression)
# define CHECK(expression)                                                    \
  do {                                                                        \
    if (!(expression)) ABORT();                                               \
  } while (0)
#else
# define ASSERT(expression)  assert(expression)
# define CHECK(expression)   assert(expression)
#endif

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_GE(a, b) ASSERT((a) >= (b))
#define ASSERT_GT(a, b) ASSERT((a) > (b))
#define ASSERT_LE(a, b) ASSERT((a) <= (b))
#define ASSERT_LT(a, b) ASSERT((a) < (b))
#define ASSERT_NE(a, b) ASSERT((a) != (b))

#define CHECK_EQ(a, b) CHECK((a) == (b))
#define CHECK_GE(a, b) CHECK((a) >= (b))
#define CHECK_GT(a, b) CHECK((a) > (b))
#define CHECK_LE(a, b) CHECK((a) <= (b))
#define CHECK_LT(a, b) CHECK((a) < (b))
#define CHECK_NE(a, b) CHECK((a) != (b))

#define UNREACHABLE() ABORT()

#endif
