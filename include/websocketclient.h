/**
 * Copyright (c) 2024 sunze
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef WEBSOCKET_CLIENT_C_API
#define WEBSOCKET_CLIENT_C_API

#include "version.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#ifdef WSC_STATIC
#define WSC_C_EXPORT
#else // dynamic library
#ifdef _WIN32
#ifdef WSC_EXPORTS
#define WSC_C_EXPORT __declspec(dllexport) // building the library
#else
#define WSC_C_EXPORT __declspec(dllimport) // using the library
#endif
#else // not WIN32
#define WSC_C_EXPORT
#endif
#endif

#define WSC_DEFAULT_MTU 1280 // IPv6 minimum guaranteed MTU

#ifdef _WIN32
#ifdef CAPI_STDCALL
#define WSC_API __stdcall
#else
#define WSC_API
#endif
#else // not WIN32
#define WSC_API
#endif

#if defined(__GNUC__) || defined(__clang__)
#define WSC_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define WSC_DEPRECATED __declspec(deprecated)
#else
#define DEPRECATED
#endif

typedef enum { // Don't change, it must match plog severity
	WSC_LOG_NONE = 0,
	WSC_LOG_FATAL = 1,
	WSC_LOG_ERROR = 2,
	WSC_LOG_WARNING = 3,
	WSC_LOG_INFO = 4,
	WSC_LOG_DEBUG = 5,
	WSC_LOG_VERBOSE = 6
} wscLogLevel;

typedef enum {
	WSC_CERTIFICATE_DEFAULT = 0, // ECDSA
	WSC_CERTIFICATE_ECDSA = 1,
	WSC_CERTIFICATE_RSA = 2,
} wscCertificateType;

typedef enum { WSC_TRANSPORT_POLICY_ALL = 0, WSC_TRANSPORT_POLICY_RELAY = 1 } wscTransportPolicy;

#define WSC_ERR_SUCCESS 0
#define WSC_ERR_INVALID -1   // invalid argument
#define WSC_ERR_FAILURE -2   // runtime error
#define WSC_ERR_NOT_AVAIL -3 // element not available
#define WSC_ERR_TOO_SMALL -4 // buffer too small

typedef void(WSC_API *wscLogCallbackFunc)(wscLogLevel level, const char *message);
typedef void(WSC_API *wscOpenCallbackFunc)(int id, void *ptr);
typedef void(WSC_API *wscClosedCallbackFunc)(int id, void *ptr);
typedef void(WSC_API *wscErrorCallbackFunc)(int id, const char *error, void *ptr);
typedef void(WSC_API *wscMessageCallbackFunc)(int id, const char *message, int size, void *ptr);
typedef void(WSC_API *wscBufferedAmountLowCallbackFunc)(int id, void *ptr);
typedef void(WSC_API *wscAvailableCallbackFunc)(int id, void *ptr);
// Log

// NULL cb on the first call will log to stdout
WSC_C_EXPORT void wscInitLogger(wscLogLevel level, wscLogCallbackFunc cb);

// User pointer
WSC_C_EXPORT void wscSetUserPointer(int id, void *ptr);
WSC_C_EXPORT void *wscGetUserPointer(int i);

// WebSocket common API

WSC_C_EXPORT int wscSetOpenCallback(int id, wscOpenCallbackFunc cb);
WSC_C_EXPORT int wscSetClosedCallback(int id, wscClosedCallbackFunc cb);
WSC_C_EXPORT int wscSetErrorCallback(int id, wscErrorCallbackFunc cb);
WSC_C_EXPORT int wscSetMessageCallback(int id, wscMessageCallbackFunc cb);
WSC_C_EXPORT int wscSendMessage(int id, const char *data, int size);
WSC_C_EXPORT int wscClose(int id);
WSC_C_EXPORT int wscDelete(int id);
WSC_C_EXPORT bool wscIsOpen(int id);
WSC_C_EXPORT bool wscIsClosed(int id);

WSC_C_EXPORT int wscMaxMessageSize(int id);
WSC_C_EXPORT int wscGetBufferedAmount(int id); // total size buffered to send
WSC_C_EXPORT int wscSetBufferedAmountLowThreshold(int id, int amount);
WSC_C_EXPORT int wscSetBufferedAmountLowCallback(int id, wscBufferedAmountLowCallbackFunc cb);

// WebSocket common extended API

WSC_C_EXPORT int wscGetAvailableAmount(int id); // total size available to receive
WSC_C_EXPORT int wscSetAvailableCallback(int id, wscAvailableCallbackFunc cb);
WSC_C_EXPORT int wscReceiveMessage(int id, char *buffer, int *size);

// WebSocket

typedef struct {
	bool disableTlsVerification; // if true, don't verify the TLS certificate
	const char *proxyServer;     // only non-authenticated http supported for now
	const char **protocols;
	int protocolsCount;
	int connectionTimeoutMs; // in milliseconds, 0 means default, < 0 means disabled
	int pingIntervalMs;      // in milliseconds, 0 means default, < 0 means disabled
	int maxOutstandingPings; // 0 means default, < 0 means disabled
	int maxMessageSize;      // <= 0 means default
} wscWsConfiguration;

WSC_C_EXPORT int wscCreateWebSocket(const char *url); // returns ws id
WSC_C_EXPORT int wscCreateWebSocketEx(const char *url, const wscWsConfiguration *config);
WSC_C_EXPORT int wscDeleteWebSocket(int ws);

WSC_C_EXPORT int wscGetWebSocketRemoteAddress(int ws, char *buffer, int size);
WSC_C_EXPORT int wscGetWebSocketPath(int ws, char *buffer, int size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
