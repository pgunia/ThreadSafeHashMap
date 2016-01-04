#include <stddef.h>

#include "Constants.hpp"
#include "HashNode.hpp"
#include "KeyHash.hpp"
#include <sstream>
#include <functional>
#include <shared_mutex>
#include <iostream>
using namespace std;

// Hash map class template
template<typename K, typename V, typename F = std::hash<K> >
class HashMap {
public:
	HashMap(int size = constants::TABLE_SIZE) :
			mTableRowCount(size) {
		init();
	}

	~HashMap() {
		purge();
	}

	bool get(const K &key, V &value) {
		// acquire read lock for map instance
		std::shared_lock < std::shared_timed_mutex
				> sharedMapLock(this->mMapMutex);

		const auto hashValue = mHashFunc(key);
		const auto index = hashValue % mTableRowCount;

		// retrieve mutex for hashmap row and lock
		std::shared_timed_mutex * mutex = this->mMutexList[index];

		// acquire shared lock for row
		std::shared_lock < std::shared_timed_mutex > sharedLock(*mutex);

		auto entry = mTable[hashValue % mTableRowCount];

		while (entry != NULL) {
			if (entry->getKey() == key) {
				value = entry->getValue();
				return true;
			}
			entry = entry->getNext();
		}
		return false;
	}

	void put(const K &key, const V &value) {
		// acquire read lock for map instance, only necessary if an exclusive lock has not already been acquired (e.g. by resize())
		// reentrant locks are not supported, thus threads are in danger of producing deadlocks
		std::shared_lock < std::shared_timed_mutex
				> sharedMapLock(this->mMapMutex);
		this->putInternal(key, value);
	}

	void remove(const K &key) {
		// acquire read lock for map instance
		std::shared_lock < std::shared_timed_mutex
				> sharedMapLock(this->mMapMutex);

		const auto hashValue = mHashFunc(key);
		HashNode<K, V> *prev = NULL;
		const size_t index = hashValue % mTableRowCount;

		// retrieve mutex for hashmap row and lock
		std::shared_timed_mutex * mutex = this->mMutexList[index];

		// acquire exclusive row lock
		std::lock_guard<std::shared_timed_mutex> lock(*mutex);
		auto entry = mTable[index];

		while (entry != NULL && entry->getKey() != key) {
			prev = entry;
			entry = entry->getNext();
		}

		if (entry == NULL) {
			// key could not be found
			return;
		} else {
			if (prev == NULL) {
				// remove first row from the list
				mTable[index] = entry->getNext();
			} else {
				prev->setNext(entry->getNext());
			}
			decrementElementCount();
			delete entry;
		}
	}

	void clear() {
		purge();
		init();
	}

	int size() {
		// acquire read lock for map instance
		std::shared_lock < std::shared_timed_mutex
				> sharedMapLock(this->mMapMutex);
		return mSize;
	}

	void resize(const int newTableSize) {
		// acquire write lock for complete map, no other operations are allowed while purging is running
		std::lock_guard < std::shared_timed_mutex> exclusiveMapLock(this->mMapMutex);
		const int numberOfEntries = mSize;
		HashNode<K, V> ** cache = new HashNode<K, V>*[numberOfEntries];
		int entryCount = 0;

		// cache all current entries in the table
		for (int i = 0; i < mTableRowCount; i++) {
			HashNode<K, V>* entry = mTable[i];
			while (entry != NULL) {
				entry = entry->getNext();
				cache[entryCount] = new HashNode<K,V>(entry->getKey(), entry->getValue());
				entryCount++;
			}
		}

		// purge old table
		// purge();
		this->mTableRowCount = newTableSize;
		init();
		std::cout << "ENTRY COUNT: " << std::to_string(numberOfEntries) << " , STARTING TO ADD ENTRIES" << std::endl;
		for (int i = 0; i < numberOfEntries; i++) {
			const HashNode<K, V>* cur = cache[i];
			if(cur == NULL) {
				cout << "CUR IS NULL" << endl;
			}
 			std::cout << "KEY: " << std::to_string(cur->getKey()) << endl;
 			cout << "TEST" << endl;
			std::cout << "VALUE: " << cur->getValue() << endl;
			putInternal(cur->getKey(), cur->getValue());
		}
		delete cache;
	}

private:
	void incrementElementCount() {
		// acquire write lock for complete map, because increment is not atomic and thus not threadsafe
		// regular read lock as in put is not sufficient to secure access
		// std::lock_guard < std::shared_timed_mutex> exclusiveMapLock(this->mMapMutex);
		mSize++;
	}

	void decrementElementCount() {
		// acquire write lock for complete map, because decrement is not atomic and thus not threadsafe
		// regular read lock as in put is not sufficient to secure access
		// std::lock_guard < std::shared_timed_mutex> exclusiveMapLock(this->mMapMutex);
		mSize--;
	}

	void putInternal(const K &key, const V &value) {
		std::cout << "PUTINTERNAL" << std::endl;
		std::cout << "ADDING " << std::to_string(key)  << " , VALUE: " << value << std::endl;
		const size_t hashValue = mHashFunc(key);

		HashNode<K, V> *prev = NULL;
		const size_t index = hashValue % mTableRowCount;
		std::cout << "KEY: " << std::to_string(key) << ", VALUE: " << value
				<< ", INDEX: " << std::to_string(index) << std::endl;

		// retrieve mutex for hashmap row and lock
		std::shared_timed_mutex * mutex = this->mMutexList[index];

		// acquire exclive lock on shared mutex to prevent modifications on the same row in the map
		std::lock_guard<std::shared_timed_mutex> lock(*mutex);

		auto entry = mTable[index];

		while (entry != NULL && entry->getKey() != key) {
			prev = entry;
			entry = entry->getNext();
		}

		if (entry == NULL) {
			entry = new HashNode<K, V>(key, value);
			if (prev == NULL) {
				// insert as first bucket
				mTable[index] = entry;
			} else {
				prev->setNext(entry);
			}

			// NOT THREADSAFE
			incrementElementCount();
		} else {
			// just update the value
			entry->setValue(value);
		}
	}

	// init is not secured by locks, because the calling methods are guarded
	void init() {
		mTable = new HashNode<K, V> *[mTableRowCount]();
		mSize = 0;

		// create a list of mutexes, one for every row in the hashmap
		mMutexList = new std::shared_timed_mutex *[mTableRowCount]();

		// initialize the mutex list
		for (int i = 0; i < mTableRowCount; i++) {
			mMutexList[i] = new std::shared_timed_mutex;
		}
	}

	// purge is not secured by locks, because the calling methods are guarded
	void purge() {

		// destroy all buckets one by one
		for (int i = 0; i < mTableRowCount; ++i) {
			auto entry = mTable[i];
			while (entry != NULL) {
				const auto prev = entry;
				entry = entry->getNext();
				delete prev;
			}
			mTable[i] = NULL;
		}

		// destroy all mutexes
		for (int i = 0; i < mTableRowCount; i++) {
			auto entry = mMutexList[i];
			delete entry;
		}
		delete[] mMutexList;

		// destroy the hash table
		delete[] mTable;
		mSize = 0;
	}

	// hash table
	HashNode<K, V> **mTable;

	// hash function used for hashing
	F mHashFunc;

	// element count
	int mSize;

	// row count within the map
	int mTableRowCount;

	// holds a list with one mutex for every row in the map, enables per-row-locking
	std::shared_timed_mutex **mMutexList;

	// needed to control map-wide lockings e.g. for purging
	std::shared_timed_mutex mMapMutex;
};
