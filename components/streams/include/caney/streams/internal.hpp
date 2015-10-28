/** @file */
// clang-format off

#pragma once

/**
  * @brief start streams module for doxygen
  * @internal
  */
#define __CANEY_DOXYGEN_GROUP_STREAMSV1_BEGIN \
	/** @addtogroup streams */ \
	/** @{ */

/**
  * @brief end streams module for doxygen
  * @internal
  */
#define __CANEY_DOXYGEN_GROUP_STREAMSV1_END \
	/** @} */

/* don't use inline namespace when generating docs */

/**
 * @brief basically `namespace caney { namespace streams { inline namespace v1 {` + some doxygen handling
 * @internal
 */
#if defined(DOXYGEN)
	#define __CANEY_STREAMSV1_BEGIN \
		/** @namespace caney */ \
		namespace caney { \
			/** @namespace caney::streams */ \
			namespace streams { \
				__CANEY_DOXYGEN_GROUP_STREAMSV1_BEGIN
#else
	#define __CANEY_STREAMSV1_BEGIN \
		/** @namespace caney */ \
		namespace caney { \
			/** @namespace caney::streams */ \
			namespace streams { \
				__CANEY_DOXYGEN_GROUP_STREAMSV1_BEGIN \
				/** @namespace caney::streams::v1 */ \
				inline namespace v1 {
#endif

/**
 * @brief basically `} } }` + some doxygen handling
 * @internal
 */
#if defined(DOXYGEN)
	#define __CANEY_STREAMSV1_END \
				__CANEY_DOXYGEN_GROUP_STREAMSV1_END \
			} /* namespace caney::streams */ \
		} /* namespace caney */
#else
	#define __CANEY_STREAMSV1_END \
				__CANEY_DOXYGEN_GROUP_STREAMSV1_END \
				} /* inline namespace caney::streams::v1 */ \
			} /* namespace caney::streams */ \
		} /* namespace caney */
#endif

/**
 * @defgroup streams caney streams component
 *
 * @brief The caney `streams` component lives in the namespace @ref caney::streams .
 */
