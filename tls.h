#include <memory>
#include <string_view>
#include <system_error>
#include <vector>

#include <gnutls/gnutls.h>

#include "buffer.h"
#include "compiler.h"
#include "socket.h"


class TLSCategory : public std::error_category {

public:
	const char * name() const noexcept override _const;
	std::string message(int condition) const override _const;

};


class TLSError : public std::system_error {

public:
	template <typename... Args>
	TLSError(int ev, Args&&... args) : std::system_error(ev, TLSCategory(), std::forward<Args>(args)...) { }

};


class TLSPriority {

private:
	gnutls_priority_t pcache;

public:
	TLSPriority() noexcept : pcache() { }
	explicit TLSPriority(const char priorities[], const char **err_pos = nullptr);
	TLSPriority(TLSPriority &&move) noexcept : pcache(move.pcache) { move.pcache = nullptr; }
	TLSPriority & operator = (TLSPriority &&move) noexcept { return this->swap(move), *this; }
	~TLSPriority() { if (pcache) ::gnutls_priority_deinit(pcache); }
	void swap(TLSPriority &other) noexcept { using std::swap; swap(pcache, other.pcache); }
	friend void swap(TLSPriority &lhs, TLSPriority &rhs) noexcept { lhs.swap(rhs); }
	_pure operator gnutls_priority_t () const noexcept { return pcache; }

private:
	TLSPriority(const TLSPriority &) = delete;
	TLSPriority & operator = (const TLSPriority &) = delete;

public:
	std::vector<unsigned int> certificate_type_list();
	std::vector<unsigned int> ecc_curve_list();
	std::vector<unsigned int> protocol_list();
	std::vector<unsigned int> sign_list();

};


class TLSCertificateCredentials {

private:
	gnutls_certificate_credentials_t cred;

public:
	TLSCertificateCredentials();
	TLSCertificateCredentials(TLSCertificateCredentials &&move) noexcept : cred(move.cred) { move.cred = nullptr; }
	TLSCertificateCredentials & operator = (TLSCertificateCredentials &&move) noexcept { return this->swap(move), *this; }
	~TLSCertificateCredentials() { if (cred) ::gnutls_certificate_free_credentials(cred); }
	void swap(TLSCertificateCredentials &other) noexcept { using std::swap; swap(cred, other.cred); }
	friend void swap(TLSCertificateCredentials &lhs, TLSCertificateCredentials &rhs) noexcept { lhs.swap(rhs); }
	_pure operator gnutls_certificate_credentials_t () const noexcept { return cred; }

private:
	TLSCertificateCredentials(const TLSCertificateCredentials &) = delete;
	TLSCertificateCredentials & operator = (const TLSCertificateCredentials &) = delete;

public:
	size_t set_x509_system_trust();
	size_t set_x509_trust_file(const char ca_file[], gnutls_x509_crt_fmt_t type = GNUTLS_X509_FMT_PEM);

};


