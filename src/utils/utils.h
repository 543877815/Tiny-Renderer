#pragma once
#include <iostream>
#include <format>
#include <string>
class DataView {
	const char* m_data;
	size_t m_size;
public:
	std::string to_string() {
		return { m_data, m_size };
	}

	const char* data() {
		return m_data;
	}

	size_t size() {
		return m_size;
	}
};

template<>
struct std::formatter<DataView> : std::formatter<std::string>
{
	constexpr auto parse(std::format_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(DataView data_view, format_context& ctx) const {
		return formatter<string>::format(std::format("{}", data_view.to_string()), ctx);
	}
};

template<typename TChar, typename TTraits>
std::basic_ostream<TChar, TTraits>& operator<<(std::basic_ostream<TChar, TTraits>& out, DataView value) {
	out.write(value.data(), static_cast<std::streamsize>(value.size()));
	return out;
}