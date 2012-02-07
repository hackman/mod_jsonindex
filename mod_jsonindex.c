/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * mod_jsonindex.c: Handles the on-the-fly JSON index generation
 * 
 * Marian Marinov <mm@yuhu.biz>
 * 11.Dec.2011
 * 
 */

#include "mod_jsonindex.h"

#ifdef APACHE2
module AP_MODULE_DECLARE_DATA jsonindex_module;
#else
module MODULE_VAR_EXPORT jsonindex_module;
#endif



static int handle_jsonindex(request_rec *r) {
    r->allowed |= (1 << M_GET);
	if (r->method_number != M_GET) {
		return DECLINED;
    }
#ifdef APACHE2
	if ( strcmp(r->handler, "httpd/json-directory") != 0 && strcmp(r->handler, "json-directory") == 0 ) {
		return DECLINED;
	}
	apr_finfo_t item;
	apr_dir_t *dir;
	apr_status_t status;
#else
    DIR *dir;
    struct dirent *item;
#endif

	int allow_opts = ap_allow_options(r);
	int not_start = 0;
	int pretty = 0;
	int simple = 0;
	char buf[1022];

    /* OK, nothing easy.  Trot out the heavy artillery... */

    if (!(allow_opts & OPT_INDEXES)) {
#ifdef APACHE2
		ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, "Directory index forbidden by rule: %s", r->filename);
#else
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, r, "Directory index forbidden by rule: %s", r->filename);
#endif
		return HTTP_FORBIDDEN;
    }
	if (r->filename[strlen(r->filename) - 1] != '/') {
#ifdef APACHE2
	    r->filename = apr_pstrcat(r->pool, r->filename, "/", NULL);
#else
	    r->filename = ap_pstrcat(r->pool, r->filename, "/", NULL);
#endif
	}

#ifdef APACHE2
	if (apr_dir_open(&dir, r->filename, r->pool) != APR_SUCCESS) {
		ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "Can't open directory for index: %s", r->filename);
#else
	if (!(dir = ap_popendir(r->pool, r->filename))) {
		ap_log_rerror(APLOG_MARK, APLOG_ERR, r, "Can't open directory for index: %s", r->filename);
#endif
		return HTTP_FORBIDDEN;
	}

	r->content_type = "text/plain";
#ifndef APACHE2
	ap_send_http_header(r);
	ap_hard_timeout("send directory", r);
