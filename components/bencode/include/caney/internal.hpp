/** @file */
// clang-format off

#pragma once

/**
  * @brief start bencode module for doxygen
  * @internal
  */
#define __CANEY_DOXYGEN_GROUP_BENCODEV1_BEGIN \
	/** @addtogroup bencode */ \
	/** @{ */

/**
  * @brief end bencode module for doxygen
  * @internal
  */
#define __CANEY_DOXYGEN_GROUP_BENCODEV1_END \
	/** @} */

/* don't use inline namespace when generating docs */

/**
 * @brief basically `namespace caney { namespace bencode { inline namespace v1 {` + some doxygen handling
 * @internal
 */
#if defined(DOXYGEN)
	#define __CANEY_BENCODEV1_BEGIN \
		/** @namespace caney */ \
		namespace caney { \
			/** @namespace caney::bencode */ \
			namespace bencode { \
				__CANEY_DOXYGEN_GROUP_BENCODEV1_BEGIN
#else
	#define __CANEY_BENCODEV1_BEGIN \
		/** @namespace caney */ \
		namespace caney { \
			/** @namespace caney::bencode */ \
			namespace bencode { \
				__CANEY_DOXYGEN_GROUP_BENCODEV1_BEGIN \
				/** @namespace caney::bencode::v1 */ \
				inline namespace v1 {
#endif

/**
 * @brief basically `} } }` + some doxygen handling
 * @internal
 */
#if defined(DOXYGEN)
	#define __CANEY_BENCODEV1_END \
				__CANEY_DOXYGEN_GROUP_BENCODEV1_END \
			} /* namespace caney::bencode */ \
		} /* namespace caney */
#else
	#define __CANEY_BENCODEV1_END \
				__CANEY_DOXYGEN_GROUP_BENCODEV1_END \
				} /* inline namespace caney::bencode::v1 */ \
			} /* namespace caney::bencode */ \
		} /* namespace caney */
#endif

/**
 * @defgroup bencode caney bencode component
 *
 * @brief The caney `bencode` component lives in the namespace @ref caney::bencode .
 */
