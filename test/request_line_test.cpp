#include "../includes/ft_webserver.hpp"
#include <gtest/gtest.h>

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

TEST(RequestLine, VTabBeforeRequestLineStart)
{
    string requestLine = "\tGET / HTTP/1.1\r\n";
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

TEST(RequestLine, requestTypeSpeacileChar)
{
    string requestLine_1 = "G@T /\r\n Host: localhost\r\n\r\n";
    HttpParser par_1;
    HttpParser::httpResult res_1 = par_1.parseHttpRequest(requestLine_1);
    ASSERT_EQ(res_1, HttpParser::HTTP_RES_BAD_REQUEST);

    string requestLine_2 = "GET! /\r\n Host: localhost\r\n\r\n";
    HttpParser par_2;
    HttpParser::httpResult res_2 = par_2.parseHttpRequest(requestLine_2);
    ASSERT_EQ(res_2, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(RequestLine, InvalidRequestType)
{
    string requestLine = "INVALID /\r\n Host: localhost\r\n\r\n";
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

TEST(RequestLine, ValidHTTP1_1RequestLine)
{
    string requestLine = "GET /foo.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);
    ASSERT_EQ(par.getRequestType(), HttpParser::GET);
    ASSERT_EQ(par.getRequestURI(), "/foo.html");

    const int *requestVersiong = par.getRequestVersion();
    ASSERT_EQ(requestVersiong[0], 1);
    ASSERT_EQ(requestVersiong[1], 1);
}

TEST(RequestLine, ValidHTTP2RequestLine)
{
    string requestLine = "POST /foo.html?foo=bar HTTP/2\r\nHost: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);
    ASSERT_EQ(par.getRequestType(), HttpParser::POST);
    ASSERT_EQ(par.getRequestURI(), "/foo.html?foo=bar");

    const int *requestVersiong = par.getRequestVersion();
    ASSERT_EQ(requestVersiong[0], 2);
    ASSERT_EQ(requestVersiong[1], 0);
}
