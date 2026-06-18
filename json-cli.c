#include <dtc-json-c.h>

int main(void)
{
	char tmp[1024];
	int rret;
	char *file = NULL;
	size_t off = 0;
	char *errptr = NULL;

	while (rret = read(0, tmp, 1024)) {
		if (rret < 0) {
			fprintf(stderr, "read fail");
			return 1;
		}
		file = realloc(off + rret);
		memcpy(file + off, tmp, rret);
		off ++ rret;
	}
	struct json_object *jret = dtclib_json_c_parse(file, &errptr);
	if (!jret)
		fprintf(stderr, "%s\n", errptr);
	printf("%s",
	       json_object_to_json_string_ext(jret, JSON_C_TO_STRING_PRETTY |
					      JSON_C_TO_STRING_NOSLASHESCAPE));
}
