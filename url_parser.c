#include "url_parser.h"
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static const char *strnstr(const char *s, size_t s_len, const char *needle)
{
	const char *end = s + s_len;
	size_t needle_len = strlen(needle);
	const char *p;

	p = s;
	while (p < end - needle_len + 1) {
		if (strncmp(p, needle, needle_len) == 0) {
			return p;
		}
		p++;
	}

	return NULL;
}

static const char *find_chars(const char *s, size_t s_len, const char *chars)
{
	const char *end = s + s_len;
	size_t chars_n = strlen(chars);
	const char *p;
	int i;

	p = s;
	while (p < end) {
		for (i = 0 ; i < chars_n ; i++) {
			if (*p == chars[i]) {
				return p;
			}
		}
		p++;
	}

	return NULL;
}

static const char *find_chars_reverse(const char *s, size_t s_len, const char *chars)
{
	const char *end = s + s_len;
	size_t chars_n = strlen(chars);
	const char *p;
	int i;

	p = end - 1;
	while (p >= s) {
		for (i = 0 ; i < chars_n ; i++) {
			if (*p == chars[i]) {
				return p;
			}
		}
		p--;
	}

	return NULL;
}

static int is_alpha(char c)
{
	if ((c >= 'a' && c <= 'z') ||
	    (c >= 'A' && c <= 'Z')) {
		return 1;
	}
	return 0;
}

static int is_digit(char c)
{
	if (c >= '0' && c <= '9') {
		return 1;
	}
	return 0;
}

static const char *lookup_scheme(const char *s)
{
	const char *p = s;
	char c;

	if (strlen(s) == 0) {
		return NULL;
	}

	if (!is_alpha(*p)) {
		return NULL;
	}
	p++;

	while (*p != '\0') {
		c = *p;
		if (c == ':') {
			return p;
		}
		if (!is_alpha(c) &&
		    !is_digit(c) &&
		    c != '+' &&
		    c != '-' &&
		    c != '.') {
			return NULL;
		}
		p++;
	}
	return NULL;
}

static int parse_user_password(const char *s, size_t s_len, URL_COMPONENTS *c)
{
	const char *end = s + s_len;
	const char *found;

	found = strnstr(s, s_len, ":");
	if (found) {
		c->user = strndup(s, found - s);
		if (c->user == NULL) {
			return -1;	/* ENOMEM */
		}
		c->password = strndup(found + 1, end - found - 1);
		if (c->password == NULL) {
			return -1;	/* ENOMEM */
		}
	} else {
		c->user = strndup(s, s_len);
		if (c->user == NULL) {
			return -1;	/* ENOMEM */
		}
	}

	return 0;
}

static int parse_authority(const char *s, size_t s_len, URL_COMPONENTS *c)
{
	const char *end = s + s_len;
	const char *p, *found, *host_start, *host_end;
	int value;

	c->port = -1;

	if (s_len == 0) {	/* empty authority */
		return 0;
	}

	found = strnstr(s, s_len, "@");
	if (found) {
		if (parse_user_password(s, found - s, c) == -1) {
			return -1;
		}

		host_start = found + 1;
	} else {
		host_start = s;
	}

	if (*host_start == '[') {
		/* IP-literal host */
		if (find_chars(host_start + 1, end - host_start - 1, "[")) {
			errno = EINVAL;
			return -1;
		}
		host_end = find_chars(host_start + 1, end - host_start - 1, "]");
		if (!host_end) {
			errno = EINVAL;
			return -1;
		}
		/* The next character of ']' is termination or ':'. */
		if (host_end + 1 != end && host_end[1] != ':') {
			errno = EINVAL;
			return -1;
		}
		host_end++;
	} else {
		/* IPv4address / reg-name host */
		host_end = find_chars_reverse(host_start, end - host_start, ":");
		if (host_end == NULL) {
			host_end = end;
		}
		if (find_chars(host_start, host_end - host_start, "[]")) {
			errno = EINVAL;
			return -1;
		}
	}
	if (find_chars(host_start, host_end - host_start, " \r\n")) {
		errno = EINVAL;
		return -1;
	}

	/* ASSERT: host_end == end or *host_end == ':' */

	if (host_end == end) {
		/* without port number */
		if (host_start == end) {	/* empty host */
			errno = EINVAL;
			return -1;
		}
		c->host = strndup(host_start, end - host_start);
		if (c->host == NULL) {
			return -1;	/* ENOMEM */
		}
		return 0;
	}

	/* ASSERT: *host_end == ':' */

	/* host and port */

	if (host_start == host_end) {	/* empty host */
		errno = EINVAL;
		return -1;
	}

	if (host_end + 1 < end) {
		p = host_end + 1;
		while (p < end) {
			if (*p < '0' || *p > '9') {
				errno = EINVAL;
				return -1;
			}
			p++;
		}
		value = atoi(host_end + 1);
		if (value < 0 || value > 65535) {
			errno = EINVAL;
			return -1;
		}
	} else {
		/* empty port number */
		value = -1;
	}

	c->host = strndup(host_start, (size_t) (host_end - host_start));
	if (c->host == NULL) {
		return -1;	/* ENOMEM */
	}
	c->port = value;

	return 0;
}

