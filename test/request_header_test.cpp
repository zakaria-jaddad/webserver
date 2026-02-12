#include "../includes/request/Request.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <iterator>
#include <vector>

using namespace std;

/*
 * A Host header field must be sent in all HTTP/1.1 request messages. A 400 Bad Request status code
 * may be sent to any HTTP/1.1 request message that lacks or contains more than one Host header
 * field.
 *
 *
 * A client MUST send a Host header field in all HTTP/1.1 request messages. A server MUST respond
 * with a 400 (Bad Request) status code to any HTTP/1.1 request message that lacks a Host header
 * field and to any request message that contains more than one Host header field line or a Host
 * header field with an invalid field value.
 */

/* NOTE: Request Semantics Tests */

TEST(RequestHeaders, ValidSimpleRequest)
{
    string requestLine = "GET index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);

    ASSERT_EQ(int(par.getHeaders().size()), 1);

    vector<string> hostHeader = par.getHeaders()["Host"];

    ASSERT_EQ(int(hostHeader.size()), 1);
    ASSERT_EQ(hostHeader[0], "localhost");
}

TEST(RequestHeaders, ValidSimpleRequestWithMultipleHeaders)
{
    string requestLine = "GET index.html HTTP/1.1\r\nHost: localhost\r\nfoo: bar\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);

    map<string, vector<string>> headers = par.getHeaders();

    ASSERT_EQ(int(headers.size()), 2);

    ASSERT_NE(headers.find("foo"), headers.end());

    vector<string> fooHeader = headers["foo"];
    ASSERT_EQ(fooHeader[0], "bar");
}

TEST(RequestHeaders, MultiHeaderValue)
{
    string requestLine = "GET index.html HTTP/1.1\r\nHost: localhost\r\nfoo: bar, baz\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);

    map<string, vector<string>> headers = par.getHeaders();

    ASSERT_EQ(int(headers.size()), 2);

    ASSERT_NE(headers.find("foo"), headers.end());

    vector<string> fooHeader = headers["foo"];
    ASSERT_EQ(fooHeader[0], "bar");
    ASSERT_EQ(fooHeader[1], "baz");
}

TEST(RequestHeaders, MultiHeaderValueNoSpace)
{
    string requestLine = "GET index.html HTTP/1.1\r\nHost: localhost\r\nfoo:bar,baz\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);

    map<string, vector<string>> headers = par.getHeaders();

    ASSERT_EQ(int(headers.size()), 2);

    ASSERT_NE(headers.find("foo"), headers.end());

    vector<string> fooHeader = headers["foo"];
    ASSERT_EQ(fooHeader[0], "bar");
    ASSERT_EQ(fooHeader[1], "baz");
}

TEST(RequestHeaders, MultiHeadersWithSameKey)
{
    string requestLine =
        "GET index.html HTTP/1.1\r\nHost: localhost\r\nfoo: bar\r\nfoo: baz\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);

    map<string, vector<string>> headers = par.getHeaders();

    ASSERT_EQ(int(headers.size()), 2);

    ASSERT_NE(headers.find("foo"), headers.end());

    vector<string> fooHeader = headers["foo"];
    ASSERT_EQ(fooHeader[0], "bar");
    ASSERT_EQ(fooHeader[1], "baz");
}

TEST(RequestHeaders, MultiHeadersWithSameKeyEmptyValue)
{
    string requestLine = "GET index.html HTTP/1.1\r\nHost: localhost\r\nfoo: bar\r\nfoo: "
                         "baz\r\nfoo:\t\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_OK);

    map<string, vector<string>> headers = par.getHeaders();

    ASSERT_EQ(int(headers.size()), 2);

    ASSERT_NE(headers.find("foo"), headers.end());

    vector<string> fooHeader = headers["foo"];
    ASSERT_EQ(fooHeader[0], "bar");
    ASSERT_EQ(fooHeader[1], "baz");
}

TEST(RequestHeaders, MissingColon)
{
    string requestLine = "GET index.html HTTP/1.1\r\nHost localhost\r\nfoo: bar\r\nfoo: "
                         "baz\r\nfoo:\t\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}

TEST(RequestHeaders, SpaceInName)
{
    string requestLine = "GET index.html HTTP/1.1\r\nHost: localhost\r\nbaz\r\nfoo:\t\r\n\r\n";
    HttpParser par;
    HttpParser::httpResult res = par.parseHttpRequest(requestLine);
    ASSERT_EQ(res, HttpParser::HTTP_RES_BAD_REQUEST);
}
