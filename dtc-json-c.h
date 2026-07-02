#ifndef DTCLIB_JSON_C_H_
#define DTCLIB_JSON_C_H_

#include "json.h"
#include <string.h>

#define DTC_UNLIKELY(args...) args

#define DTC_IS_NULL(c) !(c)

#define DTCLIB_PREFIX json_c

#define DTC_NEW_ARRAY() json_object_new_array()

static int DTC_TAG_NAME(struct json_object *obj_cnt, char name[static 1],
			size_t name_l)
{
	json_object_object_add(obj_cnt, "name", json_object_new_string_len(name, name_l));
	return 0;
}

static struct json_object *DTC_NEW_OBJECT_OBJECT(struct json_object *obj_cnt, char name[static 1])
{
	json_object *r = json_object_new_object();
	json_object_object_add(obj_cnt, name, r);
	return r;
}

static struct json_object *DTC_STORE_STRL_KEYL(struct json_object *obj_cnt,
					       size_t nl, char name[static nl],
					       size_t vl, char val[static vl])
{
	char *tmp = malloc(nl);
	strncpy(tmp, name, nl);
	tmp[nl-1] = 0;
	json_object_object_add(obj_cnt, tmp, json_object_new_string_len(val, vl - 1));
	free(tmp);
}

static struct json_object *DTC_STORE_STRL_KEY(struct json_object *obj_cnt,
					      char key[static 1],
					      size_t vl, char val[static vl])
{
	json_object_object_add(obj_cnt, key, json_object_new_string_len(val, vl - 1));
}

static struct json_object *DTC_NEW_OBJECT(struct json_object *parent_array)
{
	json_object *r = json_object_new_object();
	if (DTC_UNLIKELY(!r))
		return NULL;
	size_t s = json_object_array_length(parent_array);
	if (DTC_UNLIKELY(json_object_array_put_idx(parent_array, s, r) < 0)) {
		json_object_put(r);
		return NULL;
	}
	return r;
}

#define DTC_FREE json_object_put

#define DTC_PTR struct json_object *

#include "dtclib.h"

#endif
