#include <system_error>

#include <unistd.h>
#include <netlink/msg.h>
#include <netlink/genl/genl.h>

#include "compiler.h"

namespace nl {


class Category : public std::error_category {

public:
	const char * name() const noexcept override _const;
	std::string message(int condition) const noexcept override _const;

};


class Error : public std::system_error {

public:
	template <typename... Args>
	Error(int ev, Args&&... args) : std::system_error(ev, Category(), std::forward<Args>(args)...) { }

};


struct Attribute : nlattr {

	int type() const { return ::nla_type(this); }
	void * data() const { return ::nla_data(this); }
	int len() const { return ::nla_len(this); }
	bool ok(int remaining) const { return ::nla_ok(this, remaining); }
	Attribute * next(int &remaining) const { return static_cast<Attribute *>(::nla_next(this, &remaining)); }
	Attribute * find(int len, int attrtype) { return static_cast<Attribute *>(::nla_find(this, len, attrtype)); }

	uint8_t get_u8() { return ::nla_get_u8(this); }
	uint16_t get_u16() { return ::nla_get_u16(this); }
	uint32_t get_u32() { return ::nla_get_u32(this); }
	uint64_t get_u64() { return ::nla_get_u64(this); }
	char * get_string() { return ::nla_get_string(this); }
	bool get_flag() { return ::nla_get_flag(this); }
	unsigned long get_msecs() { return ::nla_get_msecs(this); }
	bool is_nested() { return ::nla_is_nested(this); }

};


struct GenlMessageHeader : genlmsghdr {

	int len() const { return ::genlmsg_len(this); }
	void * user_hdr() const { return ::genlmsg_user_hdr(this); }
	void * user_data(int hdrlen) const { return ::genlmsg_user_data(this, hdrlen); }
	int user_datalen(int hdrlen) const { return ::genlmsg_user_datalen(this, hdrlen); }
	Attribute * attrdata(int hdrlen) const { return static_cast<Attribute *>(::genlmsg_attrdata(this, hdrlen)); }
	int attrlen(int hdrlen) const { return ::genlmsg_attrlen(this, hdrlen); }

};


struct MessageHeader : nlmsghdr {

	void *data() const { return ::nlmsg_data(this); }
	void *tail() const { return ::nlmsg_tail(this); }
	int datalen() const { return ::nlmsg_datalen(this); }

	Attribute * attrdata(int hdrlen) const { return static_cast<Attribute *>(::nlmsg_attrdata(this, hdrlen)); }
	int attrlen(int hdrlen) const { return ::nlmsg_attrlen(this, hdrlen); }

	bool valid_hdr(int hdrlen) const { return ::nlmsg_valid_hdr(this, hdrlen); }
	bool ok(int remaining) const { return ::nlmsg_ok(this, remaining); }
	MessageHeader * next(int &remaining) { return static_cast<MessageHeader *>(::nlmsg_next(this, &remaining)); }
	Attribute * find_attr(int hdrlen, int attrtype) { return static_cast<Attribute *>(::nlmsg_find_attr(this, hdrlen, attrtype)); }

	bool genl_valid_hdr(int hdrlen) { return ::genlmsg_valid_hdr(this, hdrlen); }
	GenlMessageHeader * genl_hdr() { return static_cast<GenlMessageHeader *>(::genlmsg_hdr(this)); }

};


class Message {

private:
	struct nl_msg *msg;

public:
	explicit Message(size_t max = ::sysconf(_SC_PAGESIZE));
	explicit Message(struct nl_msg *msg) : msg(msg) { ::nlmsg_get(msg); }
	Message(Message &&move) : msg(move.msg) { move.msg = nullptr; }
	Message & operator = (Message &&move) { std::swap(msg, move.msg); return *this; }
	~Message() { if (msg) ::nlmsg_free(msg), msg = nullptr; }
	operator struct nl_msg * () { return msg; }
	operator const struct nl_msg * () const { return msg; }

private:
	Message(const Message &) = delete;
	Message & operator = (const Message &) = delete;

public:
	void * reserve(size_t len, int pad);
	void append(void *data, size_t len, int pad);
	void expand(size_t newlen);
	MessageHeader * put_hdr(uint32_t port, uint32_t seq, int type, int payload_len, int flags);
	MessageHeader * hdr() { return static_cast<MessageHeader *>(::nlmsg_hdr(msg)); }