#endif
	if (r->header_only) {
#ifndef APACHE2
		ap_kill_timeout(r);
#endif
		return OK;
	}

	if (r->args && strstr(r->args, "pretty") != NULL)
		pretty = 1;
	if (r->args && strstr(r->args, "simple") != NULL)
		simple = 1;
    
	if (simple) {
		if (pretty)
			ap_rputs("[\n", r);
		else
			ap_rputs("[", r);
	} else {
		if (pretty)
			ap_rputs("{\n", r);
		else
			ap_rputs("{", r);
	}

	while(1) {
#ifdef APACHE2
		status = apr_dir_read(&item, APR_FINFO_MIN | APR_FINFO_NAME, dir);
		if (APR_STATUS_IS_INCOMPLETE(status)) {
			continue; /* ignore un-stat()able files */
		} else if (status != APR_SUCCESS) {
			break;
		}

		if (strcmp(item.name, ".") == 0 || strcmp(item.name, "..") == 0) {
			// I should add skipping of AccessFileName
			// and if possible the files that are protected with deny from all
			continue;       // skip . and .. directories
		}
#else
		errno = 0;
		item = readdir(dir);
		if (item == NULL)
			break;  // end of the dir

		if (strcmp(item->d_name, ".") == 0 || strcmp(item->d_name, "..") == 0) {
			// I should add skipping of AccessFileName
			// and if possible the files that are protected with deny from all
			continue;       // skip . and .. directories
		}
#endif



		if (not_start) {
			ap_rputs(",", r);
			if (pretty)
				ap_rputs("\n", r);
		} else {
			not_start = 1;
		}

		if (pretty)
#ifdef APACHE2
			if (simple) {
				if (item.filetype == APR_DIR) {
					ap_rprintf(r, " [ \"%s\", 1 ]", item.name);
				} else {
					ap_rprintf(r, " [ \"%s\", 0 ]", item.name);
				}
			} else {
				if (item.filetype == APR_DIR) {
					ap_rprintf(r, "  \"%s\" : [ 1, \"\" ]", item.name);
				} else if (item.filetype == APR_LNK) {
					memset(&buf, '\0', 1022);
					readlink(item.name, buf, 1022);
					ap_rprintf(r, "  \"%s\" : [ 2, \"%s\" ]", item.name, buf);
				} else {
					ap_rprintf(r, "  \"%s\" : [ 0, \"\" ]", item.name);
				}
			}
		else
			if (simple) {
				if (item.filetype == APR_DIR) {
					ap_rprintf(r, "[\"%s\",1]", item.name);
				} else {
					ap_rprintf(r, "[\"%s\",0]", item.name);
				}
			} else {
				if (item.filetype == APR_DIR) {
					ap_rprintf(r, "\"%s\":[1,\"\"]", item.name);
				} else if (item.filetype == APR_LNK) {
					memset(&buf, '\0', 1022);
					readlink(item.name, buf, 1022);
					ap_rprintf(r, "\"%s\":[2,\"%s\"]", item.name, buf);
				} else {
					ap_rprintf(r, "\"%s\":[0,\"\"]", item.name);
				}
			}
#else
			if (simple) {
				if (item->d_type == DT_DIR) {
					ap_rprintf(r, " [ \"%s\", 1 ]", item->d_name);
				} else {
					ap_rprintf(r, " [ \"%s\", 0 ]", item->d_name);
				}
			} else {
				if (item->d_type == DT_DIR) {
					ap_rprintf(r, "  \"%s\" : [ 1, \"\" ]", item->d_name);
				} else if (item->d_type == DT_LNK) {
					memset(&buf, '\0', 1022);
					readlink(item->d_name, buf, 1022);
					ap_rprintf(r, "  \"%s\" : [ 2, \"%s\" ]", item->d_name, buf);
				} else {
					ap_rprintf(r, "  \"%s\" : [ 0, \"\" ]", item->d_name);
				}
			}
		else
			if (simple) {
				if (item->d_type == DT_DIR) {
					ap_rprintf(r, "[\"%s\",1]", item->d_name);
				} else {
					ap_rprintf(r, "[\"%s\",0]", item->d_name);
				}
			} else {
				if (item->d_type == DT_DIR) {
					ap_rprintf(r, "\"%s\":[1,\"\"]", item->d_name);
				} else if (item->d_type == DT_LNK) {
					memset(&buf, '\0', 1022);
					readlink(item->d_name, buf, 1022);
					ap_rprintf(r, "\"%s\":[2,\"%s\"]", item->d_name, buf);
				} else {
					ap_rprintf(r, "\"%s\":[0,\"\"]", item->d_name);
				}
			}
#endif // APACHE2
	} // end of the while()
	if (simple) {
		if (pretty)
			ap_rputs("\n}\n", r);
		else
			ap_rputs("}", r);
	} else {
		if (pretty)
			ap_rputs("\n]\n", r);
		else
			ap_rputs("]", r);
	}
	if (errno != 0) {
#ifdef APACHE2
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, 0, r, "Error: readdir failed\n");
#else
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, r, "Error: readdir failed\n");
#endif
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	return OK;
}

#ifdef APACHE2
static void register_hooks(apr_pool_t *p) {
	static const char * const aszPost[] = { "mod_autoindex.c", NULL };
	ap_hook_header_parser(handle_jsonindex, NULL, aszPost, APR_HOOK_MIDDLE);
}

module AP_MODULE_DECLARE_DATA jsonindex_module = {
	STANDARD20_MODULE_STUFF,
	NULL,			/* per-directory config creator */
	NULL,			/* dir config merger */
	NULL,			/* server config creator */
	NULL,			/* server config merger */
	NULL,			/* command table */
	register_hooks,	/* set up other request processing hooks */
};
#else
static const handler_rec jsonindex_handlers[] = {
	{DIR_MAGIC_TYPE, handle_jsonindex},
	{"httpd/json-directory", handle_jsonindex},
	{"json-directory", handle_jsonindex},
	{NULL}
};

module MODULE_VAR_EXPORT jsonindex_module = {
    STANDARD_MODULE_STUFF,
    NULL,			/* initializer */
    NULL,			/* dir config creater */
    NULL,			/* dir merger --- default is to override */
    NULL,			/* server config */
    NULL,			/* merge server config */
    NULL,			/* command table */
    jsonindex_handlers,		/* handlers */
    NULL,			/* filename translation */
    NULL,			/* check_user_id */
    NULL,			/* check auth */
    NULL,			/* check access */
    NULL,			/* type_checker */
    NULL,			/* fixups */
    NULL,			/* logger */
    NULL,			/* header parser */
    NULL,			/* child_init */
    NULL,			/* child_exit */
    NULL			/* post read-request */
};
#endif // APACHE2
