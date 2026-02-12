#include "../includes/request/Request.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <vector>

using namespace std;

TEST(RequestURI, ValidRequestURIPathWithoutParams)
{
    string requestLine = "GET /foo.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);
    ASSERT_EQ(par.getRequestUriPath(), "/foo.html");
    ASSERT_EQ(par.getRequestUriRawPath(), "/foo.html");
    ASSERT_EQ(par.getRequestUriRawParams(), "");
    // ASSERT_EQ(par.getRequestUriParams(), "");
}

TEST(RequestURI, ValidRequestURIPathWithOneParam)
{
    string requestLine = "GET /foo.html?foo=bar&yhosssni=1 HTTP/1.1\r\nHost: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);

    // ASSERT_EQ(par.getRequestUriPath(), "/foo.html");
    // ASSERT_EQ(par.getRequestUriRawPath(), "/foo.html?foo=bar");

    // map<string, string> params = par.getRequestUriParams();
    // ASSERT_EQ(int(params.size()), 1);
    // ASSERT_EQ(params["foo"], "bar");

    ASSERT_EQ(par.getRequestUriRawParams(), "foo=bar&yhosssni=1");
}

TEST(RequestURI, ValidRequestURIPathWithMultipleParams)
{
    string request = "GET /foo.html?foo=bar&bar=foo HTTP/1.1\r\nHost: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(request);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);

    ASSERT_EQ(par.getRequestUriPath(), "/foo.html");
    ASSERT_EQ(par.getRequestUriRawPath(), "/foo.html?foo=bar&bar=foo");

    map<string, string> params = par.getRequestUriParams();
    ASSERT_EQ(int(params.size()), 2);
    ASSERT_EQ(params["foo"], "bar");
    ASSERT_EQ(params["bar"], "foo");
}

TEST(RequestURI, ValidRequestURIPathWithOneParamWithoutValue)
{
    std::string request = "GET /foo.html?foo= HTTP/1.1\r\nHost: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(request);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);

    ASSERT_EQ(par.getRequestUriPath(), "/foo.html");

    ASSERT_EQ(par.getRequestUriRawPath(), "/foo.html?foo=");

    map<string, string> params = par.getRequestUriParams();
    ASSERT_EQ(int(params.size()), 1);
    ASSERT_EQ(params["foo"], "");
}

TEST(RequestURI, RequestWithNoPath)
{
    std::string request = "GET HTTP/1.1\r\nHost: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(request);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);

    ASSERT_EQ(par.getRequestUriPath(), "");
    ASSERT_EQ(par.getRequestUriRawPath(), "");

    map<string, string> params = par.getRequestUriParams();
    ASSERT_EQ(int(params.size()), 0);
}

TEST(RequestURI, WithScuffedPath)
{
    std::string request = "GET /index.html?\tn=n HTTP/1.1\r\nHost: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(request);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);

    ASSERT_EQ(par.getRequestUriPath(), "/index.html");
    ASSERT_EQ(par.getRequestUriRawPath(), "/index.html?\tn=n");

    map<string, string> params = par.getRequestUriParams();
    ASSERT_EQ(int(params.size()), 1);
    ASSERT_EQ(params["\tn"], "n");
    ASSERT_EQ(params["n"], "");
}