	int get_proto() { return ::nlmsg_get_proto(msg); }
	void set_proto(int protocol) { ::nlmsg_set_proto(msg, protocol); }

	size_t get_max_size() { return ::nlmsg_get_max_size(msg); }

	struct sockaddr_nl * get_src() { return ::nlmsg_get_src(msg); }
	void set_src(struct sockaddr_nl *addr) { ::nlmsg_set_src(msg, addr); }

	struct sockaddr_nl * get_dst() { return ::nlmsg_get_dst(msg); }
	void set_dst(struct sockaddr_nl *addr) { ::nlmsg_set_dst(msg, addr); }

	struct ucred * get_creds() { return ::nlmsg_get_creds(msg); }
	void set_creds(struct ucred *creds) { ::nlmsg_set_creds(msg, creds); }

	void * put_genl_hdr(uint32_t port, uint32_t seq, int family, int hdrlen, int flags, uint8_t cmd, uint8_t version);

	Attribute * reserve_attr(int attrtype, int attrlen);
	void put_attr(int attrtype, int datalen, const void *data);
	void put_data_attr(int attrtype, struct nl_data *data);
	void put_addr_attr(int attrtype, struct nl_addr *addr);
	void put_u8_attr(int attrtype, uint8_t value);
	void put_u16_attr(int attrtype, uint16_t value);
	void put_u32_attr(int attrtype, uint32_t value);
	void put_u64_attr(int attrtype, uint64_t value);
	void put_string_attr(int attrtype, const char str[]);
	void put_flag_attr(int attrtype);
	void put_msecs_attr(int attrtype, unsigned long msecs);
	void put_nested_attr(int attrtype, const Message &nested);
	Attribute * nest_start(int attrtype);
	void nest_end(Attribute *attr);
	void nest_cancel(Attribute *attr) { ::nla_nest_cancel(msg, attr); }

};


class Socket {

private:
	struct nl_sock *sk;

public:
	Socket();
	Socket(Socket &&move) : sk(move.sk) { move.sk = nullptr; }
	Socket & operator = (Socket &&move) { std::swap(sk, move.sk); return *this; }
	virtual ~Socket() { if (sk) ::nl_socket_free(sk), sk = nullptr; }
	operator struct nl_sock * () { return sk; }
	operator const struct nl_sock * () const { return sk; }
	operator int () const { return ::nl_socket_get_fd(sk); }

private:
	Socket(const Socket &) = delete;
	Socket & operator = (const Socket &) = delete;

public:
	void disable_seq_check() { ::nl_socket_disable_seq_check(sk); }
	unsigned int use_seq() { return ::nl_socket_use_seq(sk); }
	void disable_auto_ack() { ::nl_socket_disable_auto_ack(sk); }
	void enable_auto_ack() { ::nl_socket_enable_auto_ack(sk); }

	uint32_t get_local_port() const { return ::nl_socket_get_local_port(sk); }
	void set_local_port(uint32_t port) { ::nl_socket_set_local_port(sk, port); }

	void add_memberships(int group);
	template <typename... Args>
	void add_memberships(Args... group) {
		int error;
		if ((error = ::nl_socket_add_memberships(sk, group..., NFNLGRP_NONE)) < 0) {
			throw Error(-error, "nl_socket_add_memberships");
		}
	}
	void drop_memberships(int group);
	template <typename... Args>
	void drop_memberships(Args... group) {
		int error;
		if ((error = ::nl_socket_drop_memberships(sk, group..., NFNLGRP_NONE)) < 0) {
			throw Error(-error, "nl_socket_drop_memberships");
		}
	}

