#include <stddef.h>

#include "Constants.hpp"
#include "HashNode.hpp"
#include "KeyHash.hpp"
#include <sstream>
#include <Windows.h>
#include <functional>

// Hash map class template
template <typename K, typename V, typename F = std::hash<K> >
class HashMap {
public:
	HashMap() {
		// construct zero initialized hash table of size
		mTable = new HashNode<K, V> *[constants::TABLE_SIZE]();
	}

	~HashMap() {
		// destroy all buckets one by one
		for ( int i = 0; i < constants::TABLE_SIZE; ++i ) {
			auto entry = mTable[i];
			while ( entry != NULL ) {
				auto prev = entry;
				entry = entry->getNext();
				delete prev;
			}
			mTable[i] = NULL;
		}
		// destroy the hash table
		delete[] mTable;
	}

	bool get( const K &key, V &value ) {
		auto hashValue = mHashFunc( key );
		auto entry = mTable[hashValue % constants::TABLE_SIZE];

		while ( entry != NULL ) {
			if ( entry->getKey() == key ) {
				value = entry->getValue();
				return true;
			}
			entry = entry->getNext();
		}
		return false;
	}

	void put( const K &key, const V &value ) {
		size_t hashValue = mHashFunc( key );
		
		HashNode<K, V> *prev = NULL;
		size_t index = hashValue % constants::TABLE_SIZE;
		auto entry = mTable[index];
		// HashNode<K, V> *entry = table[hashValue];

		while ( entry != NULL && entry->getKey() != key ) {
			prev = entry;
			entry = entry->getNext();
		}

		if ( entry == NULL ) {
			entry = new HashNode<K, V>( key, value );
			if ( prev == NULL ) {
				// insert as first bucket
				mTable[index] = entry;
			} else {
				prev->setNext( entry );
			}
		} else {
			// just update the value
			entry->setValue( value );
		}
	}

	void remove( const K &key ) {
		auto hashValue = mHashFunc( key );
		HashNode<K, V> *prev = NULL;
		size_t index = hashValue % constants::TABLE_SIZE;
		auto entry = mTable[index];

		while ( entry != NULL && entry->getKey() != key ) {
			prev = entry;
			entry = entry->getNext();
		}

		if ( entry == NULL ) {
			// key not found
			return;
		} else {
			if ( prev == NULL ) {
				// remove first bucket of the list
				mTable[index] = entry->getNext();
			} else {
				prev->setNext( entry->getNext() );
			}
			delete entry;
		}
	}

private:
	// hash table
	HashNode<K, V> **mTable;
	F mHashFunc;
};
