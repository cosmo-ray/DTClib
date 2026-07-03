#include <ctype.h>
#include <string.h>

#define DTC_KCAT(a,b,c) a##b##c

#define DTC_FNAME(a,b,c) DTC_KCAT(a,b,c)

#ifndef DTC_COOL_FUNCTION
#define DTC_COOL_FUNCTION

#define DTC_DIE(label, errstorage, args...) do {	\
		sprintf(dtc_err_buff, args);		\
		errstorage->err = dtc_err_buff;		\
		goto label;				\
	} while (0)

#define DTC_SKIP(s, what, errptr) do {				\
		if (dtc_pre_next(s, errptr) != what)		\
			DTC_DIE(err, errptr, "%c required", what);	\
	} while (0)

struct dtc_error_ctx {
	int line;
	int col;
	char *err;
};

char dtc_err_buff[1024 * 8];

static int dtc_str_eq_nn(size_t l, char s[static l],
			 size_t to_chk_l, char to_check[static to_chk_l])
{
	if (to_chk_l != l)
		return 0;
	return !memcmp(s, to_check, l);
}

static int dtc_is_void_elem(size_t el_name_l, char el_name[static el_name_l])
{
	if (dtc_str_eq_nn(el_name_l, el_name, sizeof "br" - 1, "br"))
		return 1;
	if (dtc_str_eq_nn(el_name_l, el_name, sizeof "area" - 1, "area"))
		return 1;
	if (dtc_str_eq_nn(el_name_l, el_name, sizeof "base" - 1, "base"))
		return 1;
	if (dtc_str_eq_nn(el_name_l, el_name, sizeof "col" - 1, "col"))
		return 1;
	if (dtc_str_eq_nn(el_name_l, el_name, sizeof "embed" - 1, "embed"))
		return 1;
	if (dtc_str_eq_nn(el_name_l, el_name, sizeof "hr" - 1, "hr"))
		return 1;
	if (dtc_str_eq_nn(el_name_l, el_name, sizeof "img" - 1, "img"))
		return 1;
	if (dtc_str_eq_nn(el_name_l, el_name, sizeof "input" - 1, "input"))
		return 1;
	if (dtc_str_eq_nn(el_name_l, el_name, sizeof "link" - 1, "link"))
		return 1;
	if (dtc_str_eq_nn(el_name_l, el_name, sizeof "meta" - 1, "meta"))
		return 1;
	if (dtc_str_eq_nn(el_name_l, el_name, sizeof "source" - 1, "source"))
		return 1;
	if (dtc_str_eq_nn(el_name_l, el_name, sizeof "track" - 1, "track"))
		return 1;
	if (dtc_str_eq_nn(el_name_l, el_name, sizeof "wbr" - 1, "wbr"))
		return 1;
	return 0;
}

static void dtc_next(char **s, struct dtc_error_ctx *errptr)
{
	if (**s == '\n') {
		errptr->line++;
		errptr->col = 0;
	} else {
		errptr->col++;
	}
	++*s;
}

/* strore cur char in r, advance s pointer, if not \0 */
static inline int dtc_pre_next(char **s, struct dtc_error_ctx *errptr)
{
	int r = **s;
	if (!r)
		return r;
	dtc_next(s, errptr);
	return r;
}

static inline int dtc_skip_str(char **s, struct dtc_error_ctx *errptr)
{
	DTC_SKIP(s, '"', errptr);
	while (**s != '"') {
		if (!**s) {
			DTC_DIE(err, errptr, "unclose str");
		}
		dtc_next(s, errptr);
	}
	return 0;
err:
	return -1;
}

static inline int dtc_skip_name_chars(char **s, struct dtc_error_ctx *errptr)
{
	int cur = **s;
	int r = 0;
	while(isalnum(cur) || cur == '-' || cur == '_') {
		dtc_next(s, errptr);
		cur = **s;
		r++;
	}
	return r;
}

