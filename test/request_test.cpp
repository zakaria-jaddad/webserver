#include "../includes/ft_webserver.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;

TEST(RequestLine, OnlyRequestLine)
{
    string requestLine = "GET / HTTP/1.1\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(RequestLine, SpaceBeforeRequestLineStart)
{
    string requestLine = " GET / HTTP/1.1\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

int main(int argc, char **argv)
{

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
