#include "tls.h"

#include <poll.h>


namespace {
struct GnuTLS_Global_Init {
	GnuTLS_Global_Init() {
		int error;
		if ((error = ::gnutls_global_init()) < 0) {
			throw TLSError(error, "gnutls_global_init");
		}
	}
	~GnuTLS_Global_Init() {
		::gnutls_global_deinit();
	}
} __GnuTLS_Global_Init;
}


const char * TLSCategory::name() const noexcept {
	return "GnuTLS";
}

std::string TLSCategory::message(int condition) const {
	return ::gnutls_strerror(condition);
}


TLSPriority::TLSPriority(const char priorities[], const char **err_pos) : pcache() {
	int error;
	if ((error = ::gnutls_priority_init(&pcache, priorities, err_pos)) != GNUTLS_E_SUCCESS) {
		throw TLSError(error, "gnutls_priority_init");
	}
}

std::vector<unsigned int> TLSPriority::certificate_type_list() {
	int ret;
	const unsigned int *list;
	if ((ret = ::gnutls_priority_certificate_type_list(pcache, &list)) < 0) {
		throw TLSError(ret, "gnutls_priority_certificate_type_list");
	}
	return std::vector<unsigned int>(list, list + ret);
}

std::vector<unsigned int> TLSPriority::compression_list() {
	int ret;
	const unsigned int *list;
	if ((ret = ::gnutls_priority_compression_list(pcache, &list)) < 0) {
		throw TLSError(ret, "gnutls_priority_compression_list");
	}
	return std::vector<unsigned int>(list, list + ret);
}

std::vector<unsigned int> TLSPriority::ecc_curve_list() {
	int ret;
	const unsigned int *list;
	if ((ret = ::gnutls_priority_ecc_curve_list(pcache, &list)) < 0) {
		throw TLSError(ret, "gnutls_priority_ecc_curve_list");
	}
	return std::vector<unsigned int>(list, list + ret);
}

std::vector<unsigned int> TLSPriority::protocol_list() {
	int ret;
	const unsigned int *list;
	if ((ret = ::gnutls_priority_protocol_list(pcache, &list)) < 0) {
		throw TLSError(ret, "gnutls_priority_protocol_list");
	}
	return std::vector<unsigned int>(list, list + ret);
}

std::vector<unsigned int> TLSPriority::sign_list() {
	int ret;
	const unsigned int *list;
	if ((ret = ::gnutls_priority_sign_list(pcache, &list)) < 0) {
		throw TLSError(ret, "gnutls_priority_sign_list");
	}
	return std::vector<unsigned int>(list, list + ret);
}


TLSCertificateCredentials::TLSCertificateCredentials() : cred() {
	int error;
	if ((error = ::gnutls_certificate_allocate_credentials(&cred)) != GNUTLS_E_SUCCESS) {
		throw TLSError(error, "gnutls_certificate_allocate_credentials");
	}
	::gnutls_certificate_set_verify_function(cred, &TLSSession::verify);
}

size_t TLSCertificateCredentials::set_x509_system_trust() {
	int ret;
	if ((ret = ::gnutls_certificate_set_x509_system_trust(cred)) < 0) {
		throw TLSError(ret, "gnutls_certificate_set_x509_system_trust");
	}
	return ret;
}

size_t TLSCertificateCredentials::set_x509_trust_file(const char ca_file[], gnutls_x509_crt_fmt_t type) {
	int ret;
	if ((ret = ::gnutls_certificate_set_x509_trust_file(cred, ca_file, type)) < 0) {
		throw TLSError(ret, "gnutls_certificate_set_x509_trust_file");
	}
	return ret;
}


