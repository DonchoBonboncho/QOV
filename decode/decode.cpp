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
Pixel hashVal[modHash];

bool vizHash[modHash];

std::ifstream fin( "encodeBytes.dat", std::ios::out | std::ios::binary );

//Frame prevFrame;
int currH, currW;
void decode( int numCurrFrame ){

	std::cerr << out( numCurrFrame ) << std::endl;


	int runNum = 0;
	int numHash = 0;
	Pixel prevPixel( 1234, 1234, 1234 );
  	for( int i = 0 ; i < modHash ; i++ ){
  		hashVal[i].reset();
  		vizHash[i] = false;
  	}

	//Frame currFrame( currH, currW );

	for( int i=0 ; i < currH ; i++ ){
		for( int j=0 ; j < currW ; j++ ){

			Pixel newPixel;

			if( runNum ){
				prevPixel.print( std::cout );

				runNum --;
				continue;
			}

			int info;
			fin.read( (char*) &info, sizeof( info ) );

			int cpInfo = info;
			int type = cpInfo >> 5;
			if( type == 0 ){
				runNum = info & ( ( 1 << 5 ) -1 ); // last 5 bits
				prevPixel.print( std::cout );
				runNum --;
				continue;
			}
			if( type == 1 ){
				int dr = info & ( ( 1 << 5 ) -1 );
				dr -= 15;
				newPixel.setR( prevPixel.getR() - dr );

				int info2;
				fin.read( (char*) &info2, sizeof( info2 ) );

				int dg = ( info2 >> 4 );
				dg -= 7;
				newPixel.setG( prevPixel.getG() - dg );

				int db = info2 & ( ( 1 << 4 ) -1 );
				db -= 7;
				newPixel.setB( prevPixel.getB() - db );
			}
				
			if( type == 2 ){
				int currHashInd =  info & ( ( 1 << 5 ) -1 );
				newPixel.setPixel( hashVal[currHashInd] );
			}


			if( type == 7 ){
				std::cerr << out( type ) << std::endl;
				bool rows = ( 1 >> 4 ) & 1;

				int dist = info & ( 1 << 4 ) -1;
				if( rows ){
					//newPixel.setPixel( prevFrame.getPixel( i-dist, j ) );
				}else{
					//newPixel.setPixel( prevFrame.getPixel( i, j - dist ) );
				}
			}
					


			if( type == 3 ){

				int _r, _g, _b;
				fin.read( (char*) &_r, sizeof( _r ) );
				newPixel.setR( _r );
				
				fin.read( (char*) &_g, sizeof( _g ) );
				newPixel.setG( _g );

				fin.read( (char*) &_b, sizeof( _b ) );
				newPixel.setB( _b );

				int currHash = ( newPixel.getR() * 3 + newPixel.getG() * 5 + newPixel.getB() * 7 ) % modHash;
				if( !vizHash[currHash] and numHash < modHash ){

					vizHash[currHash] = true;
					hashVal[numHash].setPixel( newPixel );
					hashTable[currHash] = numHash ++;
				}

			}

			newPixel.print( std::cout );

			prevPixel.setPixel( newPixel );

			//currFrame.setPixelPixel( newPixel.getPixel(), i, j );
		}
	}

	//prevFrame.setFrame( currFrame );

}
int main(){

#ifdef TIME
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#endif

	int numFrames;
	fin.read( (char*)&numFrames, sizeof( numFrames ) );

	std::cerr << out( numFrames ) << std::endl;

	fin.read((char *)&currH, sizeof( currH ) );
	fin.read((char *)&currW, sizeof( currW ) );

	std::cerr << out( currH ) << out( currW ) << std::endl;

	for( int i=1 ; i <= numFrames ; i++ ){
		decode( i );
	}


#ifdef TIME
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cerr << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
#endif

    return 0;
}
