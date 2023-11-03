/*

	OPERATIONS:

	0 0 0 | _ _ _ _ _		- run
	0 0 1 | _ _ _ _ _ x2	- diff
	0 1 0 | _ _ _ _ _ 		- hash table ind
	0 1 1 | _ _ _ _ _ x2	- new

*/
#include "../QOV.h"
// frame and pixel class + some functions

#include <opencv2/core.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>
#include <chrono>

#define out(x) #x << " = " << x << "  "

using namespace cv;

const int MAX_FRAME_H = 1080;
const int MAX_FRAME_W = 1920;

const int modHash = 32;

int hashTable[modHash];
int numHash = 1;

Pixel valHash[modHash];

int main(){

#ifdef TIME
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#endif

	std::ofstream fout("../build/encodeBytes.dat", std::ios::out | std::ios::binary);

	if(!fout) {
		std::cout << "Ne moga da otvorq OUTPUT faila" << std::endl;
		return 1;
	}


	int numFrame = 42;
	std::string pathPref = "../frames/";
	std::string pathImgName = getFramNum( numFrame );
	std::string pathSuff = ".png";
	
	std::string path = pathPref + pathImgName + pathSuff;
	std::cerr << out( path ) << std::endl;
	
	Mat img = imread( path, IMREAD_COLOR);

    if(img.empty())
	{
		return 1;
	}

#ifdef SMALL_TEST

	int smallH = 5;
	int smallW = 4;
	Frame smallFrame( smallH, smallW );
	for( int i=0 ; i < smallH ; i++ ){
		for( int j=0 ; j < smallW ; j++ ){
			Pixel curr( i, i, i );
			smallFrame.setPixelPixel( curr, i, j );
		}
	}

#endif

	const int IMG_H = img.rows;
	const int IMG_W = img.cols;
	fout.write( (char *)&smallH, sizeof( smallH ) );
	fout.write( (char *)&smallW, sizeof( smallW ) );
	//fout.write( (char *)&IMG_H, sizeof( IMG_H ) );
	//fout.write( (char *)&IMG_W, sizeof( IMG_W ) );

	Frame currFrame( IMG_H, IMG_W );
	Pixel prev( 12345, 12345, 12345 );
	int cntRun = 0;
//	for(int i=0; i < IMG_H ; i++) {
//		for(int j=0; j < IMG_W; j++) {
	std::cerr << out( smallH ) << out( smallW ) << std::endl;
	for(int i=0; i < smallH ; i++) {
		for(int j=0; j < smallW; j++) {

//			int b = img.at< cv::Vec3b>( i, j )[0];
//			int g = img.at< cv::Vec3b>( i, j )[1];
//			int r = img.at< cv::Vec3b>( i, j )[2];

//			Pixel curr( r, g, b );
			Pixel curr = smallFrame.getPixel( i, j );
			std::cerr << curr.getR() << " " << curr.getG() << " " << curr.getB() << std::endl;

			currFrame.setPixelPixel( curr, i, j );


			//run operation
			// 0 0 0 | _ _ _ _ _
			if( ( i || j ) and curr == prev ){
				cntRun ++;
				if( cntRun < ( 1 << 5 )-1 and !( i == smallH-1 and j == smallW-1 ) ) continue;
			}
			if( cntRun ){
				int type = 5;
				std::cout << type << " run " << std::endl;
				int info;
				//uint8_t info;
				info = ( type << 5 );
				info |= cntRun;
				std::cerr << out( cntRun ) << std::endl;
				std::cout << out( info ) << inBinary( info ) << std::endl;
				cntRun = 0;
				fout.write( (char*) &info, sizeof( info ) );
				if( curr == prev ) continue;
			}

			int dr = prev.getR() - curr.getR();
			int dg = prev.getG() - curr.getG();
			int db = prev.getB() - curr.getB();
			if( dr >= -15 and dr <= 16 && dg >= -7 and dg <= 8 && db >= -7 and db <= 8 ){
				// diff
				// 0 0 1 | _ _ _ _ _  . _ _ _ _ | _ _ _ _
				int type = 1;
				int info1 = 0;
				//uint8_t info1 = 0;
				info1 = ( type << 5 );
				dr += 15;
				info1 |= dr;

				int info2 = 0;
				//uint8_t info2 = 0;
				dg += 7;
				info2 = ( dg << 4 );

				db += 7;
				info2 |= db;

				fout.write( (char * ) &info1, sizeof( info1 ) );
				fout.write( (char * ) &info2, sizeof( info2 ) );
				std::cout << out( dr ) << out( dg ) << out( db ) << std::endl;

				std::cout << 1 << " diff " << std::endl;
				std::cout << inBinary( info1 ) << " " << inBinary( info2 ) << std::endl;
				prev = curr;
				continue;
			}

			/*

			// hash
			// 0 1 0 | _ _ _ _ _
			int currHash = ( curr.getG() * 3 + curr.getG() * 5 + curr.getR() * 7 ) & modHash;
			if( hashTable[currHash] != 0 and valHash[currHash] == curr ){
				int currInd = hashTable[currHash];
				int type = 2;

				int info = 0;
				//uint8_t info = 0;
				info  = ( type << 5 );
				info |= currInd;

				fout.write( (char * ) & info, sizeof( info ) );
				prev = curr;

				std::cout << 2 << std::endl;
				continue;
			}
			*/


			int type = 3;
			//uint8_t infoType = ( type << 5 );
			//uint8_t infoR = curr.getR();
			//uint8_t infoG = curr.getG();
			//uint8_t infoB = curr.getB();
			int infoType = ( type << 5 );
			int infoR = curr.getR();
			int infoG = curr.getG();
			int infoB = curr.getB();

			std::cout << type << " new " << std::endl;
			std::cout << inBinary( infoType ) << std::endl;
			std::cout << out( infoR ) << out( infoG ) << out( infoB ) << std::endl;
			std::cout << inBinary( infoR ) << " " << inBinary( infoG ) << " " << inBinary( infoB ) << std::endl;
			
			fout.write( (char *) &infoType, sizeof( infoType ) );
			fout.write( (char *) &infoR, sizeof( infoR ) );
			fout.write( (char *) &infoG, sizeof( infoG ) );
			fout.write( (char *) &infoB, sizeof( infoB ) );

//			if( numHash < modHash ){
//				hashTable[ currHash ] = numHash ++;
//				valHash[ currHash ] = curr;
//			}

			prev = curr;
		}
	}

	fout.close();

	if(!fout.good()) {
		std::cout << "Error occurred at writing time!" << std::endl;
		return 1;
	}

#ifdef TIME
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cerr << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
#endif

	return 0;
}
