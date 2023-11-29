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

const uint8_t modHash = 32;

uint8_t hashTable[modHash];
Pixel hashVal[modHash];

bool vizHash[modHash];

std::ifstream fin( "encodeBytes.dat", std::ios::out | std::ios::binary );

int IMG_H;
int IMG_W;

Frame prevFrame[4];
bool haveFrame[4];
const long long MAX_DIFF = 1586304000; 
Frame lastFrame;

bool firstTime = true;

std::vector< Mat > images;

void decode( int numCurrFrame ){

	Frame currFrame( IMG_H, IMG_W );

	std::cerr << out( numCurrFrame ) << std::endl;

	uint8_t runNum = 0;
	uint8_t numHash = 0;

	for( uint8_t i = 0 ; i < modHash ; i++ ){
		vizHash[i] = false;
		hashVal[i].reset();
	}

	Pixel prevPixel( 255, 255, 255 );
	long long currDiff = 0;
	for( int i=0 ; i < IMG_H; i++ ){
		for( int j=0 ; j < IMG_W; j++ ){

			Pixel newPixel;

			if( runNum ){
				prevPixel.print( std::cout );
				runNum --;
				currFrame.setPixelPixel( prevPixel, i, j );
				continue;
			}

			uint8_t info;
			fin.read( (char*) &info, sizeof( info ) );

			uint8_t cpInfo = info;
			uint8_t type = cpInfo >> 5;
			if( type == 0 ){
				runNum = info & ( ( 1 << 5 ) -1 ); // last 5 bits
				prevPixel.print( std::cout );
				runNum --;
				currFrame.setPixelPixel( prevPixel, i, j );
				continue;
			}
			if( type == 1 ){
				int8_t dr = info & ( ( 1 << 5 ) -1 );
				dr -= 15;
				newPixel.setR( prevPixel.getR() - dr );

				uint8_t info2;
				fin.read( (char*) &info2, sizeof( info2 ) );

				int8_t dg = ( info2 >> 4 );
				dg -= 7;
				newPixel.setG( prevPixel.getG() - dg );

				int8_t db = info2 & ( ( 1 << 4 ) -1 );
				db -= 7;
				newPixel.setB( prevPixel.getB() - db );
			}
				
			if( type == 2 ){
				uint8_t currHashInd =  info & ( ( 1 << 5 ) -1 );
				newPixel.setPixel( hashVal[currHashInd] );
			}

			if( type == 3 ){

				uint8_t _r, _g, _b;
				fin.read( (char*) &_r, sizeof( _r ) );
				newPixel.setR( _r );
				
				fin.read( (char*) &_g, sizeof( _g ) );
				newPixel.setG( _g );

				fin.read( (char*) &_b, sizeof( _b ) );
				newPixel.setB( _b );

				uint8_t currHash = ( newPixel.getR() * 3 + newPixel.getG() * 5 + newPixel.getB() * 7 ) % modHash;
				if( !vizHash[currHash] and numHash < modHash ){

					vizHash[currHash] = true;
					hashVal[numHash].setPixel( newPixel );
					hashTable[currHash] = numHash ++;
				}

			}

			if( type == 4 ){
				bool rows = info & ( 1 << 4 );

				uint8_t dist = info & ( ( 1 << 4 ) -1 );
				if( rows ){
					Pixel currPixel = lastFrame.getPixel( i-dist, j );
					newPixel.setPixel( currPixel );
				}else{
					Pixel currPixel = lastFrame.getPixel( i, j - dist );
					newPixel.setPixel( currPixel );
				}
			}
					
			if( type == 5 ){

				uint8_t buff = info;
				
				uint8_t prevFrameInd = ( buff & ( ( 1 << 5 ) -1 ) ) >> 3;
				uint8_t dist = info & ( ( 1 << 3 ) -1 );

				if( !haveFrame[ prevFrameInd ] ){
					std::cerr << " losha rabota bate " << std::endl;
				}

				newPixel.setPixel( prevFrame[prevFrameInd].getPixel( i - ( dist + 1 ), j ) );
			}

			if( type == 6 ){
				uint8_t buff = info;
				
				uint8_t prevFrameInd = ( buff & ( ( 1 << 5 ) -1 ) ) >> 3;
				uint8_t dist = info & ( ( 1 << 3 ) -1 );

				newPixel.setPixel( prevFrame[prevFrameInd].getPixel( i, j - ( dist + 1 ) ) );
			}

			newPixel.print( std::cout );

			currFrame.setPixelPixel( newPixel, i, j );
			prevPixel.setPixel( newPixel );

			//currFrame.setPixelPixel( newPixel.getPixel(), i, j );
		}
	}

	if( !firstTime ){
		currDiff = currFrame.getDiff( lastFrame );
	}

	bool setFrame = false;
	if( !haveFrame[0] || currDiff >= MAX_DIFF / 2 ){
		prevFrame[0].setFrame( currFrame );
		haveFrame[0] = true;
		haveFrame[2] = haveFrame[3] = false;
		setFrame = true;

	}else if( !haveFrame[2] || currDiff >= MAX_DIFF / 10 ){
		prevFrame[2].setFrame( currFrame );
		haveFrame[2] = true;
		setFrame = true;

	}else if( !haveFrame[3] || currDiff >= MAX_DIFF / 50 ){
		prevFrame[3].setFrame( currFrame );
		haveFrame[3] = true;
		setFrame = true;

	}

	if( numCurrFrame % 7 == 0 and !setFrame ){
		prevFrame[1].setFrame( currFrame );
	}

	lastFrame.setFrame( currFrame );
	firstTime = false;
}

int main(){

#ifdef TIME
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#endif

	int numFrames;
	fin.read( (char*)&numFrames, sizeof( numFrames ) );

	std::cerr << out( numFrames ) << std::endl;

	fin.read((char *)&IMG_H, sizeof( IMG_H ) );
	fin.read((char *)&IMG_W, sizeof( IMG_W ) );

	std::cerr << out( IMG_H ) << out( IMG_W ) << std::endl;

	for( int i=1 ; i <= numFrames ; i++ ){
		decode( i );
	}


#ifdef TIME
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cerr << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
#endif

    return 0;
}
