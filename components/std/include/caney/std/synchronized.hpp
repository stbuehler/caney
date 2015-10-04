/*! @file */

#pragma once

#include "internal.hpp"

#include <mutex>
#include <type_traits>

#if __cplusplus >= 201402L
#include <shared_mutex>
#else
#include <boost/thread/shared_mutex.hpp>
#endif

__CANEY_STDV1_BEGIN

/**
 * @brief Represents a locked value; the value is accessible through pointer
 * semantics (@ref operator *() const and @ref operator ->() const operators)
 * or through an explicit @ref get() const call.
 *
 * @tparam Value value type to store a reference to while holding the lock
 * @tparam Lock  lock type to use; requires a `Lock::mutex_type` typedef to the mutex it locks.
 */
template<typename Value, typename Lock>
class locked_synchronized {
public:
	/// value type
	using value_type = Value;
	/// lock type
	using lock_type = Lock;

	/**
	 * @brief lock `mutex` and initialize contained value (reference) with `value`
	 * @param mutex the mutex to lock (using a lock of template argument Lock type)
	 * @param value reference to value this object synchronizes access to
	 */
	explicit locked_synchronized(typename Lock::mutex_type& mutex, value_type& value)
	: m_lock(mutex), m_value_ref(value) {
	}

	/** @return pointer to locked value */
	value_type* operator->() const {
		return &m_value_ref;
	}

	/** @return reference to locked value */
	value_type& operator*() const {
		return m_value_ref;
	}

	/** @return reference to locked value */
	value_type& get() const {
		return m_value_ref;
	}

private:
	Lock m_lock;
	value_type& m_value_ref;
};

/**
 * @brief enforce synchronized access to a value
 *
 * @tparam Value       value type to synchronize access to
 * @tparam Mutex       mutex type to synchronize with
 * @tparam SharedLock  template type taking a Mutex as template argument to realize shared read-only access to the value
 * @tparam Lock        template type taking a Mutex as template argument to realize exclusive access to the value
 * @tparam AllowAssign whether assigning from other @ref generic_synchronized values is allowed; when enabled this locks implicitly on assigning and should be used with care
 *
 * A mutex providing shared_lock functionality comes with a significant overhead when compared to a simple exclusive-only mutex.
 * You can use an exclusive-only mutex too, and use the same (exclusive) lock for `Lock` and `SharedLock`
 */
template<typename Value, typename Mutex, template<typename M> class SharedLock, template<typename M> class Lock, bool AllowAssign>
class generic_synchronized {
public:
	/// value type to synchronize
	using value_type = Value;
	/// mutex type to synchronize with
	using mutex_t = Mutex;
	/// lock type to hold a shared lock ("read only")
	using shared_lock_t = SharedLock<Mutex>;
	/// lock type to hold an exclusive lock ("read + write")
	using lock_t = Lock<Mutex>;
	/// type representing a shared-locked (const) value
	using shared_locked = locked_synchronized<value_type const, shared_lock_t>;
	/// type representing an exclusive locked value
	using locked = locked_synchronized<value_type, lock_t>;

private:
	mutable mutex_t m_mutex;
	value_type m_value; // decltype() below needs this before using it

public:
	/// default-construct contained value
	template<typename std::enable_if<std::is_default_constructible<value_type>::value>::type* = nullptr>
	explicit generic_synchronized()
	: m_value() {
	}

	/**
	 * @brief construct contained value from parameter
	 * @param from data to construct contained value from (types must be convertible)
	 */
	template<typename From, typename std::enable_if<std::is_convertible<From&&, value_type>::value>::type* = nullptr>
	explicit generic_synchronized(From&& from)
	: m_value(std::forward<From>(from)) {
	}

	/**
	 * @brief constuct contained value from another @ref generic_synchronized value of same type (locking it temporarily for shared access)
	 * @param other @ref generic_synchronized value to copy value from
	 */
	generic_synchronized(generic_synchronized const& other)
	: m_value(other.shared_synchronize().get()) {
	}

	/**
	 * @brief move-constuct contained value from another @ref generic_synchronized value of same type (locking it temporarily for exlusive access)
	 * @param other @ref generic_synchronized value to copy value from
	 */
	generic_synchronized(generic_synchronized&& other)
	: m_value(std::move(other.synchronize().get())) {
	}

	/**
	 * @brief constuct contained value from another @ref generic_synchronized value containting a convertible value (locking it temporarily for shared access)
	 * @param from @ref generic_synchronized value to copy value from
	 */
	template<typename FromT, typename FromMutex, template<typename M> class FromSharedLock, template<typename M> class FromLock, bool FromAllowAssign,
		typename std::enable_if<std::is_convertible<FromT const&, value_type>::value>::type* = nullptr>
	explicit generic_synchronized(generic_synchronized<FromT, FromMutex, FromSharedLock, FromLock, FromAllowAssign> const& from)
	: m_value(from.shared_synchronize().get()) {
	}

	/**
	 * @brief move-constuct contained value from another @ref generic_synchronized value containting a convertible value (locking it temporarily for exlusive access)
	 * @param from @ref generic_synchronized value to copy value from
	 */
	template<typename FromT, typename FromMutex, template<typename M> class FromSharedLock, template<typename M> class FromLock, bool FromAllowAssign,
		typename std::enable_if<std::is_convertible<FromT&&, value_type>::value>::type* = nullptr>
	explicit generic_synchronized(generic_synchronized<FromT, FromMutex, FromSharedLock, FromLock, FromAllowAssign>&& from)
	: m_value(std::move(from.synchronize().get())) {
	}

