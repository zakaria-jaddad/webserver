src/response/Response.cpp:10:24: error: no declaration matches ‘HttpParser::httpResult Response::checkMethod()’
   10 | HttpParser::httpResult Response::checkMethod( void )
      |                        ^~~~~~~~
src/response/Response.cpp:10:24: note: no functions named ‘HttpParser::httpResult Response::checkMethod()’
In file included from src/response/Response.cpp:1:
./includes/response/Response.hpp:9:7: note: ‘class Response’ defined here
    9 | class Response{
      |       ^~~~~~~~
src/response/Response.cpp: In member function ‘std::string Response::normalizePath()’:
src/response/Response.cpp:23:23: error: no matching function for call to ‘getline(std::stringstream&, std::string&, const char [2])’
   23 |         while (getline(ss, elem, "/")) {
      |                ~~~~~~~^~~~~~~~~~~~~~~
In file included from /usr/include/c++/11/string:56,
                 from /usr/include/c++/11/bits/locale_classes.h:40,
                 from /usr/include/c++/11/bits/ios_base.h:41,
                 from /usr/include/c++/11/ios:42,
                 from /usr/include/c++/11/ostream:38,
                 from /usr/include/c++/11/iostream:39,
                 from ./includes/response/../server/Server.hpp:4,
                 from ./includes/response/Response.hpp:4,
                 from src/response/Response.cpp:1:
/usr/include/c++/11/bits/basic_string.tcc:1557:5: note: candidate: ‘template<class _CharT, class _Traits, class _Alloc> std::basic_istream<_CharT, _Traits>& std::getline(std::basic_istream<_CharT, _Traits>&, std::__cxx11::basic_string<_CharT, _Traits, _Allocator>&, _CharT)’
 1557 |     getline(basic_istream<_CharT, _Traits>& __in,
      |     ^~~~~~~
/usr/include/c++/11/bits/basic_string.tcc:1557:5: note:   template argument deduction/substitution failed:
src/response/Response.cpp:23:23: note:   deduced conflicting types for parameter ‘_CharT’ (‘char’ and ‘const char*’)
   23 |         while (getline(ss, elem, "/")) {
      |                ~~~~~~~^~~~~~~~~~~~~~~
In file included from /usr/include/c++/11/string:55,
                 from /usr/include/c++/11/bits/locale_classes.h:40,
                 from /usr/include/c++/11/bits/ios_base.h:41,
                 from /usr/include/c++/11/ios:42,
                 from /usr/include/c++/11/ostream:38,
                 from /usr/include/c++/11/iostream:39,
                 from ./includes/response/../server/Server.hpp:4,
                 from ./includes/response/Response.hpp:4,
                 from src/response/Response.cpp:1:
/usr/include/c++/11/bits/basic_string.h:6571:5: note: candidate: ‘template<class _CharT, class _Traits, class _Alloc> std::basic_istream<_CharT, _Traits>& std::getline(std::basic_istream<_CharT, _Traits>&, std::__cxx11::basic_string<_CharT, _Traits, _Allocator>&)’
 6571 |     getline(basic_istream<_CharT, _Traits>& __is,
      |     ^~~~~~~
/usr/include/c++/11/bits/basic_string.h:6571:5: note:   template argument deduction/substitution failed:
src/response/Response.cpp:23:23: note:   candidate expects 2 arguments, 3 provided
   23 |         while (getline(ss, elem, "/")) {
      |                ~~~~~~~^~~~~~~~~~~~~~~
In file included from /usr/include/c++/11/string:55,
                 from /usr/include/c++/11/bits/locale_classes.h:40,
                 from /usr/include/c++/11/bits/ios_base.h:41,
                 from /usr/include/c++/11/ios:42,
                 from /usr/include/c++/11/ostream:38,
                 from /usr/include/c++/11/iostream:39,
                 from ./includes/response/../server/Server.hpp:4,
                 from ./includes/response/Response.hpp:4,
                 from src/response/Response.cpp:1:
/usr/include/c++/11/bits/basic_string.h:6579:5: note: candidate: ‘template<class _CharT, class _Traits, class _Alloc> std::basic_istream<_CharT, _Traits>& std::getline(std::basic_istream<_CharT, _Traits>&&, std::__cxx11::basic_string<_CharT, _Traits, _Allocator>&, _CharT)’
 6579 |     getline(basic_istream<_CharT, _Traits>&& __is,
      |     ^~~~~~~
/usr/include/c++/11/bits/basic_string.h:6579:5: note:   template argument deduction/substitution failed:
src/response/Response.cpp:23:23: note:   deduced conflicting types for parameter ‘_CharT’ (‘char’ and ‘const char*’)
   23 |         while (getline(ss, elem, "/")) {
      |                ~~~~~~~^~~~~~~~~~~~~~~
In file included from /usr/include/c++/11/string:55,
                 from /usr/include/c++/11/bits/locale_classes.h:40,
                 from /usr/include/c++/11/bits/ios_base.h:41,
                 from /usr/include/c++/11/ios:42,
                 from /usr/include/c++/11/ostream:38,
                 from /usr/include/c++/11/iostream:39,
                 from ./includes/response/../server/Server.hpp:4,
                 from ./includes/response/Response.hpp:4,
                 from src/response/Response.cpp:1:
/usr/include/c++/11/bits/basic_string.h:6586:5: note: candidate: ‘template<class _CharT, class _Traits, class _Alloc> std::basic_istream<_CharT, _Traits>& std::getline(std::basic_istream<_CharT, _Traits>&&, std::__cxx11::basic_string<_CharT, _Traits, _Allocator>&)’
 6586 |     getline(basic_istream<_CharT, _Traits>&& __is,
      |     ^~~~~~~
/usr/include/c++/11/bits/basic_string.h:6586:5: note:   template argument deduction/substitution failed:
src/response/Response.cpp:23:23: note:   candidate expects 2 arguments, 3 provided
   23 |         while (getline(ss, elem, "/")) {
      |                ~~~~~~~^~~~~~~~~~~~~~~
In file included from /usr/include/c++/11/cstdio:42,
                 from /usr/include/c++/11/ext/string_conversions.h:43,
                 from /usr/include/c++/11/bits/basic_string.h:6608,
                 from /usr/include/c++/11/string:55,
                 from /usr/include/c++/11/bits/locale_classes.h:40,
                 from /usr/include/c++/11/bits/ios_base.h:41,
                 from /usr/include/c++/11/ios:42,
                 from /usr/include/c++/11/ostream:38,
                 from /usr/include/c++/11/iostream:39,
                 from ./includes/response/../server/Server.hpp:4,
                 from ./includes/response/Response.hpp:4,
                 from src/response/Response.cpp:1:
/usr/include/stdio.h:645:18: note: candidate: ‘__ssize_t getline(char**, size_t*, FILE*)’
  645 | extern __ssize_t getline (char **__restrict __lineptr,
      |                  ^~~~~~~
/usr/include/stdio.h:645:45: note:   no known conversion for argument 1 from ‘std::stringstream’ {aka ‘std::__cxx11::basic_stringstream<char>’} to ‘char**’
  645 | extern __ssize_t getline (char **__restrict __lineptr,
      |                           ~~~~~~~~~~~~~~~~~~^~~~~~~~~
make: *** [Makefile:54: obj/response/Response.o] Error 1
