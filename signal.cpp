#include "signal.h"

#include <system_error>

namespace posix {


void kill(pid_t pid, int sig) {
	if (_unlikely(::kill(pid, sig) < 0)) {
		throw std::system_error(errno, std::system_category(), "kill");
	}
}

void killpg(pid_t pgrp, int sig) {
	if (_unlikely(::killpg(pgrp, sig) < 0)) {
		throw std::system_error(errno, std::system_category(), "killpg");
	}
}

void pause() {
	if (_unlikely(::pause() < 0 && errno != EINTR)) {
		throw std::system_error(errno, std::system_category(), "pause");
	}
}

unsigned ppoll(struct pollfd * _restrict fds, nfds_t nfds, const struct timespec * _restrict timeout, const sigset_t * _restrict sigmask) {
	int ret;
	if ((ret = ::ppoll(fds, nfds, timeout, sigmask)) < 0) {
		if (_unlikely(errno != EINTR)) {
			throw std::system_error(errno, std::system_category(), "ppoll");
		}
		return 0;
	}
	return static_cast<unsigned>(ret);
}

unsigned pselect(int nfds, fd_set * _restrict readfds, fd_set * _restrict writefds, fd_set * _restrict errorfds, const struct timespec * _restrict timeout, const sigset_t * _restrict sigmask) {
	int ret;
	if ((ret = ::pselect(nfds, readfds, writefds, errorfds, timeout, sigmask)) < 0) {
		if (_unlikely(errno != EINTR)) {
			throw std::system_error(errno, std::system_category(), "pselect");
		}
		return 0;
	}
	return static_cast<unsigned>(ret);
}

#if _REENTRANT

void pthread_kill(pthread_t thread, int sig) {
	if (int error = ::pthread_kill(thread, sig); _unlikely(error != 0)) {
		throw std::system_error(error, std::system_category(), "pthread_kill");
	}
}

void pthread_sigmask(int how, const sigset_t & _restrict set, sigset_t * _restrict oset) {
	if (int error = ::pthread_sigmask(how, &set, oset); _unlikely(error != 0)) {
		throw std::system_error(error, std::system_category(), "pthread_sigmask");
	}
}

void pthread_sigqueue(pthread_t thread, int sig, const union sigval value) {
	if (int error = ::pthread_sigqueue(thread, sig, value); _unlikely(error != 0)) {
		throw std::system_error(error, std::system_category(), "pthread_sigqueue");
	}
}

#endif // _REENTRANT

void raise(int sig) {
	if (_unlikely(::raise(sig) < 0)) {
		throw std::system_error(errno, std::system_category(), "raise");
	}
}

void sigaction(int sig, const struct ::sigaction * _restrict act, struct ::sigaction * _restrict oact) {
	if (_unlikely(::sigaction(sig, act, oact) < 0)) {
		throw std::system_error(errno, std::system_category(), "sigaction");
	}
}

void sigaltstack(const stack_t * _restrict ss, stack_t * _restrict oss) {
	if (_unlikely(::sigaltstack(ss, oss) < 0)) {
		throw std::system_error(errno, std::system_category(), "sigaltstack");
	}
}

sighandler_t signal(int signum, sighandler_t handler) {
	sighandler_t ret;
	if (_unlikely((ret = ::signal(signum, handler)) == SIG_ERR)) {
		throw std::system_error(errno, std::system_category(), "signal");
	}
	return ret;
}

void sigpending(sigset_t &set) {
	if (_unlikely(::sigpending(&set) < 0)) {
		throw std::system_error(errno, std::system_category(), "sigpending");
	}
}

void sigprocmask(int how, const sigset_t & _restrict set, sigset_t * _restrict oset) {
	if (_unlikely(::sigprocmask(how, &set, oset) < 0)) {
		throw std::system_error(errno, std::system_category(), "sigprocmask");
	}
}

void sigqueue(pid_t pid, int signo, const union sigval value) {
	if (_unlikely(::sigqueue(pid, signo, value) < 0)) {
		throw std::system_error(errno, std::system_category(), "sigqueue");
	}
}

void sigsuspend(const sigset_t &sigmask) {
	if (_unlikely(::sigsuspend(&sigmask) < 0 && errno != EINTR)) {
		throw std::system_error(errno, std::system_category(), "sigsuspend");
	}
}

int sigtimedwait(const sigset_t & _restrict set, siginfo_t * _restrict info, const struct timespec & _restrict timeout) {
	int sig;
	if (_unlikely((sig = ::sigtimedwait(&set, info, &timeout)) < 0)) {
		throw std::system_error(errno, std::system_category(), "sigtimedwait");
	}
	return sig;
}

int sigwait(const sigset_t &set) {
	int sig;
	if (int error = ::sigwait(&set, &sig); _unlikely(error != 0)) {
		throw std::system_error(error, std::system_category(), "sigwait");
	}
	return sig;
}

int sigwaitinfo(const sigset_t & _restrict set, siginfo_t * _restrict info) {
	int sig;
	if (_unlikely((sig = ::sigwaitinfo(&set, info)) < 0)) {
		throw std::system_error(errno, std::system_category(), "sigwaitinfo");
	}
	return sig;
}

void sigemptyset(sigset_t &set) {
	if (_unlikely(::sigemptyset(&set) < 0)) {
		throw std::system_error(errno, std::system_category(), "sigemptyset");
	}
}

void sigfillset(sigset_t &set) {
	if (_unlikely(::sigfillset(&set) < 0)) {
		throw std::system_error(errno, std::system_category(), "sigfillset");
	}
}

void sigaddset(sigset_t &set, int signo) {
	if (_unlikely(::sigaddset(&set, signo) < 0)) {
		throw std::system_error(errno, std::system_category(), "sigaddset");
	}
}

void sigdelset(sigset_t &set, int signo) {
	if (_unlikely(::sigdelset(&set, signo) < 0)) {
		throw std::system_error(errno, std::system_category(), "sigdelset");
	}
}

bool sigismember(const sigset_t &set, int signo) {
	int ret;
	if (_unlikely((ret = ::sigismember(&set, signo)) < 0)) {
		throw std::system_error(errno, std::system_category(), "sigismember");
	}
	return ret;
}

#ifdef _GNU_SOURCE
bool sigisemptyset(const sigset_t &set) {
	int ret;
	if (_unlikely((ret = ::sigisemptyset(&set)) < 0)) {
		throw std::system_error(errno, std::system_category(), "sigisemptyset");
	}
	return ret;
}

void sigorset(sigset_t &dest, const sigset_t &left, const sigset_t &right) {
	if (_unlikely(::sigorset(&dest, &left, &right) < 0)) {
		throw std::system_error(errno, std::system_category(), "sigorset");
	}
}

void sigandset(sigset_t &dest, const sigset_t &left, const sigset_t &right) {
	if (_unlikely(::sigandset(&dest, &left, &right) < 0)) {
		throw std::system_error(errno, std::system_category(), "sigandset");
	}
}
#endif


const SignalSet SignalSet::none;
const SignalSet SignalSet::all(nullptr);


} // namespace posix
