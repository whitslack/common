#include <fcntl.h>
#include <spawn.h>
#include <unistd.h>

namespace posix {


class SpawnFileActions {

private:
	posix_spawn_file_actions_t file_actions;

public:
	SpawnFileActions();
	~SpawnFileActions();
	operator posix_spawn_file_actions_t * () { return &file_actions; }
	operator const posix_spawn_file_actions_t * () const { return &file_actions; }

private:
	SpawnFileActions(const SpawnFileActions &) = delete;
	SpawnFileActions & operator = (const SpawnFileActions &) = delete;

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
	operator posix_spawnattr_t * () { return &attr; }
	operator const posix_spawnattr_t * () const { return &attr; }

private:
	SpawnAttributes(const SpawnAttributes &) = delete;
	SpawnAttributes & operator = (const SpawnAttributes &) = delete;

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


pid_t spawn(const char path[], const SpawnFileActions *file_actions, const SpawnAttributes *attr, char * const argv[], char * const envp[] = environ);

pid_t spawnp(const char file[], const SpawnFileActions *file_actions, const SpawnAttributes *attr, char * const argv[], char * const envp[] = environ);


} // namespace posix