/*
TLSSRPServerCredentials::TLSSRPServerCredentials() : cred() {
	int error;
	if ((error = ::gnutls_srp_allocate_server_credentials(&cred)) != GNUTLS_E_SUCCESS) {
		throw TLSError(error, "gnutls_srp_allocate_server_credentials");
	}
}


TLSSRPClientCredentials::TLSSRPClientCredentials() : cred() {
	int error;
	if ((error = ::gnutls_srp_allocate_client_credentials(&cred)) != GNUTLS_E_SUCCESS) {
		throw TLSError(error, "gnutls_srp_allocate_client_credentials");
	}
}


TLSPSKServerCredentials::TLSPSKServerCredentials() : cred() {
	int error;
	if ((error = ::gnutls_psk_allocate_server_credentials(&cred)) != GNUTLS_E_SUCCESS) {
		throw TLSError(error, "gnutls_psk_allocate_server_credentials");
	}
}


TLSPSKClientCredentials::TLSPSKClientCredentials() : cred() {
	int error;
	if ((error = ::gnutls_psk_allocate_client_credentials(&cred)) != GNUTLS_E_SUCCESS) {
		throw TLSError(error, "gnutls_psk_allocate_client_credentials");
	}
}


TLSAnonServerCredentials::TLSAnonServerCredentials() : cred() {
	int error;
	if ((error = ::gnutls_anon_allocate_server_credentials(&cred)) != GNUTLS_E_SUCCESS) {
		throw TLSError(error, "gnutls_anon_allocate_server_credentials");
	}
}


TLSAnonClientCredentials::TLSAnonClientCredentials() : cred() {
	int error;
	if ((error = ::gnutls_anon_allocate_client_credentials(&cred)) != GNUTLS_E_SUCCESS) {
		throw TLSError(error, "gnutls_anon_allocate_client_credentials");
	}
}
*/


TLSSession::TLSSession() {
	int error;
	if ((error = ::gnutls_init(&session, GNUTLS_CLIENT)) < 0) {
		throw TLSError(error, "gnutls_init");
	}
	::gnutls_session_set_ptr(session, this);
	::gnutls_transport_set_ptr(session, this);
	::gnutls_transport_set_pull_function(session, &pull);
	::gnutls_transport_set_pull_timeout_function(session, &pull_timeout);
	::gnutls_transport_set_push_function(session, &push);
	::gnutls_transport_set_vec_push_function(session, &vec_push);
}

TLSSession::TLSSession(TLSSession &&move) noexcept : session(move.session), credentials(std::move(move.credentials)) {
	move.session = nullptr;
	::gnutls_session_set_ptr(session, this);
	::gnutls_transport_set_ptr(session, this);
}

void TLSSession::swap(TLSSession &other) noexcept {
	using std::swap;
	swap(session, other.session);
	swap(credentials, other.credentials);
	::gnutls_session_set_ptr(session, this);
	::gnutls_session_set_ptr(other.session, &other);
	::gnutls_transport_set_ptr(session, this);
	::gnutls_transport_set_ptr(other.session, &other);
}

void TLSSession::set_priority(const TLSPriority &priority) {
	int error;
	if ((error = ::gnutls_priority_set(session, priority)) != GNUTLS_E_SUCCESS) {
		throw TLSError(error, "gnutls_priority_set");
	}
}

void TLSSession::set_priority_direct(const char priorities[], const char **err_pos) {
	int error;
	if ((error = ::gnutls_priority_set_direct(session, priorities, err_pos)) != GNUTLS_E_SUCCESS) {
		throw TLSError(error, "gnutls_priority_set_direct");
	}
}

void TLSSession::set_credentials(const std::shared_ptr<TLSCertificateCredentials> &credentials) {
	int error;
	if ((error = ::gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, *credentials)) != GNUTLS_E_SUCCESS) {
		throw TLSError(error, "gnutls_credentials_set");
	}
	this->credentials = credentials;
}

