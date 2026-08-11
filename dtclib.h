#include <ctype.h>
#include <string.h>

#ifdef DTC_NO_THREAD
#define THREAD_LOCAL
#else
#define THREAD_LOCAL thread_local
#endif

#define DTC_KCAT(a,b,c) a##b##c

#define DTC_FNAME(a,b,c) DTC_KCAT(a,b,c)

#ifndef DTC_COOL_FUNCTION
#define DTC_COOL_FUNCTION

#define DTC_ERROR_BUF_SIZE (1024 * 8)

#define DTC_DIE(label, errstorage, args...) do {	\
		snprintf(dtc_err_buff, DTC_ERROR_BUF_SIZE, args);	\
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

#define DTC_HTML_TAGS \
	DTC_TOK(a)				\
	DTC_TOK(abbr)				\
	DTC_TOK(address)			\
	DTC_TOK(area)				\
	DTC_TOK(article)			\
	DTC_TOK(aside)				\
	DTC_TOK(audio)				\
	DTC_TOK(b)				\
	DTC_TOK(base)				\
	DTC_TOK(bdi)				\
	DTC_TOK(bdo)				\
	DTC_TOK(blockquote)			\
	DTC_TOK(body)				\
	DTC_TOK(br)				\
	DTC_TOK(button)				\
	DTC_TOK(canvas)				\
	DTC_TOK(caption)			\
	DTC_TOK(cite)				\
	DTC_TOK(code)				\
	DTC_TOK(col)				\
	DTC_TOK(colgroup)			\
	DTC_TOK(data)				\
	DTC_TOK(datalist)			\
	DTC_TOK(dd)				\
	DTC_TOK(del)				\
	DTC_TOK(details)			\
	DTC_TOK(dfn)				\
	DTC_TOK(dialog)				\
	DTC_TOK(div)				\
	DTC_TOK(dl)				\
	DTC_TOK(dt)				\
	DTC_TOK(em)				\
	DTC_TOK(embed)				\
	DTC_TOK(fieldset)			\
	DTC_TOK(figcaption)			\
	DTC_TOK(figure)				\
	DTC_TOK(footer)				\
	DTC_TOK(form)				\
	DTC_TOK(h1)				\
	DTC_TOK(h2)				\
	DTC_TOK(h3)				\
	DTC_TOK(h4)				\
	DTC_TOK(h5)				\
	DTC_TOK(h6)				\
	DTC_TOK(head)				\
	DTC_TOK(header)				\
	DTC_TOK(hgroup)				\
	DTC_TOK(hr)				\
	DTC_TOK(html)				\
	DTC_TOK(i)				\
	DTC_TOK(iframe)				\
	DTC_TOK(img)				\
	DTC_TOK(input)				\
	DTC_TOK(ins)				\
	DTC_TOK(kbd)				\
	DTC_TOK(label)				\
	DTC_TOK(legend)				\
	DTC_TOK(li)				\
	DTC_TOK(link)				\
	DTC_TOK(main)				\
	DTC_TOK(map)				\
	DTC_TOK(mark)				\
	DTC_TOK(menu)				\
	DTC_TOK(meta)				\
	DTC_TOK(meter)				\
	DTC_TOK(nav)				\
	DTC_TOK(noscript)			\
	DTC_TOK(object)				\
	DTC_TOK(ol)				\
	DTC_TOK(optgroup)			\
	DTC_TOK(option)				\
	DTC_TOK(output)				\
	DTC_TOK(p)				\
	DTC_TOK(picture)			\
	DTC_TOK(pre)				\
	DTC_TOK(progress)			\
	DTC_TOK(q)				\
	DTC_TOK(rp)				\
	DTC_TOK(rt)				\
	DTC_TOK(ruby)				\
	DTC_TOK(s)				\
	DTC_TOK(samp)				\
	DTC_TOK(script)				\
	DTC_TOK(search)				\
	DTC_TOK(section)			\
	DTC_TOK(select)				\
	DTC_TOK(slot)				\
	DTC_TOK(small)				\
	DTC_TOK(source)				\
	DTC_TOK(span)				\
	DTC_TOK(strong)				\
	DTC_TOK(style)				\
	DTC_TOK(sub)				\
	DTC_TOK(summary)			\
	DTC_TOK(sup)				\
	DTC_TOK(table)				\
	DTC_TOK(tbody)				\
	DTC_TOK(td)				\
	DTC_TOK(template)			\
	DTC_TOK(textarea)			\
	DTC_TOK(tfoot)				\
	DTC_TOK(th)				\
	DTC_TOK(thead)				\
	DTC_TOK(time)				\
	DTC_TOK(title)				\
	DTC_TOK(tr)				\
	DTC_TOK(track)				\
	DTC_TOK(u)				\
	DTC_TOK(ul)				\
	DTC_TOK(var)				\
	DTC_TOK(video)				\
	DTC_TOK(wbr)

#define DTC_TOK(x) DTC_HTML_##x,
enum dtc_html_tag {
	DTC_HTML_TAGS
	DTC_HTML_TAG_END
};
#undef DTC_TOK

#define DTC_TOK(x) #x,
static const char *dtc_html_tag_names[] = {
	DTC_HTML_TAGS
	NULL
};
#undef DTC_TOK

#define DTC_TOK(x) sizeof #x - 1,
static size_t dtc_html_tag_names_l[] = {
	DTC_HTML_TAGS
	0
};
#undef DTC_TOK

THREAD_LOCAL static char dtc_err_buff[DTC_ERROR_BUF_SIZE];

