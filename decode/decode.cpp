#include "../QOV.h"
// frame and pixel classes + ...

#include <opencv2/core.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>
#include <chrono>

using namespace cv;

#define out(x) #x << " = " << x << "  "

const int modHash = 32;

int hashTable[modHash];
int valHashR[modHash];
int valHashG[modHash];
int valHashB[modHash];

int main(){

#ifdef TIME
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#endif

	std::ifstream fin( "encodeBytes.dat", std::ios::out | std::ios::binary );
	if( !fin ){
		std::cerr << " nqma otvarqne " << std::endl;
		return 1;
	}

	int currH, currW;
	fin.read((char *)&currH, sizeof( currH ) );
	fin.read((char *)&currW, sizeof( currW ) );

	std::cerr << " ! " << currH << " " << currW << std::endl;

	//std::cout << "P3" << std::endl;
	//std::cout << currW << " " << currH << " 255 " << std::endl;

	int prevR, prevB, prevG;
	int runNum = 0;
	int numHash = 1;
	for( int i=0 ; i < currH ; i++ ){
		for( int j=0 ; j < currW ; j++ ){

			std::cerr << out( i ) << out( j ) << std::endl << std::endl;

			if( runNum ){
				std::cout << prevR << " " << prevG << " " << prevB << std::endl;
				std::cerr << "oste run " << out( runNum ) << std::endl;
				runNum --;
				continue;
			}

			int currR, currB, currG;

			int info;
			fin.read( (char*) &info, sizeof( info ) );
			std::cerr << out( info ) << inBinary( info ) << std::endl;

			int cpInfo = info;
			int type = cpInfo >> 5;
			std::cerr << out( type ) << std::endl;
			//std::cerr << out( type ) << std::endl;
			if( type == 5 ){
				runNum = info & ( ( 1 << 5 ) -1 ); // last 5 bits
				std::cerr << out( runNum ) << inBinary( ( 1 << 6 ) -1 ) << std::endl;
				std::cout << prevR << " " << prevG << " " << prevB << std::endl;
				runNum --;
				continue;
			}
			if( type == 1 ){
				int dr = info & ( ( 1 << 5 ) -1 );
				dr -= 15;
				currR = prevR - dr;

				int info2;
				//uint8_t info2;
				fin.read( (char*) &info2, sizeof( info2 ) );

				int dg = ( info2 >> 4 );
				dg -= 7;
				currG = prevG - dg;

				int db = info2 & ( ( 1 << 4 ) -1 );
				db -= 7;
				currB = prevB - db;

				std::cerr << out( dr ) << out( dg ) << out( db ) << std::endl;
			}
				
			if( type == 2 ){
				int currHashInd;
				currHashInd =  info | ( ( 1 << 6 ) -1 );
				currR = valHashR[ currHashInd ];
				currG = valHashG[ currHashInd ];
				currB = valHashB[ currHashInd ];
			}

			if( type == 3 ){

				std::cerr << " new " << std::endl;

				int _r, _g, _b;
				//uint8_t _r, _g, _b;
				fin.read( (char*) &_r, sizeof( _r ) );
				currR = _r;
				fin.read( (char*) &_g, sizeof( _g ) );
				currG = _g;
				fin.read( (char*) &_b, sizeof( _b ) );
				currB = _b;

				std::cerr << out( currR ) << out( currG ) << out( currB ) << std::endl;

				int currHash = ( currR * 3 + currG * 5 + currB * 7 ) % modHash;
				if( hashTable[currHash] == 0 ){
					hashTable[currHash] = numHash ++;
					valHashR[currHash] = currR;
					valHashG[currHash] = currG;
					valHashB[currHash] = currB;
				}

			}

			std::cout << currR << " " << currG << " " << currB << std::endl;

			prevR = currR;
			prevG = currG;
			prevB = currB;
			//std::cout << currR << " " << currG << " " << currB << std::endl; 
		}
	}

	fin.close();
	if( !fin.good() ){
		std::cerr << " bruhmomento " << std::endl;
		return 1;
	}


#ifdef TIME
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cerr << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
#endif

    return 0;
}
