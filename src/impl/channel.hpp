/**
 * Copyright (c) 2024 sunze
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef WEBSOCKETC_CLIENT_IMPL_CHANNEL_H
#define WEBSOCKET_CLIENT_IMPL_CHANNEL_H

#include "common.hpp"
#include "message.hpp"

#include <atomic>
#include <functional>

namespace wsc::impl {

struct Channel {
	virtual optional<message_variant> receive() = 0;
	virtual optional<message_variant> peek() = 0;
	virtual size_t availableAmount() const = 0;

	virtual void triggerOpen();
	virtual void triggerClosed();
	virtual void triggerError(string error);
	virtual void triggerAvailable(size_t count);
	virtual void triggerBufferedAmount(size_t amount);

	virtual void flushPendingMessages();
	void resetOpenCallback();
	void resetCallbacks();

	synchronized_stored_callback<> openCallback;
	synchronized_stored_callback<> closedCallback;
	synchronized_stored_callback<string> errorCallback;
	synchronized_stored_callback<> availableCallback;
	synchronized_stored_callback<> bufferedAmountLowCallback;

	synchronized_callback<message_variant> messageCallback;

	std::atomic<size_t> bufferedAmount = 0;
	std::atomic<size_t> bufferedAmountLowThreshold = 0;

protected:
	std::atomic<bool> mOpenTriggered = false;
};

} // namespace wsc::impl

#endif
