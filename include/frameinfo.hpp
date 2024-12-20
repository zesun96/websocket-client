/**
 * Copyright (c) 2024 sunze
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef WEBSOCKET_CLIENT_FRAMEINFO_H
#define WEBSOCKET_CLIENT_FRAMEINFO_H

#include "common.hpp"

namespace wsc {

struct WSC_CPP_EXPORT FrameInfo {
	FrameInfo(uint8_t payloadType, uint32_t timestamp)
	    : payloadType(payloadType), timestamp(timestamp) {};
	uint8_t payloadType;    // Indicates codec of the frame
	uint32_t timestamp = 0; // RTP Timestamp
};

} // namespace wsc

#endif
