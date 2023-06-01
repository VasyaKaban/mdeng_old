#pragma once

#include "Vec.hpp"

namespace twv
{
	template<arithmetic T, size_t ROWS, size_t COLS>
	struct Mat
	{
		Vec<T, COLS> mat[ROWS];

		constexpr Mat() = default;

		constexpr Mat(const Mat &m)
		{
			for(size_t i = 0; i < ROWS; i++)
				mat[i] = m.mat[i];
		}

		constexpr auto operator[](size_t ind) -> Vec<T, COLS> &
		{
			return mat[ind];
		}

		constexpr auto operator[](size_t ind) const -> const Vec<T, COLS> &
		{
			return mat[ind];
		}

		constexpr auto get(size_t ind) -> std::optional<std::reference_wrapper<Vec<T, COLS> &>>
		{
			if(ind >= ROWS)
				return {};
			else
				return mat[ind];
		}

		template<typename U, size_t ROWS_U, size_t COLS_U>
		requires (ROWS_U <= ROWS) && (COLS_U <= COLS)
		constexpr auto operator=(Mat<U, ROWS_U, COLS_U> &&m) -> Mat &
		{
			for(size_t i = 0; i < ROWS_U; i++)
				for(size_t j = 0; j < COLS_U; j++)
					mat[i][j] = m[i][j];

			return *this;
		}

		template<typename U, size_t ROWS_U, size_t COLS_U>
		requires (ROWS_U <= ROWS) && (COLS_U <= COLS)
		constexpr auto operator+(const Mat<U, ROWS_U, COLS_U> &m) -> Mat<std::common_type_t<U, T>, ROWS, COLS>
		{
			Mat<std::common_type_t<U, T>, ROWS, COLS> out_m = *this;
			for(size_t i = 0; i < ROWS_U; i++)
				out_m[i] += m.mat[i];

			return out_m;
		}

		template<typename U, size_t ROWS_U, size_t COLS_U>
		requires (ROWS_U <= ROWS) && (COLS_U <= COLS)
		constexpr auto operator-(const Mat<U, ROWS_U, COLS_U> &m) -> Mat<std::common_type_t<U, T>, ROWS, COLS>
		{
			Mat<std::common_type_t<U, T>, ROWS, COLS> out_m = *this;
			for(size_t i = 0; i < ROWS_U; i++)
				out_m[i] -= m.mat[i];

			return out_m;
		}

		template<typename U, size_t ROWS_U, size_t COLS_U>
		requires (ROWS_U == COLS)
		constexpr auto operator*(const Mat<U, ROWS_U, COLS_U> &m) -> Mat<std::common_type_t<U, T>, ROWS, COLS_U>
		{
			using CommonType = std::common_type_t<U, T>;
			Mat<CommonType, ROWS, COLS_U> out_m;


			for(size_t i = 0; i < ROWS; i++)
			{
				for(size_t j = 0; j < COLS_U; j++)
				{
					out_m[i][j] = CommonType{};
					for(size_t k = 0; k < COLS; k++)
					{
						out_m[i][j] += mat[i][k] * m.mat[k][j];
					}
				}
			}

			return out_m;
		}

		template<typename U, size_t VEC_LEN>
		requires (VEC_LEN == COLS)
		constexpr friend auto operator*(const Vec<U, VEC_LEN> &v, const Mat &m) -> Vec<std::common_type_t<U, T>, VEC_LEN>
		{
			Vec<std::common_type_t<U, T>, VEC_LEN> out_v;
			for(size_t i = 0; i < VEC_LEN; i++)
			{
				for(size_t j = 0; j < COLS; j++)
				{
					out_v[i] += v[j] * m[j][i];
				}
			}

			return out_v;
		}

		template<typename U, size_t ROWS_U, size_t COLS_U>
		requires (ROWS_U <= ROWS) && (COLS_U <= COLS)
		constexpr auto operator+=(const Mat<U, ROWS_U, COLS_U> &m) -> Mat &
		{
			for(size_t i = 0; i < ROWS_U; i++)
				mat[i] += m.mat[i];

			return *this;
		}

		template<typename U, size_t ROWS_U, size_t COLS_U>
		requires (ROWS_U <= ROWS) && (COLS_U <= COLS)
		constexpr auto operator-=(const Mat<U, ROWS_U, COLS_U> &m) -> Mat &
		{
			for(size_t i = 0; i < ROWS_U; i++)
				mat[i] -= m.mat[i];

			return *this;
		}

		constexpr auto rows() const -> size_t
		{
			return ROWS;
		}

		constexpr auto cols() const -> size_t
		{
			return COLS;
		}

