#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Utils
{
    namespace Base64
    {
        std::string Encode(std::span<std::uint8_t const> const input);

        std::optional<std::vector<std::uint8_t>> Decode(
            std::string_view const encoded_str);
    }
} 
