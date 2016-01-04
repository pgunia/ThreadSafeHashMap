/*
 * KeyHash.hpp
 *
 *  Created on: 30.12.2015
 *      Author: Patrick
 */

#ifndef KEYHASH_HPP_
#define KEYHASH_HPP_

#include "Constants.hpp"

// Default hash function class
template <typename K>
struct KeyHash {
	/*
    unsigned long operator()(const K& key) const
    {
        return reinterpret_cast<unsigned long>(key) % constants::TABLE_SIZE;
    }
    */
};

#endif /* KEYHASH_HPP_ */
