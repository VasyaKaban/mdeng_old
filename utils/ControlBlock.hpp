#pragma once

#include <type_traits>
#include <cstddef>
#include <utility>
#include <memory>

namespace hrs
{
	template<typename DATA_T>
	struct ControlBlock
	{
		using data_t = DATA_T;
		data_t *data;
		size_t spectator_count;

		constexpr ControlBlock(data_t *ptr = nullptr, size_t count = 1)
		{
			data = ptr;
			spectator_count = count;
		}

		constexpr ~ControlBlock()
		{
			/*spectator_count--;
			if(spectator_count == 0)
			{
				delete data;
				data = nullptr;
			}*/
		}

		constexpr auto add_spectator() -> void
		{
			spectator_count++;
		}
	};

	template<typename DATA_T>
	class SpectatorPtr
	{
	public:
		using control_block_ptr_t = ControlBlock<DATA_T> *;
	private:
		control_block_ptr_t block;
	public:
		constexpr SpectatorPtr(control_block_ptr_t ptr = nullptr)
		{
			block = ptr;
			if(block != nullptr)
			{
				block->add_spectator();
			}
		}

		constexpr ~SpectatorPtr()
		{
			if(block != nullptr)
			{
				block->spectator_count--;
				if(block->spectator_count == 0)
					delete block;
			}
		}

		constexpr SpectatorPtr(const SpectatorPtr &sp)
		{
			block = sp.block;
			if(block != nullptr)
				block->add_spectator();
		}

		constexpr SpectatorPtr(SpectatorPtr &&sp) noexcept
		{
			block = sp.block;
			sp.block = nullptr;
			//sp.~SpectatorPtr();
		}

		constexpr auto is_empty() -> bool
		{
			if(block == nullptr)
				return true;
			else if(block->data == nullptr)
				return true;

			return false;
		}

		constexpr auto get() -> DATA_T *
		{
			if(block == nullptr)
				return nullptr;
			else
				return block->data;
		}

		constexpr auto count() -> size_t
		{
			if(block == nullptr)
				return 0;
			else
				return block->spectator_count;
		}

		constexpr operator bool()
		{
			if(block == nullptr)
				return false;

			return true;
		}

		constexpr auto operator=(const SpectatorPtr &sp) -> SpectatorPtr &
		{
			~SpectatorPtr();
			block = sp.block;
			if(block != nullptr)
				block->add_spectator();
			return *this;
		}

		constexpr auto operator=(SpectatorPtr &&sp) noexcept -> SpectatorPtr &
		{
			~SpectatorPtr();
			block = sp.block;
			sp.block = nullptr;
			//sp.~SpectatorPtr();
			return *this;
		}

	};
};
