#pragma once

#include "internal.hpp"

__CANEY_STDV1_BEGIN

/**
 * @defgroup bitstring BitString
 * @{
  BitString concept:
   type `S` satisfies `BitString` if
   - `S` satisfies `MoveConstructible`
   And, given
   - `s`, `a`, `b`: expressions of type `S` or `S const`
   - `l`, `ndx`; expressions of type `size_t`
   The following expressions must be valid and have their specified effects:
   - `s.length()` returning a `size_t` containing the length of the BitString in bits
   - `s.truncate(l)` returning another bitstring of type `S` which is a prefix of `s` with a maximum length of `l` (but may be shorter)
   - `s[ndx]`: returns a `bool`: whether bit at given index `ndx` is set
   - `a == b`: returns a `bool`: whether the two bitstrings are equal in length and all contained bits
   - `a != b`: `!(a == b)`
   - `is_lexicographic_less(a, b)`: returns a `bool`: whether `a` is smaller than `b` in the lexicographic ordering:
     - if `a` != `b` and `a` is a prefix of `b`: `a` is smaller than `b`
     - otherwise simply compare `a.truncate(std::min(a.length(), b.length())` and `b.truncate(std::min(a.length(), b.length())`
   - `is_tree_less(a, b)`: returns a `bool`: whether `a` is smaller than `b` in the binary tree ordering:
     a node is greater than all elements in the left subtree and smaller than all elements in the right subtree.
     - if (`a` != `b` and) `a` is a prefix of `b`:
       - if the first bit in `b` after the prefix `a` is `1`: `a` is smaller than `b`
       - otherwise: `a` is NOT smaller than `b`
     - otherwise simply compare `a.truncate(std::min(a.length(), b.length())` and `b.truncate(std::min(a.length(), b.length())`
   - `is_prefix(a, b)`: returns a `bool`: whether `a` is a prefix of `b` (this is especially true if `a == b`!), i.e. if there is a bitstring x such that `a == b . x`
   - `longest_common_prefix(a, b)`: returns a bitstring of type `S` which represents the longest bitstring x such that there are bitstrings u and v satisfying: `a == x . u` and `b == x . v`.
     it might use storage from `a` in the returned value

   `BitString`s do not necessarily own the data of the bitstring they contain, they might simply contain a pointer to the data.
 * @}
 */

__CANEY_STDV1_END
