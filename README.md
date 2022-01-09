# Summary

This is a URL parser written in C. It works like parse_url() in PHP. It split the URL into components such as scheme, host, path, etc.


# Installing

Copy url_parser.c and url_parser.h into your project.


# Functions

## extern URL_COMPONENTS *parse_url(const char *url);

The parse_url() function parses URL which is given by url parameter. It returns URL_COMPONENTS structure pointer or NULL if an error occurred.

The returned URL_COMPONENTS must be released using free_url_components().

In case of error, errno will be set.
- EINVAL - Invalid URL was given.
- ENOMEM - Insufficient memory available

This function simply splits the URL into components. It does not perform strict validation of URLs. Invalid URL is also accepted. URL that only have path is also accepted.

But it returns an error (EINVAL) in some cases.

- The host contains space, \r or \n.
- The port number contains non-numeric characters.
- The port number is out of range.

and so on.

## extern void free_url_components(URL_COMPONENTS *c);

The free_url_components() function frees up URL_COMPONENTS.


# Types

## URL_COMPONENTS

<pre>
typedef struct url_components {
        char *scheme;
        char *user;
        char *password;
        char *host;
        int  port;
        char *path;
        char *query;
        char *fragment;
} URL_COMPONENTS;
</pre>

URL_COMPONENTS is returned by parse_url().

*scheme, user, password, host, path, query, fragment fields:*

If they are not in the URL, they will be set to NULL.

*port field:*

It's a port number. If it is not in the URL, it will be set to -1.


# Input/output examples of parse_url()

http://user:pass@example.com:8080/foo?bar=baz#qux

    .scheme    http
    .user      user
    .password  pass
    .host      example.com
    .port      8080
    .path      /foo
    .query     bar=baz
    .fragment  qux

http://example.com/foo

    .scheme  http
    .host   example.com
    .port   -1
    .path   /foo
    Other fields are NULL.

http://example.com/foo?

    .scheme  http
    .host   example.com
    .port   -1
    .path   /foo
    .query  ""(empty string)
    Other fields are NULL.

/foo

    .port   -1
    .path   /foo
    Other fields are NULL.

file:///foo/bar

    .scheme  file
    .port   -1
    .path   /foo/bar
    Other fields are NULL.

//example.com/foo

    .host  example.com
    .port  -1
    .path  /foo
    Other fields are NULL.

See also unittest.c .


# Example Code

<pre>
#include &lt;stdio.h&gt;
#include "url_parser.h"

int main()
{
	const char *url = "https://example.com:8080/foo/?bar=baz";
	URL_COMPONENTS *c;

	c = parse_url(url);
	if (!c) {
		return -1;
	}

	printf("Scheme: %s\n", c-&gt;scheme ? c-&gt;scheme : "");
	printf("Host:   %s\n", c-&gt;host ? c-&gt;host : "");
	if (c-&gt;port != -1) {
		printf("Port:   %d\n", c-&gt;port);
	}
	printf("Path:   %s\n", c-&gt;path ? c-&gt;path : "");
	printf("Query:  %s\n", c-&gt;query ? c-&gt;query : "");

	free_url_components(c);

	return 0;
}
</pre>

Result

<pre>
Scheme: https
Host:   example.com
Port:   8080
Path:   /foo/
Query:  bar=baz
</pre>
