#include "Settings.h"
#include <fstream>
#include <charconv>
#include <concepts>

using
    std::filesystem::path,
    std::filesystem::file_type,
    std::ifstream,
    std::string,
    std::istringstream,
    std::error_code,
    std::ofstream,
    std::endl,
    std::string_view,
    std::from_chars,
    std::convertible_to,
    std::same_as;

Settings::Settings()
{

}

auto Settings::read_settings(const std::filesystem::path &read_path) -> hrs::ResultDef<Result>
{
    using namespace std::string_literals;

	if(status(read_path).type() == file_type::not_found)
        return {Result::error_code::InputOpenError};

	auto settings_file_size = file_size(read_path);
    ifstream settings_input_stream;
	settings_input_stream.open(read_path);

    string readed_settings(settings_file_size, ' ');
    settings_input_stream.read(readed_settings.data(), settings_file_size);
    if(settings_input_stream.rdstate() & std::ios_base::failbit)
        return {Result::error_code::ReadError};

    settings_input_stream.close();

    istringstream settings_stream(readed_settings);
    string setting_string;

    Result res;
    string skipped_lines;
    while(true)
    {
        getline(settings_stream, setting_string);
        if(settings_stream.eof())
            break;

        res = set_setting(setting_string);
        if(res.code != Result::error_code::Success)
        {
            if(res.code == Result::error_code::ParameterBadValue)
                skipped_lines += "Bad value: ";
            else if(res.code == Result::error_code::ParameterNotRecognized)
                skipped_lines += "Not recognized: ";
            else if(res.code == Result::error_code::ParameterNoAssign)
                skipped_lines += "No assign: ";

            skipped_lines += setting_string;
            skipped_lines += '\n';
        }
    }

    if(!skipped_lines.empty())
    {
        skipped_lines.pop_back();
        return hrs::ResultDef<Result>(Result::error_code::ParametersParseError, skipped_lines);
    }

    return {Result::error_code::Success};
}

auto Settings::write_settings(std::filesystem::path write_path) -> Result
{
	path filename = write_path.filename();
	write_path.remove_filename();
	if(!exists(write_path))
    {
        error_code err;
		create_directories(write_path, err);
        if(err)
            return Result::error_code::OutputOpenError;
    }

	ofstream output_settings_stream(write_path / filename);
    if(!output_settings_stream.is_open())
        return Result::error_code::OutputOpenError;

	output_settings_stream<<log_output_path.name<<" = "<<log_output_path.value.string()<<endl;
	output_settings_stream<<clog_is_enabled.name<<" = "<<clog_is_enabled.value<<endl;
	output_settings_stream<<shaders_path.name<<" = "<<shaders_path.value.string()<<endl;
    output_settings_stream<<window_width.name<<" = "<<window_width.value<<endl;
    output_settings_stream<<window_height.name<<" = "<<window_height.value<<endl;
    output_settings_stream<<window_is_fullscreen.name<<" = "<<window_is_fullscreen.value<<endl;

    output_settings_stream.close();

    return Result::error_code::Success;
}

#include <iostream>

auto Settings::set_setting(const string_view &setting_string_represenation) -> Result
{
    using namespace std::string_literals;

    auto check_is_comment = [](const string_view &param_space)
    {
        for(auto &it : param_space)
        {
            if(it == '#')
                return true;
            else if(it != ' ' && it != '\t')
                return false;
        }

        return false;
    };

    auto extract_param = [](const string_view &param_space)
    {
        string_view::iterator first = param_space.end(), second = param_space.end();
        bool is_space_tab_after_second = false;
        for(string_view::iterator it = param_space.begin(); it != param_space.end(); it++)
        {
            if(*it != ' ' && *it != '\t')
            {
                if(first == param_space.end())
                    first = it;
                second = it;

                if(is_space_tab_after_second)
                    return string_view();
            }
            else
            {
                if(second != param_space.end())
                    is_space_tab_after_second = true;
            }
        }

        if(first == param_space.end())
            return string_view();

        return string_view(first, second + 1);
    };

    auto is_comm = check_is_comment(setting_string_represenation);
    if(is_comm == true)
        return Result::error_code::Success;

    auto assign_op_ind = setting_string_represenation.find("=");
    if(assign_op_ind == setting_string_represenation.npos)
        return Result::error_code::ParameterNoAssign;
    else if(assign_op_ind == (setting_string_represenation.size() - 1))
        return Result::error_code::ParameterBadValue;


    auto extracted_param_name = extract_param(setting_string_represenation.substr(0, assign_op_ind));
    if(extracted_param_name.empty())
        return Result::error_code::ParameterNotRecognized;

    auto extracted_value = extract_param(setting_string_represenation.substr(assign_op_ind + 1));
    if(extracted_value.empty())
        return Result::error_code::ParameterBadValue;

    if(auto extracted_res = set_extracted_value(log_output_path, extracted_param_name, extracted_value); extracted_res != Result::error_code::ParameterNotRecognized)
        return extracted_res;
	else if(auto extracted_res = set_extracted_value(clog_is_enabled, extracted_param_name, extracted_value); extracted_res != Result::error_code::ParameterNotRecognized)
		return extracted_res;
    else if(auto extracted_res = set_extracted_value(shaders_path, extracted_param_name, extracted_value); extracted_res != Result::error_code::ParameterNotRecognized)
        return extracted_res;
    else if(auto extracted_res = set_extracted_value(window_width, extracted_param_name, extracted_value); extracted_res != Result::error_code::ParameterNotRecognized)
        return extracted_res;
    else if(auto extracted_res = set_extracted_value(window_height, extracted_param_name, extracted_value); extracted_res != Result::error_code::ParameterNotRecognized)
        return extracted_res;
    else if(auto extracted_res = set_extracted_value(window_is_fullscreen, extracted_param_name, extracted_value); extracted_res != Result::error_code::ParameterNotRecognized)
        return extracted_res;
    else
        return Result::error_code::ParameterNotRecognized;

    return Result::error_code::Success;
}
