#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "url_parser.h"

struct test_case {
	char *url;
	int expect_null;
	struct url_components expected;
};

struct test_case test_cases[] = {
		{"http://example.com",
		 0,
		 {"http",
		  NULL, NULL,
		  "example.com", -1,
		  NULL,
		  NULL,
		 }},
		{"http://example.com/",
		 0,
		 {"http",
		  NULL, NULL,
		  "example.com", -1,
		  "/",
		  NULL,
		  NULL,
		 }},
		{"http://example.com/foo/bar",
		 0,
		 {"http",
		  NULL, NULL,
		  "example.com", -1,
		  "/foo/bar",
		  NULL,
		  NULL,
		 }},

		/* Authority部分の解析テスト */
		{"http://example.com:8080",
		 0,
		 {"http",
		  NULL, NULL,
		  "example.com", 8080,
		  NULL,
		  NULL,
		  NULL,
		 }},
		{"http://example.com:8080/foo",
		 0,
		 {"http",
		  NULL, NULL,
		  "example.com", 8080,
		  "/foo",
		  NULL,
		  NULL,
		 }},
		{"http://example.com:/foo",
		 0,
		 {"http",
		  NULL, NULL,
		  "example.com", -1,
		  "/foo",
		  NULL,
		  NULL,
		 }},
		{"http://example.com:8080?foo",
		 0,
		 {"http",
		  NULL, NULL,
		  "example.com", 8080,
		  NULL,
		  "foo",
		  NULL,
		 }},
		{"http://example.com:8080#foo",
		 0,
		 {"http",
		  NULL, NULL,
		  "example.com", 8080,
		  NULL,
		  NULL,
		  "foo",
		 }},
		{"http://aaa:bbb@example.com",
		 0,
		 {"http",
		  "aaa", "bbb",
		  "example.com", -1,
		  NULL,
		  NULL,
		  NULL,
		 }},
		{"http://aaa@example.com",
		 0,
		 {"http",
		  "aaa", NULL,
		  "example.com", -1,
		  NULL,
		  NULL,
		  NULL,
		 }},
		{"http://aaa:bbb:ccc@example.com/",
		 0,
		 {"http",
		  "aaa", "bbb:ccc",
		  "example.com", -1,
		  "/",
		  NULL,
		  NULL,
		 }},
		{"http://127.0.0.1/foo",
		 0,
		 {"http",
		  NULL, NULL,
		  "127.0.0.1", -1,
		  "/foo",
		  NULL,
		  NULL,
		 }},
		{"http://127.0.0.1:8080/foo",
		 0,
		 {"http",
		  NULL, NULL,
		  "127.0.0.1", 8080,
		  "/foo",
		  NULL,
		  NULL,
		 }},
		{"http://fe80::1/foo",		/*hostname + port */
		 0,
		 {"http",
		  NULL, NULL,
		  "fe80:", 1,
		  "/foo",
		  NULL,
		  NULL,
		 }},
		{"http://[fe80::1]/foo",	/* IPv6 */
		 0,
		 {"http",
		  NULL, NULL,
		  "[fe80::1]", -1,
		  "/foo",
		  NULL,
		  NULL,
		 }},
		{"http://fe80::1:8080/foo",	/* hostname + port */
		 0,
		 {"http",
		  NULL, NULL,
		  "fe80::1", 8080,
		  "/foo",
		  NULL,
		  NULL,
		 }},
		{"http://[fe80::1]:8080/foo",	/* IPv6 + port */
		 0,
		 {"http",
		  NULL, NULL,
		  "[fe80::1]", 8080,
		  "/foo",
		  NULL,
		  NULL,
		 }},
		{"http://[fe80::1:8080]/foo",	/* IPv6 */
		 0,
		 {"http",
		  NULL, NULL,
		  "[fe80::1:8080]", -1,
		  "/foo",
		  NULL,
		  NULL,
		 }},
		{"http://[fe80::1:8080[/foo",	/* Invalid host */
		 1,
		 {}},
		{"http://[fe80::1:8080/foo",	/* Invalid host */
		 1,
		 {}},
		{"http://[fe80::1:8080]aa/foo",	/* Invalid host */
		 1,
		 {}},
		{"http://aa[fe80::1:8080]/foo",	/* Invalid host */
		 1,
		 {}},

		/* query */
		{"http://example.com/foo?bar=baz",
		 0,
		 {"http",
		  NULL, NULL,
		  "example.com", -1,
		  "/foo",
		  "bar=baz",
		  NULL,
		 }},
		{"http://example.com/?",
		 0,
		 {"http",
		  NULL, NULL,
		  "example.com", -1,
		  "/",
		  "",
		  NULL,
		 }},
		{"http://example.com?bar=baz",
		 0,
		 {"http",
		  NULL, NULL,
		  "example.com", -1,
		  NULL,
		  "bar=baz",
		  NULL,
		 }},

		/* fragment */
		{"http://example.com/bar#baz",
		 0,
		 {"http",
		  NULL, NULL,
		  "example.com", -1,
		  "/bar",
		  NULL,
		  "baz",
		 }},
		{"http://example.com/bar#",
		 0,
		 {"http",
		  NULL, NULL,
		  "example.com", -1,
		  "/bar",
		  NULL,
		  "",
		 }},
		{"http://example.com#baz",
		 0,
		 {"http",
		  NULL, NULL,
		  "example.com", -1,
		  NULL,
		  NULL,
		  "baz",
		 }},

		/* full */
		{"http://aaa:bbbb@example.com:8080/foo?bar=baz#qux",
		 0,
		 {"http",
		  "aaa", "bbbb",
		  "example.com", 8080,
		  "/foo",
		  "bar=baz",
		  "qux",
		 }},

