#include "ResourceManager.h"
#include <fstream>

using
    std::string,
    std::ifstream,
    std::filesystem::file_size,
    std::error_code,
    std::vector,
    std::filesystem::path,
    std::ios_base,
    std::move,
    std::span;


auto operator==(const ResourceManager::ResourceManagerFS &l, const ResourceManager::ResourceManagerFS &r) -> bool
{
    return l.shader_search_path == r.shader_search_path;
}

ResourceManager::ResourceManager()
{
    start_fs_path = path();
}

ResourceManager::ResourceManager(const ResourceManager &rm)
{
    #warning TBA!
}

ResourceManager::ResourceManager(ResourceManager &&rm) noexcept
{
    #warning TBA!
}

ResourceManager::~ResourceManager()
{

}

auto ResourceManager::init(const ResourceManagerFS &init_path) -> ResourceManager::Result
{
    start_fs_path = init_path;
    return Result::error_code::Success;
}

auto ResourceManager::LoadShaders(const std::vector<std::string_view> &shaders_names) -> hrs::ResultDef<ResourceManager::Result>
{
    string missed_shaders;
    ifstream input_shader_stream;
    uintmax_t shader_file_size = 0;
    error_code erc;
    path shader_path = start_fs_path.shader_search_path;

    for(auto &sh_name : shaders_names)
    {
        erc.clear();

        shader_path.append(sh_name);
        shader_file_size = file_size(shader_path, erc);
        if(erc)
        {
            missed_shaders += sh_name;
            missed_shaders += ' ';
        }
        else
        {
            size_t shader_code_size = (shader_file_size / 4);

			vector<uint32_t> shader_code(shader_code_size, 0);
            input_shader_stream.open(shader_path, ios_base::in | ios_base::binary);
			if(!input_shader_stream.is_open())
				return {Result::error_code::InputOpenError, string("This path is not accessable: ") + shader_path.string()};
            input_shader_stream.read(reinterpret_cast<ifstream::char_type *>(shader_code.data()), shader_file_size);
            input_shader_stream.close();

            shaders.insert({string(sh_name), move(shader_code)});
        }

        shader_path = shader_path.parent_path();
    }

    if(!missed_shaders.empty())
    {
        missed_shaders.pop_back();
        return {Result::error_code::ShaderLoadError, string("This shaders doesn't exist and haven't been loaded: ") + missed_shaders};
    }

    return {Result::error_code::Success};
}
