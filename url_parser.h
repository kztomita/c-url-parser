#ifndef URL_PARSER_H
#define URL_PARSER_H

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

extern URL_COMPONENTS *parse_url(const char *url);
extern void free_url_components(URL_COMPONENTS *c);

#endif
