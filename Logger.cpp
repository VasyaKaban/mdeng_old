#include "Logger.h"
#include <iostream>
#include <charconv>

using
    std::move,
    std::filesystem::path,
    std::filesystem::create_directories,
    std::filesystem::exists,
    std::chrono::system_clock,
    std::chrono::hh_mm_ss,
	std::chrono::floor,
	std::chrono::year_month_day,
	std::chrono::days,
    std::string,
    std::ostringstream,
    std::error_code,
    std::ofstream,
    std::setw,
    std::setfill,
    std::string_view,
	std::underlying_type_t,
	std::clog,
	std::to_chars;

Logger::Logger()
{
    log_start_time = system_clock::now();
	#ifdef NDEBUG
		is_out_to_clog_enabled = false;
	#else
		is_out_to_clog_enabled = true;
	#endif
}

auto Logger::create_output(std::filesystem::path log_path, bool update_start_time) -> Result
{
    if(output_log_file.is_open())
        output_log_file.close();

    if(update_start_time)
        log_start_time = system_clock::now();

    path file_name;
    if(log_path.has_filename())
    {
        file_name = log_path.filename();
        log_path.remove_filename();
    }
    else
    {
		auto floor_days = floor<days>(log_start_time);
		year_month_day creation_date(floor_days);
		hh_mm_ss creation_time(log_start_time - floor_days);

		string file_name_str("dd.mm.yyyy_hh:mm:ss.log");
		to_chars(file_name_str.data(), file_name_str.data() + 2, static_cast<unsigned>(creation_date.day()));
		to_chars(file_name_str.data() + 3, file_name_str.data() + 5, static_cast<unsigned>(creation_date.month()));
		to_chars(file_name_str.data() + 6, file_name_str.data() + 10, static_cast<int>(creation_date.year()));
		to_chars(file_name_str.data() + 11, file_name_str.data() + 13, creation_time.hours().count());
		to_chars(file_name_str.data() + 14, file_name_str.data() + 16, creation_time.minutes().count());
		to_chars(file_name_str.data() + 17, file_name_str.data() + 19, creation_time.seconds().count());


		file_name = move(file_name_str);
    }

    if(!exists(log_path) && !log_path.empty())
    {
        error_code err;
        create_directories(log_path, err);
        if(err)
            return Result::error_code::IOOpenError;
    }

    log_path /= file_name;

    output_log_file = ofstream(log_path);
	if(!output_log_file.is_open())
		return Result::error_code::IOOpenError;

    return Result::error_code::Success;
}

Logger::~Logger()
{
    if(output_log_file.is_open())
        output_log_file.close();
}

Logger::Logger(Logger &&log) noexcept
{
    output_log_file = move(log.output_log_file);
    log_start_time = log.log_start_time;
}

auto Logger::init(path log_path, bool update_start_time) -> Result
{
    return create_output(log_path, update_start_time);
}

auto Logger::change_output(std::filesystem::path log_path, bool update_start_time) -> Result
{
    return create_output(log_path, update_start_time);
}

auto Logger::create_log_string(const std::string_view &emitter, const std::string_view &msg, const std::string_view &msg_decorator) -> string
{
    hh_mm_ss target_time(system_clock::now() - log_start_time);
	string reserved_str;
	reserved_str.reserve(emitter.size() + msg.size() + msg_decorator.size() + 16);
	ostringstream formated_log(reserved_str);

	if(!emitter.empty())
	{
		formated_log << '[' << emitter << ']'<< " | ";
	}

    formated_log<<
    "("<<target_time.hours().count()<<":"<<
    setw(2)<<setfill('0')<<target_time.minutes().count()<<":"<<
    setw(2)<<setfill('0')<<target_time.seconds().count()<<")"<<
    msg_decorator<<msg;
    if(msg.back() != '\n')
        formated_log<<"\n";

    return formated_log.str();
}

auto Logger::is_file_opened() -> bool
{
	return output_log_file.is_open();
}

auto Logger::log(const string_view &plain_msg) -> void
{
	if(!is_file_opened() && !is_out_to_clog_enabled)
		return;

	 auto log_str = create_log_string("", plain_msg, " -> ");

	if(is_out_to_clog_enabled)
		std::clog<<log_str;

	if(output_log_file.is_open())
		output_log_file<<log_str;
}
