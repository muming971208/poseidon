// 这个文件是 Poseidon 服务器应用程序框架的一部分。
// Copyleft 2014 - 2018, LH_Mouse. All wrongs reserved.

#include "precompiled.hpp"
#include "recursive_mutex.hpp"
#include "log.hpp"
#include "errno.hpp"
#include "system_exception.hpp"

namespace Poseidon {

#define TERMINATE_UNLESS(c_, ...)   do { if(c_){ break; } POSEIDON_LOG_FATAL(__VA_ARGS__); std::terminate(); } while(false)

namespace {
	class Mutex_attribute : NONCOPYABLE {
	private:
		::pthread_mutexattr_t m_attr;

	public:
		Mutex_attribute(){
			int err = ::pthread_mutexattr_init(&m_attr);
			POSEIDON_THROW_UNLESS(err == 0, System_exception);
			err = ::pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_RECURSIVE);
			TERMINATE_UNLESS(err == 0, "::pthread_mutexattr_settype() failed with ", err, " (", get_error_desc(err), ")");
		}
		~Mutex_attribute(){
			int err = ::pthread_mutexattr_destroy(&m_attr);
			TERMINATE_UNLESS(err == 0, "::pthread_mutexattr_destroy() failed with ", err, " (", get_error_desc(err), ")");
		}

	public:
		operator ::pthread_mutexattr_t *(){
			return &m_attr;
		}
	};
}

Recursive_mutex::Unique_lock::Unique_lock()
	: m_target(NULLPTR), m_locked(false)
{
	//
}
Recursive_mutex::Unique_lock::Unique_lock(Recursive_mutex &target, bool locks_target)
	: m_target(&target), m_locked(false)
{
	if(locks_target){
		lock();
	}
}
Recursive_mutex::Unique_lock::~Unique_lock(){
	if(m_locked){
		unlock();
	}
}

bool Recursive_mutex::Unique_lock::is_locked() const NOEXCEPT {
	return m_locked;
}
void Recursive_mutex::Unique_lock::lock() NOEXCEPT {
	TERMINATE_UNLESS(m_target, "No recursive mutex has been assigned to this Unique_lock.");
	TERMINATE_UNLESS(!m_locked, "The recursive mutex has already been locked by this Unique_lock.");

	int err = ::pthread_mutex_lock(&(m_target->m_mutex));
	TERMINATE_UNLESS(err == 0, "::pthread_mutex_lock() failed with ", err, " (", get_error_desc(err), ")");
	m_locked = true;
}
void Recursive_mutex::Unique_lock::unlock() NOEXCEPT {
	TERMINATE_UNLESS(m_target, "No recursive mutex has been assigned to this Unique_lock.");
	TERMINATE_UNLESS(m_locked, "The recursive mutex has not already been locked by this Unique_lock.");

	int err = ::pthread_mutex_unlock(&(m_target->m_mutex));
	TERMINATE_UNLESS(err == 0, "::pthread_mutex_unlock() failed with ", err, " (", get_error_desc(err), ")");
	m_locked = false;
}

Recursive_mutex::Recursive_mutex(){
	int err = ::pthread_mutex_init(&m_mutex, Mutex_attribute());
	POSEIDON_THROW_UNLESS(err == 0, System_exception, err);
}
Recursive_mutex::~Recursive_mutex(){
	int err = ::pthread_mutex_destroy(&m_mutex);
	TERMINATE_UNLESS(err == 0, "::pthread_mutex_destroy() failed with ", err, " (", get_error_desc(err), ")");
}

}
