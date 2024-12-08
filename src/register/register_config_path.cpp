#include "register_config_path.h"
#include <assert.h>
#include <iostream>
REGISTER_BEGIN
std::vector<std::string> RegisterConfigPath::GetConfigPath(const std::vector<size_t> indices)
{
	std::vector<std::string> ret;
	for (auto idx : indices)
	{
		ret.emplace_back(m_configPaths[idx]);
	}
	return ret;
}

std::vector<std::string> RegisterConfigPath::GetConfigPath(Operator op)
{
	auto size = m_configPaths.size();
	if (op == NEXT)
	{
		m_currentIdx = (m_currentIdx + 1) % size;
	}
	else if (op == PREVIOUS)
	{
		m_currentIdx = (m_currentIdx - 1 + size) % size;
	}
	return { m_configPaths[m_currentIdx] };
}

REGISTER_END