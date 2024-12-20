/**
 * Copyright (c) 2024 sunze
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef WEBSOCKET_IMPL_HTTP_H
#define WEBSOCKET_IMPL_HTTP_H

#include "common.hpp"

#include <list>
#include <map>

namespace wsc::impl {

// Check the buffer contains the beginning of an HTTP request
bool isHttpRequest(const byte *buffer, size_t size);

// Parse an http message into lines
size_t parseHttpLines(const byte *buffer, size_t size, std::list<string> &lines);

// Parse headers of a http message
std::multimap<string, string> parseHttpHeaders(const std::list<string> &lines);

} // namespace wsc::impl

#endif
