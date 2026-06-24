#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <dtc-json-c.h>

int main(int ac, char **av)
{
	char tmp[1024];
	int rret;
	char *file = NULL;
	size_t off = 0;
	int fd = 0;
	struct dtc_error_ctx errctx = {};

	for (int i = 1; i < ac; ++i) {
		if (!strcmp(av[i], "-f") || !strcmp(av[i], "--file")) {
			if (i + 1 == ac) {
				fprintf(stderr, "-f need an agument\n");
				return 1;
			}
			++i;
			fd = open(av[i], O_RDONLY);
		}
	}

	while (rret = read(fd, tmp, 1024)) {
		if (rret < 0) {
			fprintf(stderr, "read fail");
			return 1;
		}
		file = realloc(file, off + rret);
		memcpy(file + off, tmp, rret);
		off += rret;
	}
	struct json_object *jret = dtc_json_c_parse(file, &errctx);
	if (!jret)
		fprintf(stderr, "error at line %d col %d: %s\n", errctx.line,
			errctx.col,
			errctx.err);
	printf("%s",
	       json_object_to_json_string_ext(jret, JSON_C_TO_STRING_PRETTY |
					      JSON_C_TO_STRING_NOSLASHESCAPE));
}
