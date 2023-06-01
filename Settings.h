#pragma once

#include <cstdint>
#include <string>
#include <filesystem>
#include <charconv>
#include "utils/ResultDef.hpp"

struct Settings
{
    enum class representation : uint8_t
    {
        LOG_OUTPUT_PATH = 0,
        SHADERS_PATH = 1,
        WINDOW_WIDTH = 2,
        WINDOW_HEIGHT = 3,
        WINDOW_IS_FULLSCREEN = 4,

        RREPRESENTATION_ENUM_MAX
    };

    struct Result
    {
        enum class error_code : uint8_t
        {
            //common
            Success,

            // I/O
            OutputOpenError,
            WriteError,
            InputOpenError,
            ReadError,

            //parse
            ParameterNotRecognized,
            ParameterBadValue,
            ParameterNoAssign,
            ParametersParseError
        } code;

        constexpr Result(error_code err = error_code::Success) : code(err)
        {}

        constexpr auto message() const -> std::string_view;
        constexpr auto to_view() const -> std::string_view;

        constexpr auto operator=(const Result::error_code err) -> Result &;

        constexpr friend auto operator==(const Result &res, const Result::error_code &err_code) -> bool;
    };

    static_assert(hrs::ResultType<Result>);


    template<typename VAL_T>
    struct parameter
    {
        using VALUE_T = VAL_T;

        std::string name;
        VALUE_T value;

        parameter(const std::string_view &param_name, const VALUE_T &param_val)
        {
            name = param_name;
            value = param_val;
        }
    };

    parameter<std::filesystem::path> log_output_path {"log_output_path", "../logs/"};
	parameter<bool> clog_is_enabled {"clog_is_enabled", true};
    parameter<std::filesystem::path> shaders_path {"shaders_path", "../res/shaders/spv"};
    parameter<int> window_width {"window_width", 800};
    parameter<int> window_height {"window_height", 600};
    parameter<bool> window_is_fullscreen {"window_is_fullscreen", false};

	Settings();

	auto read_settings(const std::filesystem::path &read_path) -> hrs::ResultDef<Result>;
	auto write_settings(std::filesystem::path write_path) -> Result;

    auto set_setting(const std::string_view &setting_string_represenation) -> Result;

    template<typename PARAM_T>
    requires
        requires(PARAM_T &p)
        {
            {p.value};
            {p.name};
        }
    constexpr auto set_extracted_value(PARAM_T &pr, const std::string_view &extracted_name, const std::string_view &extracted_value) -> Result;
};

constexpr auto Settings::Result::operator=(const Settings::Result::error_code err) -> Settings::Result &
{
    code = err;
    return *this;
}

constexpr auto operator==(const Settings::Result &res, const Settings::Result::error_code &err_code) -> bool
{
    return res.code == err_code;
}

constexpr auto Settings::Result::message() const -> std::string_view
{
    std::string_view res;
    switch(code)
    {
        case Result::error_code::Success:
            res = "Parse successful";
            break;
        case Result::error_code::OutputOpenError:
            res = "Output stream opening error";
            break;
        case Result::error_code::WriteError:
            res = "Output stream writing error";
            break;
        case Result::error_code::InputOpenError:
            res = "Input stream opening error";
            break;
        case Result::error_code::ReadError:
            res = "Output stream reading error";
            break;
        case Result::error_code::ParameterNotRecognized:
            res = "Parameter not recognized";
            break;
        case Result::error_code::ParameterBadValue:
            res = "Parameter has a bad value";
            break;
        case Result::error_code::ParameterNoAssign:
            res = "Parameter doesn't has assign operator";
            break;
        case Result::error_code::ParametersParseError:
            res = "Parameters have parse errors";
            break;
    }

    return res;
}

constexpr auto Settings::Result::to_view() const -> std::string_view
{
    std::string_view res;
    switch(code)
    {
        case Result::error_code::Success:
            res = "Success";
            break;
        case Result::error_code::OutputOpenError:
            res = "OutputOpenError";
            break;
        case Result::error_code::WriteError:
            res = "WriteError";
            break;
        case Result::error_code::InputOpenError:
            res = "InputOpenError";
            break;
        case Result::error_code::ReadError:
            res = "ReadError";
            break;
        case Result::error_code::ParameterNotRecognized:
            res = "ParameterNotRecognized";
            break;
        case Result::error_code::ParameterBadValue:
            res = "ParameterBadValue";
            break;
        case Result::error_code::ParameterNoAssign:
            res = "ParameterNoAssign";
            break;
        case Result::error_code::ParametersParseError:
            res = "ParametersParseError";
            break;
    }

    return res;
}

template<typename PARAM_T>
requires
        requires(PARAM_T &p)
        {
            {p.value};
            {p.name};
        }
constexpr auto Settings::set_extracted_value(PARAM_T &pr, const std::string_view &extracted_name, const std::string_view &extracted_value) -> Settings::Result
{
    if(pr.name != extracted_name)
        return Result::error_code::ParameterNotRecognized;

    auto extract_arithmetic = [&extracted_value](auto &val)
    {
        auto res = std::from_chars(extracted_value.data(), extracted_value.data() + extracted_value.size(), val);
        if(res.ec == std::errc() && res.ptr == extracted_value.data() + extracted_value.size())
            return true;

        return false;
    };

    if constexpr(std::convertible_to<typename PARAM_T::VALUE_T, std::string>)//if param value - is string or convertible to it
        pr.value = extracted_value;
    else if constexpr(std::same_as<typename PARAM_T::VALUE_T, bool>)//if param is bool
    {
        bool parsed_val;
        if(extracted_value == "true")
            parsed_val = true;
        else if(extracted_value == "false")
            parsed_val = false;
        else
        {
            int result_value;
            if(extract_arithmetic(result_value))
                parsed_val = result_value;
            else
                return Result::error_code::ParameterBadValue;
        }

        pr.value = parsed_val;
    }
    else//if param is arithmetic
    {
        typename PARAM_T::VALUE_T result_value;
        if(extract_arithmetic(result_value))
            pr.value = result_value;
        else
            return Result::error_code::ParameterBadValue;
    }

    return Result::error_code::Success;
}
