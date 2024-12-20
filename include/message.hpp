/**
 * Copyright (c) 2024 sunze
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef WEBSOCKET_CLIENT_MESSAGE_H
#define WEBSOCKET_CLIENT_MESSAGE_H

#include "common.hpp"
#include "frameinfo.hpp"
#include "reliability.hpp"

#include <functional>

namespace wsc {

struct WSC_CPP_EXPORT Message : binary {
	enum Type { Binary, String, Control, Reset };

	Message(const Message &message) = default;
	Message(size_t size, Type type_ = Binary) : binary(size), type(type_) {}

	template <typename Iterator>
	Message(Iterator begin_, Iterator end_, Type type_ = Binary)
	    : binary(begin_, end_), type(type_) {}

	Message(binary &&data, Type type_ = Binary) : binary(std::move(data)), type(type_) {}

	Type type;
	unsigned int stream = 0; // Stream id (SCTP stream or SSRC)
	unsigned int dscp = 0;   // Differentiated Services Code Point
	shared_ptr<Reliability> reliability;
	shared_ptr<FrameInfo> frameInfo;
};

using message_ptr = shared_ptr<Message>;
using message_callback = std::function<void(message_ptr message)>;
using message_vector = std::vector<message_ptr>;

inline size_t message_size_func(const message_ptr &m) {
	return m->type == Message::Binary || m->type == Message::String ? m->size() : 0;
}

template <typename Iterator>
message_ptr make_message(Iterator begin, Iterator end, Message::Type type = Message::Binary,
                         unsigned int stream = 0, shared_ptr<Reliability> reliability = nullptr,
                         shared_ptr<FrameInfo> frameInfo = nullptr) {
	auto message = std::make_shared<Message>(begin, end, type);
	message->stream = stream;
	message->reliability = reliability;
	message->frameInfo = frameInfo;
	return message;
}

WSC_CPP_EXPORT message_ptr make_message(size_t size, Message::Type type = Message::Binary,
                                        unsigned int stream = 0,
                                        shared_ptr<Reliability> reliability = nullptr);

WSC_CPP_EXPORT message_ptr make_message(binary &&data, Message::Type type = Message::Binary,
                                        unsigned int stream = 0,
                                        shared_ptr<Reliability> reliability = nullptr,
                                        shared_ptr<FrameInfo> frameInfo = nullptr);

WSC_CPP_EXPORT message_ptr make_message(size_t size, message_ptr orig);

WSC_CPP_EXPORT message_ptr make_message(message_variant data);

WSC_CPP_EXPORT message_variant to_variant(Message &&message);
WSC_CPP_EXPORT message_variant to_variant(const Message &message);

} // namespace wsc

#endif
