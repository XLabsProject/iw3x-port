#include "STDInclude.hpp"

namespace Utils
{
	namespace Base64
	{
		std::array<char, 64> constexpr encode_table{ 'A', 'B', 'C', 'D', 'E', 'F',
			'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
			'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
			'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
			'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
			'+', '/' };

		std::array<std::uint8_t, 128> constexpr decode_table{ 0x64, 0x64, 0x64, 0x64,
			0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
			0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
			0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
			0x64, 0x64, 0x64, 0x3E, 0x64, 0x64, 0x64, 0x3F, 0x34, 0x35, 0x36, 0x37,
			0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
			0x64, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
			0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
			0x17, 0x18, 0x19, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x1A, 0x1B, 0x1C,
			0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
			0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x64,
			0x64, 0x64, 0x64, 0x64 };

		std::array<char, 4> EncodeTriplet(
			std::uint8_t a, std::uint8_t b, std::uint8_t c)
		{
			std::uint32_t const concat_bits = (a << 16) | (b << 8) | c;

			auto const b64_char1 = encode_table[(concat_bits >> 18) & 0b0011'1111];
			auto const b64_char2 = encode_table[(concat_bits >> 12) & 0b0011'1111];
			auto const b64_char3 = encode_table[(concat_bits >> 6) & 0b0011'1111];
			auto const b64_char4 = encode_table[concat_bits & 0b0011'1111];
			return { b64_char1, b64_char2, b64_char3, b64_char4 };
		}

		inline bool IsValidBase64Char(char c)
		{
			if ((c >= 'A') && (c <= 'Z'))
			{
				return true;
			}

			if ((c >= 'a') && ('z'))
			{
				return true;
			}

			if ((c >= '0') && (c <= '9'))
			{
				return true;
			}

			if ((c == '+') || (c == '/'))
			{
				return true;
			}

			return false;
		}

		inline bool IsValidBase64(std::string_view const encoded_str)
		{
			if ((encoded_str.size() % 4) != 0)
			{
				return false;
			}

			if (!std::all_of(begin(encoded_str), end(encoded_str) - 2,
				[](char c) { return IsValidBase64Char(c); }))
			{
				return false;
			}

			auto const last = rbegin(encoded_str);
			if (!IsValidBase64Char(*next(last)))
			{
				return (*next(last) == '=') && (*last == '=');
			}

			return IsValidBase64Char(*last) || (*last == '=');
		}

		std::array<std::uint8_t, 3> DecodeQuad(char a, char b, char c, char d)
		{
			std::uint32_t const concat_bytes =
				(decode_table[a] << 18) | (decode_table[b] << 12)
				| (decode_table[c] << 6) | decode_table[d];

			std::uint8_t const byte1 = (concat_bytes >> 16) & 0b1111'1111;
			std::uint8_t const byte2 = (concat_bytes >> 8) & 0b1111'1111;
			std::uint8_t const byte3 = concat_bytes & 0b1111'1111;
			return { byte1, byte2, byte3 };
		}
	} // namespace

	std::string Utils::Base64::Encode(std::span<std::uint8_t const> const input)
	{
		auto const size = input.size();
		auto const full_tripples = size / 3;

		std::string output;
		output.reserve((full_tripples + 2) * 4);

		for (unsigned int i = 0; i < full_tripples; ++i)
		{
			auto const tripplet = input.subspan(i * 3, 3);
			auto const base64_chars =
				Utils::Base64::EncodeTriplet(tripplet[0], tripplet[1], tripplet[2]);
			std::copy(
				begin(base64_chars), end(base64_chars), back_inserter(output));
		}

		if (auto const remaining_chars = size - full_tripples * 3;
			remaining_chars == 2)
		{
			auto const last_two = input.last(2);
			auto const base64_chars =
				Utils::Base64::EncodeTriplet(last_two[0], last_two[1], 0x00);

			output.push_back(base64_chars[0]);
			output.push_back(base64_chars[1]);
			output.push_back(base64_chars[2]);
			output.push_back('=');
		}
		else if (remaining_chars == 1)
		{
			auto const base64_chars = Utils::Base64::EncodeTriplet(input.back(), 0x00, 0x00);

			output.push_back(base64_chars[0]);
			output.push_back(base64_chars[1]);
			output.push_back('=');
			output.push_back('=');
		}

		return output;
	}

	std::optional<std::vector<std::uint8_t>> Utils::Base64::Decode(
		std::string_view const encoded_str)
	{
		auto const size = encoded_str.size();

		if (size == 0)
		{
			return std::vector<std::uint8_t>{};
		}

		if (((size % 4) != 0) || !Utils::Base64::IsValidBase64(encoded_str))
		{
			return std::nullopt;
		}

		auto const full_quadruples = size / 4 - 1;

		std::vector<std::uint8_t> decoded_bytes;
		decoded_bytes.reserve(((full_quadruples + 2) * 3) / 4);

		for (unsigned int i = 0; i < full_quadruples; ++i)
		{
			auto const quad = encoded_str.substr(i * 4, 4);
			auto const bytes = Utils::Base64::DecodeQuad(quad[0], quad[1], quad[2], quad[3]);
			std::copy(begin(bytes), end(bytes), back_inserter(decoded_bytes));
		}

		if (auto const last_quad = encoded_str.substr(full_quadruples * 4, 4);
			last_quad[2] == '=')
		{
			auto const bytes = Utils::Base64::DecodeQuad(last_quad[0], last_quad[1], 'A', 'A');
			decoded_bytes.push_back(bytes[0]);
		}
		else if (last_quad[3] == '=')
		{
			auto const bytes =
				Utils::Base64::DecodeQuad(last_quad[0], last_quad[1], last_quad[2], 'A');
			std::copy_n(begin(bytes), 2, back_inserter(decoded_bytes));
		}
		else
		{
			auto const bytes =
				Utils::Base64::DecodeQuad(last_quad[0], last_quad[1], last_quad[2], last_quad[3]);
			std::copy_n(begin(bytes), 3, back_inserter(decoded_bytes));
		}

		return decoded_bytes;
	}
}
