/** @file */

#pragma once

/**
  * @brief start util module for doxygen
  * @internal
  */
#define __CANEY_DOXYGEN_GROUP_UTILV1_BEGIN \
	/** @addtogroup util */ \
	/** @{ */

/**
  * @brief end util module for doxygen
  * @internal
  */
#define __CANEY_DOXYGEN_GROUP_UTILV1_END \
	/** @} */

/* don't use inline namespace when generating docs */

/**
 * @brief basically `namespace caney { namespace util { inline namespace v1 {` + some doxygen handling
 * @internal
 */
#if defined(DOXYGEN)
	#define __CANEY_UTILV1_BEGIN \
		/** @namespace caney */ \
		namespace caney { \
			/** @namespace caney::util */ \
			namespace util { \
				__CANEY_DOXYGEN_GROUP_UTILV1_BEGIN
#else
	#define __CANEY_UTILV1_BEGIN \
		/** @namespace caney */ \
		namespace caney { \
			/** @namespace caney::util */ \
			namespace util { \
				__CANEY_DOXYGEN_GROUP_UTILV1_BEGIN \
				/** @namespace caney::util::v1 */ \
				inline namespace v1 {
#endif

/**
 * @brief basically `} } }` + some doxygen handling
 * @internal
 */
#if defined(DOXYGEN)
	#define __CANEY_UTILV1_END \
				__CANEY_DOXYGEN_GROUP_UTILV1_END \
			} /* namespace caney::util */ \
		} /* namespace caney */
#else
	#define __CANEY_UTILV1_END \
				__CANEY_DOXYGEN_GROUP_UTILV1_END \
				} /* inline namespace caney::util::v1 */ \
			} /* namespace caney::util */ \
		} /* namespace caney */
#endif

/**
 * @defgroup util caney util component
 *
 * @brief The caney `util` component lives in the namespace @ref caney::util .
 */