/*
void TLSSession::set_credentials(const std::shared_ptr<TLSSRPClientCredentials> &credentials) {
	int error;
	if ((error = ::gnutls_credentials_set(session, GNUTLS_CRD_SRP, *credentials)) != GNUTLS_E_SUCCESS) {
		throw TLSError(error, "gnutls_credentials_set");
	}
	this->credentials = credentials;
}

void TLSSession::set_credentials(const std::shared_ptr<TLSPSKClientCredentials> &credentials) {
	int error;
	if ((error = ::gnutls_credentials_set(session, GNUTLS_CRD_PSK, *credentials)) != GNUTLS_E_SUCCESS) {
		throw TLSError(error, "gnutls_credentials_set");
	}
	this->credentials = credentials;
}

void TLSSession::set_credentials(const std::shared_ptr<TLSAnonClientCredentials> &credentials) {
	int error;
	if ((error = ::gnutls_credentials_set(session, GNUTLS_CRD_ANON, *credentials)) != GNUTLS_E_SUCCESS) {
		throw TLSError(error, "gnutls_credentials_set");
	}
	this->credentials = credentials;
}
*/

std::string TLSSession::get_server_name() {
	std::string server_name;
	int error;
	size_t size = 0;
	unsigned int type;
	if ((error = ::gnutls_server_name_get(session, nullptr, &size, &type, 0)) != GNUTLS_E_SUCCESS && error != GNUTLS_E_SHORT_MEMORY_BUFFER) {
		if (error == GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE) {
			return server_name;
		}
		throw TLSError(error, "gnutls_server_name_get");
	}
	server_name.resize(size);
	if ((error = ::gnutls_server_name_get(session, &server_name.front(), &size, &type, 0)) != GNUTLS_E_SUCCESS) {
		throw TLSError(error, "gnutls_server_name_get");
	}
	return server_name;
}

void TLSSession::set_server_name(const char server_name[], size_t len) {
	int error;
	if ((error = ::gnutls_server_name_set(session, GNUTLS_NAME_DNS, server_name, len)) != GNUTLS_E_SUCCESS) {
		throw TLSError(error, "gnutls_server_name_set");
	}
}

Buffer TLSSession::get_session_data() {
	int error;
	size_t size = 0;
	if ((error = ::gnutls_session_get_data(session, nullptr, &size)) != GNUTLS_E_SUCCESS && error != GNUTLS_E_SHORT_MEMORY_BUFFER) {
		throw TLSError(error, "gnutls_session_get_data");
	}
	Buffer session_data(size);
	if ((error = ::gnutls_session_get_data(session, session_data.pptr, &size)) != GNUTLS_E_SUCCESS) {
		throw TLSError(error, "gnutls_session_get_data");
	}
	session_data.ppos(size);
	return session_data;
}

void TLSSession::set_session_data(const void *session_data, size_t session_data_size) {
	int error;
	if ((error = ::gnutls_session_set_data(session, session_data, session_data_size)) != GNUTLS_E_SUCCESS) {
		throw TLSError(error, "gnutls_session_set_data");
	}
}

unsigned int TLSSession::verify_peers(const char hostname[]) {
	int error;
	unsigned int status;
	if ((error = ::gnutls_certificate_verify_peers3(session, hostname, &status)) != GNUTLS_E_SUCCESS) {
		throw TLSError(error, "gnutls_certificate_verify_peers3");
	}
	return status;
}

bool TLSSession::handshake() {
	int error;
	if ((error = ::gnutls_handshake(session)) != GNUTLS_E_SUCCESS) {
		if (error == GNUTLS_E_AGAIN || error == GNUTLS_E_INTERRUPTED) {
			return false;
		}
		throw TLSError(error, "gnutls_handshake");
	}
	return true;
}

ssize_t TLSSession::read(void *buf, size_t n) {
	ssize_t r;
	if ((r = ::gnutls_record_recv(session, buf, n)) < 0) {
		if (r == GNUTLS_E_AGAIN || r == GNUTLS_E_INTERRUPTED) {
			return 0;
		}
		throw TLSError(static_cast<int>(r), "gnutls_record_recv");
	}
	return r == 0 ? n == 0 ? 0 : -1 : r;
}

