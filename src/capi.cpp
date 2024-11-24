/**
 * Copyright (c) 2024 sunze
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "websocketclient.h"
#include "websocketclient.hpp"

#include "impl/internals.hpp"

#include <algorithm>
#include <chrono>
#include <exception>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <utility>

using namespace wsc;
using namespace std::chrono_literals;
using std::chrono::milliseconds;

namespace {

std::unordered_map<int, shared_ptr<WebSocket>> webSocketMap;

std::unordered_map<int, void *> userPointerMap;
std::mutex mutex;
int lastId = 0;

optional<void *> getUserPointer(int id) {
	std::lock_guard lock(mutex);
	auto it = userPointerMap.find(id);
	return it != userPointerMap.end() ? std::make_optional(it->second) : nullopt;
}

void setUserPointer(int i, void *ptr) {
	std::lock_guard lock(mutex);
	userPointerMap[i] = ptr;
}

size_t eraseAll() {
	std::lock_guard lock(mutex);
	size_t count = 0;
	count += webSocketMap.size();
	webSocketMap.clear();

	userPointerMap.clear();
	return count;
}

shared_ptr<Channel> getChannel(int id) {
	std::lock_guard lock(mutex);

	if (auto it = webSocketMap.find(id); it != webSocketMap.end())
		return it->second;

	throw std::invalid_argument("WebSocket ID does not exist");
}

void eraseChannel(int id) {
	std::lock_guard lock(mutex);

	if (webSocketMap.erase(id) != 0) {
		userPointerMap.erase(id);
		return;
	}

	throw std::invalid_argument("WebSocket ID does not exist");
}

int copyAndReturn(string s, char *buffer, int size) {
	if (!buffer)
		return int(s.size() + 1);

	if (size < int(s.size() + 1))
		return WSC_ERR_TOO_SMALL;

	std::copy(s.begin(), s.end(), buffer);
	buffer[s.size()] = '\0';
	return int(s.size() + 1);
}

int copyAndReturn(binary b, char *buffer, int size) {
	if (!buffer)
		return int(b.size());

	if (size < int(b.size()))
		return WSC_ERR_TOO_SMALL;

	auto data = reinterpret_cast<const char *>(b.data());
	std::copy(data, data + b.size(), buffer);
	return int(b.size());
}

template <typename T> int copyAndReturn(std::vector<T> b, T *buffer, int size) {
	if (!buffer)
		return int(b.size());

	if (size < int(b.size()))
		return WSC_ERR_TOO_SMALL;
	std::copy(b.begin(), b.end(), buffer);
	return int(b.size());
}

template <typename F> int wrap(F func) {
	try {
		return int(func());

	} catch (const std::invalid_argument &e) {
		// PLOG_ERROR << e.what();
		return WSC_ERR_INVALID;
	} catch (const std::exception &e) {
		// PLOG_ERROR << e.what();
		return WSC_ERR_FAILURE;
	}
}

shared_ptr<WebSocket> getWebSocket(int id) {
	std::lock_guard lock(mutex);
	if (auto it = webSocketMap.find(id); it != webSocketMap.end())
		return it->second;
	else
		throw std::invalid_argument("WebSocket ID does not exist");
}

int emplaceWebSocket(shared_ptr<WebSocket> ptr) {
	std::lock_guard lock(mutex);
	int ws = ++lastId;
	webSocketMap.emplace(std::make_pair(ws, ptr));
	userPointerMap.emplace(std::make_pair(ws, nullptr));
	return ws;
}

void eraseWebSocket(int ws) {
	std::lock_guard lock(mutex);
	if (webSocketMap.erase(ws) == 0)
		throw std::invalid_argument("WebSocket ID does not exist");
	userPointerMap.erase(ws);
}

} // namespace

void wscInitLogger(wscLogLevel level, wscLogCallbackFunc cb) {
	LogCallback callback = nullptr;
	if (cb)
		callback = [cb](LogLevel level, string message) {
			cb(static_cast<wscLogLevel>(level), message.c_str());
		};

	InitLogger(static_cast<LogLevel>(level), callback);
}

void wscSetUserPointer(int i, void *ptr) { setUserPointer(i, ptr); }

void *wscGetUserPointer(int i) { return getUserPointer(i).value_or(nullptr); }

int wscSetOpenCallback(int id, wscOpenCallbackFunc cb) {
	return wrap([&] {
		auto channel = getChannel(id);
		if (cb)
			channel->onOpen([id, cb]() {
				if (auto ptr = getUserPointer(id))
					cb(id, *ptr);
			});
		else
			channel->onOpen(nullptr);
		return WSC_ERR_SUCCESS;
	});
}

int wscSetClosedCallback(int id, wscClosedCallbackFunc cb) {
	return wrap([&] {
		auto channel = getChannel(id);
		if (cb)
			channel->onClosed([id, cb]() {
				if (auto ptr = getUserPointer(id))
					cb(id, *ptr);
			});
		else
			channel->onClosed(nullptr);
		return WSC_ERR_SUCCESS;
	});
}

int wscSetErrorCallback(int id, wscErrorCallbackFunc cb) {
	return wrap([&] {
		auto channel = getChannel(id);
		if (cb)
			channel->onError([id, cb](string error) {
				if (auto ptr = getUserPointer(id))
					cb(id, error.c_str(), *ptr);
			});
		else
			channel->onError(nullptr);
		return WSC_ERR_SUCCESS;
	});
}

int wscSetMessageCallback(int id, wscMessageCallbackFunc cb) {
	return wrap([&] {
		auto channel = getChannel(id);
		if (cb)
			channel->onMessage(
			    [id, cb](binary b) {
				    if (auto ptr = getUserPointer(id))
					    cb(id, reinterpret_cast<const char *>(b.data()), int(b.size()), *ptr);
			    },
			    [id, cb](string s) {
				    if (auto ptr = getUserPointer(id))
					    cb(id, s.c_str(), -int(s.size() + 1), *ptr);
			    });
		else
			channel->onMessage(nullptr);
		return WSC_ERR_SUCCESS;
	});
}

int wscSendMessage(int id, const char *data, int size) {
	return wrap([&] {
		auto channel = getChannel(id);

		if (!data && size != 0)
			throw std::invalid_argument("Unexpected null pointer for data");

		if (size >= 0) {
			auto b = reinterpret_cast<const byte *>(data);
			channel->send(binary(b, b + size));
		} else {
			channel->send(string(data));
		}
		return WSC_ERR_SUCCESS;
	});
}

int wscClose(int id) {
	return wrap([&] {
		auto channel = getChannel(id);
		channel->close();
		return WSC_ERR_SUCCESS;
	});
}

int wscDelete(int id) {
	return wrap([id] {
		auto channel = getChannel(id);
		channel->close();
		eraseChannel(id);
		return WSC_ERR_SUCCESS;
	});
}

bool wscIsOpen(int id) {
	return wrap([id] { return getChannel(id)->isOpen() ? 0 : 1; }) == 0 ? true : false;
}

bool wscIsClosed(int id) {
	return wrap([id] { return getChannel(id)->isClosed() ? 0 : 1; }) == 0 ? true : false;
}

int wscMaxMessageSize(int id) {
	return wrap([id] {
		auto channel = getChannel(id);
		return int(channel->maxMessageSize());
	});
}

int wscGetBufferedAmount(int id) {
	return wrap([id] {
		auto channel = getChannel(id);
		return int(channel->bufferedAmount());
	});
}

int wscSetBufferedAmountLowThreshold(int id, int amount) {
	return wrap([&] {
		auto channel = getChannel(id);
		channel->setBufferedAmountLowThreshold(size_t(amount));
		return WSC_ERR_SUCCESS;
	});
}

int wscSetBufferedAmountLowCallback(int id, wscBufferedAmountLowCallbackFunc cb) {
	return wrap([&] {
		auto channel = getChannel(id);
		if (cb)
			channel->onBufferedAmountLow([id, cb]() {
				if (auto ptr = getUserPointer(id))
					cb(id, *ptr);
			});
		else
			channel->onBufferedAmountLow(nullptr);
		return WSC_ERR_SUCCESS;
	});
}

int wscGetAvailableAmount(int id) {
	return wrap([id] { return int(getChannel(id)->availableAmount()); });
}

int wscSetAvailableCallback(int id, wscAvailableCallbackFunc cb) {
	return wrap([&] {
		auto channel = getChannel(id);
		if (cb)
			channel->onAvailable([id, cb]() {
				if (auto ptr = getUserPointer(id))
					cb(id, *ptr);
			});
		else
			channel->onAvailable(nullptr);
		return WSC_ERR_SUCCESS;
	});
}

int wscReceiveMessage(int id, char *buffer, int *size) {
	return wrap([&] {
		auto channel = getChannel(id);

		if (!size)
			throw std::invalid_argument("Unexpected null pointer for size");

		*size = std::abs(*size);

		auto message = channel->peek();
		if (!message)
			return WSC_ERR_NOT_AVAIL;

		return std::visit( //
		    overloaded{
		        [&](binary b) {
			        int ret = copyAndReturn(std::move(b), buffer, *size);
			        if (ret >= 0) {
				        *size = ret;
				        if (buffer) {
					        channel->receive(); // discard
				        }

				        return WSC_ERR_SUCCESS;
			        } else {
				        *size = int(b.size());
				        return ret;
			        }
		        },
		        [&](string s) {
			        int ret = copyAndReturn(std::move(s), buffer, *size);
			        if (ret >= 0) {
				        *size = -ret;
				        if (buffer) {
					        channel->receive(); // discard
				        }

				        return WSC_ERR_SUCCESS;
			        } else {
				        *size = -int(s.size() + 1);
				        return ret;
			        }
		        },
		    },
		    *message);
	});
}

int wscCreateWebSocket(const char *url) {
	return wrap([&] {
		auto webSocket = std::make_shared<WebSocket>();
		webSocket->open(url);
		return emplaceWebSocket(webSocket);
	});
}

int wscCreateWebSocketEx(const char *url, const wscWsConfiguration *config) {
	return wrap([&] {
		if (!url)
			throw std::invalid_argument("Unexpected null pointer for URL");

		if (!config)
			throw std::invalid_argument("Unexpected null pointer for config");

		WebSocket::Configuration c;
		c.disableTlsVerification = config->disableTlsVerification;

		if (config->proxyServer)
			c.proxyServer.emplace(config->proxyServer);

		for (int i = 0; i < config->protocolsCount; ++i)
			c.protocols.emplace_back(string(config->protocols[i]));

		if (config->connectionTimeoutMs > 0)
			c.connectionTimeout = milliseconds(config->connectionTimeoutMs);
		else if (config->connectionTimeoutMs < 0)
			c.connectionTimeout = milliseconds::zero(); // setting to 0 disables,
			                                            // not setting keeps default
		if (config->pingIntervalMs > 0)
			c.pingInterval = milliseconds(config->pingIntervalMs);
		else if (config->pingIntervalMs < 0)
			c.pingInterval = milliseconds::zero(); // setting to 0 disables,
			                                       // not setting keeps default
		if (config->maxOutstandingPings > 0)
			c.maxOutstandingPings = config->maxOutstandingPings;
		else if (config->maxOutstandingPings < 0)
			c.maxOutstandingPings = 0; // setting to 0 disables, not setting keeps default

		if (config->maxMessageSize > 0)
			c.maxMessageSize = size_t(config->maxMessageSize);

		auto webSocket = std::make_shared<WebSocket>(std::move(c));
		webSocket->open(url);
		return emplaceWebSocket(webSocket);
	});
}

int wscDeleteWebSocket(int ws) {
	return wrap([&] {
		auto webSocket = getWebSocket(ws);
		webSocket->forceClose();
		webSocket->resetCallbacks(); // not done on close by WebSocket
		eraseWebSocket(ws);
		return WSC_ERR_SUCCESS;
	});
}

int wscGetWebSocketRemoteAddress(int ws, char *buffer, int size) {
	return wrap([&] {
		auto webSocket = getWebSocket(ws);
		if (auto remoteAddress = webSocket->remoteAddress())
			return copyAndReturn(*remoteAddress, buffer, size);
		else
			return WSC_ERR_NOT_AVAIL;
	});
}

int wscGetWebSocketPath(int ws, char *buffer, int size) {
	return wrap([&] {
		auto webSocket = getWebSocket(ws);
		if (auto path = webSocket->path())
			return copyAndReturn(*path, buffer, size);
		else
			return WSC_ERR_NOT_AVAIL;
	});
}

void wscPreload() {
	try {
		wsc::Preload();
	} catch (const std::exception &e) {
		// PLOG_ERROR << e.what();
	}
}

void wscCleanup() {
	try {
		size_t count = eraseAll();
		if (count != 0) {
			// PLOG_INFO << count << " objects were not properly destroyed before cleanup";
		}

		if (wsc::Cleanup().wait_for(10s) == std::future_status::timeout)
			throw std::runtime_error(
			    "Cleanup timeout (possible deadlock or undestructible object)");

	} catch (const std::exception &e) {
		// PLOG_ERROR << e.what();
	}
}
