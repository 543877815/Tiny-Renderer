#pragma once
#include <iostream>
#include <format>
#include <string>
#ifndef NDEBUG
#define GL_ERROR_CHECK(x) GLErrorCheck(x)
void GLErrorCheck(const char* message);
#else
#define GL_ERROR_CHECK(x)
#endif


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

void limitValueRange(float& value, float lowerLimit, float upperLimit);
#ifndef NDEBUG
// If there is a glError this outputs it along with a message to stderr.
// otherwise there is no output.
void GLErrorCheck(const char* message);

#endif
