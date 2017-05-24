#include "spawn.h"

#include <system_error>

namespace posix {


SpawnFileActions::SpawnFileActions() {
	int error;
	if ((error = ::posix_spawn_file_actions_init(&file_actions)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawn_file_actions_init");
	}
}

SpawnFileActions::~SpawnFileActions() {
	int error;
	if ((error = ::posix_spawn_file_actions_destroy(&file_actions)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawn_file_actions_destroy");
	}
}

void SpawnFileActions::add_close(int fd) {
	int error;
	if ((error = ::posix_spawn_file_actions_addclose(&file_actions, fd)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawn_file_actions_addclose");
	}
}

void SpawnFileActions::add_open(int fd, const char pathname[], int flags, mode_t mode) {
	int error;
	if ((error = ::posix_spawn_file_actions_addopen(&file_actions, fd, pathname, flags, mode)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawn_file_actions_addopen");
	}
}

void SpawnFileActions::add_dup2(int fd, int new_fd) {
	int error;
	if ((error = ::posix_spawn_file_actions_adddup2(&file_actions, fd, new_fd)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawn_file_actions_adddup2");
	}
}


SpawnAttributes::SpawnAttributes() {
	int error;
	if ((error = ::posix_spawnattr_init(&attr)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawnattr_init");
	}
}

SpawnAttributes::~SpawnAttributes() {
	int error;
	if ((error = ::posix_spawnattr_destroy(&attr)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawnattr_destroy");
	}
}

short SpawnAttributes::get_flags() const {
	short flags;
	int error;
	if ((error = ::posix_spawnattr_getflags(&attr, &flags)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawnattr_getflags");
	}
	return flags;
}

void SpawnAttributes::set_flags(short flags) {
	int error;
	if ((error = ::posix_spawnattr_setflags(&attr, flags)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawnattr_setflags");
	}
}

pid_t SpawnAttributes::get_pgroup() const {
	pid_t pgroup;
	int error;
	if ((error = ::posix_spawnattr_getpgroup(&attr, &pgroup)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawnattr_getpgroup");
	}
	return pgroup;
}

void SpawnAttributes::set_pgroup(pid_t pgroup) {
	int error;
	if ((error = ::posix_spawnattr_setpgroup(&attr, pgroup)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawnattr_setpgroup");
	}
}

int SpawnAttributes::get_sched_policy() const {
	int sched_policy;
	int error;
	if ((error = ::posix_spawnattr_getschedpolicy(&attr, &sched_policy)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawnattr_getschedpolicy");
	}
	return sched_policy;
}

void SpawnAttributes::set_sched_policy(int sched_policy) {
	int error;
	if ((error = ::posix_spawnattr_setschedpolicy(&attr, sched_policy)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawnattr_setschedpolicy");
	}
}

void SpawnAttributes::get_sched_param(struct sched_param *sched_param) const {
	int error;
	if ((error = ::posix_spawnattr_getschedparam(&attr, sched_param)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawnattr_getschedparam");
	}
}

void SpawnAttributes::set_sched_param(const struct sched_param *sched_param) {
	int error;
	if ((error = ::posix_spawnattr_setschedparam(&attr, sched_param)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawnattr_setschedparam");
	}
}

void SpawnAttributes::get_sig_mask(sigset_t *sig_mask) const {
	int error;
	if ((error = ::posix_spawnattr_getsigmask(&attr, sig_mask)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawnattr_getsigmask");
	}
}

void SpawnAttributes::set_sig_mask(const sigset_t *sig_mask) {
	int error;
	if ((error = ::posix_spawnattr_setsigmask(&attr, sig_mask)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawnattr_setsigmask");
	}
}

void SpawnAttributes::get_sig_default(sigset_t *sig_default) const {
	int error;
	if ((error = ::posix_spawnattr_getsigdefault(&attr, sig_default)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawnattr_getsigdefault");
	}
}

void SpawnAttributes::set_sig_default(const sigset_t *sig_default) {
	int error;
	if ((error = ::posix_spawnattr_setsigdefault(&attr, sig_default)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawnattr_setsigdefault");
	}
}


pid_t spawn(const char path[], const SpawnFileActions *file_actions, const SpawnAttributes *attr, char * const argv[], char * const envp[]) {
	pid_t pid;
	int error;
	if ((error = ::posix_spawn(&pid, path, file_actions ? static_cast<const posix_spawn_file_actions_t *>(*file_actions) : nullptr, attr ? static_cast<const posix_spawnattr_t *>(*attr) : nullptr, argv, envp)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawn");
	}
	return pid;
}

pid_t spawnp(const char file[], const SpawnFileActions *file_actions, const SpawnAttributes *attr, char * const argv[], char * const envp[]) {
	pid_t pid;
	int error;
	if ((error = ::posix_spawnp(&pid, file, file_actions ? static_cast<const posix_spawn_file_actions_t *>(*file_actions) : nullptr, attr ? static_cast<const posix_spawnattr_t *>(*attr) : nullptr, argv, envp)) < 0) {
		throw std::system_error(error, std::system_category(), "posix_spawnp");
	}
	return pid;
}


} // namespace posix
