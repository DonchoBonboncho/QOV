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

int IMG_H;
int IMG_W;

Frame prevFrame[4];
bool haveFrame[4];
const long long MAX_DIFF = 1586304000; 
Frame lastFrame;

bool firstTime = true;
void decode( int numCurrFrame ){

	Frame currFrame( IMG_H, IMG_W );

	std::cerr << out( numCurrFrame ) << std::endl;

	int runNum = 0;
	int numHash = 0;

	for( int i = 0 ; i < modHash ; i++ ){
		vizHash[i] = false;
		hashVal[i].reset();
	}

	Pixel prevPixel( 1234, 1234, 1234 );
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

			int info;
			fin.read( (char*) &info, sizeof( info ) );

			int cpInfo = info;
			int type = cpInfo >> 5;
			if( type == 0 ){
				runNum = info & ( ( 1 << 5 ) -1 ); // last 5 bits
				prevPixel.print( std::cout );
				runNum --;
				currFrame.setPixelPixel( prevPixel, i, j );
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

			if( type == 4 ){
				bool rows = info & ( 1 << 4 );

				int dist = info & ( ( 1 << 4 ) -1 );
				if( rows ){
					Pixel currPixel = lastFrame.getPixel( i-dist, j );
					newPixel.setPixel( currPixel );
				}else{
					Pixel currPixel = lastFrame.getPixel( i, j - dist );
					newPixel.setPixel( currPixel );
				}
			}
					
			if( type == 5 ){

				int buff = info;
				
				int prevFrameInd = ( buff & ( ( 1 << 5 ) -1 ) ) >> 3;
				int dist = info & ( ( 1 << 3 ) -1 );
				//std::cerr << out( prevFrameInd ) << out( dist ) << std::endl;

				//std::cout << type << " " << i << " " << j << " " << prevFrameInd << " " << dist << std::endl;

				if( !haveFrame[ prevFrameInd ] ){
					std::cerr << " losha rabota bate " << std::endl;
				}

				newPixel.setPixel( prevFrame[prevFrameInd].getPixel( i - ( dist + 1 ), j ) );
			}

			if( type == 6 ){
				int buff = info;
				
				int prevFrameInd = ( buff & ( ( 1 << 5 ) -1 ) ) >> 3;
				int dist = info & ( ( 1 << 3 ) -1 );

				//std::cerr << out( prevFrameInd ) << out( dist ) << std::endl;

				//std::cout << type << " " << i << " " << j << " " << prevFrameInd << " " << dist << std::endl;

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

		//std::cout << 0 << " " << numCurrFrame << std::endl;
	}else if( !haveFrame[2] || currDiff >= MAX_DIFF / 10 ){
		prevFrame[2].setFrame( currFrame );
		haveFrame[2] = true;
		setFrame = true;

		//std::cout << 2 << " " << numCurrFrame << std::endl;
	}else if( !haveFrame[3] || currDiff >= MAX_DIFF / 50 ){
		prevFrame[3].setFrame( currFrame );
		haveFrame[3] = true;
		setFrame = true;

		//std::cout << 3 << " " << numCurrFrame << std::endl;
	}

	if( numCurrFrame % 7 == 0 and !setFrame ){
		prevFrame[1].setFrame( currFrame );
		//std::cout << 1 << " " << numCurrFrame << std::endl;
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
