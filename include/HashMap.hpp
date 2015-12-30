#include <stddef.h>

#include "Constants.hpp"
#include "HashNode.hpp"
#include "KeyHash.hpp"
#include <functional>

// Hash map class template
template <typename K, typename V, typename F = std::hash<K> >
class HashMap {
public:
    HashMap() {
        // construct zero initialized hash table of size
        table = new HashNode<K, V> *[constants::TABLE_SIZE]();
    }

    ~HashMap() {
        // destroy all buckets one by one
        for (int i = 0; i < constants::TABLE_SIZE; ++i) {
            auto entry = table[i];
            while (entry != NULL) {
                auto prev = entry;
                entry = entry->getNext();
                delete prev;
            }
            table[i] = NULL;
        }
        // destroy the hash table
        delete [] table;
    }

    bool get(const K &key, V &value) {
        auto hashValue = hashFunc(key);
        auto entry = table[hashValue];

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
        auto hashValue = hashFunc(key);
        HashNode<K, V> *prev = NULL;
        auto entry = table[hashValue];
        // HashNode<K, V> *entry = table[hashValue];

        while (entry != NULL && entry->getKey() != key) {
            prev = entry;
            entry = entry->getNext();
        }

        if (entry == NULL) {
            entry = new HashNode<K, V>(key, value);
            if (prev == NULL) {
                // insert as first bucket
                table[hashValue] = entry;
            } else {
                prev->setNext(entry);
            }
        } else {
            // just update the value
            entry->setValue(value);
        }
    }

    void remove(const K &key) {
        auto hashValue = hashFunc(key);
        HashNode<K, V> *prev = NULL;
        auto entry = table[hashValue];

        while (entry != NULL && entry->getKey() != key) {
            prev = entry;
            entry = entry->getNext();
        }

        if (entry == NULL) {
            // key not found
            return;
        }
        else {
            if (prev == NULL) {
                // remove first bucket of the list
                table[hashValue] = entry->getNext();
            } else {
                prev->setNext(entry->getNext());
            }
            delete entry;
        }
    }

private:
    // hash table
    HashNode<K, V> **table;
    F hashFunc;
};
