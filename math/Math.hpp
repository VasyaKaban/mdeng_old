#include "Mat.hpp"

#include <iostream>


namespace twv
{
	template<typename T, size_t ROWS, size_t COLS>
	auto Print(const Mat<T, ROWS, COLS> &matrix, std::ostream &os = std::cout) -> void
	{
		os<<"{"<<std::endl;
		for(auto &row : matrix)
		{
			os<<"\t";
			for(auto &el : row)
			{
				os<<el<<" ";
			}
			os<<std::endl;
		}
		os<<"}"<<std::endl;
	}

	template<typename T, size_t LEN>
	auto Print(const Vec<T, LEN> &vec, std::ostream &os = std::cout) -> void
	{
		//for(size_t i = 0; i < vec.len(); i++)
		//	os<<vec[i]<<" ";
		os<<"{"<<std::endl;
		os<<"\t";
		for(auto &el : vec)
		{
			os<<el<<" ";
		}

		os<<std::endl<<"}"<<std::endl;
	}

	template<arithmetic T>
	constexpr auto to_rad(T angle) -> std::common_type_t<T, float>
	{
		using CommType = std::common_type_t<T, float>;
		return angle * CommType(M_PI) / CommType(180);
	}

	template<arithmetic T>
	constexpr auto to_degree(T rads) -> std::common_type_t<T, float>
	{
		using CommType = std::common_type_t<T, float>;

		return rads * CommType(180) / CommType(M_PI);
	}

	template<std::floating_point T>
	constexpr auto Perspective(T near, std::type_identity_t<T> far, std::type_identity_t<T> fov, std::type_identity_t<T> wh_factor) -> Mat<T, 4, 4>
	{
		T h = tan(to_rad(fov / 2));
		T w = wh_factor * h;
		Mat<T, 4, 4> out_m;
		out_m[0][0] = T(1) / w;
		out_m[1][1] = -T(1) / h;
		out_m[2][2] = -far / (near - far);
		out_m[3][2] = (far * near) / (near - far);
		out_m[2][3] = 1;
		return out_m;
	}

	template<arithmetic T>
	constexpr auto Translate(const Vec<T, 3> &delta) -> Mat<std::common_type_t<T, float>, 4, 4>
	{
		Mat<std::common_type_t<T, float>, 4, 4> matrix;
		matrix[3][0] = delta[0];
		matrix[3][1] = delta[1];
		matrix[3][2] = delta[2];

		return matrix;
	}

	template<std::floating_point T>
	constexpr auto RotateVector(const Vec<T, 3> &axis, const Vec<T, 3> &v, std::type_identity_t<T> angle) -> Vec<T, 3>
	{
		auto n_axis = axis.normalize();
		auto a = n_axis * (v * n_axis);
		auto angle_rad = to_rad(angle);
		T cosa = cos(angle_rad);
		T sina = sin(angle_rad);
		return (v * cosa) + ((v ^ n_axis) * sina) + (a * (T(1) - cosa));
		//return (v * cosa) + (((v - a) ^ n_axis) * sina) + (a * (T(1) - cosa));
	}

	template<std::floating_point T>
	constexpr auto RotateMatrix(const Vec<T, 3> &axis, std::type_identity_t<T> angle) -> Mat<T, 3, 3>
	{
		auto n_axis = axis.normalize();
		auto angle_rad = to_rad(angle);
		T cosa = cos(static_cast<T>(angle_rad));
		T sina = sin(static_cast<T>(angle_rad));
		T one_min_cosa = T(1) - static_cast<T>(cosa);

		Mat<T, 3, 3> out_m;
		out_m[0] = {
			cosa + static_cast<T>(pow(n_axis[0], static_cast<T>(2))) * one_min_cosa,
			-n_axis[2] * sina + n_axis[0] * n_axis[1] * one_min_cosa,
			n_axis[1] * sina + n_axis[0] * n_axis[2] * one_min_cosa
		};
		out_m[1] = {
			n_axis[2] * sina + n_axis[0] * n_axis[1] * one_min_cosa,
			cosa + static_cast<T>(pow(n_axis[1], static_cast<T>(2))) * one_min_cosa,
			-n_axis[0] * sina + n_axis[1] * n_axis[2] * one_min_cosa
		};
		out_m[2] = {
			-n_axis[1] * sina + n_axis[0] * n_axis[2] * one_min_cosa,
			n_axis[0] * sina + n_axis[1] * n_axis[2] * one_min_cosa,
			cosa + static_cast<T>(pow(n_axis[2], static_cast<T>(2))) * one_min_cosa
		};

		return out_m;
	}
}
