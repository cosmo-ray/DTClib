#include <ctype.h>
#include <string.h>

#define DTC_KCAT(a,b,c) a##b##c

#define DTC_FNAME(a,b,c) DTC_KCAT(a,b,c)

#ifndef DTC_COOL_FUNCTION
#define DTC_COOL_FUNCTION

struct dtc_error_ctx {
	int line;
	int col;
	char *err;
};

char dtc_err_buff[1024 * 8];

static int dtc_check_tag_name(size_t l, char s[static l],
			      size_t to_chk_l, char to_check[static to_chk_l])
{
	if (to_chk_l != l)
		return 0;
	return !memcmp(s, to_check, l);
}

/* strore cur char in r, advance s pointer, if not \0 */
static inline int dtc_pre_next(char **s, struct dtc_error_ctx *errptr)
{
	int r = **s;
	if (!r)
		return r;
	if (**s == '\n') {
		errptr->line++;
		errptr->col = 0;
	} else {
		errptr->col++;
	}
	++*s;
	return r;
}

static inline int dtc_skip_alplanum(char **s, struct dtc_error_ctx *errptr)
{
	int r = 0;
	while(isalnum(dtc_pre_next(s, errptr)))
		r++;
	return r;
}

static inline int dtc_skip_name(char **s, struct dtc_error_ctx *errptr)
{
	if (!isalpha(dtc_pre_next(s, errptr))) {
		return -1;
	}
	return dtc_skip_alplanum(s, errptr);
}


static inline char *dtc_skip_blank(char *s, struct dtc_error_ctx *errptr)
{
	while (isblank(*s))
		dtc_pre_next(&s, errptr);
	return s;
}

#endif  /* DTC_COOL_FUNCTION */


#define DTC_DIE(label, errstorage, args...) do {	\
		sprintf(dtc_err_buff, args);		\
		errstorage->err = dtc_err_buff;		\
		goto label;				\
	} while (0)

#define DTC_SKIP(s, what, errptr) do {				\
		if (dtc_pre_next(s, errptr) != what)		\
			DTC_DIE(err, errptr, "%c required", what);	\
	} while (0)

int DTC_FNAME(dtc_, DTCLIB_PREFIX, _parse_int)(char **html, DTC_PTR parent_array,
						   struct dtc_error_ctx *errptr)
{

again:
	if (**html == '<') {
		DTC_PTR cur = DTC_NEW_OBJECT(parent_array);
		if (DTC_IS_NULL(cur))
			goto err;
		*html += 1;
		*html = dtc_skip_blank(*html, errptr);
		if (!*html) {
			DTC_DIE(err, errptr, "early end");
		}
		int have_atribute = 0;
		for (char *walker = *html; *walker; dtc_pre_next(&walker, errptr)) {
			if (*walker == '>' || isblank(*walker)) {
				DTC_TAG_NAME(cur, *html, walker - *html);
				if (dtc_check_tag_name(walker - *html, *html,
						       sizeof "!doctype" - 1, "!doctype")) {
					*html = dtc_skip_blank(walker, errptr);
					walker = *html;
					while (walker && *walker && *walker != '>')
						dtc_pre_next(&walker, errptr);
					if (!walker || !*walker)
						DTC_DIE(err, errptr, "unclose doctype");
					DTC_STORE_STRL_KEY(cur, "content", walker - *html, *html);
					*html = walker;
					goto again;
				}
				have_atribute = isblank(*walker);
				*html = walker;
				goto tag_ok;
			}
		}
		goto err;
	tag_ok:
		if (have_atribute) {
			char *name;
			char *value;
			size_t name_l;
			DTC_PTR attribute = DTC_NEW_OBJECT_OBJECT(cur, "attributes");
		anew_attribute:
			*html = dtc_skip_blank(*html, errptr);
			name = *html;
			name_l = *html - name;
			if (dtc_skip_name(html, errptr) < 0) {
				DTC_DIE(err, errptr, "atribute name require");
			}
			DTC_SKIP(html, '=', errptr);
			value = *html;
			if (!dtc_skip_alplanum(html, errptr)) {
				DTC_DIE(err, errptr, "atribute name require");
			}
			DTC_STORE_STRL_KEYL(attribute, name_l, name, *html - value, value);
			if (isblank(**html)) {
				goto anew_attribute;
			}
		}
	}
	// look if in walker == '<'
	//   parse ball, till > or ','
	//   push elem into ret
	//   if in ',', push attribute
	//   new array contant
	// else in paragraph
	return 0;
err:
	return -1;
}

DTC_PTR DTC_FNAME(dtc_, DTCLIB_PREFIX, _parse)(char html[static 1],
					       struct dtc_error_ctx *errptr)
{
	DTC_PTR ret = DTC_NEW_ARRAY();
	if (ret < 0)
		return NULL;
	printf(html);
	if (DTC_FNAME(dtc_, DTCLIB_PREFIX, _parse_int)(&html, ret, errptr) < 0) {
		DTC_FREE(ret);
		return NULL;
	}
	return ret;
}
