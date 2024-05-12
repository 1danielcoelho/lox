#pragma once

#include "memory.h"

#include <stddef.h>
#include <stdint.h>
#include <climits>
#include <string>
#include <unordered_map>
#include <vector>

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

namespace Lox
{
	template<typename T>
	using Vec = std::vector<T, Lox::TrackingAllocator<T>>;

	using String = std::basic_string<char, std::char_traits<char>, Lox::TrackingAllocator<char>>;

	template<typename TKey, typename TVal>
	using Map = std::unordered_map<	   //
		TKey,
		TVal,
		std::hash<TKey>,
		std::equal_to<TKey>,
		Lox::TrackingAllocator<std::pair<const TKey, TVal>>>;
}