/*
class TLSSRPServerCredentials {

private:
	gnutls_srp_server_credentials_t cred;

public:
	TLSSRPServerCredentials();
	TLSSRPServerCredentials(TLSSRPServerCredentials &&move) noexcept : cred(move.cred) { move.cred = nullptr; }
	TLSSRPServerCredentials & operator = (TLSSRPServerCredentials &&move) noexcept { return this->swap(move), *this; }
	~TLSSRPServerCredentials() { if (cred) ::gnutls_srp_free_server_credentials(cred); }
	void swap(TLSSRPServerCredentials &other) noexcept { using std::swap; swap(cred, other.cred); }
	friend void swap(TLSSRPServerCredentials &lhs, TLSSRPServerCredentials &rhs) noexcept { lhs.swap(rhs); }
	_pure operator gnutls_srp_server_credentials_t () const noexcept { return cred; }

private:
	TLSSRPServerCredentials(const TLSSRPServerCredentials &) = delete;
	TLSSRPServerCredentials & operator = (const TLSSRPServerCredentials &) = delete;

};


class TLSSRPClientCredentials {

private:
	gnutls_srp_client_credentials_t cred;

public:
	TLSSRPClientCredentials();
	TLSSRPClientCredentials(TLSSRPClientCredentials &&move) noexcept : cred(move.cred) { move.cred = nullptr; }
	TLSSRPClientCredentials & operator = (TLSSRPClientCredentials &&move) noexcept { return this->swap(move), *this; }
	~TLSSRPClientCredentials() { if (cred) ::gnutls_srp_free_client_credentials(cred); }
	void swap(TLSSRPClientCredentials &other) noexcept { using std::swap; swap(cred, other.cred); }
	friend void swap(TLSSRPClientCredentials &lhs, TLSSRPClientCredentials &rhs) noexcept { lhs.swap(rhs); }
	_pure operator gnutls_srp_client_credentials_t () const noexcept { return cred; }

private:
	TLSSRPClientCredentials(const TLSSRPClientCredentials &) = delete;
	TLSSRPClientCredentials & operator = (const TLSSRPClientCredentials &) = delete;

};


class TLSPSKServerCredentials {

private:
	gnutls_psk_server_credentials_t cred;

public:
	TLSPSKServerCredentials();
	TLSPSKServerCredentials(TLSPSKServerCredentials &&move) noexcept : cred(move.cred) { move.cred = nullptr; }
	TLSPSKServerCredentials & operator = (TLSPSKServerCredentials &&move) noexcept { return this->swap(move), *this; }
	~TLSPSKServerCredentials() { if (cred) ::gnutls_psk_free_server_credentials(cred); }
	void swap(TLSPSKServerCredentials &other) noexcept { using std::swap; swap(cred, other.cred); }
	friend void swap(TLSPSKServerCredentials &lhs, TLSPSKServerCredentials &rhs) noexcept { lhs.swap(rhs); }
	_pure operator gnutls_psk_server_credentials_t () const noexcept { return cred; }

private:
	TLSPSKServerCredentials(const TLSPSKServerCredentials &) = delete;
	TLSPSKServerCredentials & operator = (const TLSPSKServerCredentials &) = delete;

};


class TLSPSKClientCredentials {

private:
	gnutls_psk_client_credentials_t cred;

public:
	TLSPSKClientCredentials();
	TLSPSKClientCredentials(TLSPSKClientCredentials &&move) noexcept : cred(move.cred) { move.cred = nullptr; }
	TLSPSKClientCredentials & operator = (TLSPSKClientCredentials &&move) noexcept { return this->swap(move), *this; }
	~TLSPSKClientCredentials() { if (cred) ::gnutls_psk_free_client_credentials(cred); }
	void swap(TLSPSKClientCredentials &other) noexcept { using std::swap; swap(cred, other.cred); }
	friend void swap(TLSPSKClientCredentials &lhs, TLSPSKClientCredentials &rhs) noexcept { lhs.swap(rhs); }
	_pure operator gnutls_psk_client_credentials_t () const noexcept { return cred; }

private:
	TLSPSKClientCredentials(const TLSPSKClientCredentials &) = delete;
	TLSPSKClientCredentials & operator = (const TLSPSKClientCredentials &) = delete;

};


class TLSAnonServerCredentials {

private:
	gnutls_anon_server_credentials_t cred;

public:
	TLSAnonServerCredentials();
	TLSAnonServerCredentials(TLSAnonServerCredentials &&move) noexcept : cred(move.cred) { move.cred = nullptr; }
	TLSAnonServerCredentials & operator = (TLSAnonServerCredentials &&move) noexcept { return this->swap(move), *this; }
	~TLSAnonServerCredentials() { if (cred) ::gnutls_anon_free_server_credentials(cred); }
	void swap(TLSAnonServerCredentials &other) noexcept { using std::swap; swap(cred, other.cred); }
	friend void swap(TLSAnonServerCredentials &lhs, TLSAnonServerCredentials &rhs) noexcept { lhs.swap(rhs); }
	_pure operator gnutls_anon_server_credentials_t () const noexcept { return cred; }

private:
	TLSAnonServerCredentials(const TLSAnonServerCredentials &) = delete;
	TLSAnonServerCredentials & operator = (const TLSAnonServerCredentials &) = delete;

};


class TLSAnonClientCredentials {

private:
	gnutls_anon_client_credentials_t cred;

public:
	TLSAnonClientCredentials();
	TLSAnonClientCredentials(TLSAnonClientCredentials &&move) noexcept : cred(move.cred) { move.cred = nullptr; }
	TLSAnonClientCredentials & operator = (TLSAnonClientCredentials &&move) noexcept { return this->swap(move), *this; }
	~TLSAnonClientCredentials() { if (cred) ::gnutls_anon_free_client_credentials(cred); }
	void swap(TLSAnonClientCredentials &other) noexcept { using std::swap; swap(cred, other.cred); }
	friend void swap(TLSAnonClientCredentials &lhs, TLSAnonClientCredentials &rhs) noexcept { lhs.swap(rhs); }
	_pure operator gnutls_anon_client_credentials_t () const noexcept { return cred; }

private:
	TLSAnonClientCredentials(const TLSAnonClientCredentials &) = delete;
	TLSAnonClientCredentials & operator = (const TLSAnonClientCredentials &) = delete;

};
*/


