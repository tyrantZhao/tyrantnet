#ifndef __TYRANTNET_NET_NOEXCEPT_H__
#define __TYRANTNET_NET_NOEXCEPT_H__

#include <tyrant/common/cppversion.h>

#ifdef HAVE_LANG_CXX17
#define TYRANT_NOEXCEPT noexcept
#else
#define TYRANT_NOEXCEPT
#endif

#endif //__TYRANTNET_NET_NOEXCEPT_H__
