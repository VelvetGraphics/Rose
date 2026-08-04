#pragma once
#include <cstdint>

using U8 = uint8_t;
using U16 = uint16_t;
using U32 = uint32_t;
using U64 = uint64_t;

constexpr U8 U8Limit = UINT8_MAX;
constexpr U16 U16Limit = UINT16_MAX;
constexpr U32 U32Limit = UINT32_MAX;
constexpr U64 U64Limit = UINT64_MAX;

using S8 = int8_t;
using S16 = int16_t;
using S32 = int32_t;
using S64 = int64_t;

constexpr S8 S8Limit = INT8_MAX;
constexpr S16 S16Limit = INT16_MAX;
constexpr S32 S32Limit = INT32_MAX;
constexpr S64 S64Limit = INT64_MAX;