size_t TLSSession::write(const void *buf, size_t n) {
	::gnutls_record_cork(session);
	ssize_t s;
	if ((s = ::gnutls_record_send(session, buf, n)) < 0) {
		if (s == GNUTLS_E_AGAIN || s == GNUTLS_E_INTERRUPTED) {
			return 0;
		}
		throw TLSError(static_cast<int>(s), "gnutls_record_send");
	}
	return s;
}

bool TLSSession::flush() {
	int error;
	if ((error = ::gnutls_record_uncork(session, 0)) < 0) {
		throw TLSError(error, "gnutls_record_uncork");
	}
	return true;
}


bool TLSSession::bye(gnutls_close_request_t how) {
	int error;
	if ((error = ::gnutls_bye(session, how)) != GNUTLS_E_SUCCESS) {
		if (error == GNUTLS_E_AGAIN || error == GNUTLS_E_INTERRUPTED) {
			return false;
		}
		throw TLSError(error, "gnutls_bye");
	}
	return true;
}

ssize_t TLSSession::pull(gnutls_transport_ptr_t ptr, void *buf, size_t n) noexcept {
	try {
		ssize_t r = static_cast<TLSSession *>(ptr)->pull(buf, n);
		if (r == 0 && n > 0) {
			errno = EAGAIN;
			return -1;
		}
		if (r < 0) {
			return 0;
		}
		return r;
	}
	catch (...) {
		return -1;
	}
}

int TLSSession::pull_timeout(gnutls_transport_ptr_t ptr, unsigned int ms) noexcept {
	try {
		return static_cast<TLSSession *>(ptr)->pull_timeout(ms) ? 1 : 0;
	}
	catch (...) {
		return -1;
	}
}

ssize_t TLSSession::push(gnutls_transport_ptr_t ptr, const void *buf, size_t n) noexcept {
	try {
		size_t w = static_cast<TLSSession *>(ptr)->push(buf, n);
		if (w == 0 && n > 0) {
			errno = EAGAIN;
			return -1;
		}
		return w;
	}
	catch (...) {
		return -1;
	}
}

ssize_t TLSSession::vec_push(gnutls_transport_ptr_t ptr, const giovec_t iov[], int iovcnt) noexcept {
	try {
		size_t w = static_cast<TLSSession *>(ptr)->vec_push(reinterpret_cast<const struct iovec *>(iov), iovcnt);
		if (w == 0) {
			for (; iovcnt > 0; --iovcnt) {
				if (iov++->iov_len > 0) {
					errno = EAGAIN;
					return -1;
				}
			}
		}
		return w;
	}
	catch (...) {
		return -1;
	}
}

int TLSSession::verify(gnutls_session_t session) noexcept {
	try {
		return static_cast<TLSSession *>(::gnutls_session_get_ptr(session))->verify() ? 0 : -1;
	}
	catch (...) {
		return -1;
	}
}


TLSSocket::TLSSocket(std::string host_name, Socket &&socket)
		: host_name(std::move(host_name)), socket(std::move(socket)) {
	this->set_server_name(this->host_name.c_str(), this->host_name.size());
}

ssize_t TLSSocket::pull(void *buf, size_t n) {
	return socket.read(buf, n);
}

bool TLSSocket::pull_timeout(unsigned int ms) {
	struct pollfd pfd = { socket, POLLIN, 0 };
	return posix::poll(&pfd, 1, ms) > 0;
}

size_t TLSSocket::push(const void *buf, size_t n) {
	return socket.write(buf, n);
}

size_t TLSSocket::vec_push(const struct iovec iov[], size_t iovcnt) {
	ssize_t w;
	if ((w = ::writev(socket, iov, static_cast<int>(iovcnt))) < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "writev");
	}
	return w;
}

bool TLSSocket::verify() {
	return this->verify_peers(host_name.empty() ? nullptr : host_name.c_str()) == 0;
}
