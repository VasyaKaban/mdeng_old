#pragma once

#include <vector>
#include <string_view>
#include <filesystem>
#include <map>
#include <span>
#include "utils/ResultDef.hpp"

class ResourceManager
{
public:
    struct Result
    {
        enum class error_code : uint8_t
        {
            //common
            Success,

            // I/O
			InputOpenError,
            //ReadError,

            //path
            //PathChanged

            //load
            ShaderLoadError,
            ShaderRecieveError
        } code;

        constexpr Result(error_code err = error_code::Success) : code(err)
        {}

        constexpr auto message() const -> std::string_view;
        constexpr auto to_view() const -> std::string_view;

        constexpr auto operator=(const Result::error_code err) -> Result &;

        constexpr friend auto operator==(Result &res, const Result::error_code &err_code) -> bool;
    };

    static_assert(hrs::ResultType<Result>);

    struct ResourceManagerFS
    {
        std::filesystem::path shader_search_path;
        ResourceManagerFS(std::filesystem::path shader_path = std::filesystem::path()) : shader_search_path(shader_path)
        {}

        friend auto operator==(const ResourceManagerFS &l, const ResourceManagerFS &r) -> bool;
    };
private:
    ResourceManagerFS start_fs_path;
    std::map<std::string, std::vector<uint32_t>> shaders;
public:
    ResourceManager();
    ResourceManager(const ResourceManager &rm);
    ResourceManager(ResourceManager &&rm) noexcept;
    ~ResourceManager();

    auto init(const ResourceManagerFS &init_path = ResourceManagerFS()) -> Result;

    auto LoadShaders(const std::vector<std::string_view> &shaders_names) -> hrs::ResultDef<ResourceManager::Result>;

    template<typename T_INFO_CONTAINER>
    requires requires(T_INFO_CONTAINER &t)
    {
        {t.name} -> std::convertible_to<std::string_view>;
        {t.code} -> std::convertible_to<std::span<uint32_t>>;
    }
    auto GetShaders(std::vector<T_INFO_CONTAINER> &out_shaders) -> hrs::ResultDef<Result>;
};

constexpr auto ResourceManager::Result::operator=(const ResourceManager::Result::error_code err) -> ResourceManager::Result &
{
    code = err;
    return *this;
}

constexpr auto operator==(const ResourceManager::Result &res, const ResourceManager::Result::error_code &err_code) -> bool
{
    return res.code == err_code;
}

constexpr auto ResourceManager::Result::message() const -> std::string_view
{
    std::string_view res;
    switch(code)
    {
        case Result::error_code::Success:
            res = "Resource manager successful operation";
            break;
        case Result::error_code::ShaderLoadError:
            res = "Shader loading error";
            break;
		case Result::error_code::InputOpenError:
			res = "Input stream reading error";
            break;
		/*case Result::error_code::ReadError:
            res = "Input stream reading error";
            break;
        case Result::error_code::PathChanged:
            res = "Resources input path changed";
            break;*/
        case Result::error_code::ShaderRecieveError:
            res = "Shader receiving error";
            break;
    }

    return res;
}

constexpr auto ResourceManager::Result::to_view() const -> std::string_view
{
    std::string_view res;
    switch(code)
    {
        case Result::error_code::Success:
            res = "Success";
            break;
        case Result::error_code::ShaderLoadError:
            res = "ShaderLoadError";
            break;
		case Result::error_code::InputOpenError:
            res = "InputOpenError";
            break;
		/*case Result::error_code::ReadError:
            res = "ReadError";
            break;
        case Result::error_code::PathChanged:
            res = "PathChanged";
            break;*/
        case Result::error_code::ShaderRecieveError:
            res = "ShaderRecieveError";
            break;
    }

    return res;
}

template<typename T_INFO_CONTAINER>
requires
    requires(T_INFO_CONTAINER &t)
    {
        {t.name} -> std::convertible_to<std::string_view>;
        {t.code} -> std::convertible_to<std::span<uint32_t>>;
    }
auto ResourceManager::GetShaders(std::vector<T_INFO_CONTAINER> &out_shaders) -> hrs::ResultDef<ResourceManager::Result>
{
    std::string missed_shaders;
    for(auto &out : out_shaders)
    {
        auto it = shaders.find(std::string(out.name));
        if(it == shaders.end())
        {
            missed_shaders += out.name;
            missed_shaders += ' ';
        }
        else
            out.code = it->second;
    }

    if(!missed_shaders.empty())
    {
        missed_shaders.pop_back();
        return {Result::error_code::ShaderRecieveError, std::string("This shaders doesn't exist and haven't been loaded: ") + missed_shaders};
    }

    return {Result::error_code::Success};
}