class TLSSession : public Source, public Sink {
	friend TLSCertificateCredentials;

private:
	gnutls_session_t session;
	std::shared_ptr<void> credentials;

public:
	TLSSession();
	TLSSession(TLSSession &&move) noexcept;
	TLSSession & operator = (TLSSession &&move) noexcept { return this->swap(move), *this; }
	virtual ~TLSSession() { if (session) ::gnutls_deinit(session); }
	void swap(TLSSession &other) noexcept;
	friend void swap(TLSSession &lhs, TLSSession &rhs) noexcept { lhs.swap(rhs); }
	_pure operator gnutls_session_t () const noexcept { return session; }

private:
	TLSSession(const TLSSession &) = delete;
	TLSSession & operator = (const TLSSession &) = delete;

public:
	void set_priority(const TLSPriority &priority);
	void set_priority_direct(const char priorities[], const char **err_pos = nullptr);
	void set_credentials(std::shared_ptr<TLSCertificateCredentials> credentials);
/*
	void set_credentials(std::shared_ptr<TLSSRPClientCredentials> credentials);
	void set_credentials(std::shared_ptr<TLSPSKClientCredentials> credentials);
	void set_credentials(std::shared_ptr<TLSAnonClientCredentials> credentials);
*/
	std::string get_server_name();
	void set_server_name(std::string_view server_name);
	DynamicBuffer get_session_data();
	void set_session_data(const void *session_data, size_t session_data_size);
	unsigned int verify_peers(const char hostname[] = nullptr);
	bool handshake();
	_nodiscard ssize_t read(void *buf, size_t n) override;
	_nodiscard size_t write(const void *buf, size_t n) override;
	bool flush() override;
	bool bye(gnutls_close_request_t how = GNUTLS_SHUT_RDWR);

protected:
	virtual ssize_t pull(void *buf, size_t n) = 0;
	virtual bool pull_timeout(unsigned int ms) = 0;
	virtual size_t push(const void *buf, size_t n) = 0;
	virtual size_t vec_push(const struct iovec iov[], size_t iovcnt) = 0;
	virtual bool verify() { return true; }

private:
	static ssize_t pull(gnutls_transport_ptr_t ptr, void *buf, size_t n) noexcept;
	static int pull_timeout(gnutls_transport_ptr_t ptr, unsigned int ms) noexcept;
	static ssize_t push(gnutls_transport_ptr_t ptr, const void *buf, size_t n) noexcept;
	static ssize_t vec_push(gnutls_transport_ptr_t ptr, const giovec_t iov[], int iovcnt) noexcept;
	static int verify(gnutls_session_t session) noexcept;

};


class TLSSocket : public TLSSession {

public:
	std::string host_name;

protected:
	Socket socket;

public:
	TLSSocket() = default;
	TLSSocket(std::string host_name, Socket &&socket);

protected:
	ssize_t pull(void *buf, size_t n) override;
	bool pull_timeout(unsigned int ms) override;
	size_t push(const void *buf, size_t n) override;
	size_t vec_push(const struct iovec iov[], size_t iovcnt) override;
	bool verify() override;

};
