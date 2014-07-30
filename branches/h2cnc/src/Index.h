// Copyright (c) 2011, Joseph Coffland, Cauldron Development LLC
//
// This program is released under the BSD license. See the file COPYING for
// details.

#pragma once

#include <map>
#include <stdexcept>
#include <assert.h>

/// Maintain a map of objects with unique indices
template <typename INDEX_T, typename OBJECT_T>
class Index {
  INDEX_T next;
  typedef std::map<INDEX_T, OBJECT_T> index_t;
  index_t index;

public:
  Index() : next(1) {}

  OBJECT_T find(INDEX_T x) {
    typename index_t::iterator it = index.find(x);
    assert(it != index.end());
    return it->second;
  }

  INDEX_T insert(OBJECT_T o) {
    // This is essentially the same algorithm SQLite uses for choosing indices.

    // First try the next number in order.  Most of the time this will work.
    if (!next) next++;  // Skip zero
    if (index.find(next) == index.end()) return do_insert(o);

    // If 'next' is in use then do a random search.  Things will slow down if 
    // we start running out of indices, which normally wont happen.
    while (true) {
      next = (next << 15) + rand();
      if (next && index.find(next) == index.end()) return do_insert(o);
    }
  }

  void erase(const INDEX_T &x) {index.erase(x);}
  void clear() {next = 1; index.clear();}

protected:
  INDEX_T do_insert(OBJECT_T o) {
    index.insert(typename index_t::value_type(next, o));
    return next++;
  }
};