URL_COMPONENTS *parse_url(const char *url)
{
	URL_COMPONENTS *c;
	const char *p;
	const char *end = url + strlen(url);
	const char *found;
	size_t len;

	c = malloc(sizeof(URL_COMPONENTS));
	if (!c) {
		return NULL;
	}
	memset(c, 0, sizeof(URL_COMPONENTS));
	c->port = -1;

	p = url;

	/* lookup scheme */
	found = lookup_scheme(p);
	if (found) {
		c->scheme = strndup(url, (size_t) (found - p));
		if (c->scheme == NULL) {
			goto error;
		}
		p = found + 1;	/* skip a colon */
		if (p >= end) {
			return c;
		}
	}

	if (strlen(p) >= 2 &&
	    p[0] == '/' && p[1] == '/') {
		/* authority */
		p = p + 2;
		found = find_chars(p, strlen(p), "/?#");
		if (found == NULL) {
			len = strlen(p);
		} else {
			len = (size_t) (found - p);
		}
		if (parse_authority(p, len, c) == -1) {
			goto error;	/* ENOMEM,EINVAL */
		}

		if (!found) {
			return c;
		}

		p = found;
	}

	if (*p != '?' && *p != '#') {
		/* path */
		found = find_chars(p, strlen(p), "?#");
		if (found == NULL) {
			c->path = strdup(p);
			if (c->path == NULL) {
				goto error;
			}
		} else {
			if (found != p) {
				c->path = strndup(p,  (size_t) (found - p));
				if (c->path == NULL) {
					goto error;
				}
			}
		}

		if (!found) {
			return c;
		}

		p = found;
	}

	/* ASSERT: *p is '?' or '#' */

	if (*p == '?') {
		/* query */
		p = p + 1;
		found = find_chars(p, strlen(p), "#");
		if (found == NULL) {
			c->query = strdup(p);
		} else {
			c->query = strndup(p,  (size_t) (found - p));
		}

		if (c->query == NULL) {
			goto error;
		}

		if (!found) {
			return c;
		}

		p = found;
	}

	/* ASSERT: *p is '#' */

	/* fragment */
	p = p + 1;
	c->fragment = strdup(p);
	if (c->fragment == NULL) {
		goto error;
	}

	return c;

error:
	free(c);

	return NULL;
}

void free_url_components(URL_COMPONENTS *c)
{
	if (c->scheme) {
		free(c->scheme);
	}
	if (c->user) {
		free(c->user);
	}
	if (c->password) {
		free(c->password);
	}
	if (c->host) {
		free(c->host);
	}
	if (c->path) {
		free(c->path);
	}
	if (c->query) {
		free(c->query);
	}
	if (c->fragment) {
		free(c->fragment);
	}
	free(c);
}

