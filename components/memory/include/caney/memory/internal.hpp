/** @file */

#pragma once

/**
  * @brief start memory module for doxygen
  * @internal
  */
#define __CANEY_DOXYGEN_GROUP_MEMORYV1_BEGIN \
	/** @addtogroup memory */ \
	/** @{ */

/**
  * @brief end memory module for doxygen
  * @internal
  */
#define __CANEY_DOXYGEN_GROUP_MEMORYV1_END \
	/** @} */

/* don't use inline namespace when generating docs */

/**
 * @brief basically `namespace caney { namespace memory { inline namespace v1 {` + some doxygen handling
 * @internal
 */
#if defined(DOXYGEN)
	#define __CANEY_MEMORYV1_BEGIN \
		/** @namespace caney */ \
		namespace caney { \
			/** @namespace caney::memory */ \
			namespace memory { \
				__CANEY_DOXYGEN_GROUP_MEMORYV1_BEGIN
#else
	#define __CANEY_MEMORYV1_BEGIN \
		/** @namespace caney */ \
		namespace caney { \
			/** @namespace caney::memory */ \
			namespace memory { \
				__CANEY_DOXYGEN_GROUP_MEMORYV1_BEGIN \
				/** @namespace caney::memory::v1 */ \
				inline namespace v1 {
#endif

/**
 * @brief basically `} } }` + some doxygen handling
 * @internal
 */
#if defined(DOXYGEN)
	#define __CANEY_MEMORYV1_END \
				__CANEY_DOXYGEN_GROUP_MEMORYV1_END \
			} /* namespace caney::memory */ \
		} /* namespace caney */
#else
	#define __CANEY_MEMORYV1_END \
				__CANEY_DOXYGEN_GROUP_MEMORYV1_END \
				} /* inline namespace caney::memory::v1 */ \
			} /* namespace caney::memory */ \
		} /* namespace caney */
#endif

/**
 * @defgroup memory caney memory component
 *
 * @brief The caney `memory` component lives in the namespace @ref caney::memory .
 */
