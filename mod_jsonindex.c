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

#include <dirent.h>
#include <errno.h>

#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_request.h"
#include "http_protocol.h"
#include "http_log.h"
#include "http_main.h"
#include "util_script.h"

module MODULE_VAR_EXPORT jsonindex_module;

static int handle_jsonindex(request_rec *r) {
    r->allowed |= (1 << M_GET);
	if (r->method_number != M_GET) {
		return DECLINED;
    }

	int allow_opts = ap_allow_options(r);
    DIR *dir;
    struct dirent *item;
	int not_start = 0;
	int pretty = 0;
	char buf[1022];

    /* OK, nothing easy.  Trot out the heavy artillery... */

    if (!(allow_opts & OPT_INDEXES)) {
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, r, "Directory index forbidden by rule: %s", r->filename);
		return HTTP_FORBIDDEN;
    }
	if (r->filename[strlen(r->filename) - 1] != '/') {
	    r->filename = ap_pstrcat(r->pool, r->filename, "/", NULL);
	}

	if (!(dir = ap_popendir(r->pool, r->filename))) {
		ap_log_rerror(APLOG_MARK, APLOG_ERR, r, "Can't open directory for index: %s", r->filename);
		return HTTP_FORBIDDEN;
	}

	r->content_type = "text/plain";
	ap_send_http_header(r);
	ap_hard_timeout("send directory", r);
	if (r->header_only) {
		ap_kill_timeout(r);
		return OK;
	}

	if (r->args && strstr(r->args, "pretty") != NULL)
		pretty = 1;
    
	if (pretty)
		ap_rputs("{\n", r);
	else
		ap_rputs("{", r);

	for(;;) {
		errno = 0;
		item = readdir(dir);
		if (item == NULL)
			break;  // end of the dir

		if (strcmp(item->d_name, ".") == 0 || strcmp(item->d_name, "..") == 0) {
			// I should add skipping of AccessFileName
			// and if possible the files that are protected with deny from all
			continue;       // skip . and .. directories
		}

		if (not_start) {
			ap_rputs(",", r);
			if (pretty)
				ap_rputs("\n", r);
		} else {
			not_start = 1;
		}

		if (pretty)
			if (item->d_type == DT_DIR) {
				ap_rprintf(r, "  \"%s\" : [ 1, \"\" ]", item->d_name);
			} else if (item->d_type == DT_LNK) {
				memset(&buf, '\0', 1022);
				readlink(item->d_name, buf, 1022);
				ap_rprintf(r, "  \"%s\" : [ 2, \"%s\" ]", item->d_name, buf);
			} else {
				ap_rprintf(r, "  \"%s\" : [ 0, \"\" ]", item->d_name);
			}
		else
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
	if (pretty)
		ap_rputs("\n}\n", r);
	else
		ap_rputs("}", r);

	if (errno != 0) {
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, r, "Error: readdir failed\n");
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	return OK;
}

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
