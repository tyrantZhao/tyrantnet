#ifndef __NET_NOEXCEPT_H__
#define __NET_NOEXCEPT_H__

#include <tyrant/common/cppversion.h>

#ifdef HAVE_LANG_CXX17
#define TYRANT_NOEXCEPT noexcept
#else
#define TYRANT_NOEXCEPT
#endif

#endif //noexcept.h
