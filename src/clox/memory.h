#pragma once

#include <iostream>
#include <limits>
#include <new>

namespace Lox
{
	inline std::size_t total_heap_bytes = 0;

	// Since we use std types like Lox::Vec and Lox::String, we'll use this
	// allocator to track our total memory usage in order to decide when to run
	// the Lox garbage collector.
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
				throw std::bad_array_new_length();

			if (auto p = static_cast<T*>(std::malloc(n * sizeof(T))))
			{
				total_heap_bytes += n * sizeof(T);
				report(p, n);
				return p;
			}

			throw std::bad_alloc();
		}

		void deallocate(T* p, std::size_t n) noexcept
		{
			total_heap_bytes -= n * sizeof(T);
			report(p, n, 0);
			std::free(p);
		}

	private:
		void report(T* p, std::size_t n, bool alloc = true) const
		{
			std::cout << (alloc ? "Alloc: " : "Dealloc: ") << sizeof(T) * n << " bytes at " << std::hex << std::showbase << reinterpret_cast<void*>(p)
					  << std::dec << " total: " << total_heap_bytes << '\n';
		}
	};

	void collect_garbage();
	void free_objects();
}	 // namespace Lox
