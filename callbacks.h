#pragma once

#include <list>
#include <mutex>


template <typename... Args>
class Callbacks {
	friend class CallbackRegistration;

public:
	typedef std::function<void (Args...) /* noexcept */> callback_t;

private:
	typedef typename std::list<callback_t>::const_iterator callback_itr_t;

public:
	class CallbackRegistration {
		friend class Callbacks;
	private:
		Callbacks *callbacks_ptr;
		callback_itr_t callback_itr;
	public:
		CallbackRegistration(CallbackRegistration &&move) noexcept : callbacks_ptr(move.callbacks_ptr), callback_itr(move.callback_itr) { move.callbacks_ptr = nullptr; }
		CallbackRegistration & operator=(CallbackRegistration &&move) noexcept { return this->swap(move), *this; }
		~CallbackRegistration() {
			if (callbacks_ptr) {
				callbacks_ptr->remove_callback(callback_itr), callbacks_ptr = nullptr;
			}
		}
		void swap(CallbackRegistration &other) noexcept { using std::swap; swap(callbacks_ptr, other.callbacks_ptr), swap(callback_itr, other.callback_itr); }
		friend void swap(CallbackRegistration &lhs, CallbackRegistration &rhs) noexcept { lhs.swap(rhs)); }
	private:
		explicit CallbackRegistration(Callbacks *callbacks_ptr, callback_itr_t callback_itr) noexcept : callbacks_ptr(callbacks_ptr), callback_itr(callback_itr) { }
		CallbackRegistration(const CallbackRegistration &) = delete;
		CallbackRegistration & operator=(const CallbackRegistration &) = delete;
	};

private:
	std::mutex mutex;
	std::list<callback_t> callbacks;

public:
	CallbackRegistration add_callback(callback_t &&callback) {
		std::lock_guard<std::mutex> lock(mutex);
		return CallbackRegistration(this, callbacks.emplace(callbacks.end(), std::move(callback)));
	}

protected:
	void do_callbacks(Args&&... args) {
		std::lock_guard<std::mutex> lock(mutex);
		for (auto &callback : callbacks) {
			callback(std::forward<Args>(args)...);
		}
	}

private:
	void remove_callback(callback_itr_t callback_itr) {
		std::lock_guard<std::mutex> lock(mutex);
		callbacks.erase(callback_itr);
	}

};
