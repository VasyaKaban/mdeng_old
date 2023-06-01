#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <concepts>
#include <utility>
#include <cmath>
#include <optional>

namespace twv
{
	template<typename T>
	concept arithmetic = std::is_arithmetic_v<T>;

	template<arithmetic T, size_t LEN>
	requires
		(LEN >= 2)
	struct Vec
	{
		T vec[LEN];

		constexpr Vec()
		{
			for(auto &it : vec)
				it = T(0);
		}

		/*template<arithmetic U>
		constexpr Vec(const U arr[LEN])
		{
			for(size_t i = 0; i < LEN; i++)
				vec[i] = arr[i];
		}*/

		template<arithmetic U, size_t ARR_LEN>
		constexpr Vec(const U arr[ARR_LEN])
		{
			if(ARR_LEN < LEN)
			{
				size_t i = 0;
				for(; i < ARR_LEN; i++)
					vec[i] = arr[i];

				for(size_t j = i; j < LEN; j++)
					vec[j] = T(0);
			}
			else
			{
				for(size_t i = 0; i < LEN; i++)
					vec[i] = arr[i];
			}
		}

		template<arithmetic ...Args>
		constexpr Vec(Args &&...args) : vec{static_cast<std::common_type_t<Args...>>(std::forward<Args>(args))...}
		{}

		/*constexpr Vec(std::initializer_list<T> l)
		{
			auto len = std::min(LEN, l.size());
			size_t i = 0;
			for(auto &el : l)
			{
				if(i == len)
					break;
				vec[i] = el;

				i++;
			}
		}*/

		template<typename U, size_t ANOTHER_LEN>
		constexpr Vec(const Vec<U, ANOTHER_LEN> &v)
		{
			if constexpr(ANOTHER_LEN < LEN)
			{
				for(size_t i = 0; i < ANOTHER_LEN; i++)
					vec[i] = v.vec[i];
			}
			else
			{
				for(size_t i = 0; i < LEN; i++)
					vec[i] = v.vec[i];
			}
		}

		template<typename U, size_t ANOTHER_LEN>
		constexpr auto operator=(const Vec<U, ANOTHER_LEN> &v) -> Vec &
		{
			if constexpr(ANOTHER_LEN < LEN)
			{
				for(size_t i = 0; i < ANOTHER_LEN; i++)
					vec[i] = v.vec[i];
			}
			else
			{
				for(size_t i = 0; i < LEN; i++)
					vec[i] = v.vec[i];
			}
			return *this;
		}

		template<arithmetic U>
		constexpr auto operator=(const U arr[LEN]) -> Vec &
		{
			for(size_t i = 0; i < LEN; i++)
				vec[i] = arr[i];

			return *this;
		}

		template<arithmetic U, size_t ARR_LEN>
		constexpr auto operator=(const U arr[ARR_LEN]) -> Vec &
		{
			if(ARR_LEN < LEN)
			{
				size_t i = 0;
				for(; i < ARR_LEN; i++)
					vec[i] = arr[i];

				for(size_t j = i; j < LEN; j++)
					vec[j] = T(0);
			}
			else
			{
				for(size_t i = 0; i < LEN; i++)
					vec[i] = arr[i];
			}

			return *this;
		}


		template<typename U, size_t LEN_U>
		requires (LEN_U <= LEN)
		constexpr auto operator+(const Vec<U, LEN_U> &v) const -> Vec<std::common_type_t<T, U>, LEN>
		{
			Vec<std::common_type_t<T, U>, LEN> out_v = vec;
			for(size_t i = 0; i < LEN_U; i++)
				out_v[i] += v.vec[i];

			return out_v;
		}

		template<typename U, size_t LEN_U>
		requires (LEN_U <= LEN)
		constexpr auto operator-(const Vec<U, LEN_U> &v) const -> Vec<std::common_type_t<T, U>, LEN>
		{
			Vec<std::common_type_t<T, U>, LEN> out_v = vec;
			for(size_t i = 0; i < LEN_U; i++)
				out_v[i] -= v.vec[i];

			return out_v;
		}