	uint32_t get_peer_port() const { return ::nl_socket_get_peer_port(sk); }
	void set_peer_port(uint32_t port) { ::nl_socket_set_peer_port(sk, port); }
	uint32_t get_peer_groups() const { return ::nl_socket_get_peer_groups(sk); }
	void set_peer_groups(uint32_t groups) { ::nl_socket_set_peer_groups(sk, groups); }

	void set_nonblocking();
	void enable_msg_peek() { ::nl_socket_enable_msg_peek(sk); }
	void disable_msg_peek() { ::nl_socket_disable_msg_peek(sk); }

	void set_buffer_size(int rxbuf, int txbuf);
	size_t get_msg_buf_size() const { return ::nl_socket_get_msg_buf_size(sk); }
	void set_msg_buf_size(size_t bufsize);
	void recv_pktinfo(bool state);

	void connect(int protocol);
	void close() { ::nl_close(sk); }

	size_t send(Message &msg);
	void complete_msg(Message &msg) { ::nl_complete_msg(sk, msg); }
	size_t send_auto(Message &msg);
	void send_sync(Message &msg);
	size_t send_simple(int type, int flags = 0, void *buf = nullptr, size_t size = 0);

	void recvmsgs();
	void wait_for_ack();

	void set_callback(enum nl_cb_type type, enum nl_cb_kind kind);
	void set_err_callback(enum nl_cb_kind kind);

	void genl_connect();

	void genl_send_simple(int family, int cmd, int version, int flags = 0);

	int genl_ctrl_resolve(const char name[]);
	int genl_ctrl_resolve_grp(const char family_name[], const char grp_name[]);

protected:
	virtual enum nl_cb_action valid(Message &&msg) noexcept _const;
	virtual enum nl_cb_action finish(Message &&msg) noexcept _const;
	virtual enum nl_cb_action overrun(Message &&msg) noexcept _const;
	virtual enum nl_cb_action skipped(Message &&msg) noexcept _const;
	virtual enum nl_cb_action ack(Message &&msg) noexcept _const;
	virtual enum nl_cb_action msg_in(Message &&msg) noexcept _const;
	virtual int msg_out(Message &&msg) noexcept _const;
	virtual enum nl_cb_action invalid(Message &&msg) noexcept _const;
	virtual enum nl_cb_action seq_check(Message &&msg) noexcept _const;
	virtual enum nl_cb_action send_ack(Message &&msg) noexcept _const;
	virtual int err(const struct sockaddr_nl *addr, const struct nlmsgerr *nlerr) noexcept _const;

private:
	static enum nl_cb_action cb_valid(struct nl_msg *msg, Socket *socket) noexcept;
	static enum nl_cb_action cb_finish(struct nl_msg *msg, Socket *socket) noexcept;
	static enum nl_cb_action cb_overrun(struct nl_msg *msg, Socket *socket) noexcept;
	static enum nl_cb_action cb_skipped(struct nl_msg *msg, Socket *socket) noexcept;
	static enum nl_cb_action cb_ack(struct nl_msg *msg, Socket *socket) noexcept;
	static enum nl_cb_action cb_msg_in(struct nl_msg *msg, Socket *socket) noexcept;
	static int cb_msg_out(struct nl_msg *msg, Socket *socket) noexcept;
	static enum nl_cb_action cb_invalid(struct nl_msg *msg, Socket *socket) noexcept;
	static enum nl_cb_action cb_seq_check(struct nl_msg *msg, Socket *socket) noexcept;
	static enum nl_cb_action cb_send_ack(struct nl_msg *msg, Socket *socket) noexcept;
	static int cb_err(struct sockaddr_nl *addr, struct nlmsgerr *nlerr, Socket *socket) noexcept;

};


} // namespace nl
