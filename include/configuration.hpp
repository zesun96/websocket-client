/**
 * Copyright (c) 2024 sunze
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef WEBSOCKET_CLIENT_CONFIGURATION_H
#define WEBSOCKET_CLIENT_CONFIGURATION_H

#include "common.hpp"

#include <vector>

namespace wsc {

struct WSC_CPP_EXPORT ProxyServer {
	enum class Type { Http, Socks5 };

	ProxyServer(const string &url);

	ProxyServer(Type type_, string hostname_, uint16_t port_);
	ProxyServer(Type type_, string hostname_, uint16_t port_, string username_, string password_);

	Type type;
	string hostname;
	uint16_t port;
	optional<string> username;
	optional<string> password;
};

enum class CertificateType {
	Default = WSC_CERTIFICATE_DEFAULT, // ECDSA
	Ecdsa = WSC_CERTIFICATE_ECDSA,
	Rsa = WSC_CERTIFICATE_RSA
};

enum class TransportPolicy { All = WSC_TRANSPORT_POLICY_ALL, Relay = WSC_TRANSPORT_POLICY_RELAY };

struct WSC_CPP_EXPORT WebSocketConfiguration {
	bool disableTlsVerification = false; // if true, don't verify the TLS certificate
	optional<ProxyServer> proxyServer;   // only non-authenticated http supported for now
	std::vector<string> protocols;
	optional<std::chrono::milliseconds> connectionTimeout; // zero to disable
	optional<std::chrono::milliseconds> pingInterval;      // zero to disable
	optional<int> maxOutstandingPings;
	optional<string> caCertificatePemFile;
	optional<string> certificatePemFile;
	optional<string> keyPemFile;
	optional<string> keyPemPass;
	optional<size_t> maxMessageSize;
};

struct WebSocketServerConfiguration {
	uint16_t port = 8080;
	bool enableTls = false;
	optional<string> certificatePemFile;
	optional<string> keyPemFile;
	optional<string> keyPemPass;
	optional<string> bindAddress;
	optional<std::chrono::milliseconds> connectionTimeout;
	optional<size_t> maxMessageSize;
};

} // namespace wsc

#endif
