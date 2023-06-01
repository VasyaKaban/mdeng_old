#pragma once

#include <utility>
#include <string>
#include <type_traits>
#include <concepts>

#include <span>
#include <tuple>

namespace hrs
{
	template<typename RESULT_T>
	concept ResultType = requires(RESULT_T &&res, const RESULT_T &other_res)
	{
		{res.code};
		{std::is_enum_v<decltype(res.code)>};
		{res.message()} -> std::convertible_to<std::string_view>;
		{res.to_view()} -> std::convertible_to<std::string_view>;
		{res == other_res.code} -> std::convertible_to<bool>;
		{res = other_res.code} -> std::convertible_to<RESULT_T>;
		{std::remove_cvref_t<RESULT_T>(res.code)};
	};

	template<typename RESULT_T>
	concept ResultTypeIncludesInnerErrorExt =
			ResultType<RESULT_T> &&
			requires(RESULT_T &&res)
			{
				{res.inner_error};
				{res.inner_to_message()} -> std::convertible_to<std::string_view>;
				{RESULT_T(res.inner_error, res.code)};
			};

	template<ResultType ERR_T>
	struct ResultDef
	{
		using error_t = ERR_T;

		error_t error;
		std::string description;

		constexpr ResultDef(const error_t &err = {}, const std::string &desc = "") : error(err), description(desc)
		{}

		constexpr ~ResultDef()
		{
			if constexpr(std::is_destructible_v<error_t>)
				error.~error_t();
		}

		constexpr ResultDef(const ResultDef &res)
		{
			error = res.error;
			description = res.description;
		}

		constexpr ResultDef(ResultDef &&res) noexcept
		{
			error = std::move(res.error);
			description = std::move(res.description);
		}

		auto operator=(const ResultDef &res) -> ResultDef &
		{
			error = res.error;
			description = res.description;
			return *this;
		}

		auto operator=(ResultDef &res) noexcept -> ResultDef &
		{
			error = std::move(res.error);
			description = std::move(res.description);
			return *this;
		}

	};

	template<typename RESULT_DEF_T>
	concept ResultDefInst = requires(RESULT_DEF_T &&res_def, const RESULT_DEF_T &other_res)
	{
		{res_def.error} -> ResultType;
		{res_def} -> std::convertible_to<ResultDef<decltype(res_def.error)>>;
		{res_def.description} -> std::convertible_to<std::string>;
		{std::remove_cvref_t<RESULT_DEF_T>(res_def.error, res_def.description)};
		{std::remove_cvref_t<RESULT_DEF_T>(other_res)};
	};

	/*template<typename ENUM_RES_T, size_t MSG_LEN, size_t VIEW_LEN>
	requires std::is_enum_v<ENUM_RES_T>
	struct ResultEnumWrapper
	{
		ENUM_RES_T error;
		std::array<char, MSG_LEN> msg;
		std::array<char, VIEW_LEN> v;
	};

	template<typename ENUM_RES_T, typename WRAPPER_T, WRAPPER_T t>
	requires
		std::is_enum_v<ENUM_RES_T>
	struct CommonResult
	{
		using error_code = ENUM_RES_T;
		error_code code;
		constexpr auto message() const
		{
			for(size_t i = 0; i < t.get_len(); i++)
			{
				if(t.get<i>() == code)
					return t.get<i>().msg;
			}
		}

        constexpr auto to_view() const -> const char *
        {
			if(std::get<0>(vals...) == code)
				return std::get<2>(vals...);
		}

        constexpr CommonResult(error_code err) : code(err)
        {}

        constexpr auto operator=(const error_code err) -> CommonResult &
        {
			code = err;
			return *this;
		}

        constexpr friend auto operator==(const CommonResult &res, const error_code &err_code) -> bool
        {
			return res.code == err_code;
		}
    };*/
}