		template<arithmetic S>
		constexpr auto operator+(S s) -> Mat
		{
			Mat out_m = *this;
			for(auto &it : out_m.mat)
			{
				it += s;
			}
			return out_m;
		}

		template<arithmetic S>
		constexpr auto operator-(S s) -> Mat
		{
			Mat out_m = *this;
			for(auto &it : out_m.mat)
			{
				it -= s;
			}
			return out_m;
		}

		template<arithmetic S>
		constexpr auto operator*(S s) -> Mat
		{
			Mat out_m = *this;
			for(auto &it : out_m.mat)
			{
				it *= s;
			}
			return out_m;
		}

		template<arithmetic S>
		constexpr auto operator/(S s) -> Mat
		{
			Mat out_m = *this;
			for(auto &it : out_m.mat)
			{
				it /= s;
			}
			return out_m;
		}

		template<arithmetic S>
		constexpr auto operator+=(S s) -> Mat &
		{
			for(auto &it : mat)
				it += s;

			return *this;
		}

		template<arithmetic S>
		constexpr auto operator-=(S s) -> Mat &
		{
			for(auto &it : mat)
				it -= s;

			return *this;
		}

		template<arithmetic S>
		constexpr auto operator*=(S s) -> Mat &
		{
			for(auto &it : mat)
				it *= s;

			return *this;
		}

		template<arithmetic S>
		constexpr auto operator/=(S s) -> Mat &
		{
			for(auto &it : mat)
				it /= s;

			return *this;
		}

		constexpr auto transpose() -> Mat
		{
			if constexpr(ROWS == COLS)
			{
				Mat<T, ROWS, COLS> out_m = *this;
				for(size_t i = 0; i < ROWS - 1; i++)
				{
					for(size_t j = i; j < COLS; j++)
						std::swap(out_m.mat[i][j], out_m.mat[j][i]);
				}
				return out_m;
			}
			else
			{
				Mat<T, COLS, ROWS> out_m;
				for(size_t i = 0; i < ROWS; i++)
					for(size_t j = 0; j < COLS; j++)
						out_m[j][i] = mat[i][j];

				return out_m;
			}
		}

		struct iterator
		{
			Vec<T, COLS> *target_ptr;
			Vec<T, COLS> *start_ptr;
			size_t len;

			constexpr iterator()
			{
				start_ptr = nullptr;
				this->len = 0;
				target_ptr = nullptr;
			}

			constexpr iterator(Mat<T, ROWS, COLS> &m, size_t step)
			{
				start_ptr = &m.mat[0];
				len = m.rows();
				if(step >= m.rows())
					target_ptr = nullptr;
				else
					target_ptr = start_ptr + step;
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

			constexpr auto operator*() -> Vec<T, COLS> &
			{
				return *target_ptr;
			}

			constexpr auto operator->() -> Vec<T, COLS> *
			{
				return target_ptr;
			}

			constexpr auto operator!=(const iterator &it) -> bool
			{
				return target_ptr != it.target_ptr;
			}
		};

		struct const_iterator
		{
			const Vec<T, COLS> *target_ptr;
			const Vec<T, COLS> *start_ptr;
			size_t len;

			constexpr const_iterator()
			{
				start_ptr = nullptr;
				this->len = 0;
				target_ptr = nullptr;
			}

			constexpr const_iterator(const Mat<T, ROWS, COLS> &m, size_t step)
			{
				start_ptr = &m.mat[0];
				len = m.rows();
				if(step >= m.rows())
					target_ptr = nullptr;
				else
					target_ptr = &m.mat[step];
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

			constexpr auto operator*() const -> const Vec<T, COLS> &
			{
				return *target_ptr;
			}

			constexpr auto operator->() const -> const Vec<T, COLS> *
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
			return iterator(*this, ROWS);
		}

		constexpr auto begin() const -> const_iterator
		{
			return const_iterator(*this, 0);
		}

		constexpr auto end() const -> const_iterator
		{
			return const_iterator(*this, ROWS);
		}

		static constexpr auto identity(T val = T(1)) -> Mat
		requires (ROWS == COLS)
		{
			Mat out_m;
			for(size_t i = 0; i < ROWS; i++)
				for(size_t j = 0; j < COLS; j++)
					if(i == j)
						out_m[i][j] = val;

			return out_m;
		}
	};

	namespace glsl
	{
		using Mat2x2 = Mat<float, 2, 2>;
		using Mat3x3 = Mat<float, 3, 3>;
		using Mat4x4 = Mat<float, 4, 4>;

		using Mat2x2d = Mat<double, 2, 2>;
		using Mat3x3d = Mat<double, 3, 3>;
		using Mat4x4d = Mat<double, 4, 4>;
	}
}
