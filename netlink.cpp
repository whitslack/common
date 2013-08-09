#include "netlink.h"

#include <cerrno>

#include <netlink/genl/ctrl.h>

namespace nl {


const char * Category::name() const noexcept {
	return "Netlink";
}

std::string Category::message(int condition) const noexcept {
	return ::nl_geterror(condition);
}


Message::Message(size_t max) : msg(::nlmsg_alloc_size(max)) {
	if (!msg) {
		throw Error(NLE_NOMEM, "nlmsg_alloc_size");
	}
}

void * Message::reserve(size_t len, int pad) {
	void *data;
	if (!(data = ::nlmsg_reserve(msg, len, pad))) {
		throw Error(NLE_NOMEM, "nlmsg_reserve");
	}
	return data;
}

void Message::append(void *data, size_t len, int pad) {
	int error;
	if ((error = ::nlmsg_append(msg, data, len, pad)) < 0) {
		throw Error(-error, "nlmsg_append");
	}
}

void Message::expand(size_t newlen) {
	int error;
	if ((error = ::nlmsg_expand(msg, newlen)) < 0) {
		throw Error(-error, "nlmsg_expand");
	}
}

MessageHeader * Message::put_hdr(uint32_t port, uint32_t seq, int type, int payload_len, int flags) {
	MessageHeader *hdr;
	if (!(hdr = static_cast<MessageHeader *>(::nlmsg_put(msg, port, seq, type, payload_len, flags)))) {
		throw Error(NLE_NOMEM, "nlmsg_put");
	}
	return hdr;
}

void * Message::put_genl_hdr(uint32_t port, uint32_t seq, int family, int hdrlen, int flags, uint8_t cmd, uint8_t version) {
	void *data;
	if (!(data = ::genlmsg_put(msg, port, seq, family, hdrlen, flags, cmd, version))) {
		throw Error(NLE_NOMEM, "genlmsg_put");
	}
	return data;
}

Attribute * Message::reserve_attr(int attrtype, int attrlen) {
	Attribute *attr;
	if (!(attr = static_cast<Attribute *>(::nla_reserve(msg, attrtype, attrlen)))) {
		throw Error(NLE_NOMEM, "nla_reserve");
	}
	return attr;
}

void Message::put_attr(int attrtype, int datalen, const void *data) {
	int error;
	if ((error = ::nla_put(msg, attrtype, datalen, data)) < 0) {
		throw Error(-error, "nla_put");
	}
}

void Message::put_data_attr(int attrtype, struct nl_data *data) {
	int error;
	if ((error = ::nla_put_data(msg, attrtype, data)) < 0) {
		throw Error(-error, "nla_put_data");
	}
}

void Message::put_addr_attr(int attrtype, struct nl_addr *addr) {
	int error;
	if ((error = ::nla_put_addr(msg, attrtype, addr)) < 0) {
		throw Error(-error, "nla_put_addr");
	}
}

void Message::put_u8_attr(int attrtype, uint8_t value) {
	int error;
	if ((error = ::nla_put_u8(msg, attrtype, value)) < 0) {
		throw Error(-error, "nla_put_u8");
	}
}

void Message::put_u16_attr(int attrtype, uint16_t value) {
	int error;
	if ((error = ::nla_put_u16(msg, attrtype, value)) < 0) {
		throw Error(-error, "nla_put_u16");
	}
}

void Message::put_u32_attr(int attrtype, uint32_t value) {
	int error;
	if ((error = ::nla_put_u32(msg, attrtype, value)) < 0) {
		throw Error(-error, "nla_put_u32");
	}
}

void Message::put_u64_attr(int attrtype, uint64_t value) {
	int error;
	if ((error = ::nla_put_u64(msg, attrtype, value)) < 0) {
		throw Error(-error, "nla_put_u64");
	}
}

void Message::put_string_attr(int attrtype, const char str[]) {
	int error;
	if ((error = ::nla_put_string(msg, attrtype, str)) < 0) {
		throw Error(-error, "nla_put_string");
	}
}

void Message::put_flag_attr(int attrtype) {
	int error;
	if ((error = ::nla_put_flag(msg, attrtype)) < 0) {
		throw Error(-error, "nla_put_flag");
	}
}

void Message::put_msecs_attr(int attrtype, unsigned long msecs) {
	int error;
	if ((error = ::nla_put_msecs(msg, attrtype, msecs)) < 0) {
		throw Error(-error, "nla_put_msecs");
	}
}

void Message::put_nested_attr(int attrtype, const Message &nested) {
	int error;
	if ((error = ::nla_put_nested(msg, attrtype, nested.msg)) < 0) {
		throw Error(-error, "nla_put_nested");
	}
}

Attribute * Message::nest_start(int attrtype) {
	Attribute *attr;
	if (!(attr = static_cast<Attribute *>(::nla_nest_start(msg, attrtype)))) {
		throw Error(NLE_NOMEM, "nla_nest_start");
	}
	return attr;
}

void Message::nest_end(Attribute *attr) {
	int error;
	if ((error = ::nla_nest_end(msg, attr)) < 0) {
		throw Error(-error, "nla_nest_end");
	}
}


Socket::Socket() : sk(::nl_socket_alloc()) {
	if (!sk) {
		throw Error(NLE_NOMEM, "nl_socket_alloc");
	}
}

void Socket::add_memberships(int group) {
	int error;
	if ((error = ::nl_socket_add_memberships(sk, group, NFNLGRP_NONE)) < 0) {
		throw Error(-error, "nl_socket_add_memberships");
	}
}

void Socket::drop_memberships(int group) {
	int error;
	if ((error = ::nl_socket_drop_memberships(sk, group, NFNLGRP_NONE)) < 0) {
		throw Error(-error, "nl_socket_drop_memberships");
	}
}

void Socket::set_nonblocking() {
	int error;
	if ((error = ::nl_socket_set_nonblocking(sk)) < 0) {
		throw Error(-error, "nl_socket_set_nonblocking");
	}
}

void Socket::set_buffer_size(int rxbuf, int txbuf) {
	int error;
	if ((error = ::nl_socket_set_buffer_size(sk, rxbuf, txbuf)) < 0) {
		throw Error(-error, "nl_socket_set_buffer_size");
	}
}

void Socket::set_msg_buf_size(size_t bufsize) {
	int error;
	if ((error = ::nl_socket_set_msg_buf_size(sk, bufsize)) < 0) {
		throw Error(-error, "nl_socket_set_msg_buf_size");
	}
}

void Socket::recv_pktinfo(bool state) {
	int error;
	if ((error = ::nl_socket_recv_pktinfo(sk, state)) < 0) {
		throw Error(-error, "nl_socket_recv_pktinfo");
	}
}

void Socket::connect(int protocol) {
	int error;
	if ((error = ::nl_connect(sk, protocol)) < 0) {
		throw Error(-error, "nl_connect");
	}
}

size_t Socket::send(Message &msg) {
	int s;
	if ((s = ::nl_send(sk, msg)) < 0) {
		throw Error(-s, "nl_send");
	}
	return s;
}

size_t Socket::send_auto(Message &msg) {
	int s;
	if ((s = ::nl_send_auto(sk, msg)) < 0) {
		throw Error(-s, "nl_send_auto");
	}
	return s;
}

void Socket::send_sync(Message &msg) {
	int error;
	if ((error = ::nl_send_sync(sk, msg)) < 0) {
		throw Error(-error, "nl_send_sync");
	}
}

size_t Socket::send_simple(int type, int flags, void *buf, size_t size) {
	int s;
	if ((s = ::nl_send_simple(sk, type, flags, buf, size)) < 0) {
		throw Error(-s, "nl_send_simple");
	}
	return s;
}

void Socket::recvmsgs() {
	int error;
	if ((error = ::nl_recvmsgs_default(sk)) < 0) {
		throw Error(-error, "nl_recvmsgs_default");
	}
}

void Socket::wait_for_ack() {
	int error;
	if ((error = ::nl_wait_for_ack(sk)) < 0) {
		throw Error(-error, "nl_wait_for_ack");
	}
}

void Socket::set_callback(enum nl_cb_type type, enum nl_cb_kind kind) {
	nl_recvmsg_msg_cb_t func;
	switch (type) {
		case NL_CB_VALID:
			func = reinterpret_cast<nl_recvmsg_msg_cb_t>(&Socket::cb_valid);
			break;
		case NL_CB_FINISH:
			func = reinterpret_cast<nl_recvmsg_msg_cb_t>(&Socket::cb_finish);
			break;
		case NL_CB_OVERRUN:
			func = reinterpret_cast<nl_recvmsg_msg_cb_t>(&Socket::cb_overrun);
			break;
		case NL_CB_SKIPPED:
			func = reinterpret_cast<nl_recvmsg_msg_cb_t>(&Socket::cb_skipped);
			break;
		case NL_CB_ACK:
			func = reinterpret_cast<nl_recvmsg_msg_cb_t>(&Socket::cb_ack);
			break;
		case NL_CB_MSG_IN:
			func = reinterpret_cast<nl_recvmsg_msg_cb_t>(&Socket::cb_msg_in);
			break;
		case NL_CB_MSG_OUT:
			func = reinterpret_cast<nl_recvmsg_msg_cb_t>(&Socket::cb_msg_out);
			break;
		case NL_CB_INVALID:
			func = reinterpret_cast<nl_recvmsg_msg_cb_t>(&Socket::cb_invalid);
			break;
		case NL_CB_SEQ_CHECK:
			func = reinterpret_cast<nl_recvmsg_msg_cb_t>(&Socket::cb_seq_check);
			break;
		case NL_CB_SEND_ACK:
			func = reinterpret_cast<nl_recvmsg_msg_cb_t>(&Socket::cb_send_ack);
			break;
		default:
			throw std::invalid_argument(__FUNCTION__);
	}
	int error;
	if ((error = ::nl_socket_modify_cb(sk, type, kind, func, this)) < 0) {
		throw Error(-error, "nl_socket_modify_cb");
	}
}

void Socket::set_err_callback(enum nl_cb_kind kind) {
	int error;
	if ((error = ::nl_socket_modify_err_cb(sk, kind, reinterpret_cast<nl_recvmsg_err_cb_t>(&Socket::cb_err), this)) < 0) {
		throw Error(-error, "nl_socket_modify_err_cb");
	}
}

void Socket::genl_connect() {
	int error;
	if ((error = ::genl_connect(sk)) < 0) {
		throw Error(-error, "genl_connect");
	}
}

void Socket::genl_send_simple(int family, int cmd, int version, int flags) {
	int error;
	if ((error = ::genl_send_simple(sk, family, cmd, version, flags)) < 0) {
		throw Error(-error, "genl_send_simple");
	}
}

int Socket::genl_ctrl_resolve(const char name[]) {
	int fid;
	if ((fid = ::genl_ctrl_resolve(sk, name)) < 0) {
		throw Error(-fid, "genl_ctrl_resolve");
	}
	return fid;
}

int Socket::genl_ctrl_resolve_grp(const char family_name[], const char grp_name[]) {
	int gid;
	if ((gid = ::genl_ctrl_resolve_grp(sk, family_name, grp_name)) < 0) {
		throw Error(-gid, "genl_ctrl_resolve_grp");
	}
	return gid;
}

enum nl_cb_action Socket::valid(Message &&) noexcept {
	return NL_OK;
}

enum nl_cb_action Socket::finish(Message &&) noexcept {
	return NL_STOP;
}

enum nl_cb_action Socket::overrun(Message &&) noexcept {
	return NL_STOP;
}

enum nl_cb_action Socket::skipped(Message &&) noexcept {
	return NL_SKIP;
}

enum nl_cb_action Socket::ack(Message &&) noexcept {
	return NL_STOP;
}

enum nl_cb_action Socket::msg_in(Message &&) noexcept {
	return NL_OK;
}

int Socket::msg_out(Message &&) noexcept {
	return NL_OK;
}

enum nl_cb_action Socket::invalid(Message &&) noexcept {
	return NL_STOP;
}

enum nl_cb_action Socket::seq_check(Message &&) noexcept {
	return NL_OK;
}

enum nl_cb_action Socket::send_ack(Message &&) noexcept {
	return NL_OK;
}

int Socket::err(const struct sockaddr_nl *, const struct nlmsgerr *) noexcept {
	return NL_OK;
}

enum nl_cb_action Socket::cb_valid(struct nl_msg *msg, Socket *socket) noexcept {
	return socket->valid(Message(msg));
}

enum nl_cb_action Socket::cb_finish(struct nl_msg *msg, Socket *socket) noexcept {
	return socket->finish(Message(msg));
}

enum nl_cb_action Socket::cb_overrun(struct nl_msg *msg, Socket *socket) noexcept {
	return socket->overrun(Message(msg));
}

enum nl_cb_action Socket::cb_skipped(struct nl_msg *msg, Socket *socket) noexcept {
	return socket->skipped(Message(msg));
}

enum nl_cb_action Socket::cb_ack(struct nl_msg *msg, Socket *socket) noexcept {
	return socket->ack(Message(msg));
}

enum nl_cb_action Socket::cb_msg_in(struct nl_msg *msg, Socket *socket) noexcept {
	return socket->msg_in(Message(msg));
}

int Socket::cb_msg_out(struct nl_msg *msg, Socket *socket) noexcept {
	return socket->msg_out(Message(msg));
}

enum nl_cb_action Socket::cb_invalid(struct nl_msg *msg, Socket *socket) noexcept {
	return socket->invalid(Message(msg));
}

enum nl_cb_action Socket::cb_seq_check(struct nl_msg *msg, Socket *socket) noexcept {
	return socket->seq_check(Message(msg));
}

enum nl_cb_action Socket::cb_send_ack(struct nl_msg *msg, Socket *socket) noexcept {
	return socket->send_ack(Message(msg));
}

int Socket::cb_err(struct sockaddr_nl *addr, struct nlmsgerr *nlerr, Socket *socket) noexcept {
	return socket->err(addr, nlerr);
}


} // namespace nl
