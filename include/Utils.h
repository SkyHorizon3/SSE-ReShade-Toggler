#pragma once

class Bool
{
public:
	Bool() : m_Value() {}
	Bool(bool value) : m_Value(value) {}

	operator bool() const { return m_Value; }

	// The following operators are to allow bool* b = &v[0]; (v is a vector here).
	bool* operator& () { return &m_Value; }
	const bool* operator& () const { return &m_Value; }

private:
	bool m_Value;
};

namespace Utils
{
	inline std::vector<std::string> SplitString(const std::string& str, char delimiter)
	{
		std::vector<std::string> tokens;
		std::istringstream iss(str);
		std::string token;
		while (std::getline(iss, token, delimiter))
		{
			tokens.emplace_back(token);
		}
		return tokens;
	}
}