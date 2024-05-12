#pragma once

#include "defines.h"

#include <format>
#include <iostream>
#include <limits>
#include <new>
#include <variant>

namespace Lox
{
	// Ugly forward declares because we can't include "common.h" or "value.h" as they include "memory.h" already...
	class Object;
	using f64 = double;
	using Value = std::variant<bool, nullptr_t, f64, Object*>;

	void mark_object(Object* object);
	void mark_value(Value value);
	void collect_garbage();
	void free_objects();

	inline std::size_t total_heap_bytes = 0;

	// We'll use types like Lox::Vec and Lox::String instead of std::vector and std::string
	// to track our total memory usage in order to decide when to run the Lox garbage collector.
	// Those types will use instances of this allocator, which tracks total memory usage by
	// reading/writing to total_heap_bytes.
	//
	// This is mostly taken from the example at https://en.cppreference.com/w/cpp/named_req/Allocator
	template<class T>
	struct TrackingAllocator
	{
		typedef T value_type;

		TrackingAllocator() = default;

		template<class U>
		constexpr TrackingAllocator(const TrackingAllocator<U>&) noexcept
		{
		}

		[[nodiscard]] T* allocate(std::size_t n)
		{
			if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
			{
				throw std::bad_array_new_length();
			}

#if DEBUG_STRESS_GC
			collect_garbage();
#endif

			if (auto p = static_cast<T*>(std::malloc(n * sizeof(T))))
			{
				total_heap_bytes += n * sizeof(T);
#if DEBUG_LOG_GC
				report(p, n);
#endif
				return p;
			}

			throw std::bad_alloc();
		}

		void deallocate(T* p, std::size_t n) noexcept
		{
			total_heap_bytes -= n * sizeof(T);
#if DEBUG_LOG_GC
			report(p, n, 0);
#endif
			std::free(p);
		}

	private:
		void report(T* p, std::size_t n, bool alloc = true) const
		{
			std::cout << (alloc ? "Alloc: " : "Dealloc: ") << sizeof(T) * n << " bytes at " << std::hex << std::showbase << reinterpret_cast<void*>(p)
					  << std::dec << " for '" << std::format("{:.64}", typeid(T).name()) << "' (total: " << total_heap_bytes << " bytes)\n";
		}
	};
}	 // namespace Lox
