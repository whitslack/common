#include <fcntl.h>
#include <spawn.h>
#include <unistd.h>

#include "compiler.h"


namespace posix {


class SpawnFileActions {

private:
	posix_spawn_file_actions_t file_actions;

public:
	SpawnFileActions();
	~SpawnFileActions();
	_pure operator posix_spawn_file_actions_t * () noexcept { return &file_actions; }
	_pure operator const posix_spawn_file_actions_t * () const noexcept { return &file_actions; }

private:
	SpawnFileActions(const SpawnFileActions &) = delete;
	SpawnFileActions & operator=(const SpawnFileActions &) = delete;

public:
	void add_close(int fd);
	void add_open(int fd, const char pathname[], int flags = O_RDONLY, mode_t mode = 0666);
	void add_dup2(int fd, int new_fd);

};


class SpawnAttributes {

private:
	posix_spawnattr_t attr;

public:
	SpawnAttributes();
	~SpawnAttributes();
	_pure operator posix_spawnattr_t * () noexcept { return &attr; }
	_pure operator const posix_spawnattr_t * () const noexcept { return &attr; }

private:
	SpawnAttributes(const SpawnAttributes &) = delete;
	SpawnAttributes & operator=(const SpawnAttributes &) = delete;

public:
	short get_flags() const;
	void set_flags(short flags);

	pid_t get_pgroup() const;
	void set_pgroup(pid_t pgroup);

	int get_sched_policy() const;
	void set_sched_policy(int sched_policy);

	void get_sched_param(struct sched_param *sched_param) const;
	void set_sched_param(const struct sched_param *sched_param);

	void get_sig_mask(sigset_t *sig_mask) const;
	void set_sig_mask(const sigset_t *sig_mask);

	void get_sig_default(sigset_t *sig_default) const;
	void set_sig_default(const sigset_t *sig_default);

};


pid_t spawn(const char path[], const SpawnFileActions *file_actions, const SpawnAttributes *attr, const char * const argv[], const char * const envp[] = const_cast<const char * const *>(environ));

pid_t spawnp(const char file[], const SpawnFileActions *file_actions, const SpawnAttributes *attr, const char * const argv[], const char * const envp[] = const_cast<const char * const *>(environ));


} // namespace posix
