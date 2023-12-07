/*
 * if( oke ){
 * prev = currPixel;
 * continue;
 * }
OPERATIONS:

0 0 0 | _ _ _ _ _    - run
0 0 1 | _ _ _ _ _ x2  - diff 1
0 1 0 | _ _ _ _ _    - diff 2
0 1 1 | _ _ _ _ _     - hash table ind
1 0 1 | _ _ _ _ _    - new
0 1 1 | _ _ _ _ _ x4  - new
1 0 0 | _ _ _ _ _    - last frame 1
1 0 1 | _ _ _ _ _	 - last frame 2
1 0 1 | _ _ _ _ _    - prev frames 1
1 1 1 | _ _ _ _ _    - prev frames 2

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
const uint8_t modHash = 32;
const long long MAX_DIFF = 1586304000;
const uint8_t NUM_PREV_FRAMES = 2;

uint8_t hashTable[modHash];
bool hashViz[modHash];
uint8_t numHash = 0;
std::pair< int8_t, int8_t> lastFrameDelta[64] =  {  { -5 , 0 }  ,  { -4 , -1 }  ,  { -4 , 0 }  ,  { -4 , 1 }  ,  { -3 , -3 }  ,  { -3 , -2 }  ,  { -3 , -1 }  ,  { -3 , 0 }  ,  { -3 , 1 }  ,  { -3 , 2 }  ,  { -3 , 3 }  ,  { -2 , -3 }  ,  { -2 , -2 }  ,  { -2 , -1 }  ,  { -2 , 0 }  ,  { -2 , 1 }  ,  { -2 , 2 }  ,  { -2 , 3 }  ,  { -1 , -4 }  ,  { -1 , -3 }  ,  { -1 , -2 }  ,  { -1 , -1 }  ,  { -1 , 0 }  ,  { -1 , 1 }  ,  { -1 , 2 }  ,  { -1 , 3 }  ,  { -1 , 4 }  ,  { 0 , -5 }  ,  { 0 , -4 }  ,  { 0 , -3 }  ,  { 0 , -2 }  ,  { 0 , -1 }  ,  { 0 , 1 }  ,  { 0 , 2 }  ,  { 0 , 3 }  ,  { 0 , 4 }  ,  { 0 , 5 }  ,  { 1 , -4 }  ,  { 1 , -3 }  ,  { 1 , -2 }  ,  { 1 , -1 }  ,  { 1 , 0 }  ,  { 1 , 1 }  ,  { 1 , 2 }  ,  { 1 , 3 }  ,  { 1 , 4 }  ,  { 2 , -3 }  ,  { 2 , -2 }  ,  { 2 , -1 }  ,  { 2 , 0 }  ,  { 2 , 1 }  ,  { 2 , 2 }  ,  { 2 , 3 }  ,  { 3 , -3 }  ,  { 3 , -2 }  ,  { 3 , -1 }  ,  { 3 , 0 }  ,  { 3 , 1 }  ,  { 3 , 2 }  ,  { 3 , 3 }  ,  { 4 , -1 }  ,  { 4 , 0 }  ,  { 4 , 1 }  ,  { 5 , 0 }  };

std::pair< int8_t, int8_t > prevFrameDelta[32] =  {  { -3 , -1 }  ,  { -3 , 1 }  ,  { -2 , -2 }  ,  { -2 , -1 }  ,  { -2 , 0 }  ,  { -2 , 1 }  ,  { -2 , 2 }  ,  { -1 , -3 }  ,  { -1 , -2 }  ,  { -1 , -1 }  ,  { -1 , 0 }  ,  { -1 , 1 }  ,  { -1 , 2 }  ,  { -1 , 3 }  ,  { 0 , -2 }  ,  { 0 , -1 }  ,  { 0 , 1 }  ,  { 0 , 2 }  ,  { 1 , -3 }  ,  { 1 , -2 }  ,  { 1 , -1 }  ,  { 1 , 0 }  ,  { 1 , 1 }  ,  { 1 , 2 }  ,  { 1 , 3 }  ,  { 2 , -2 }  ,  { 2 , -1 }  ,  { 2 , 0 }  ,  { 2 , 1 }  ,  { 2 , 2 }  ,  { 3 , -1 }  ,  { 3 , 1 }  };

Pixel valHash[modHash];

int IMG_H;
int IMG_W;

std::ofstream fout("../build/encodeBytes.dat", std::ios::out | std::ios::binary);

bool firstTime = true;

Frame lastFrame;
Frame prevFrames[4];
bool haveFrame[2];

bool STOP = false;
void encode( int numCurrFrame ){

	std::string pathPref = "../frames/kubche/";
	std::string pathImgName = getFramNum( numCurrFrame, 4, 0 );
	//std::string pathPref = "../frames/animaciq1/";
	//std::string pathImgName = getFramNum( numCurrFrame, 6, 99 );
	std::string pathSuff = ".bmp";

	std::string path = pathPref + pathImgName + pathSuff;

	std::cerr << out( path ) << std::endl;

	Mat img = imread( path, IMREAD_COLOR);

	if(img.empty()) {
		STOP = true;
		std::cerr << " can't open the image " << std::endl;
		return ;
	}

	if( firstTime ){

		IMG_H = img.rows;
		IMG_W = img.cols;

		fout.write( (char *)&IMG_H, sizeof( IMG_H ) );
		fout.write( (char *)&IMG_W, sizeof( IMG_W ) );
		std::cerr << out( IMG_H ) << out( IMG_W ) << std::endl;
		std::cerr << out( IMG_H ) << out( IMG_W ) << std::endl;
	}

	Frame currFrame( IMG_H, IMG_W );
	Pixel prev( 255, 255, 255 );
	uint8_t cntRun = 0;
	uint8_t cntLastRun = 0;
	for( uint8_t i=0 ; i < modHash ; i++ ){
		hashViz[i] = false;
		valHash[i].reset();
	}

	for( uint8_t i=0 ; i < modHash ; i++ ){
		valHash[i].reset();
		hashViz[i] = false;
	}

	unsigned long long diff = 0;
	for(int i=0; i < IMG_H ; i++) {
		for(int j=0; j < IMG_W; j++) {

			uint8_t b = img.at< cv::Vec3b>( i, j )[0];
			uint8_t g = img.at< cv::Vec3b>( i, j )[1];
			uint8_t r = img.at< cv::Vec3b>( i, j )[2];

			Pixel currPixel( r, g, b );

			//currPixel.print( std::cout );
			currFrame.setPixelPixel( currPixel, i, j );



			//run operation
			// 0 0 0 | _ _ _ _ _

			if( cntLastRun == 0 and ( i || j ) and currPixel == prev ){
				cntRun ++;
				if( cntRun < ( 1 << 5 )-1 and !( i == IMG_H-1 and j == IMG_W-1 ) ) continue;
			}
			if( cntRun ){
				uint8_t type = 0;
				uint8_t info;
				info = ( type << 5 );
				info |= cntRun;
				cntRun = 0;
				fout.write( (char*) &info, sizeof( info ) );
				if( currPixel == prev ) continue;
			}

			// same sa last frame
			// 0 1 1
			if( !firstTime and cntRun == 0 ){
				if( lastFrame.getPixel( i, j ) == currPixel ){
					cntLastRun++;
					if( cntLastRun < ( 1 << 5 ) -1 and !( i == IMG_H-1 and j == IMG_W-1 ) ){
						prev = currPixel;
						continue;
					}
				}
			}

			if( cntLastRun ){
				uint8_t type = 3;
				uint8_t info = ( type << 5 );
				info |= cntLastRun;
				cntLastRun = 0;
				fout.write( (char*)&info, sizeof( info ) );
				if( lastFrame.getPixel( i, j ) == currPixel ){
					prev = currPixel;
					continue;
				}
			}

			//run operation
			// 0 0 0 | _ _ _ _ _

			if( cntLastRun == 0 and ( i || j ) and currPixel == prev ){
				cntRun ++;
				if( cntRun < ( 1 << 5 )-1 and !( i == IMG_H-1 and j == IMG_W-1 ) ) continue;
			}

			if( cntRun ){
				uint8_t type = 0;
				uint8_t info;
				info = ( type << 5 );
				info |= cntRun;
				cntRun = 0;
				fout.write( (char*) &info, sizeof( info ) );
				if( currPixel == prev ) continue;
			}
			
			bool oke = false;
  			if( !firstTime ){
				// LAST FRAME OPERATOINS
    			for( size_t currInd=0 ; !oke and currInd < 64 ; currInd ++ ){
    
    				int currX = i + lastFrameDelta[currInd].first;
    				int currY = j + lastFrameDelta[currInd].second;
    
    				if( currX < 0 || currX >= IMG_H ) continue;
    				if( currY < 0 || currY >= IMG_W ) continue;
    
    				if( lastFrame.getPixel( currX, currY ) == currPixel ){
    
    					uint8_t type;
    					uint8_t infoInd;
    					if( currInd < 32 ){
							type = 4;
							infoInd = currInd;
						}else{
    						type = 5;
    						infoInd = currInd - 32;
    					}
    
						//std::cout << i << " " << j << " " << currX << " " << currY << std::endl;
    
    					uint8_t info = ( type << 5 );
    					info |= infoInd;
						
    					fout.write( (char *)&info, sizeof( info ) );
    					oke = true;
    					break;
    				}
    			}
  			}

			if( oke ){
				prev = currPixel;
				continue;
			}

			// TODO tva ne raboti mn
//			for( uint8_t frameInd = 0 ; false and !oke and frameInd < NUM_PREV_FRAMES ; frameInd ++ ){
//				// PREV FRAME OPERATIONS
//				if( !haveFrame[frameInd] ) continue;
//				for( uint8_t currInd = 0 ; !oke and currInd < 31 ; currInd ++ ){
// 
//					int currX = i + prevFrameDelta[currInd].first;
//					int currY = j + prevFrameDelta[currInd].second;
//
//					if( currX < 0 || currX >= IMG_H ) continue;
//					if( currY < 0 || currY >= IMG_W ) continue;
//
//					if( prevFrames[frameInd].getPixel( currX, currY ) == currPixel ){
//
//    					uint8_t type;
//    					uint8_t infoInd;
//    					if( currInd < 16 ){
//							type = 6;
//							infoInd = currInd;
//						}else{
//    						type = 7;
//    						infoInd = currInd - 16;
//    					}
//    
//    					uint8_t info = ( type << 5 );
//						info |= ( frameInd << 4 );
//    					info |= infoInd;
//    					
//    					fout.write( (char *)&info, sizeof( info ) );
//    					oke = true;
//    					break;
//					}
//				}
//			}
//
//			if( oke ){
//				prev = currPixel;
//				continue;
//			}

			int8_t dr = prev.getR() - currPixel.getR();
			int8_t dg = prev.getG() - currPixel.getG();
			int8_t db = prev.getB() - currPixel.getB();

			int8_t dgr = dg - dr;
			int8_t dgb = dg - db;

  			if( dg >= -1 and dg <= 2 && dgr >= -1 and dgr <= 2 && dgb >= 0 and dgb <= 1 ){
  				uint8_t type = 2;
  				uint8_t info = 0;
  				info = ( type << 5 );
  
  				dg += 1;
  				info |= ( dg << 3 );
  
  				dgr += 1;
  				info |= ( dgr << 1 );
  
  				info |= dgb;
  
  				fout.write( (char *)&info, sizeof( info ) );
  
  				prev = currPixel;
  				continue;
  			}

			if( !firstTime ){
				Pixel currP = lastFrame.getPixel( i, j );
				dr = currP.getR() - currPixel.getR();
				dg = currP.getG() - currPixel.getG();
				db = currP.getB() - currPixel.getB();

				dgr = dg - dr;
				dgb = dg - db;
			}

			if( !firstTime and dg >= -1 and dg <= 2 && dgr >= -1 and dgr <= 2 && dgb >= 0 and dgb <= 1 ){
				uint8_t type = 6;
				uint8_t info = 0;
				info = ( type << 5 );

				dg += 1;
				info |= ( dg << 3 );

				dgr += 1;
				info |= ( dgr << 1 );

				info |= dgb;

				fout.write( (char *)&info, sizeof( info ) );

				prev = currPixel;
				continue;
			}
  
    
  			dr = prev.getR() - currPixel.getR();
  			dg = prev.getG() - currPixel.getG();
  			db = prev.getB() - currPixel.getB();
  
  			dgr = dg - dr;
  			dgb = dg - db;
  
			if( dg >= -15 and dg <= 16 && dgr >= -7 and dgr <= 8 && dgb >= -7 and dgb <= 8 ){
				// diff
				// 0 0 1 | _ _ _ _ _  . _ _ _ _ | _ _ _ _
				uint8_t type = 1;
				uint8_t info1 = 0;
				info1 = ( type << 5 );
				dg += 15;
				info1 |= dg;

				uint8_t info2 = 0;
				dgr += 7;
				info2 = ( dgr << 4 );

				dgb += 7;
				info2 |= dgb;

				fout.write( (char * ) &info1, sizeof( info1 ) );
				fout.write( (char * ) &info2, sizeof( info2 ) );

				prev = currPixel;
				continue;
			}

			if( !firstTime ){
				Pixel currP = lastFrame.getPixel( i, j );
				dr = currP.getR() - currPixel.getR();
				dg = currP.getG() - currPixel.getG();
				db = currP.getB() - currPixel.getB();

				dgr = dg - dr;
				dgb = dg - db;
			}


			if( !firstTime and dg >= -15 and dg <= 15 && dgr >= -7 and dgr <= 8 && dgb >= -7 and dgb <= 8 ){
				// diff
				// 0 1 0 | _ _ _ _ _  . _ _ _ _ | _ _ _ _
				uint8_t type = 7;
				uint8_t info1 = 0;
				info1 = ( type << 5 );
				dg += 15;
				info1 |= dg;

				uint8_t info2 = 0;
				dgr += 7;
				info2 = ( dgr << 4 );

				dgb += 7;
				info2 |= dgb;

				fout.write( (char * ) &info1, sizeof( info1 ) );
				fout.write( (char * ) &info2, sizeof( info2 ) );

				prev = currPixel;
				continue;
			}
    

			uint8_t infoType = 255;
			uint8_t infoR = currPixel.getR();
			uint8_t infoG = currPixel.getG();
			uint8_t infoB = currPixel.getB();

			fout.write( (char *) &infoType, sizeof( infoType ) );
			fout.write( (char *) &infoR, sizeof( infoR ) );
			fout.write( (char *) &infoG, sizeof( infoG ) );
			fout.write( (char *) &infoB, sizeof( infoB ) );

//			if( valHash[currHash].isZero() and numHash < modHash ){
//				valHash[ currHash ] = currPixel;
//				hashTable[ currHash ] = numHash ++;
//			}


			prev = currPixel;
		}
	}


	if( !firstTime ){
		diff = currFrame.getDiff( lastFrame );
	}

	// max Diff = 255 * 3 * 1080 * 1920
	// max Diff = 1586304000 = 1e9
	
	if( diff >= MAX_DIFF / 2 || !haveFrame[0] ){
		prevFrames[0].setFrame( currFrame );
		haveFrame[0] = true;

		haveFrame[1] = false;
	}else if( diff >= MAX_DIFF / 10 || !haveFrame[1] ){
		prevFrames[1].setFrame( currFrame );
		haveFrame[1] = true;
	}

	lastFrame.setFrame( currFrame );
	firstTime = false;

}

int main(){

#ifdef TIME
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#endif

	std::srand( 69 );

	int numFrames = 60;
	fout.write( (char*)&numFrames, sizeof( numFrames ) );

	firstTime = true;
	for( int i=1 ; i <= numFrames and !STOP ; i++ ){
		encode( i );
	}

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