static int dtc_str_eq_nn(size_t l, const char s[static l],
			 size_t to_chk_l, const char to_check[static to_chk_l])
{
	if (to_chk_l != l)
		return 0;
	return !strncasecmp(s, to_check, l);
}

static int dtc_is_void_elem(int tag)
{
	if (tag == DTC_HTML_br)
		return 1;
	if (tag == DTC_HTML_area)
		return 1;
	if (tag == DTC_HTML_base)
		return 1;
	if (tag == DTC_HTML_col)
		return 1;
	if (tag == DTC_HTML_embed)
		return 1;
	if (tag == DTC_HTML_hr)
		return 1;
	if (tag == DTC_HTML_img)
		return 1;
	if (tag == DTC_HTML_input)
		return 1;
	if (tag == DTC_HTML_link)
		return 1;
	if (tag == DTC_HTML_meta)
		return 1;
	if (tag == DTC_HTML_source)
		return 1;
	if (tag == DTC_HTML_track)
		return 1;
	if (tag == DTC_HTML_wbr)
		return 1;
	return 0;
}

int dtc_html_tag(size_t tag_name_l, char tag_name[static tag_name_l])
{
	/* cast in int for C++ compat, maybe someday */
	for (int i = 0; i < (int)DTC_HTML_TAG_END; ++i) {
		if (dtc_str_eq_nn(tag_name_l, tag_name,
				  dtc_html_tag_names_l[i], dtc_html_tag_names[i])) {
			return i;
		}
	}
	return -1;
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
	dtc_next(s, errptr); /* skip closing " */
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
	char *in_tmp;
	struct dtc_error_ctx errbak = *errptr;

again:
	*html = dtc_skip_blank_n_return(*html, errptr);
	in_tmp = *html;
	if (**html == '<') {
		char *tag_name = NULL;
		size_t tag_name_l = 0;
		int what_tag;

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
		if (!*html) {
			DTC_DIE(err, errptr, "early end");
		}

		*html += 1;
		*html = dtc_skip_blank(*html, errptr);

		char *walker;
		int have_atribute = 0;
		for (walker = *html; *walker; dtc_pre_next(&walker, errptr)) {
			if (*walker == '>' || isblank(*walker))
				goto tag_ok;
		}
		goto err;
	tag_ok:
		tag_name = *html;
		tag_name_l = walker - *html;
		DTC_TAG_NAME(cur, *html, tag_name_l);
		if (dtc_str_eq_nn(tag_name_l, *html,
				  sizeof "!doctype" - 1, "!doctype")) {
			*html = dtc_skip_blank(walker, errptr);
			walker = *html;
			while (walker && *walker && *walker != '>')
				dtc_pre_next(&walker, errptr);
			if (!walker || !*walker)
				DTC_DIE(err, errptr, "unclose doctype");
			DTC_STORE_STRL_KEY(cur, "content", walker - *html + 1, *html);
			*html = walker + 1;
			goto again;
		}
		what_tag = dtc_html_tag(tag_name_l, tag_name);
		if (what_tag < 0) {
			*errptr = errbak;
			*html = in_tmp;
			printf("go to not a tag\n");
			goto not_a_tag;
		}
		have_atribute = isblank(*walker);
		*html = walker;

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
			value = *html + 1;
			if (dtc_skip_str(html, errptr) < 0) {
				DTC_DIE(err, errptr, "atribute name require");
			}
			DTC_STORE_STRL_KEYL(attribute, name_l, name, *html - value, value);
			if (isblank(**html)) {
				goto anew_attribute;
			}


		}
		DTC_SKIP(html, '>', errptr);
		if (dtc_is_void_elem(what_tag))
			return 0;
		char *end = NULL;
		DTC_PTR content = DTC_STORE_ARRAY(cur, "content");
		int rec_ret;

	not_close_yet:
		rec_ret = DTC_FNAME(dtc_, DTCLIB_PREFIX, _parse_int)(html, content,
								     &end, errptr);
		if (!end)
			goto not_close_yet;
		if (!dtc_str_eq_nn(tag_name_l, tag_name, *html - end, end)) {
			DTC_DIE(err, errptr, "trying to close the wrong tag got '%*.s' instead of '%.*s' :(\n", (int)(*html - end), end, (int)tag_name_l, tag_name);
		}
		DTC_SKIP(html, '>', errptr);
	} else {
		char *walker;
	not_a_tag:
		for (walker = *html; *walker; dtc_pre_next(&walker, errptr)) {
			if (*walker == '<') {
				errbak = *errptr;
				char *tmp = dtc_skip_blank(walker + 1, errptr);

				if (*tmp == '/')
					dtc_next(&tmp, errptr);
				int len = 1;
				while (tmp[len] && !isblank(tmp[len]) &&tmp[len] != '>')
					++len;
				if (dtc_html_tag(len, tmp) >= 0) {
					*errptr = errbak;
					break;
				}
				walker = tmp - 1;
			}
		}
		DTC_PUSH_STRL(parent_array, walker - *html + 1, *html);
		*html = walker;
	}
	return 0;
err:
	return -1;
}

DTC_PTR DTC_FNAME(dtc_, DTCLIB_PREFIX, _parse)(char html[static 1],
					       struct dtc_error_ctx *errptr)
{
	DTC_PTR ret = DTC_NEW_ARRAY();
	if (DTC_IS_NULL(ret))
		return DTC_NULL;
	if (DTC_FNAME(dtc_, DTCLIB_PREFIX, _parse_int)(&html, ret, NULL, errptr) < 0) {
		DTC_FREE(ret);
		return DTC_NULL;
	}
	return ret;
}
