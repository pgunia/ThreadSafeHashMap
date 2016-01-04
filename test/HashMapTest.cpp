/*
 * HashMapTest.cpp
 *
 *  Created on: 31.12.2015
 *      Author: Patrick Gunia
 */

#include <gtest/gtest.h>
#include <HashMap.hpp>
#include <thread>

using namespace std;

TEST(HashMapTest, ValidPutTest) {

    HashMap<int, string> map;
    const string value1 = "value1";
    map.put(1, value1);

    string result;
    bool success;

    success = map.get(1, result);
    EXPECT_EQ(true, success);
    EXPECT_EQ(result, value1);
}

TEST(HashMapTest, ValidSecondPutSameKey) {

    HashMap<int, string> map;
    const string value1 = "value1";
    map.put(1, value1);

    // overwrite with same key
    string result;
    bool success;

    const string value2 = "value2";
    map.put(1, value2);

    success = map.get(1, result);
    EXPECT_EQ(true, success);
    EXPECT_EQ(result, value2);
}

TEST(HashMapTest, RemovePut) {

    HashMap<int, string> map;
    const string value1 = "value1";
    map.put(1, value1);
    map.remove(1);

    string result;
    bool success;

    success = map.get(1, result);
    EXPECT_EQ(false, success);
}

TEST(HashMapTest, Clear) {

    HashMap<int, string> map;
    const string value = "value";
    const int numberEntries = 100;

    // add 100 entries
    for (int i = 0; i < numberEntries; i++) {
        map.put(i, value);
    }

    // clear map
    map.clear();
    bool success;
    string result;
    // non of the values should be in the map any longer
    for (int i = 0; i < numberEntries; i++) {
        success = map.get(i, result);
        EXPECT_EQ(false, success);
    }

    // size should be 0
    EXPECT_EQ(0, map.size());
}

TEST(HashMapTest, Size) {

    HashMap<int, string> map;
    const string value = "value";
    const int numberEntries = 100;

    // add 100 entries
    for (int i = 0; i < numberEntries; i++) {
        map.put(i, value);
    }

    EXPECT_EQ(numberEntries, map.size());
}

TEST(HashMapTest, SizeOverwriteExisting) {

    HashMap<int, string> map;
    const string value = "value";
    map.put(0, value);
    EXPECT_EQ(1, map.size());

    // overwrite existing key, size should not change
    const string newValue = "newValue";
    map.put(0, newValue);
    EXPECT_EQ(1, map.size());

}

TEST(HashMapTest, SizeRemove) {
    HashMap<int, string> map;
    const string value = "value";
    const int numberEntries = 100;

    // add 100 entries
    for (int i = 0; i < numberEntries; i++) {
        map.put(i, value);
    }

    EXPECT_EQ(numberEntries, map.size());

    // remove all previously added entries, size should be back to 0
    // add 100 entries
    for (int i = 0; i < numberEntries; i++) {
        map.remove(i);
    }

    EXPECT_EQ(0, map.size());
}

TEST(HashMapTest, Resize) {
    HashMap<int, string> map;
    const string value = "value";
    const int numberEntries = 200;
    const int newTableSize = 10;

    // add 200 entries
    for (int i = 0; i < numberEntries; i++) {
        map.put(i, value);
    }

    map.resize(newTableSize);

    // number of entries should not have changed
    EXPECT_EQ(numberEntries, map.size());

    // test if all entries can be retrieved from the map and contain the correct value
    string result;
    for (int i = 0; i < numberEntries; i++) {
        const bool success = map.get(i, result);
        EXPECT_EQ(true, success);
        EXPECT_EQ(result, value);
    }
}

struct add_entries_struct {
    HashMap<int, string> * mMap;

    int mNumberOfIterations;

    const string mValue = "value";

    const long mSleepTimeInMilliseconds = 2;

    add_entries_struct(HashMap<int, string> *mapParam, int numberOfEntries) :
            mMap(mapParam), mNumberOfIterations(numberOfEntries) {
    }

    // add elements on random positions to the map
    void operator()() {
        for (int i = 0; i < mNumberOfIterations; i++) {
            this->mMap->put((std::rand() % constants::MAX_INTEGER_KEY), mValue);
            std::this_thread::sleep_for(std::chrono::milliseconds(mSleepTimeInMilliseconds));
        }
    }
};

struct remove_entries_struct {
    HashMap<int, string> * mMap;

    int mNumberOfIterations;

    long mSleepTimeInMilliseconds = 2;

    remove_entries_struct(HashMap<int, string> *mapParam, int numberOfIterations) :
            mMap(mapParam), mNumberOfIterations(numberOfIterations) {
    }

    // remove elements randomly from the map
    void operator()() {
        for (int i = 0; i < mNumberOfIterations; i++) {
            this->mMap->remove(std::rand() % constants::MAX_INTEGER_KEY);
            std::this_thread::sleep_for(std::chrono::milliseconds(mSleepTimeInMilliseconds));
        }
    }
};

struct get_entries_struct {
    HashMap<int, string> * mMap;

    int mNumberOfIterations;

    long mSleepTimeInMilliseconds = 1;

    get_entries_struct(HashMap<int, string> *mapParam, int numberOfIterations) :
            mMap(mapParam), mNumberOfIterations(numberOfIterations) {
    }

    // remove elements randomly from the map
    void operator()() {
        string value;
        for (int i = 0; i < mNumberOfIterations; i++) {
            this->mMap->get(std::rand() % constants::MAX_INTEGER_KEY, value);
            std::this_thread::sleep_for(std::chrono::milliseconds(mSleepTimeInMilliseconds));
        }
    }
};

struct resize_map_struct {
    HashMap<int, string> * mMap;

    int mNumberOfIterations;

    int mNewTableRowCount = 1000;

    long mSleepTimeInMilliseconds = 5;

    resize_map_struct(HashMap<int, string> *mapParam, int numberOfIterations) :
            mMap(mapParam), mNumberOfIterations(numberOfIterations) {
    }

    // do a complete resize of the map, then sleep for some time to simulate a "realistic" usage of the map
    void operator()() {
        for (int i = 0; i < mNumberOfIterations; i++) {
            this->mMap->resize(mNewTableRowCount);
            std::this_thread::sleep_for(std::chrono::milliseconds(mSleepTimeInMilliseconds));
        }
    }
};

// this test is intended to simulate a system with multiple threads working on a single map instance
// the threads perform operations on random keys
// the intention of this test is to prove that the map and the locking approach does not produce deadlocks when it´s heavily accessed
TEST(HashMapTest, MultithreadAccess) {
    const int iterations = 100000;
    HashMap<int, string> map;

    // create threads that randomly add entries to the map
    add_entries_struct addStruct(&map, iterations);
    std::thread tAdd(addStruct);
    std::thread tAdd2(addStruct);

    // do a resize while the other threads are working on the map
    resize_map_struct resizeStruct(&map, iterations / 100);
    std::thread tResize(resizeStruct);

    // create threads that randomly get entries to the map
    get_entries_struct getStruct(&map, iterations);
    std::thread tGet(addStruct);
    std::thread tGet2(addStruct);
    std::thread tGet3(addStruct);

    // create threads that randomly remove entries to the map
    remove_entries_struct removeStruct(&map, iterations);
    std::thread tRemove(removeStruct);
    std::thread tRemove2(removeStruct);

    tAdd.join();
    tAdd2.join();
    tResize.join();
    tGet.join();
    tGet2.join();
    tGet3.join();
    tRemove.join();
    tRemove2.join();
}

