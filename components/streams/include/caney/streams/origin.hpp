#pragma once

#include "caney/std/object.hpp"

#include "internal.hpp"

#include <memory>

__CANEY_STREAMSV1_BEGIN

namespace impl {
	class origin_pause_watcher;
} // namespace impl

class origin;
class origin_pause;

/** @brief (interface) origin receives the pause/resume notifications.
 *
 * Usually streams are piped together like:
 *   ``[TCP #origin# source] ---> [sink #some filter# source] --> [sink #other filter# source] --> [sink #target# TCP]
 *
 * When any sink along the way doesn't want more data to limit memory usage it will pause;
 * the origin should therefore reflect the origin source of the data in a list of connected streams.
 *
 * The origin should pause generating data (i.e. stop reading from TCP) when asked to pause.
 */
class origin : private caney::object {
public:
	origin() = default;
	~origin();

	/**
	 * @brief usually called by @ref sink_base. the origin will be paused
	 * until all @ref origin_pause instances returned (and copies of them)
	 * are destroyed or reset.
	 */
	origin_pause pause();

	/**
	 * @brief returns whether origin is paused right now
	 */
	bool is_paused() const;

protected:
	/** @brief called when origin gets paused */
	virtual void on_pause() = 0;

	/** @brief called when origin gets resumed */
	virtual void on_resume() = 0;

private:
	friend class impl::origin_pause_watcher;

	void resume();

	std::weak_ptr<impl::origin_pause_watcher> m_weak_pause;
};

/** @brief pauses an @ref origin until destroyed or reset (see @ref origin::pause()) */
class origin_pause {
public:
	origin_pause() = default;

	/** whether currently pausing an @ref origin */
	explicit operator bool();

	/** resume @ref origin (see @ref origin::pause()) */
	void reset();

private:
	friend class origin;

	origin_pause(std::shared_ptr<impl::origin_pause_watcher> watcher) : m_watcher(std::move(watcher)) {}

	std::shared_ptr<impl::origin_pause_watcher> m_watcher;
};

__CANEY_STREAMSV1_END
