/**
 * Copyright (c) 2024 sunze
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef WEBSOCKET_IMPL_POLL_INTERRUPTER_H
#define WEBSOCKET_IMPL_POLL_INTERRUPTER_H

#include "common.hpp"
#include "socket.hpp"

namespace wsc::impl {

// Utility class to interrupt poll()
class PollInterrupter final {
public:
	PollInterrupter();
	~PollInterrupter();

	PollInterrupter(const PollInterrupter &other) = delete;
	void operator=(const PollInterrupter &other) = delete;

	void prepare(struct pollfd &pfd);
	void process(struct pollfd &pfd);
	void interrupt();

private:
#ifdef _WIN32
	socket_t mSock;
#else // assume POSIX
	int mPipeIn, mPipeOut;
#endif
};

} // namespace wsc::impl

#endif
