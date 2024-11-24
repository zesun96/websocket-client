/**
 * Copyright (c) 2024 sunze
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef WEBSOCKET_IMPL_SHA_H
#define WEBSOCKET_IMPL_SHA_H

#include "common.hpp"

namespace wsc::impl {

binary Sha1(const binary &input);
binary Sha1(const string &input);

} // namespace wsc::impl

#endif