		template<typename U, size_t LEN_U>
		requires (LEN_U <= LEN)
		constexpr auto operator+=(const Vec<U, LEN_U> &v)-> Vec &
		{
			for(size_t i = 0; i < LEN_U; i++)
				vec[i] += v.vec[i];

			return *this;
		}

		template<typename U, size_t LEN_U>
		requires (LEN_U <= LEN)
		constexpr auto operator-=(const Vec<U, LEN_U> &v) -> Vec &
		{
			for(size_t i = 0; i < LEN_U; i++)
				vec[i] -= v.vec[i];

			return *this;
		}

		template<typename U>
		constexpr auto operator^(const Vec<U, LEN> &v) const -> Vec<std::common_type_t<T, U>, LEN>
		requires (LEN == 3)
		{
			return Vec<std::common_type_t<T, U>, LEN>
			{
				vec[1] * v.vec[2] - vec[2] * v.vec[1],
				-(vec[0] * v.vec[2] - vec[2] * v.vec[0]),
				vec[0] * v.vec[1] - vec[1] * v.vec[0]
			};
			/*Vec<std::common_type_t<T, U>, LEN> out_v;
			out_v.vec[0] = vec[1] * v.vec[2] - vec[2] * v.vec[1];
			out_v.vec[1] = -(vec[0] * v.vec[2] - vec[2] * v.vec[0]);
			out_v.vec[2] = vec[0] * v.vec[1] - vec[1] * v.vec[0];
			return out_v;*/
		}

		template<typename U>
		constexpr auto operator*(const Vec<U, LEN> &v) const -> std::common_type_t<T, U>
		{
			std::common_type_t<T, U> dot_pr{};
			for(size_t i = 0; i < LEN; i++)
				dot_pr += vec[i] * v.vec[i];

			return dot_pr;
		}

		constexpr auto operator[](size_t ind) -> T &
		{
			return vec[ind];
		}

		constexpr auto operator[](size_t ind) const -> const T &
		{
			return vec[ind];
		}

		constexpr auto get(size_t ind) const -> std::optional<std::reference_wrapper<T&>>
		{
			if(ind >= LEN)
				return {};

			return vec[ind];
		}

		constexpr auto len(const std::common_type_t<T, float> eps = 0.00001) const
		{
			std::common_type_t<T, float> powered{};
			for(auto &i : vec)
				powered += i * i;

			if(1.0 - eps <= powered && powered <= 1.0 + eps)
				return powered;

			return static_cast<std::common_type_t<T, float>>(sqrt(powered));
		}

		template<arithmetic S>
		constexpr auto operator+(S s) const -> Vec<std::common_type_t<T, S>, LEN>
		{
			Vec<std::common_type_t<T, S>, LEN> v = vec;
			for(auto &it : v.vec)
				it += s;

			return v;
		}

		template<arithmetic S>
		constexpr auto operator-(S s) const -> Vec<std::common_type_t<T, S>, LEN>
		{
			Vec<std::common_type_t<T, S>, LEN> v = vec;
			for(auto &it : v.vec)
				it -= s;

			return v;
		}

		template<arithmetic S>
		constexpr auto operator*(S s) const -> Vec<std::common_type_t<T, S>, LEN>
		{
			Vec<std::common_type_t<T, S>, LEN> v = vec;
			for(auto &it : v.vec)
				it *= s;

			return v;
		}

		template<arithmetic S>
		constexpr auto operator/(S s) const -> Vec<std::common_type_t<T, S>, LEN>
		{
			Vec<std::common_type_t<T, S>, LEN> v = *this;
			for(auto &it : v.vec)
				it /= s;

			return v;
		}

		template<arithmetic S>
		constexpr auto operator+=(S s) -> Vec &
		{
			for(auto &it : vec)
				it += s;

			return *this;
		}

		template<arithmetic S>
		constexpr auto operator-=(S s) -> Vec &
		{
			for(auto &it : vec)
				it -= s;

			return *this;
		}

		template<arithmetic S>
		constexpr auto operator*=(S s) -> Vec &
		{
			for(auto &it : vec)
				it *= s;

			return *this;
		}

		template<arithmetic S>
		constexpr auto operator/=(S s) -> Vec &
		{
			for(auto &it : vec)
				it /= s;

			return *this;
		}

