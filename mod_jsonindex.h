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
 * 
 * Marian Marinov <mm@yuhu.biz>
 * 11.Dec.2011
 * 
 */
#ifndef _JSONINDEX_H

#include <dirent.h>
#include <errno.h>
#include <unistd.h>

#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_request.h"
#include "http_protocol.h"
#include "http_log.h"
#include "http_main.h"
#include "util_script.h"

#define JSONINDEX_VERSION "0.04"

#ifndef APACHE_RELEASE
#define APACHE2
#endif

#ifdef APACHE2
#include <strings.h>

#include "http_request.h"
#include "http_connection.h"
#include "apr_strings.h"
#endif

#endif // _JSONINDEX_H
