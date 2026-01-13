#include "../includes/ft_webserver.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;

TEST(RequestHeader, MissingPath)
{
    string requestLine = "GET HTTP/1.1\r\nHost: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(RequestHeader, ValidRequestWithLongPathAndParams)
{
    string requestLine =
        "GET /path/with/query?param=value&param=123 HTTP/1.1\r\nHost: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);
    ASSERT_EQ(par.getRequestType(), HttpParser::GET);
    ASSERT_EQ(par.getRequestURI(), "/path/with/query?param=value&param=123");

    const int *version = par.getRequestVersion();
    ASSERT_EQ(version[0], 1);
    ASSERT_EQ(version[1], 1);
}

TEST(RequestHeader, VTabBeforeColn)
{
    string requestLine = "GET / HTTP/1.1\r\nHost\t: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(RequestHeader, SpaceBeforeFieldValue)
{
    string requestLine = "GET / HTTP/1.1\r\nHost:                localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);
}

TEST(RequestHeader, SpaceAfterFieldValue)
{
    string requestLine = "GET / HTTP/1.1\r\nHost: localhost               \r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);
}

TEST(RequestHeader, SpaceBeforeAndAfterFieldValue)
{
    string requestLine = "GET / HTTP/1.1\r\nHost:                 localhost \r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);
}

TEST(RequestHeader, NoSpaceAfterColn)
{
    string requestLine = "GET / HTTP/1.1\r\nHost:localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);
}

TEST(RequestHeader, NoDoubleCRLFAfterHeaders)
{
    string requestLine = "GET / HTTP/1.1\r\nHost: localhost\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);
}

TEST(RequestHeader, ValidRequest)
{
    string requestLine = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);
    ASSERT_EQ(par.getRequestType(), HttpParser::GET);
    ASSERT_EQ(par.getRequestURI(), "/");

    const int *version = par.getRequestVersion();
    ASSERT_EQ(version[0], 1);
    ASSERT_EQ(version[1], 1);
}

TEST(RequestHeader, SpaceBeforeFieldName)
{
    string requestLine = "GET / HTTP/1.1\r\n Host: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}
