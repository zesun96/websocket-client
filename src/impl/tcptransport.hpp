/**
 * Copyright (c) 2024 sunze
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef WEBSOCKET_IMPL_TCP_TRANSPORT_H
#define WEBSOCKET_IMPL_TCP_TRANSPORT_H

#include "common.hpp"
#include "pollservice.hpp"
#include "queue.hpp"
#include "socket.hpp"
#include "transport.hpp"

#include <chrono>
#include <list>
#include <mutex>
#include <tuple>

namespace wsc::impl {

class TcpTransport final : public Transport, public std::enable_shared_from_this<TcpTransport> {
public:
	using amount_callback = std::function<void(size_t amount)>;

	TcpTransport(string hostname, string service, state_callback callback); // active
	TcpTransport(socket_t sock, state_callback callback);                   // passive
	~TcpTransport();

	void onBufferedAmount(amount_callback callback);
	void setReadTimeout(std::chrono::milliseconds readTimeout);

	void start() override;
	bool send(message_ptr message) override;

	void incoming(message_ptr message) override;
	bool outgoing(message_ptr message) override;

	bool isActive() const;
	string remoteAddress() const;

private:
	void connect();
	void resolve();
	void attempt();
	void createSocket(const struct sockaddr *addr, socklen_t addrlen);
	void configureSocket();
	void setPoll(PollService::Direction direction);
	void close();

	bool trySendQueue();
	bool trySendMessage(message_ptr &message);
	void updateBufferedAmount(ptrdiff_t delta);
	void triggerBufferedAmount(size_t amount);

	void process(PollService::Event event);

	const bool mIsActive;
	string mHostname, mService;
	amount_callback mBufferedAmountCallback;
	optional<std::chrono::milliseconds> mReadTimeout;

	std::list<std::tuple<struct sockaddr_storage, socklen_t>> mResolved;

	socket_t mSock;
	Queue<message_ptr> mSendQueue;
	size_t mBufferedAmount = 0;
	std::mutex mSendMutex;
};

} // namespace wsc::impl

#endif
