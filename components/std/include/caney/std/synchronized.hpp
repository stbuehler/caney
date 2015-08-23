#pragma once

#include <mutex>
#include <type_traits>

#if __cplusplus >= 201402L
#include <shared_mutex>
#else
#include <boost/thread/shared_mutex.hpp>
#endif

namespace caney {
	inline namespace stdv1 {
		template<typename T, typename Lock>
		class locked_synchronized {
		public:
			using value_type = T;

			explicit locked_synchronized(typename Lock::mutex_type& mutex, value_type& value)
			: m_lock(mutex), m_value_ref(value) {
			}

			value_type* operator->() const {
				return &m_value_ref;
			}

			value_type& operator*() const {
				return m_value_ref;
			}

			value_type& get() const {
				return m_value_ref;
			}

		private:
			Lock m_lock;
			value_type& m_value_ref;
		};

		namespace impl {
			template<typename T, typename Mutex, template<typename M> class SharedLock, template<typename M> class Lock, bool AllowAssign>
			class synchronized {
			public:
				using value_type = T;
				using mutex_t = Mutex;
				using shared_lock_t = SharedLock<Mutex>;
				using lock_t = Lock<Mutex>;
				using shared_locked = locked_synchronized<value_type const, shared_lock_t>;
				using locked = locked_synchronized<value_type, lock_t>;

			private:
				mutable mutex_t m_mutex;
				value_type m_value; // decltype() below needs this before using it

			public:
				template<typename std::enable_if<std::is_default_constructible<value_type>::value>::type* = nullptr>
				explicit synchronized() {
				}

				template<typename From, typename std::enable_if<std::is_convertible<From&&, value_type>::value>::type* = nullptr>
				explicit synchronized(From&& from)
				: m_value(std::forward<From>(from)) {
				}

				synchronized(synchronized const& other)
				: m_value(other.shared_synchronize().get()) {
				}

				synchronized(synchronized&& other)
				: m_value(std::move(other.synchronize().get())) {
				}

				template<typename FromT, typename FromMutex, template<typename M> class FromSharedLock, template<typename M> class FromLock, bool FromAllowAssign,
					typename std::enable_if<std::is_convertible<FromT const&, value_type>::value>::type* = nullptr>
				explicit synchronized(synchronized<FromT, FromMutex, FromSharedLock, FromLock, FromAllowAssign> const& from)
				: m_value(from.shared_synchronize().get()) {
				}

				template<typename FromT, typename FromMutex, template<typename M> class FromSharedLock, template<typename M> class FromLock, bool FromAllowAssign,
					typename std::enable_if<std::is_convertible<FromT&&, value_type>::value>::type* = nullptr>
				explicit synchronized(synchronized<FromT, FromMutex, FromSharedLock, FromLock, FromAllowAssign>&& from)
				: m_value(std::move(from.synchronize().get())) {
				}

				template<typename FromT, bool TAllowAssign = AllowAssign, typename std::enable_if<TAllowAssign && AllowAssign>::type* = nullptr,
					typename FromMutex, template<typename M> class FromSharedLock, template<typename M> class FromLock, bool FromAllowAssign,
				typename std::enable_if<std::is_convertible<FromT&&, value_type>::value>::type* = nullptr>
				synchronized& operator=(synchronized<FromT, FromMutex, FromSharedLock, FromLock, FromAllowAssign> const& other) {
					auto tmp = other.shared_synchronize().get();
					synchronize().get() = std::move(tmp);
					return *this;
				}

				template<typename FromT, bool TAllowAssign = AllowAssign, typename std::enable_if<TAllowAssign && AllowAssign>::type* = nullptr,
					typename FromMutex, template<typename M> class FromSharedLock, template<typename M> class FromLock, bool FromAllowAssign,
				typename std::enable_if<std::is_convertible<FromT&&, value_type>::value>::type* = nullptr>
				synchronized& operator=(synchronized<FromT, FromMutex, FromSharedLock, FromLock, FromAllowAssign>& other) {
					auto tmp = other.shared_synchronize().get();
					synchronize().get() = std::move(tmp);
					return *this;
				}

				template<typename FromT, bool TAllowAssign = AllowAssign, typename std::enable_if<TAllowAssign && AllowAssign>::type* = nullptr,
					typename FromMutex, template<typename M> class FromSharedLock, template<typename M> class FromLock, bool FromAllowAssign,
				typename std::enable_if<std::is_convertible<FromT&&, value_type>::value>::type* = nullptr>
				synchronized& operator=(synchronized<FromT, FromMutex, FromSharedLock, FromLock, FromAllowAssign>&& other) {
					auto tmp = std::move(other.synchronize().get());
					synchronize().get() = std::move(tmp);
					return *this;
				}

				template<typename From, typename std::enable_if<std::is_convertible<From&&, value_type>::value>::type* = nullptr>
				synchronized& operator=(From&& from) {
					synchronize().get() = std::forward<From>(from);
					return *this;
				}

				locked synchronize() {
					return locked(m_mutex, m_value);
				}

				shared_locked shared_synchronize() const {
					return shared_locked(m_mutex, m_value);
				}

				template<typename Callable>
				auto synchronize(Callable&& callback) -> decltype(callback(m_value)) {
					lock_t lock(m_mutex);
					return callback(m_value);
				}

				template<typename Callable>
				auto shared_synchronize(Callable&& callback) const -> decltype(callback(m_value)) {
					shared_lock_t lock(m_mutex);
					return callback(m_value);
				}
			};
		}

		template<typename T, bool AllowAssign = false>
		using synchronized = impl::synchronized<T, std::mutex, std::unique_lock, std::unique_lock, AllowAssign>;

#if __cplusplus >= 201402L
		template<typename T, bool AllowAssign = false>
		using shared_synchronized = impl::synchronized<T, std::shared_timed_mutex, std::shared_lock, std::unique_lock, AllowAssign>;
#else
		template<typename T, bool AllowAssign = false>
		using shared_synchronized = impl::synchronized<T, boost::shared_mutex, boost::shared_lock, boost::unique_lock, AllowAssign>;
#endif
	}
} // namespace caney

