#pragma once

#include <fstream>
#include <chrono>
#include <filesystem>
#include <list>
#include <vector>
#include <type_traits>
#include "utils/ResultDef.hpp"

#ifndef NDEBUG
#include <iostream>
#endif

class Logger
{
public:
    struct Result
    {
        enum class error_code : uint8_t
        {
            //common
            Success,

            // I/O
            IOOpenError,
        } code;

        constexpr Result(error_code err = error_code::Success) : code(err)
        {}

        constexpr auto message() const -> std::string_view;
        constexpr auto to_view() const -> std::string_view;

        constexpr auto operator=(const Result::error_code err) -> Result &;

        constexpr friend auto operator==(const Result &res, const Result::error_code &err_code) -> bool;
    };

    static_assert(hrs::ResultType<Result>);

private:
    std::ofstream output_log_file;
    std::chrono::time_point<std::chrono::system_clock> log_start_time;
	bool is_out_to_clog_enabled;

    auto create_output(std::filesystem::path log_path, bool update_start_time = false) -> Result;
public:
    Logger();
    ~Logger();
    Logger(const Logger &log) = delete;
    Logger(Logger &&log) noexcept;

    auto init(std::filesystem::path log_path = std::filesystem::path(), bool update_start_time = false) -> Result;

    auto change_output(std::filesystem::path log_path, bool update_start_time = false) -> Result;

    auto create_log_string(const std::string_view &emitter, const std::string_view &msg, const std::string_view &msg_decorator) -> std::string;

	auto is_file_opened() -> bool;

    auto log(const std::string_view &plain_msg) -> void;

    template<hrs::ResultType RES_T>
    auto log(RES_T &&res) -> void;

    template<hrs::ResultType RES_T>
    auto log_return(RES_T &&res);

    template<hrs::ResultDefInst RES_DEF_T>
    auto log(RES_DEF_T &&res) -> void;

    template<hrs::ResultDefInst RES_DEF_T>
    auto log_return(RES_DEF_T &res);
};

constexpr auto Logger::Result::operator=(const Logger::Result::error_code err) -> Logger::Result &
{
    code = err;
    return *this;
}

constexpr auto operator==(const Logger::Result &res, const Logger::Result::error_code &err_code) -> bool
{
    return res.code == err_code;
}

constexpr auto Logger::Result::message() const -> std::string_view
{
    std::string_view res;
    switch(code)
    {
        case Result::error_code::Success:
            res = "Logger successfull operation";
            break;
        case Result::error_code::IOOpenError:
            res = "I/O stream opening error";
            break;
    }

    return res;
}

constexpr auto Logger::Result::to_view() const -> std::string_view
{
    std::string_view res;
    switch(code)
    {
        case Result::error_code::Success:
            res = "Success";
            break;
        case Result::error_code::IOOpenError:
            res = "IOOpenError";
            break;
    }

    return res;
}

template<hrs::ResultType RES_T>
auto Logger::log(RES_T &&res) -> void
{
	if(!is_file_opened() && !is_out_to_clog_enabled)
		return;

    auto log_str = create_log_string(std::forward<RES_T>(res).to_view(), std::forward<RES_T>(res).message(), " -> message: ");

	if(is_out_to_clog_enabled)
        std::clog<<log_str;

    if(output_log_file.is_open())
        output_log_file<<log_str;
}

template<hrs::ResultType RES_T>
auto Logger::log_return(RES_T &&res)
{
    log(std::forward<RES_T>(res));
    return std::forward<RES_T>(res);
}

template<hrs::ResultDefInst RES_DEF_T>
auto Logger::log(RES_DEF_T &&res) -> void
{
    if(std::forward<RES_DEF_T>(res).description.empty())
        log(std::forward<RES_DEF_T>(res).error);
    else
	{
		if(!is_file_opened() && !is_out_to_clog_enabled)
			return;

        auto log_str = create_log_string(std::forward<RES_DEF_T>(res).error.to_view(), std::forward<RES_DEF_T>(res).description, "\ndescription: ");

		if(is_out_to_clog_enabled)
            std::clog<<log_str;

		if(output_log_file.is_open())
			output_log_file<<log_str;
    }
}

template<hrs::ResultDefInst RES_DEF_T>
auto Logger::log_return(RES_DEF_T &res)
{
    if(std::forward<RES_DEF_T>(res).description.empty())
        return log_return(std::forward<RES_DEF_T>(res).error);
    else
    {
        log(std::forward<RES_DEF_T>(res));
        return res;
    }
}
