/*
 * HashMapTest.cpp
 *
 *  Created on: 30.12.2015
 *      Author: Patrick
 */

#include <gtest/gtest.h>
#include <HashMap.hpp>

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
	// EXPECT_EQ(result, NULL);
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

	// add 100 entries
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
