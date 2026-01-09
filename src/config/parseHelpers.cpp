#include "../../includes/config/parseHelpers.hpp"

bool isNumber(const std::string& s)
{
  if (s.empty())
    return false;

  for (size_t i = 0; i < s.size(); ++i)
    if (!std::isdigit(s[i]))
      return false;

  return true;
}

bool isValidIPv4(const std::string& ip)
{
  if (std::count(ip.begin(), ip.end(), '.') != 3)
    return false;

  size_t start = 0;
  for (int i = 0; i < 4; ++i)
  {
    size_t end = ip.find('.', start);
    std::string part = ip.substr(start, end - start);

    if (!isNumber(part))
      return false;

    int n = std::atoi(part.c_str());
    if (n < 0 || n > 255)
      return false;

    start = end + 1;
  }
  return true;
}
