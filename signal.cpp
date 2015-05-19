#include "signal.h"

#include <system_error>

namespace posix {


void kill(pid_t pid, int sig) {
	if (::kill(pid, sig) < 0) {
		throw std::system_error(errno, std::system_category(), "kill");
	}
}

void killpg(pid_t pgrp, int sig) {
	if (::killpg(pgrp, sig) < 0) {
		throw std::system_error(errno, std::system_category(), "killpg");
	}
}

void pause() {
	if (::pause() < 0 && errno != EINTR) {
		throw std::system_error(errno, std::system_category(), "pause");
	}
}

unsigned ppoll(struct pollfd * _restrict fds, nfds_t nfds, const struct timespec * _restrict timeout, const sigset_t * _restrict sigmask) {
	int ret;
	if ((ret = ::ppoll(fds, nfds, timeout, sigmask)) < 0) {
		if (errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "ppoll");
	}
	return static_cast<unsigned>(ret);
}

unsigned pselect(int nfds, fd_set * _restrict readfds, fd_set * _restrict writefds, fd_set * _restrict errorfds, const struct timespec * _restrict timeout, const sigset_t * _restrict sigmask) {
	int ret;
	if ((ret = ::pselect(nfds, readfds, writefds, errorfds, timeout, sigmask)) < 0) {
		if (errno == EINTR) {
			return 0;
		}
		throw std::system_error(errno, std::system_category(), "pselect");
	}
	return static_cast<unsigned>(ret);
}

void pthread_kill(pthread_t thread, int sig) {
	int error;
	if ((error = ::pthread_kill(thread, sig)) != 0) {
		throw std::system_error(error, std::system_category(), "pthread_kill");
	}
}

void pthread_sigmask(int how, const sigset_t & _restrict set, sigset_t * _restrict oset) {
	int error;
	if ((error = ::pthread_sigmask(how, &set, oset)) != 0) {
		throw std::system_error(error, std::system_category(), "pthread_sigmask");
	}
}

void pthread_sigqueue(pthread_t thread, int sig, const union sigval value) {
	int error;
	if ((error = ::pthread_sigqueue(thread, sig, value)) != 0) {
		throw std::system_error(error, std::system_category(), "pthread_sigqueue");
	}
}

void raise(int sig) {
	if (::raise(sig) < 0) {
		throw std::system_error(errno, std::system_category(), "raise");
	}
}

void sigaction(int sig, const struct ::sigaction * _restrict act, struct ::sigaction * _restrict oact) {
	if (::sigaction(sig, act, oact) < 0) {
		throw std::system_error(errno, std::system_category(), "sigaction");
	}
}

void sigaltstack(const stack_t * _restrict ss, stack_t * _restrict oss) {
	if (::sigaltstack(ss, oss) < 0) {
		throw std::system_error(errno, std::system_category(), "sigaltstack");
	}
}

void sigpending(sigset_t &set) {
	if (::sigpending(&set) < 0) {
		throw std::system_error(errno, std::system_category(), "sigpending");
	}
}

void sigprocmask(int how, const sigset_t & _restrict set, sigset_t * _restrict oset) {
	if (::sigprocmask(how, &set, oset) < 0) {
		throw std::system_error(errno, std::system_category(), "sigprocmask");
	}
}

void sigqueue(pid_t pid, int signo, const union sigval value) {
	if (::sigqueue(pid, signo, value) < 0) {
		throw std::system_error(errno, std::system_category(), "sigqueue");
	}
}

void sigsuspend(const sigset_t &sigmask) {
	if (::sigsuspend(&sigmask) < 0 && errno != EINTR) {
		throw std::system_error(errno, std::system_category(), "sigsuspend");
	}
}

int sigtimedwait(const sigset_t & _restrict set, siginfo_t * _restrict info, const struct timespec & _restrict timeout) {
	int sig;
	if ((sig = ::sigtimedwait(&set, info, &timeout)) < 0) {
		throw std::system_error(errno, std::system_category(), "sigtimedwait");
	}
	return sig;
}

int sigwait(const sigset_t &set) {
	int error, sig;
	if ((error = ::sigwait(&set, &sig)) != 0) {
		throw std::system_error(error, std::system_category(), "sigwait");
	}
	return sig;
}

int sigwaitinfo(const sigset_t & _restrict set, siginfo_t * _restrict info) {
	int sig;
	if ((sig = ::sigwaitinfo(&set, info)) < 0) {
		throw std::system_error(errno, std::system_category(), "sigwaitinfo");
	}
	return sig;
}

void sigemptyset(sigset_t &set) {
	if (::sigemptyset(&set) < 0) {
		throw std::system_error(errno, std::system_category(), "sigemptyset");
	}
}

void sigfillset(sigset_t &set) {
	if (::sigfillset(&set) < 0) {
		throw std::system_error(errno, std::system_category(), "sigfillset");
	}
}

void sigaddset(sigset_t &set, int signo) {
	if (::sigaddset(&set, signo) < 0) {
		throw std::system_error(errno, std::system_category(), "sigaddset");
	}
}

void sigdelset(sigset_t &set, int signo) {
	if (::sigdelset(&set, signo) < 0) {
		throw std::system_error(errno, std::system_category(), "sigdelset");
	}
}

bool sigismember(const sigset_t &set, int signo) {
	int ret;
	if ((ret = ::sigismember(&set, signo)) < 0) {
		throw std::system_error(errno, std::system_category(), "sigismember");
	}
	return ret;
}

#ifdef _GNU_SOURCE
bool sigisemptyset(const sigset_t &set) {
	int ret;
	if ((ret = ::sigisemptyset(&set)) < 0) {
		throw std::system_error(errno, std::system_category(), "sigisemptyset");
	}
	return ret;
}

void sigorset(sigset_t &dest, const sigset_t &left, const sigset_t &right) {
	if (::sigorset(&dest, &left, &right) < 0) {
		throw std::system_error(errno, std::system_category(), "sigorset");
	}
}

void sigandset(sigset_t &dest, const sigset_t &left, const sigset_t &right) {
	if (::sigandset(&dest, &left, &right) < 0) {
		throw std::system_error(errno, std::system_category(), "sigandset");
	}
}
#endif


} // namespace posix
