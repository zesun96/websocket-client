/**
 * Copyright (c) 2024 sunze
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef WEBSOCKET_CLIENT_WEBSOCKET_H
#define WEBSOCKET_CLIENT_WEBSOCKET_H

#include "channel.hpp"
#include "common.hpp"
#include "configuration.hpp"

namespace wsc {

namespace impl {

struct WebSocket;

}

class WSC_CPP_EXPORT WebSocket final : private CheshireCat<impl::WebSocket>, public Channel {
public:
	enum class State : int {
		Connecting = 0,
		Open = 1,
		Closing = 2,
		Closed = 3,
	};

	using Configuration = WebSocketConfiguration;

	WebSocket();
	WebSocket(Configuration config);
	WebSocket(impl_ptr<impl::WebSocket> impl);
	~WebSocket() override;

	State readyState() const;

	bool isOpen() const override;
	bool isClosed() const override;
	size_t maxMessageSize() const override;

	void open(const string &url);
	void close() override;
	void forceClose();
	bool send(const message_variant data) override;
	bool send(const byte *data, size_t size) override;

	optional<string> remoteAddress() const;
	optional<string> path() const;

private:
	using CheshireCat<impl::WebSocket>::impl;
};

std::ostream &operator<<(std::ostream &out, WebSocket::State state);

} // namespace wsc

#endif
