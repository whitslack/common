#include "tls.h"

#include <poll.h>


namespace {
struct GnuTLS_Global_Init {
	GnuTLS_Global_Init() {
		if (int error = ::gnutls_global_init(); _unlikely(error < 0)) {
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
	if (int error = ::gnutls_priority_init(&pcache, priorities, err_pos); _unlikely(error != GNUTLS_E_SUCCESS)) {
		throw TLSError(error, "gnutls_priority_init");
	}
}

std::vector<unsigned int> TLSPriority::certificate_type_list() {
	int ret;
	const unsigned int *list;
	if (_unlikely((ret = ::gnutls_priority_certificate_type_list(pcache, &list)) < 0)) {
		throw TLSError(ret, "gnutls_priority_certificate_type_list");
	}
	return std::vector<unsigned int>(list, list + ret);
}

std::vector<unsigned int> TLSPriority::ecc_curve_list() {
	int ret;
	const unsigned int *list;
	if (_unlikely((ret = ::gnutls_priority_ecc_curve_list(pcache, &list)) < 0)) {
		throw TLSError(ret, "gnutls_priority_ecc_curve_list");
	}
	return std::vector<unsigned int>(list, list + ret);
}

std::vector<unsigned int> TLSPriority::protocol_list() {
	int ret;
	const unsigned int *list;
	if (_unlikely((ret = ::gnutls_priority_protocol_list(pcache, &list)) < 0)) {
		throw TLSError(ret, "gnutls_priority_protocol_list");
	}
	return std::vector<unsigned int>(list, list + ret);
}

std::vector<unsigned int> TLSPriority::sign_list() {
	int ret;
	const unsigned int *list;
	if (_unlikely((ret = ::gnutls_priority_sign_list(pcache, &list)) < 0)) {
		throw TLSError(ret, "gnutls_priority_sign_list");
	}
	return std::vector<unsigned int>(list, list + ret);
}


TLSCertificateCredentials::TLSCertificateCredentials() : cred() {
	if (int error = ::gnutls_certificate_allocate_credentials(&cred); _unlikely(error != GNUTLS_E_SUCCESS)) {
		throw TLSError(error, "gnutls_certificate_allocate_credentials");
	}
	::gnutls_certificate_set_verify_function(cred, &TLSSession::verify);
}

size_t TLSCertificateCredentials::set_x509_system_trust() {
	int ret;
	if (_unlikely((ret = ::gnutls_certificate_set_x509_system_trust(cred)) < 0)) {
		throw TLSError(ret, "gnutls_certificate_set_x509_system_trust");
	}
	return ret;
}

size_t TLSCertificateCredentials::set_x509_trust_file(const char ca_file[], gnutls_x509_crt_fmt_t type) {
	int ret;
	if (_unlikely((ret = ::gnutls_certificate_set_x509_trust_file(cred, ca_file, type)) < 0)) {
		throw TLSError(ret, "gnutls_certificate_set_x509_trust_file");
	}
	return ret;
}


/*
TLSSRPServerCredentials::TLSSRPServerCredentials() : cred() {
	if (int error = ::gnutls_srp_allocate_server_credentials(&cred); _unlikely(error != GNUTLS_E_SUCCESS)) {
		throw TLSError(error, "gnutls_srp_allocate_server_credentials");
	}
}


TLSSRPClientCredentials::TLSSRPClientCredentials() : cred() {
	if (int error = ::gnutls_srp_allocate_client_credentials(&cred); _unlikely(error != GNUTLS_E_SUCCESS)) {
		throw TLSError(error, "gnutls_srp_allocate_client_credentials");
	}
}


TLSPSKServerCredentials::TLSPSKServerCredentials() : cred() {
	if (int error = ::gnutls_psk_allocate_server_credentials(&cred); _unlikely(error != GNUTLS_E_SUCCESS)) {
		throw TLSError(error, "gnutls_psk_allocate_server_credentials");
	}
}


TLSPSKClientCredentials::TLSPSKClientCredentials() : cred() {
	if (int error = ::gnutls_psk_allocate_client_credentials(&cred); _unlikely(error != GNUTLS_E_SUCCESS)) {
		throw TLSError(error, "gnutls_psk_allocate_client_credentials");
	}
}


TLSAnonServerCredentials::TLSAnonServerCredentials() : cred() {
	if (int error = ::gnutls_anon_allocate_server_credentials(&cred); _unlikely(error != GNUTLS_E_SUCCESS)) {
		throw TLSError(error, "gnutls_anon_allocate_server_credentials");
	}
}


TLSAnonClientCredentials::TLSAnonClientCredentials() : cred() {
	if (int error = ::gnutls_anon_allocate_client_credentials(&cred); _unlikely(error != GNUTLS_E_SUCCESS)) {
		throw TLSError(error, "gnutls_anon_allocate_client_credentials");
	}
}
*/


TLSSession::TLSSession() {
	if (int error = ::gnutls_init(&session, GNUTLS_CLIENT); _unlikely(error < 0)) {
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
	if (int error = ::gnutls_priority_set(session, priority); _unlikely(error != GNUTLS_E_SUCCESS)) {
		throw TLSError(error, "gnutls_priority_set");
	}
}

void TLSSession::set_priority_direct(const char priorities[], const char **err_pos) {
	if (int error = ::gnutls_priority_set_direct(session, priorities, err_pos); _unlikely(error != GNUTLS_E_SUCCESS)) {
		throw TLSError(error, "gnutls_priority_set_direct");
	}
}

void TLSSession::set_credentials(std::shared_ptr<TLSCertificateCredentials> credentials) {
	if (int error = ::gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, *credentials); _unlikely(error != GNUTLS_E_SUCCESS)) {
		throw TLSError(error, "gnutls_credentials_set");
	}
	this->credentials = std::move(credentials);
}

/*
void TLSSession::set_credentials(std::shared_ptr<TLSSRPClientCredentials> credentials) {
	if (int error = ::gnutls_credentials_set(session, GNUTLS_CRD_SRP, *credentials); _unlikely(error != GNUTLS_E_SUCCESS)) {
		throw TLSError(error, "gnutls_credentials_set");
	}
	this->credentials = std::move(credentials);
}

void TLSSession::set_credentials(std::shared_ptr<TLSPSKClientCredentials> credentials) {
	if (int error = ::gnutls_credentials_set(session, GNUTLS_CRD_PSK, *credentials); _unlikely(error != GNUTLS_E_SUCCESS)) {
		throw TLSError(error, "gnutls_credentials_set");
	}
	this->credentials = std::move(credentials);
}

void TLSSession::set_credentials(std::shared_ptr<TLSAnonClientCredentials> credentials) {
	if (int error = ::gnutls_credentials_set(session, GNUTLS_CRD_ANON, *credentials); _unlikely(error != GNUTLS_E_SUCCESS)) {
		throw TLSError(error, "gnutls_credentials_set");
	}
	this->credentials = std::move(credentials);
}
*/

std::string TLSSession::get_server_name() {
	std::string server_name;
	size_t size = 0;
	unsigned int type;
	if (int error = ::gnutls_server_name_get(session, nullptr, &size, &type, 0); error != GNUTLS_E_SUCCESS && error != GNUTLS_E_SHORT_MEMORY_BUFFER) {
		if (_unlikely(error != GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE)) {
			throw TLSError(error, "gnutls_server_name_get");
		}
		return server_name;
	}
	server_name.resize(size);
	if (int error = ::gnutls_server_name_get(session, &server_name.front(), &size, &type, 0); _unlikely(error != GNUTLS_E_SUCCESS)) {
		throw TLSError(error, "gnutls_server_name_get");
	}
	return server_name;
}

void TLSSession::set_server_name(std::string_view server_name) {
	if (int error = ::gnutls_server_name_set(session, GNUTLS_NAME_DNS, server_name.data(), server_name.size()); _unlikely(error != GNUTLS_E_SUCCESS)) {
		throw TLSError(error, "gnutls_server_name_set");
	}
}

DynamicBuffer TLSSession::get_session_data() {
	size_t size = 0;
	if (int error = ::gnutls_session_get_data(session, nullptr, &size); _unlikely(error != GNUTLS_E_SUCCESS && error != GNUTLS_E_SHORT_MEMORY_BUFFER)) {
		throw TLSError(error, "gnutls_session_get_data");
	}
	DynamicBuffer session_data(size);
	if (int error = ::gnutls_session_get_data(session, session_data.pptr, &size); _unlikely(error != GNUTLS_E_SUCCESS)) {
		throw TLSError(error, "gnutls_session_get_data");
	}
	session_data.ppos(size);
	return session_data;
}

void TLSSession::set_session_data(const void *session_data, size_t session_data_size) {
	if (int error = ::gnutls_session_set_data(session, session_data, session_data_size); _unlikely(error != GNUTLS_E_SUCCESS)) {
		throw TLSError(error, "gnutls_session_set_data");
	}
}

unsigned int TLSSession::verify_peers(const char hostname[]) {
	unsigned int status;
	if (int error = ::gnutls_certificate_verify_peers3(session, hostname, &status); _unlikely(error != GNUTLS_E_SUCCESS)) {
		throw TLSError(error, "gnutls_certificate_verify_peers3");
	}
	return status;
}

bool TLSSession::handshake() {
	if (int error = ::gnutls_handshake(session); error != GNUTLS_E_SUCCESS) {
		if (_unlikely(error != GNUTLS_E_AGAIN && error != GNUTLS_E_INTERRUPTED)) {
			throw TLSError(error, "gnutls_handshake");
		}
		return false;
	}
	return true;
}

ssize_t TLSSession::read(void *buf, size_t n) {
	ssize_t r;
	if ((r = ::gnutls_record_recv(session, buf, n)) < 0) {
		if (_unlikely(r != GNUTLS_E_AGAIN && r != GNUTLS_E_INTERRUPTED)) {
			throw TLSError(static_cast<int>(r), "gnutls_record_recv");
		}
		return 0;
	}
	return r == 0 ? n == 0 ? 0 : -1 : r;
}

size_t TLSSession::write(const void *buf, size_t n) {
	::gnutls_record_cork(session);
	ssize_t s;
	if ((s = ::gnutls_record_send(session, buf, n)) < 0) {
		if (_unlikely(s != GNUTLS_E_AGAIN && s != GNUTLS_E_INTERRUPTED)) {
			throw TLSError(static_cast<int>(s), "gnutls_record_send");
		}
		return 0;
	}
	return s;
}

bool TLSSession::flush() {
	if (int error = ::gnutls_record_uncork(session, 0); _unlikely(error < 0)) {
		throw TLSError(error, "gnutls_record_uncork");
	}
	return true;
}


bool TLSSession::bye(gnutls_close_request_t how) {
	if (int error = ::gnutls_bye(session, how); error != GNUTLS_E_SUCCESS) {
		if (_unlikely(error != GNUTLS_E_AGAIN && error != GNUTLS_E_INTERRUPTED)) {
			throw TLSError(error, "gnutls_bye");
		}
		return false;
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
	if (!this->host_name.empty()) {
		this->set_server_name(this->host_name);
	}
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
