#include "matrixalchemy/platform/FileSystem.hpp"

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <array>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace matrixalchemy
{

    std::filesystem::path executableDirectory()
    {
#if defined(_WIN32)
        std::array<wchar_t, 4096> buffer{};
        const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length == 0 || length == buffer.size())
        {
            throw std::runtime_error("Failed to determine the executable path.");
        }
        return std::filesystem::path(buffer.data()).parent_path();
#else
        std::array<char, 4096> buffer{};
        const ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
        if (length <= 0)
        {
            throw std::runtime_error("Failed to determine the executable path.");
        }
        buffer[static_cast<std::size_t>(length)] = '\0';
        return std::filesystem::path(buffer.data()).parent_path();
#endif
    }

    std::filesystem::path resolveAssetPath(const std::filesystem::path &relativePath)
    {
        const std::array<std::filesystem::path, 3> roots = {
            executableDirectory(),
            std::filesystem::current_path(),
            std::filesystem::path(MATRIXALCHEMY_SOURCE_DIR),
        };

        for (const auto &root : roots)
        {
            const auto candidate = root / relativePath;
            if (std::filesystem::exists(candidate))
            {
                return candidate;
            }
        }

        throw std::runtime_error("Asset not found: " + relativePath.generic_string());
    }

    std::string readTextFile(const std::filesystem::path &path)
    {
        std::ifstream file(path);
        if (!file)
        {
            throw std::runtime_error("Failed to open text file: " + path.string());
        }

        std::ostringstream stream;
        stream << file.rdbuf();
        return stream.str();
    }

} // namespace matrixalchemy
