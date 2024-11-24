/**
 * Copyright (c) 2024 sunze
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef WEBSOCKET_CLIENT_IMPL_TCP_PROXY_TRANSPORT_H
#define WEBSOCKET_CLIENT_IMPL_TCP_PROXY_TRANSPORT_H

#include "common.hpp"
#include "transport.hpp"

namespace wsc::impl {

class TcpTransport;

class HttpProxyTransport final : public Transport,
                                 public std::enable_shared_from_this<HttpProxyTransport> {
public:
	HttpProxyTransport(shared_ptr<TcpTransport> lower, std::string hostname, std::string service,
	                   state_callback stateCallback);
	~HttpProxyTransport();

	void start() override;
	void stop() override;
	bool send(message_ptr message) override;

	bool isActive() const;

private:
	void incoming(message_ptr message) override;
	bool sendHttpRequest();
	string generateHttpRequest();
	size_t parseHttpResponse(std::byte *buffer, size_t size);

	string mHostname;
	string mService;
	binary mBuffer;
};

} // namespace wsc::impl

#endif