		constexpr auto normalize() const -> Vec
		{
			return (*this) / len();
		}

		constexpr auto cosine(const Vec &v) const -> std::common_type_t<float, T>
		requires (LEN <= 3)
		{
			auto fv = normalize();
			auto sv = v.normalize();

			return fv * sv;
		}

		struct iterator
		{
			const T *start_ptr;
			size_t len;
			T *target_ptr;

			constexpr iterator()
			{
				start_ptr = nullptr;
				len = 0;
				target_ptr = nullptr;
			}

			constexpr iterator(Vec &v, size_t step)
			{
				start_ptr = &v.vec[0];
				len = v.dim();
				if(step < len)
					target_ptr = &v.vec[step];
				else
					target_ptr = nullptr;
			}

			constexpr auto operator++() -> iterator &
			{
				if(target_ptr != nullptr)
				{
					target_ptr++;
					if(target_ptr - start_ptr == len)
						target_ptr = nullptr;
				}

				return *this;
			}

			constexpr auto operator*() const -> T &
			{
				return *target_ptr;
			}

			constexpr auto operator->() const -> T *
			{
				return target_ptr;
			}

			constexpr auto operator!=(const iterator &it) const -> bool
			{
				if(start_ptr == it.start_ptr)
					return target_ptr != it.target_ptr;

				return true;
			}
		};

		struct const_iterator
		{
			const T *start_ptr;
			size_t len;
			const T *target_ptr;

			constexpr const_iterator()
			{
				start_ptr = nullptr;
				len = 0;
				target_ptr = nullptr;
			}

			constexpr const_iterator(const Vec &v, size_t step)
			{
				start_ptr = &v.vec[0];
				len = v.dim();
				if(step < len)
					target_ptr = &v.vec[step];
				else
					target_ptr = nullptr;
			}

			constexpr auto operator++() -> const_iterator &
			{
				if(target_ptr != nullptr)
				{
					target_ptr++;
					if(target_ptr - start_ptr == len)
						target_ptr = nullptr;
				}

				return *this;
			}

			constexpr auto operator*() const -> const T &
			{
				return *target_ptr;
			}

			constexpr auto operator->() const -> T *
			{
				return target_ptr;
			}

			constexpr auto operator!=(const const_iterator &it) const -> bool
			{
				if(start_ptr == it.start_ptr)
					return target_ptr != it.target_ptr;

				return true;
			}
		};

		constexpr auto begin() -> iterator
		{
			return iterator(*this, 0);
		}

		constexpr auto end() -> iterator
		{
			return iterator(*this, LEN);
		}

		constexpr auto begin() const -> const_iterator
		{
			return const_iterator(*this, 0);
		}

		constexpr auto end() const -> const_iterator
		{
			return const_iterator(*this, LEN);
		}

		constexpr auto dim() const -> size_t
		{
			return LEN;
		}

		template<arithmetic U>
		constexpr operator Vec<U, LEN>()
		{
			if(std::same_as<T, U>)
				return *this;

			return Vec<U, LEN>(vec);
		}

		/*template<arithmetic U, size_t ANOTHER_LEN>
		constexpr operator Vec<U, ANOTHER_LEN>()
		{
			return Vec<U, ANOTHER_LEN>(vec);
		}*/
	};

	template<arithmetic ...Args>
	Vec(Args ...args) -> Vec<std::common_type_t<Args...>, sizeof...(Args)>;

	namespace glsl
	{
		using Vec2 = Vec<float, 2>;
		using Vec3 = Vec<float, 3>;
		using Vec4 = Vec<float, 4>;

		using Vec2d = Vec<double, 2>;
		using Vec3d = Vec<double, 3>;
		using Vec4d = Vec<double, 4>;

		using Vec2i = Vec<int32_t, 2>;
		using Vec3i = Vec<int32_t, 3>;
		using Vec4i = Vec<int32_t, 4>;

		using Vec2u = Vec<uint32_t, 2>;
		using Vec3u = Vec<uint32_t, 3>;
		using Vec4u = Vec<uint32_t, 4>;

		using Vec2b = Vec<bool, 2>;
		using Vec3b = Vec<bool, 3>;
		using Vec4b = Vec<bool, 4>;
	}
}
