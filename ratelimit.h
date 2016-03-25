#include <algorithm>
#include <chrono>

template <typename Clock, typename FillRate, typename Clock::rep Max>
class TokenBucket {

public:
	typedef typename Clock::rep rep_t;

private:
	typedef std::chrono::duration<rep_t, std::ratio_divide<std::ratio<1, 1>, FillRate>> tokens_t;
	typedef std::common_type_t<typename Clock::duration, tokens_t> ticks_t;

private:
	ticks_t ref;

public:
	TokenBucket(rep_t initial_fill = Max) : ref(Clock::now().time_since_epoch() - tokens_t(initial_fill)) { }

	rep_t take(rep_t take) {
		ticks_t now = Clock::now().time_since_epoch();
		ticks_t avail = std::min(now - ref, ticks_t(tokens_t(Max)));
		if (avail <= ticks_t::zero()) {
			return 0;
		}
		ticks_t taken = std::min(ticks_t(tokens_t(take)), avail);
		ref = now - avail + taken;
		return std::chrono::duration_cast<tokens_t>(taken).count();
	}

	void overdraw(rep_t take) {
		ticks_t now = Clock::now().time_since_epoch();
		ticks_t avail = std::min(now - ref, ticks_t(tokens_t(Max)));
		ref = now - avail + tokens_t(take);
	}

};
