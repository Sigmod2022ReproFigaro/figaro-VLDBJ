// Structure that defines hashing and comparison operations for user's type.
#ifndef _FIGARO_HASH_H_
#define _FIGARO_HASH_H_

#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_hash_map.h>

namespace tbb
{
    template<typename... TupleArgs>
    struct tbb_hash<std::tuple<TupleArgs...> >
    {
        tbb_hash() {}
        size_t operator()(const std::tuple<TupleArgs...>& key) const {
            return std::hash<std::tuple<TupleArgs...>>{}(key);
        }
    };
}

#endif