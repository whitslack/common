#include <array>
#include <cstdint>
#include <streambuf>

#include "compiler.h"
#include "http.h"
#include "socket.h"


class WebSocket {

public:
	enum Opcode {
		Continuation = 0x0, Text = 0x1, Binary = 0x2,
		Close = 0x8, Ping = 0x9, Pong = 0xA,
		End = -1
	};

public:
	Socket socket;

private:
	/**
	 * @brief The number of bytes of payload data remaining to be read in the current frame.
	 *
	 * This data member is valid only when the MSB of @ref recv_hdr_pos is set.
	 */
	size_t recv_data_rem = 0;

	/**
	 * @brief The mask that applies to the frame being read.
	 */
	uint32_t recv_mask;

	/**
	 * @brief A scratch area to hold receive state.
	 *
	 * If @ref recv_hdr_pos is non-zero, then the MSB of this data member holds the \c FIN bit of the frame being read.
	 */
	uint8_t recv_state = 0;

	/**
	 * @brief The number of bytes of header data that have been read in the current frame.
	 *
	 * If the MSB of this data member is set, then the header has been fully read.
	 */
	uint8_t recv_hdr_pos = 0;

	/**
	 * @brief Whether to include a mask when writing frames.
	 */
	const bool send_mask;

	/**
	 * @brief The number of bytes of header data that have been written in the current frame.
	 *
	 * If the MSB of this data member is set, then the header has been fully written.
	 */
	uint8_t send_hdr_pos = 0;

	/**
	 * @brief The number of bytes of payload data remaining to be written in the current frame.
	 */
	size_t send_data_rem = 0;

public:
	explicit WebSocket(Socket &&socket, bool send_mask) noexcept : socket(std::move(socket)), send_mask(send_mask) { }

public:
	/**
	 * @brief Returns whether the current frame is the final frame of its message.
	 *
	 * This should be called only after at least one byte of payload data has been received.
	 * Typically it will be called after \ref receive has indicated the end of the frame.
	 */
	bool _pure is_final() const noexcept { return static_cast<int8_t>(recv_state) < 0; }

	/**
	 * @brief Returns whether the current frame has been fully received.
	 *
	 * If this function returns \c true, then @ref receive is guaranteed to return negative without blocking.
	 */
	bool _pure is_frame_fully_received() const noexcept { return static_cast<int8_t>(recv_hdr_pos) < 0 && recv_data_rem == 0; }

	/**
	 * @brief Receives (part of) a WebSocket frame.
	 *
	 * @param[out] opcode The opcode of the received frame.
	 * This is set only when the beginning of the frame is read, at which time this function may yet return zero(!).
	 * When the end of the connection is reached without reading any (more) payload data, this is set to \ref Opcode::End.
	 * @param[out] buf The buffer into which to store the received payload data bytes.
	 * Bytes are stored starting immediately at this location, even if some payload data of the current frame were already received in a previous call.
	 * @param[in] n The size of the buffer at \p buf.
	 * @return the number of bytes of payload data that were read and stored into \p buf,
	 * or zero if no bytes of payload data were read,
	 * or negative if the end of the current frame or the end of the connection was reached without reading any (more) payload data.
	 */
	_nodiscard ssize_t receive(Opcode &opcode, void *buf, size_t n);

	/**
	 * @brief Sends a WebSocket frame.
	 *
	 * @param[in] opcode The opcode of the frame to send.
	 * @param[in] buf The buffer containing the payload data of the frame.
	 * All payload data of the frame must be supplied. A lengthy message may be split into multiple frames using continuation frames.
	 * @param[in] n The number of bytes of payload data of the frame.
	 * @param[in] more Whether the message being sent will continue in a subsequent frame.
	 * This controls whether the current frame is sent with the \c FIN bit set.
	 * @return whether the frame was completely sent.
	 * If this function returns \c false, then it should be called again **with exactly the same arguments** after the underlying socket has become writable again.
	 */
	_nodiscard bool send(Opcode opcode, const void *buf, size_t n, bool more = false);

};


class WebSocketServerHandshake {

public:
	Socket socket;

private:
	std::array<uint8_t, 1460> request_buf;
	size_t request_pos;

public:
	WebSocketServerHandshake(Socket &&socket) noexcept : socket(std::move(socket)), request_pos() { }

public:
	bool ready();

protected:
	virtual std::pair<unsigned, const char *> validate_request_headers(const HttpRequestHeaders &request_headers) _pure;
	virtual void prepare_response_headers(const HttpRequestHeaders &request_headers, HttpResponseHeaders &response_headers);
	virtual void connected(const HttpRequestHeaders &request_headers, const HttpResponseHeaders &response_headers) = 0;

private:
	void send_error(int status_code, const char reason_phrase[]);

};


class WebSocketClientHandshake {

public:
	Socket socket;

private:
	std::string key;
	DelimitedSource delimited_source;
	std::array<uint8_t, 1460> response_buf;
	size_t response_pos;

public:
	WebSocketClientHandshake(Socket &&socket) noexcept : socket(std::move(socket)), delimited_source(this->socket, "\r\n\r\n"), response_pos() { }

public:
	void start(const char host[], in_port_t port = 0, const char request_uri[] = "/");
	bool ready();

protected:
	virtual void prepare_request_headers(HttpRequestHeaders &request_headers);
	virtual void validate_response_headers(const HttpResponseHeaders &response_headers);
	virtual void connected(const HttpResponseHeaders &response_headers) = 0;

};


class WebSocketBuf : public std::streambuf {

protected:
	WebSocket * const ws;
	WebSocket::Opcode opcode;
	std::array<char_type, 1 << 12> buf;

public:
	WebSocketBuf(WebSocket *ws, WebSocket::Opcode opcode = WebSocket::Text) noexcept;

protected:
	virtual int sync(bool more);
	int sync() override final;
	int_type overflow(int_type ch) override;

};


Socket connect_websocket(const char host[], in_port_t port, const char request_uri[], const char origin_uri[] = nullptr);