static inline int dtc_skip_name(char **s, struct dtc_error_ctx *errptr)
{
	if (!isalpha(dtc_pre_next(s, errptr))) {
		return -1;
	}
	return dtc_skip_name_chars(s, errptr);
}


static inline char *dtc_skip_blank(char *s, struct dtc_error_ctx *errptr)
{
	while (isblank(*s))
		dtc_pre_next(&s, errptr);
	return s;
}

static inline char *dtc_skip_blank_n_return(char *s, struct dtc_error_ctx *errptr)
{
	while (isblank(*s) || *s == '\n')
		dtc_pre_next(&s, errptr);
	return s;
}


#endif  /* DTC_COOL_FUNCTION */


int DTC_FNAME(dtc_, DTCLIB_PREFIX, _parse_int)(char **html, DTC_PTR parent_array,
					       char **tag_end,
					       struct dtc_error_ctx *errptr)
{

again:
	*html = dtc_skip_blank_n_return(*html, errptr);
	if (**html == '<') {
		char *tag_name = NULL;
		size_t tag_name_l = 0;
		if ((*html)[1] == '/') {
			if (!tag_end) {
				DTC_DIE(err, errptr, "closing tag shouldn't be present here\n");
			}
			*html += 2; /*</*/
			*html = dtc_skip_blank(*html, errptr);
			*tag_end = *html;
			for (;**html != '>'; dtc_pre_next(html, errptr)) {
				if (!**html) {
					DTC_DIE(err, errptr, "bah t'est mort\n");
				}
			}
			return 0;

		}
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
				tag_name = *html;
				tag_name_l = walker - *html;
				DTC_TAG_NAME(cur, *html, walker - *html);
				if (dtc_str_eq_nn(walker - *html, *html,
						  sizeof "!doctype" - 1, "!doctype")) {
					*html = dtc_skip_blank(walker, errptr);
					walker = *html;
					while (walker && *walker && *walker != '>')
						dtc_pre_next(&walker, errptr);
					if (!walker || !*walker)
						DTC_DIE(err, errptr, "unclose doctype");
					DTC_STORE_STRL_KEY(cur, "content", walker - *html, *html);
					*html = walker + 1;
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
			if (dtc_skip_name(html, errptr) < 0) {
				DTC_DIE(err, errptr, "atribute name require");
			}
			name_l = *html - name;
			DTC_SKIP(html, '=', errptr);
			value = *html;
			if (dtc_skip_str(html, errptr) < 0) {
				DTC_DIE(err, errptr, "atribute name require");
			}
			DTC_STORE_STRL_KEYL(attribute, name_l, name, *html - value, value);
			if (isblank(**html)) {
				goto anew_attribute;
			}


		}
		DTC_SKIP(html, '>', errptr);
		if (dtc_is_void_elem(tag_name_l, tag_name))
			return 0;
		char *end = NULL;
		DTC_PTR content = DTC_STORE_ARRAY(cur, "content");
		int rec_ret;

	not_close_yet:
		rec_ret = DTC_FNAME(dtc_, DTCLIB_PREFIX, _parse_int)(html, content,
								     &end, errptr);
		if (!end)
			goto not_close_yet;
		if (dtc_str_eq_nn(tag_name_l, tag_name, *html - end, end)) {
			DTC_DIE(err, errptr, "trying to sloce the wrong tag :(\n");
		}
		DTC_SKIP(html, '>', errptr);
	} else {
		char *walker;
		for (walker = *html; *walker != '<' && *walker; dtc_pre_next(&walker, errptr));
		DTC_PUSH_STRL(parent_array, walker - *html - 1, *html);
		*html = walker;
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
	if (DTC_FNAME(dtc_, DTCLIB_PREFIX, _parse_int)(&html, ret, NULL, errptr) < 0) {
		DTC_FREE(ret);
		return NULL;
	}
	return ret;
}
