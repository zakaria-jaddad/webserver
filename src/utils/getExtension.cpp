#include "../../includes/utils/utils.hpp"

std::string getExtension( std::string &path )
{
  // remove query string if any
  size_t qpos = path.find('?');
  std::string cleanPath = (qpos == std::string::npos)
    ? path
    : path.substr(0, qpos);

  // find last '/'
  size_t slash = cleanPath.find_last_of('/');
  std::string filename = (slash == std::string::npos)
    ? cleanPath
    : cleanPath.substr(slash + 1);

  // find last '.'
  size_t dot = filename.find_last_of('.');

  // no dot or dot is first char (hidden file like .env)
  if (dot == std::string::npos || dot == 0)
    return "";

  return filename.substr(dot);
}
