/*
hashVal
	OPERATIONS:

	0 0 0 | _ _ _ _ _		- run
	0 0 1 | _ _ _ _ _ x2	- diff
	0 1 0 | _ _ _ _ _ 		- hash table ind
	0 1 1 | _ _ _ _ _ x4	- new
	1 0 0 | _ _ _ _ _		- last frame
	1 0 1 | _ _ _ _ _		- prev frames

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
const long long MAX_DIFF = 1586304000;

int hashTable[modHash];
bool hashViz[modHash];
int numHash = 0;

Pixel valHash[modHash];

int IMG_H;
int IMG_W;

std::ofstream fout("../build/encodeBytes.dat", std::ios::out | std::ios::binary);

bool firstTime = true;

Frame lastFrame;
Frame prevFrames[4];
bool haveFrame[4];
// 0 - I frame , 1 - 3 P frames


long long cnt5 = 0;
long long cnt6 = 0;
void encode( int numCurrFrame ){

	std::string pathPref = "../frames/";
	std::string pathImgName = getFramNum( numCurrFrame );
	std::string pathSuff = ".png";
	
	std::string path = pathPref + pathImgName + pathSuff;

	std::cerr << out( path ) << std::endl;
	
	Mat img = imread( path, IMREAD_COLOR);

    if(img.empty()) {
		std::cerr << " can't open the image " << std::endl;
		return ;
	}

#ifdef SMALL_TEST

	int smallH = 1000;
	int smallW = 1000;
	Frame smallFrame( smallH, smallW );
	for( int i=0 ; i < smallH ; i++ ){
		for( int j=0 ; j < smallW ; j++ ){
			int currR = i + ( rand() % ( i+1 ) );
			currR %= 256;
			int currG = i + ( rand() % ( i+1 ) );
			currG %= 256;
			int currB = i + ( rand() % ( i+1 ) );
			currB %= 256;
			Pixel currPixel( currR, currG, currB );
			smallFrame.setPixelPixel( currPixel, i, j );
		}
	}

	IMG_H = smallH;
	IMG_W = smallW;

#else

	IMG_H = img.rows;
	IMG_W = img.rows;

#endif

	if( firstTime ){

		fout.write( (char *)&IMG_H, sizeof( IMG_H ) );
		fout.write( (char *)&IMG_W, sizeof( IMG_W ) );
		std::cerr << out( IMG_H ) << out( IMG_W ) << std::endl;
	}

	Frame currFrame( IMG_H, IMG_W );
	Pixel prev( 12345, 12345, 12345 );
	int cntRun = 0;

	for( int i=0 ; i < modHash ; i++ ){
		hashViz[i] = false;
		valHash[i].reset();
	}
	
	for( int i=0 ; i < modHash ; i++ ){
		valHash[i].reset();
		hashViz[i] = false;
	}

	unsigned long long diff = 0;
	for(int i=0; i < IMG_H ; i++) {
		for(int j=0; j < IMG_W; j++) {

#ifdef SMALL_TEST
			Pixel currPixel = smallFrame.getPixel( i, j );
#else
			int b = img.at< cv::Vec3b>( i, j )[0];
  			int g = img.at< cv::Vec3b>( i, j )[1];
  			int r = img.at< cv::Vec3b>( i, j )[2];

  			Pixel currPixel( r, g, b );
#endif
			currPixel.print( std::cout );
			currFrame.setPixelPixel( currPixel, i, j );

			//run operation
			// 0 0 0 | _ _ _ _ _

			if( ( i || j ) and currPixel == prev ){
				cntRun ++;
#ifdef SMALL_TEST
				if( cntRun < ( 1 << 5 )-1 and !( i == smallH-1 and j == smallW-1 ) ) continue;
#else
				if( cntRun < ( 1 << 5 )-1 and !( i == IMG_H-1 and j == IMG_W-1 ) ) continue;
#endif
			}
			if( cntRun ){
				int type = 0;
				int info;
				info = ( type << 5 );
				info |= cntRun;
				cntRun = 0;
				fout.write( (char*) &info, sizeof( info ) );
				if( currPixel == prev ) continue;
			}


			// hash
			// 0 1 0 | _ _ _ _ _
			int currHash = ( currPixel.getR() * 3 + currPixel.getG() * 5 + currPixel.getB() * 7 ) % modHash;
			if( !hashViz[currHash] and valHash[currHash] == currPixel ){
				hashViz[currHash] = true;

				int currInd = hashTable[ currHash ];
				int type = 2;

				int info = 0;
				info  = ( type << 5 );
				info |= currInd;

				fout.write( (char *) & info, sizeof( info ) );

				prev = currPixel;
				continue;
			}

			bool oke = false;
			if( !firstTime ){

				// prev frame
				// _ _ _ | _  _ _ _ _
				//  type  r/u   dist 

				for( int r=0 ; !oke and r < (1 << 4) ; r++){
					if( i - r < 0 ) break;
					if( lastFrame.getPixel( i - r, j ) == currPixel.getPixel() ){

						int type = 4;

						int info = ( type << 5 );
						info |= ( 1 << 4 );
						info |= r;

						fout.write( (char*)&info, sizeof( info ) );
						oke = true;
						prev = currPixel;
					}
					if( oke ) break;
				}

				for( int c = 0 ; !oke and c < ( 1 << 4 ) ; c++ ){
					if( j - c < 0 ) break;
					if( lastFrame.getPixel( i, j - c ) == currPixel.getPixel() ){

						int type = 4;
						
						int info = ( type << 5 );
						info |= c;

						fout.write( (char*)&info, sizeof( info ) );
						oke = true;
						prev = currPixel;
					}
					if( oke ) break;
				}

			}

			if( oke ){
				continue;
			}

			for( int frameInd = 0 ; frameInd < 4 ; frameInd ++ ){
				// 1 0 1 | _ _   _ _ _
				//  type   num  dist left
				if( !haveFrame[frameInd] ) continue;
				
				for( int r=0 ; !oke and r < ( 1 << 3 ) ; r ++ ){
					if( i - ( r + 1 ) < 0 ) break;
					if( prevFrames[frameInd].getPixel( i-( r + 1 ), j ) == currPixel ){

						int type = 5;

						cnt5 ++;

						int info = ( type << 5 );
						info |= ( frameInd << 3 );
						info |= r;

						fout.write( (char*)&info, sizeof( info ) );
						oke = true;
					}
				}
				if( oke ) break;
			}

			if( oke ){
				prev = currPixel;
				continue;
			}

			for( int frameInd = 0 ; frameInd < 4 ; frameInd ++ ){
				// 1 1 0 | _ _   _ _ _
				//  type   num  dist left
				if( !haveFrame[frameInd] ) continue;
				
				for( int c=0 ; !oke and c < ( 1 << 3 ) ; c ++ ){
					if( j - ( c + 1 ) < 0 ) break;
					if( prevFrames[frameInd].getPixel( i, j - ( c + 1 ) ) == currPixel ){

						int type = 6;

						cnt6 ++;

						int info = ( type << 5 );
						info |= ( frameInd << 3 );
						info |= c;

						fout.write( (char*)&info, sizeof( info ) );
						oke = true;
					}
				}
				if( oke ) break;
			}

			if( oke ){
				prev = currPixel;
				continue;
			}

			int dr = prev.getR() - currPixel.getR();
			int dg = prev.getG() - currPixel.getG();
			int db = prev.getB() - currPixel.getB();
			if( dr >= -15 and dr <= 16 && dg >= -7 and dg <= 8 && db >= -7 and db <= 8 ){
				// diff
				// 0 0 1 | _ _ _ _ _  . _ _ _ _ | _ _ _ _
				int type = 1;
				int info1 = 0;
				info1 = ( type << 5 );
				dr += 15;
				info1 |= dr;

				int info2 = 0;
				dg += 7;
				info2 = ( dg << 4 );

				db += 7;
				info2 |= db;

				fout.write( (char * ) &info1, sizeof( info1 ) );
				fout.write( (char * ) &info2, sizeof( info2 ) );

				prev = currPixel;
				continue;
			}

			int type = 3;
			int infoType = ( type << 5 );
			int infoR = currPixel.getR();
			int infoG = currPixel.getG();
			int infoB = currPixel.getB();

			fout.write( (char *) &infoType, sizeof( infoType ) );
			fout.write( (char *) &infoR, sizeof( infoR ) );
			fout.write( (char *) &infoG, sizeof( infoG ) );
			fout.write( (char *) &infoB, sizeof( infoB ) );

  			if( valHash[currHash] == Pixel() and numHash < modHash ){
  				valHash[ currHash ] = currPixel;
  				hashTable[ currHash ] = numHash ++;
  			}


			prev = currPixel;
		}
	}


	if( !firstTime ){
		diff = currFrame.getDiff( lastFrame );
	}

	// max Diff = 255 * 3 * 1080 * 1920
	// max Diff = 1586304000 = 1e9
	bool setFrame = false;

	if( diff >= MAX_DIFF / 2 || !haveFrame[0] ){
		prevFrames[0].setFrame( currFrame );
		haveFrame[0] = true;
		setFrame = true;

		//std::cerr << " set 0 Frame " << std::endl;
		haveFrame[2] = haveFrame[3] = false;
	}else if( diff >= MAX_DIFF / 10 || !haveFrame[2] ){
		prevFrames[2].setFrame( currFrame );
		haveFrame[2] = true;
		setFrame = true;
		//std::cerr << " set 2 Frame " << std::endl;
	}else if( diff >= MAX_DIFF / 50 || !haveFrame[3] ){
		prevFrames[3].setFrame( currFrame );
		haveFrame[3] = true;
		setFrame = true;
		//std::cerr << " set 3 Frame " << std::endl;
	}

	if( !setFrame and numCurrFrame % 7 == 0 ){
		prevFrames[1].setFrame( currFrame );
		//std::cerr << " set 1 Frame " << std::endl;
	}

	lastFrame.setFrame( currFrame );
	firstTime = false;

}

int main(){

#ifdef TIME
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#endif

	std::srand( 69 );

	int numFrames = 50;
	fout.write( (char*)&numFrames, sizeof( numFrames ) );

	firstTime = true;
	for( int i=1 ; i <= numFrames ; i++ ){
		encode( i );
	}

	std::cerr << out( cnt5 ) << out( cnt6 ) << std::endl;

	fout.close();
	if(!fout.good()) {
		std::cerr << "Error occurred at writing time!" << std::endl;
		return 1;
	}

#ifdef TIME
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cerr << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
#endif

	return 0;
}
