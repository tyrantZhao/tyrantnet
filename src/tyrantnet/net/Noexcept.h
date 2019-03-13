#pragma once

#include <tyrantnet/common/CPP_VERSION.h>

#ifdef HAVE_LANG_CXX17
#define TYRANTNET_NOEXCEPT noexcept
#else
#define TYRANTNET_NOEXCEPT
#endif
