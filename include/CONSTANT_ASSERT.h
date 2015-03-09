#pragma once
#define CONSTANT_ASSERT(x) static_assert(decltype(x){}, #x)
