#include "register_config_path.h"
#include <assert.h>
#include <iostream>
REGISTER_BEGIN
std::vector<std::string> RegisterConfigPath::GetConfigPath(const std::vector<size_t> indices)
{
	std::vector<std::string> ret;
	for (auto idx : indices)
	{
		ret.emplace_back(m_config_paths[idx]);
	}
	return ret;
}

std::vector<std::string> RegisterConfigPath::GetConfigPath(Operator op)
{
	auto size = m_config_paths.size();
	if (op == NEXT)
	{
		m_current_idx = (m_current_idx + 1) % size;
	}
	else if (op == PREVIOUS)
	{
		m_current_idx = (m_current_idx - 1 + size) % size;
	}
	std::cout << m_current_idx << std::endl;
	return { m_config_paths[m_current_idx] };
}

REGISTER_END