	/**
	 * @brief assign contained value from another @ref generic_synchronized instance containting a convertible value (locking both temporarily)
	 * @param other @ref generic_synchronized value to copy value from
	 *
	 * First copies new value into a temporary while holding a shared lock on the other object;
	 * then moves into place while holding an exclusing lock on itself.
	 */
	template<typename FromT, bool TAllowAssign = AllowAssign, typename std::enable_if<TAllowAssign && AllowAssign>::type* = nullptr,
		typename FromMutex, template<typename M> class FromSharedLock, template<typename M> class FromLock, bool FromAllowAssign,
	typename std::enable_if<std::is_convertible<FromT&&, value_type>::value>::type* = nullptr>
	generic_synchronized& operator=(generic_synchronized<FromT, FromMutex, FromSharedLock, FromLock, FromAllowAssign> const& other) {
		auto tmp = other.shared_synchronize().get();
		synchronize().get() = std::move(tmp);
		return *this;
	}

	/**
	 * @brief assign contained value from another @ref generic_synchronized instance containting a convertible value (locking both temporarily)
	 * @param other @ref generic_synchronized value to copy value from
	 *
	 * First copies new value into a temporary while holding a shared lock on the other object;
	 * then moves into place while holding an exclusing lock on itself.
	 */
	template<typename FromT, bool TAllowAssign = AllowAssign, typename std::enable_if<TAllowAssign && AllowAssign>::type* = nullptr,
		typename FromMutex, template<typename M> class FromSharedLock, template<typename M> class FromLock, bool FromAllowAssign,
	typename std::enable_if<std::is_convertible<FromT&&, value_type>::value>::type* = nullptr>
	generic_synchronized& operator=(generic_synchronized<FromT, FromMutex, FromSharedLock, FromLock, FromAllowAssign>& other) {
		auto tmp = other.shared_synchronize().get();
		synchronize().get() = std::move(tmp);
		return *this;
	}

	/**
	 * @brief move-assign contained value from another @ref generic_synchronized instance containting a convertible value (locking both temporarily)
	 * @param other @ref generic_synchronized value to copy value from
	 *
	 * First moves new value into a temporary while holding a shared lock on the other object;
	 * then moves into place while holding an exclusing lock on itself.
	 */
	template<typename FromT, bool TAllowAssign = AllowAssign, typename std::enable_if<TAllowAssign && AllowAssign>::type* = nullptr,
		typename FromMutex, template<typename M> class FromSharedLock, template<typename M> class FromLock, bool FromAllowAssign,
	typename std::enable_if<std::is_convertible<FromT&&, value_type>::value>::type* = nullptr>
	generic_synchronized& operator=(generic_synchronized<FromT, FromMutex, FromSharedLock, FromLock, FromAllowAssign>&& other) {
		auto tmp = std::move(other.synchronize().get());
		synchronize().get() = std::move(tmp);
		return *this;
	}

	/**
	 * @brief assign contained value from parameter
	 * @param from data to assign contained value from (types must be convertible)
	 */
	template<typename From, typename std::enable_if<std::is_convertible<From&&, value_type>::value>::type* = nullptr>
	generic_synchronized& operator=(From&& from) {
		synchronize().get() = std::forward<From>(from);
		return *this;
	}

	/**
	 * @brief get exclusive access
	 * @return object representing locked value
	 */
	locked synchronize() {
		return locked(m_mutex, m_value);
	}

	/**
	 * @brief get shared access
	 * @return object representing locked value
	 */
	shared_locked shared_synchronize() const {
		return shared_locked(m_mutex, m_value);
	}

	/**
	 * @brief run `callback` with reference to value while holding exlusive lock
	 * @param callback callback to call with reference to value
	 * @return whatever the callback returned
	 */
	template<typename Callable>
	auto synchronize(Callable&& callback) -> decltype(callback(m_value)) {
		lock_t lock(m_mutex);
		return callback(m_value);
	}

	/**
	 * @brief run `callback` with reference to const value while holding shared lock
	 * @param callback callback to call with reference to const value
	 * @return whatever the callback returned
	 */
	template<typename Callable>
	auto shared_synchronize(Callable&& callback) const -> decltype(callback(m_value)) {
		shared_lock_t lock(m_mutex);
		return callback(m_value);
	}
};

/**
 * @brief template alias to provide synchronized access to a value using exclusive locks
 * @tparam Value       value type to synchronize access to
 * @tparam AllowAssign AllowAssign for @ref generic_synchronized, default to false
 */
template<typename Value, bool AllowAssign = false>
using synchronized = generic_synchronized<Value, std::mutex, std::unique_lock, std::unique_lock, AllowAssign>;

/**
 * @brief template alias to provide synchronized access to a value using shared and exclusive locks
 * @tparam Value       value type to synchronize access to
 * @tparam AllowAssign AllowAssign for @ref generic_synchronized, default to false
 */
#if __cplusplus >= 201402L
	template<typename Value, bool AllowAssign = false>
	using shared_synchronized = generic_synchronized<Value, std::shared_timed_mutex, std::shared_lock, std::unique_lock, AllowAssign>;
#else
	template<typename Value, bool AllowAssign = false>
	using shared_synchronized = generic_synchronized<Value, boost::shared_mutex, boost::shared_lock, boost::unique_lock, AllowAssign>;
#endif

__CANEY_STDV1_END
