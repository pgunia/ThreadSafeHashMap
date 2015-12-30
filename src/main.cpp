//
//  main.cpp
//  CeleraOneRecursive
//
//  Created by Patrick Gunia on 29.12.15.
//  Copyright © 2015 Patrick Gunia. All rights reserved.
//

#include <iostream>
#include <string>
#include <Windows.h>
#include "../include/HashMap.hpp"
#include "Constants.hpp"

using namespace std;

void platformOutl( string s ) {
	stringstream ss;
	ss << s << endl;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
	OutputDebugString( ss.str().c_str() );
#else
	cout( ss.str() );
#endif

}

int main( int argc, const char * argv[] ) {

	HashMap<int, string> hmap;
	hmap.put( 1, "val1" );
	hmap.put( 2, "val2" );
	hmap.put( 3, "val3" );

	string value;
	hmap.get( 2, value );
	platformOutl( value );
	bool res = hmap.get( 3, value );
	if ( res )
		platformOutl( value );
	hmap.remove( 3 );
	res = hmap.get( 3, value );

	for ( int i = 0; i < 120; i++ ) {
		hmap.put( i, "asd" );
	}

	if ( res )
		platformOutl( value );;
}

