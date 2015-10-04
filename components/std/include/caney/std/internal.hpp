/** @file */

#pragma once

/**
  * @brief start std module for doxygen
  * @internal
  */
#define __CANEY_DOXYGEN_GROUP_STDV1_BEGIN \
	/** @addtogroup std */ \
	/** @{ */

/**
  * @brief end std module for doxygen
  * @internal
  */
#define __CANEY_DOXYGEN_GROUP_STDV1_END \
	/** @} */

/* don't use inline namespace when generating docs */

/**
 * @brief basically `namespace caney { inline namespace stdv1 {` + some doxygen handling
 * @internal
 */
#if defined(DOXYGEN)
	#define __CANEY_STDV1_BEGIN \
		/** @namespace caney */ \
		namespace caney { \
			__CANEY_DOXYGEN_GROUP_STDV1_BEGIN
#else
	#define __CANEY_STDV1_BEGIN \
		/** @namespace caney */ \
		namespace caney { \
			__CANEY_DOXYGEN_GROUP_STDV1_BEGIN \
			/** @namespace stdv1 */ \
			inline namespace stdv1 {
#endif

/**
 * @brief basically `} }` + some doxygen handling
 * @internal
 */
#if defined(DOXYGEN)
	#define __CANEY_STDV1_END \
			__CANEY_DOXYGEN_GROUP_STDV1_END \
		} /* namespace caney */
#else
	#define __CANEY_STDV1_END \
			__CANEY_DOXYGEN_GROUP_STDV1_END \
			} /* inline namespace stdv1 */ \
		} /* namespace caney */
#endif

/**
 * @defgroup std caney std component
 *
 * @brief The caney `std` component lives in the root namespace @ref caney; other components will use nested namespace in @ref caney.
 */
