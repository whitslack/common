#include <chrono>
#include <initializer_list>

#include <poll.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>

#include "compiler.h"

namespace posix {


using ::alarm;
void kill(pid_t pid, int sig);
void killpg(pid_t pgrp, int sig);
void pause();
unsigned ppoll(struct pollfd * _restrict fds, nfds_t nfds, const struct timespec * _restrict timeout = nullptr, const sigset_t * _restrict sigmask = nullptr);
unsigned pselect(int nfds, fd_set * _restrict readfds, fd_set * _restrict writefds = nullptr, fd_set * _restrict errorfds = nullptr, const struct timespec * _restrict timeout = nullptr, const sigset_t * _restrict sigmask = nullptr);
void pthread_kill(pthread_t thread, int sig);
void pthread_sigmask(int how, const sigset_t & _restrict set, sigset_t * _restrict oset = nullptr);
void pthread_sigqueue(pthread_t thread, int sig, const union sigval value);
void raise(int sig);
void sigaction(int sig, const struct ::sigaction * _restrict act, struct ::sigaction * _restrict oact = nullptr);
void sigaltstack(const stack_t * _restrict ss, stack_t * _restrict oss = nullptr);
sighandler_t signal(int signum, sighandler_t handler);
void sigpending(sigset_t &set);
void sigprocmask(int how, const sigset_t & _restrict set, sigset_t * _restrict oset = nullptr);
void sigqueue(pid_t pid, int signo, const union sigval value);
void sigsuspend(const sigset_t &sigmask);
int sigtimedwait(const sigset_t & _restrict set, siginfo_t * _restrict info, const struct timespec & _restrict timeout);
int sigwait(const sigset_t &set);
int sigwaitinfo(const sigset_t & _restrict set, siginfo_t * _restrict info);

void sigemptyset(sigset_t &set);
void sigfillset(sigset_t &set);
void sigaddset(sigset_t &set, int signo);
void sigdelset(sigset_t &set, int signo);
bool sigismember(const sigset_t &set, int signo) _pure;
#ifdef _GNU_SOURCE
bool sigisemptyset(const sigset_t &set) _pure;
void sigorset(sigset_t &dest, const sigset_t &left, const sigset_t &right);
void sigandset(sigset_t &dest, const sigset_t &left, const sigset_t &right);
#endif

static inline unsigned ppoll(struct pollfd * _restrict fds, nfds_t nfds, std::chrono::nanoseconds timeout, const sigset_t * _restrict sigmask = nullptr) {
	struct timespec spec;
	spec.tv_sec = static_cast<std::time_t>(std::chrono::duration_cast<std::chrono::seconds>(timeout).count());
	spec.tv_nsec = static_cast<long>((timeout % std::chrono::seconds(1)).count());
	return posix::ppoll(fds, nfds, &spec, sigmask);
}

static inline unsigned pselect(int nfds, fd_set * _restrict readfds, fd_set * _restrict writefds, fd_set * _restrict errorfds, std::chrono::nanoseconds timeout, const sigset_t * _restrict sigmask = nullptr) {
	struct timespec spec;
	spec.tv_sec = static_cast<std::time_t>(std::chrono::duration_cast<std::chrono::seconds>(timeout).count());
	spec.tv_nsec = static_cast<long>((timeout % std::chrono::seconds(1)).count());
	return posix::pselect(nfds, readfds, writefds, errorfds, &spec, sigmask);
}

static inline void pthread_sigqueue(pthread_t thread, int sig, int value) {
	union sigval sv;
	sv.sival_int = value;
	posix::pthread_sigqueue(thread, sig, sv);
}

static inline void pthread_sigqueue(pthread_t thread, int sig, void *value) {
	union sigval sv;
	sv.sival_ptr = value;
	posix::pthread_sigqueue(thread, sig, sv);
}

static inline int sigtimedwait(const sigset_t & _restrict set, siginfo_t * _restrict info, std::chrono::nanoseconds timeout) {
	struct timespec spec;
	spec.tv_sec = static_cast<std::time_t>(std::chrono::duration_cast<std::chrono::seconds>(timeout).count());
	spec.tv_nsec = static_cast<long>((timeout % std::chrono::seconds(1)).count());
	return sigtimedwait(set, info, spec);
}


class SignalSet {

public:
	static const SignalSet none, all;

private:
	sigset_t set;

public:
	SignalSet() { this->clear(); }
	SignalSet(std::initializer_list<int> signos) { this->clear(); for (int signo : signos) *this += signo; }

private:
	explicit SignalSet(std::nullptr_t) { this->fill(); }

public:
	operator const sigset_t & () const { return set; }
	operator sigset_t & () { return set; }
	operator const sigset_t * () const { return &set; }
	operator sigset_t * () { return &set; }

	void clear() { sigemptyset(set); }
	void fill() { sigfillset(set); }
	SignalSet & operator += (int signo) { sigaddset(set, signo); return *this; }
	SignalSet & operator -= (int signo) { sigdelset(set, signo); return *this; }
	bool operator & (int signo) const { return sigismember(set, signo); }
#ifdef _GNU_SOURCE
	bool empty() const { return sigisemptyset(set); }
	SignalSet & operator |= (const SignalSet &rhs) { sigorset(set, set, rhs.set); return *this; }
	SignalSet & operator &= (const SignalSet &rhs) { sigandset(set, set, rhs.set); return *this; }
#endif

};


class SignalBlock {

private:
	SignalSet orig_sigmask;

public:
	explicit SignalBlock(std::initializer_list<int> signos) : SignalBlock(SignalSet(signos)) { }
#if _REENTRANT
	explicit SignalBlock(const sigset_t &sigmask) { pthread_sigmask(SIG_BLOCK, sigmask, orig_sigmask); }
	~SignalBlock() { pthread_sigmask(SIG_SETMASK, orig_sigmask); }
#else
	explicit SignalBlock(const sigset_t &sigmask) { sigprocmask(SIG_BLOCK, sigmask, orig_sigmask); }
	~SignalBlock() { sigprocmask(SIG_SETMASK, orig_sigmask); }
#endif

};


} // namespace posix