		/* without host */
		{"/foo",
		 0,
		 {NULL,
		  NULL, NULL,
		  NULL, -1,
		 "/foo",
		  NULL,
		  NULL,
		 }},
		{"foo/bar",
		 0,
		 {NULL,
		  NULL, NULL,
		  NULL, -1,
		 "foo/bar",
		  NULL,
		  NULL,
		 }},
		{"/foo?foo=bar",
		 0,
		 {NULL,
		  NULL, NULL,
		  NULL, -1,
		  "/foo",
		  "foo=bar",
		  NULL,
		 }},
		{"?foo=bar",
		 0,
		 {NULL,
		  NULL, NULL,
		  NULL, -1,
		  NULL,
		  "foo=bar",
		  NULL,
		 }},
		{"/foo#bar",
		 0,
		 {NULL,
		  NULL, NULL,
		  NULL, -1,
		  "/foo",
		  NULL,
		  "bar",
		 }},
		{"#bar",
		 0,
		 {NULL,
		  NULL, NULL,
		  NULL, -1,
		  NULL,
		  NULL,
		  "bar",
		 }},
		{"?bar=baz#qux",
		 0,
		 {NULL,
		  NULL, NULL,
		  NULL, -1,
		  NULL,
		  "bar=baz",
		  "qux",
		 }},
		{"/foo?bar=baz#qux",
		 0,
		 {NULL,
		  NULL, NULL,
		  NULL, -1,
		  "/foo",
		  "bar=baz",
		  "qux",
		 }},
		/* 相対パスの最初のセグメント:を含めることはできない。
		 * schemeとみなされる。
		 * RFC3986 3.3 Path
		 */
		{"foo:ooo/bar",
		 0,
		 {"foo",
		  NULL, NULL,
		  NULL, -1,
		 "ooo/bar",
		  NULL,
		  NULL,
		 }},
		/* 二番目以降のセグメントなら:を含めることができる */
		{"foo/ba:r",
		 0,
		 {NULL,
		  NULL, NULL,
		  NULL, -1,
		 "foo/ba:r",
		  NULL,
		  NULL,
		 }},

		{"",
		 0,
		 {NULL,
		  NULL, NULL,
		  NULL, -1,
		  "",
		  NULL,
		  NULL,
		 }},

		{"file:///foo/bar",	/* empty authority */
		 0,
		 {"file",
		  NULL, NULL,
		  NULL, -1,
		  "/foo/bar",
		  NULL,
		  NULL,
		 }},

		/* 変則的なURL */
		{"//example.com",		/* without scheme */
		 0,
		 {NULL,
		  NULL, NULL,
		  "example.com", -1,
		  NULL,
		  NULL,
		  NULL,
		 }},
		{"http:/foo/bar",	/* without authority */
		 0,
		 {"http",
		  NULL, NULL,
		  NULL, -1,
		  "/foo/bar",
		  NULL,
		  NULL,
		 }},
		{"http:foo/bar",	/* without authority */
		 0,
		 {"http",
		  NULL, NULL,
		  NULL, -1,
		  "foo/bar",
		  NULL,
		  NULL,
		 }},
		{"://foo",
		 0,
		 {NULL,
		  NULL, NULL,
		  NULL, -1,
		  "://foo",
		  NULL,
		  NULL,
		 }},

		/* error */
		{"http://example.com:808080/",	/* invalid port */
		 1,
		 {}},
		{"http://example.com:8080a/",	/* invalid port */
		 1,
		 {}},
		{"http://:8080/",	/* empty host */
		 1,
		 {}},
		{"http://aaa:bbb@/",	/* empty host */
		 1,
		 {}},
		{"http://foo[bar].com/",
		 1,
		 {}},
		{"http://foo bar.com/",
		 1,
		 {}},
		{"http://foo\r\nbar.com/",
		 1,
		 {}},
};

#define CMP_STRING(a, b, field, name) \
	if (a->field == NULL && b->field) { \
		printf(".%s NULL != %s\n", name, b->field); \
		diff++; \
	} else if (a->field && b->field == NULL) { \
		printf(".%s %s != NULL\n", name, a->field); \
		diff++; \
	} else if (a->field && b->field) { \
		if (strcmp(a->field, b->field)) { \
			printf(".%s %s != %s\n", name, a->field, b->field); \
			diff++; \
		} \
	}

int compare_url_components(URL_COMPONENTS *a, URL_COMPONENTS *b)
{
	int diff = 0;

	CMP_STRING(a, b, scheme, "scheme");
	CMP_STRING(a, b, user, "user");
	CMP_STRING(a, b, password, "password");
	CMP_STRING(a, b, host, "host");

	if (a->port != b->port) {
		printf(".port %d != %d\n", a->port, b->port);
		diff++;
	}

	CMP_STRING(a, b, path, "path");
	CMP_STRING(a, b, query, "query");
	CMP_STRING(a, b, fragment, "fragment");

	return diff == 0 ? 0 : 1;
}

int main()
{
	int i;
	URL_COMPONENTS *c;
	int error = 0;

	for (i = 0 ; i < sizeof(test_cases) / sizeof(struct test_case) ; i++) {
		printf("Case %2d  %s\n", i + 1, test_cases[i].url);
		c = parse_url(test_cases[i].url);

		if (test_cases[i].expect_null) {
			if (c) {
				printf("Returned not NULL.\n");
				error++;
			} else if (errno != EINVAL) {
				printf("errno is not EINVAL.\n");
				error++;
			}
		} else {
			if (!c) {
				printf("Returned NULL.\n");
				error++;
				continue;
			}

			if (compare_url_components(&test_cases[i].expected, c)) {
				error++;
			}
		}
		if (c) {
			free_url_components(c);
		}
	}

	printf("%d error\n", error);

	return 0;
}
