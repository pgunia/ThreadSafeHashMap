/*
 * HashMapTest.cpp
 *
 *  Created on: 30.12.2015
 *      Author: Patrick
 */

#include "HashMap.hpp"
#include "gtest/gtest.h"
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

