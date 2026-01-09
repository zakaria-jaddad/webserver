#include "../includes/ft_webserver.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;

TEST(RequestLine, EmptyRequestLine)
{
    string requestLine = "";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(RequestLine, VTab)
{
    string requestLine = "\t\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(RequestLine, OnlyCRLF)
{
    string requestLine = "\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(RequestLine, DoubleCRLF)
{
    string requestLine = "\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(RequestLine, OnlyRequestLine)
{
    string requestLine = "GET / HTTP/1.1\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(RequestLine, NoNewLine)
{
    string requestLine = "GET / HTTP/1.1\r";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(RequestLine, NoReturn)
{
    string requestLine = "GET / HTTP/1.1\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(RequestLine, NoCRLF)
{
    string requestLine = "GET / HTTP/1.1";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(RequestLine, SpaceBeforeCRLF)
{
    string requestLine = "GET / HTTP/1.1 \r\n";
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

TEST(RequestLine, DoubleSpaceAfterMethod)
{
    string requestLine = "GET  / HTTP/1.1\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(RequestLine, DoubleSpaceAfterURI)
{
    string requestLine = "GET /  HTTP/1.1\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(RequestLine, MissingVersion)
{
    string requestLine = "GET /\r\n Host: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(RequestLine, MissingVersionWithSpace)
{
    string requestLine = "GET / \r\n Host: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(RequestLine, MoreThen3Elements)
{
    string requestLine = "GET / HTTP/1.1 ThirdElement \r\n Host: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(Request, SpaceBeforeFieldName)
{
    string requestLine = "GET / HTTP/1.1\r\n Host: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(Request, MissingPath)
{
    string requestLine = "GET HTTP/1.1\r\nHost: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(Request, ValidRequestWithLongPathAndParams)
{
    string requestLine =
        "GET /path/with/query?param=value&param=123 HTTP/1.1\r\nHost: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);
    ASSERT_EQ(par.getRequestType(), "GET");
    ASSERT_EQ(par.getRequestURI(), "/path/with/query?param=value&param=123");
    ASSERT_EQ(par.getRequestVersion(), "HTTP/1.1");
}

TEST(Request, VTabBeforeCooln)
{
    string requestLine = "GET / HTTP/1.1\r\nHost\t: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(Request, SpaceBeforeFieldValue)
{
    string requestLine = "GET / HTTP/1.1\r\nHost:                localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);
}

TEST(Request, SpaceAfterFieldValue)
{
    string requestLine = "GET / HTTP/1.1\r\nHost: localhost               \r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);
}

TEST(Request, SpaceBeforeAndAfterFieldValue)
{
    string requestLine = "GET / HTTP/1.1\r\nHost:                 localhost \r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);
}

TEST(Request, NoSpaceAfterCooln)
{
    string requestLine = "GET / HTTP/1.1\r\nHost:localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);
}

/*
 * NOTE: Should pass since request can be chucked
 */
TEST(Request, NoDoubleCRLFAfterHeaders)
{
    string requestLine = "GET / HTTP/1.1\r\nHost: localhost\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);
}

TEST(Request, ValidRequest)
{
    string requestLine = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);
    ASSERT_EQ(par.getRequestType(), "GET");
    ASSERT_EQ(par.getRequestURI(), "/");
    ASSERT_EQ(par.getRequestVersion(), "HTTP/1.1");
}

int main(int argc, char **argv)
{

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
