#pragma once

#include <utility>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <type_traits>

namespace hrs
{
	struct union_size_align
	{
		size_t size;
		size_t align;
	};

	template<typename T, typename ...Args>
	consteval auto common_size_align() -> union_size_align
	{
		size_t max_size = std::max({sizeof(T), sizeof(Args)...});
		size_t max_align = std::max({alignof(T), alignof(Args)...});

		if(max_size > max_align)
		{
			if(max_size % max_align != 0)
				max_size += (max_size % max_align);
				//max_size = (max_size / max_align) * max_align + max_align;
		}


		return {max_size, max_align};
	}

	template<typename E>
	struct unexpected
	{
		E error;
		auto operator=(const E &err) -> E &
		{
			error = err;
		}
	};

	template<typename T, typename E>
		//requires std::is_destructible_v<T> && std::is_destructible_v<E>
	class expected
	{
	private:
		alignas(common_size_align<T, E>().align) uint8_t union_block[common_size_align<T, E>().size];
		bool is_error;
	public:
		expected()
		{
			is_error = false;
			new(union_block) T{};
		}

		expected(const T &val)
		{
			is_error = false;
			new(union_block) T{val};
		}

		expected(T &&val) noexcept
		{
			is_error = false;
			new(union_block) T{std::move(val)};
		}

		expected(const E &err)
			requires (!std::is_same_v<T, E>)
		{
			is_error = true;
			new(union_block) E{err};
		}

		expected(E &&err) noexcept
			requires (!std::is_same_v<T, E>)
		{
			is_error = true;
			new(union_block) E{std::move(err)};
		}

		expected(unexpected<E> unex)
		{
			is_error = true;
			new(union_block) E{unex.error};
		}

		~expected()
		{
			if(is_error)
				reinterpret_cast<E *>(union_block)->~E();
			else
				reinterpret_cast<T *>(union_block)->~T();
		}

		expected(const expected &ex)
		{
			is_error = ex.is_error;
			copy(ex.union_block, &ex.union_block[common_size_align<T, E>().first], union_block);
		}

		expected(expected &&ex) noexcept
		{
			if(ex.is_error)
			{
				is_error = true;
				new(union_block) E{std::move(*reinterpret_cast<E *>(ex.union_block))};
			}
			else
			{
				is_error = false;
				new(union_block) T{std::move(*reinterpret_cast<T *>(ex.union_block))};
			}
		}

		auto error() -> E &
		{
			return *reinterpret_cast<E *>(union_block);
		}

		auto value() -> T &
		{
			return *reinterpret_cast<T *>(union_block);
		}

		auto has_value() -> bool
		{
			if(is_error)
				return false;
			else
				return true;
		}


	};
}
