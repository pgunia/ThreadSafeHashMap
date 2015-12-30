#include <stddef.h>

#include "Constants.hpp"
#include "HashNode.hpp"
#include "KeyHash.hpp"
#include <sstream>
// #include <Windows.h>
#include <functional>
#include <mutex>

// Hash map class template
template<typename K, typename V, typename F = std::hash<K> >
class HashMap {
public:
	HashMap(int size = constants::TABLE_SIZE) :
			mTableSize(size) {
		// construct zero initialized hash table of size
		init();
	}

	~HashMap() {
		purge();
	}

	bool get(const K &key, V &value) {

		const auto hashValue = mHashFunc(key);
		const auto index = hashValue % mTableSize;

		// retrieve mutex for hashmap row and lock
		std::mutex * mutex = this->mMutexList[index];

		// automatically released when lock goes out of scope
		std::lock_guard<std::mutex> lock(*mutex);
		auto entry = mTable[hashValue % mTableSize];

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
		const size_t hashValue = mHashFunc(key);

		HashNode<K, V> *prev = NULL;
		const size_t index = hashValue % mTableSize;

		// retrieve mutex for hashmap row and lock
		std::mutex * mutex = this->mMutexList[index];

		// automatically released when lock goes out of scope
		std::lock_guard<std::mutex> lock(*mutex);

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
			mSize++;
		} else {
			// just update the value
			entry->setValue(value);
		}
	}

	void remove(const K &key) {
		const auto hashValue = mHashFunc(key);
		HashNode<K, V> *prev = NULL;
		const size_t index = hashValue % mTableSize;

		// retrieve mutex for hashmap row and lock
		std::mutex * mutex = this->mMutexList[index];

		// automatically released when lock goes out of scope
		std::lock_guard<std::mutex> lock(*mutex);
		auto entry = mTable[index];

		while (entry != NULL && entry->getKey() != key) {
			prev = entry;
			entry = entry->getNext();
		}

		if (entry == NULL) {
			// key not found
			return;
		} else {
			if (prev == NULL) {
				// remove first bucket of the list
				mTable[index] = entry->getNext();
			} else {
				prev->setNext(entry->getNext());
			}
			mSize--;
			delete entry;
		}
	}

	void clear() {
		purge();
		init();
	}

	int size() {
		return mSize;
	}

	void resize(const int newTableSize) {
		const int numberOfEntries = mSize;
		HashNode<K, V> ** cache = new HashNode<K, V>*[numberOfEntries];
		int entryCount = 0;

		// cache all current entries in the table
		for (int i = 0; i < mTableSize; i++) {
			HashNode<K, V> * entry = mTable[i];
			while (entry != NULL) {
				entry = entry->getNext();
				cache[entryCount] = entry;
				entryCount++;
			}
		}

		// purge old table
		purge();
		this->mTableSize = newTableSize;
		init();

		for (int i = 0; i < numberOfEntries; i++) {
			const HashNode<K, V>* cur = cache[i];
			put(cur->getKey(), cur->getValue());
		}
		delete cache;
	}

private:
	void init() {
		mTable = new HashNode<K, V> *[mTableSize]();
		mSize = 0;

		// create a list of mutexes, one for every row in the hashmap
		mMutexList = new std::mutex *[mTableSize]();

		// initialize the mutex list
		for (int i = 0; i < mTableSize; i++) {
			mMutexList[i] = new std::mutex;
		}
	}

	void purge() {
		// destroy all buckets one by one
		for (int i = 0; i < mTableSize; ++i) {
			auto entry = mTable[i];
			while (entry != NULL) {
				const auto prev = entry;
				entry = entry->getNext();
				delete prev;
			}
			mTable[i] = NULL;
		}

		// destroy all mutexes
		for (int i = 0; i < mTableSize; i++) {
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
	F mHashFunc;
	int mSize;
	int mTableSize;
	std::mutex **mMutexList;
};
