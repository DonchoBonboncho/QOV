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

unsigned long long cnt[10]; // operation count

const uint8_t modHash = 32;

uint8_t hashTable[modHash];
Pixel hashVal[modHash];

std::pair< int8_t, int8_t> lastFrameDelta[64] =  {  { -5 , 0 }  ,  { -4 , -1 }  ,  { -4 , 0 }  ,  { -4 , 1 }  ,  { -3 , -3 }  ,  { -3 , -2 }      ,  { -3 , -1 }  ,  { -3 , 0 }  ,  { -3 , 1 }  ,  { -3 , 2 }  ,  { -3 , 3 }  ,  { -2 , -3 }  ,  { -2 , -2 }  ,  { -2 , -1 }  ,  { -2 , 0 }      ,  { -2 , 1 }  ,  { -2 , 2 }  ,  { -2 , 3 }  ,  { -1 , -4 }  ,  { -1 , -3 }  ,  { -1 , -2 }  ,  { -1 , -1 }  ,  { -1 , 0 }  ,  { -1 , 1 }      ,  { -1 , 2 }  ,  { -1 , 3 }  ,  { -1 , 4 }  ,  { 0 , -5 }  ,  { 0 , -4 }  ,  { 0 , -3 }  ,  { 0 , -2 }  ,  { 0 , -1 }  ,  { 0 , 1 }  ,  { 0     , 2 }  ,  { 0 , 3 }  ,  { 0 , 4 }  ,  { 0 , 5 }  ,  { 1 , -4 }  ,  { 1 , -3 }  ,  { 1 , -2 }  ,  { 1 , -1 }  ,  { 1 , 0 }  ,  { 1 , 1 }  ,      { 1 , 2 }  ,  { 1 , 3 }  ,  { 1 , 4 }  ,  { 2 , -3 }  ,  { 2 , -2 }  ,  { 2 , -1 }  ,  { 2 , 0 }  ,  { 2 , 1 }  ,  { 2 , 2 }  ,  { 2 , 3 }      ,  { 3 , -3 }  ,  { 3 , -2 }  ,  { 3 , -1 }  ,  { 3 , 0 }  ,  { 3 , 1 }  ,  { 3 , 2 }  ,  { 3 , 3 }  ,  { 4 , -1 }  ,  { 4 , 0 }  ,  { 4 ,     1 }  ,  { 5 , 0 }  };

std::pair< int8_t, int8_t > prevFrameDelta[32] =  {  { -3 , -1 }  ,  { -3 , 1 }  ,  { -2 , -2 }  ,  { -2 , -1 }  ,  { -2 , 0 }  ,  { -2 , 1     }  ,  { -2 , 2 }  ,  { -1 , -3 }  ,  { -1 , -2 }  ,  { -1 , -1 }  ,  { -1 , 0 }  ,  { -1 , 1 }  ,  { -1 , 2 }  ,  { -1 , 3 }  ,  { 0 , -2 }      ,  { 0 , -1 }  ,  { 0 , 1 }  ,  { 0 , 2 }  ,  { 1 , -3 }  ,  { 1 , -2 }  ,  { 1 , -1 }  ,  { 1 , 0 }  ,  { 1 , 1 }  ,  { 1 , 2 }  ,  { 1 ,     3 }  ,  { 2 , -2 }  ,  { 2 , -1 }  ,  { 2 , 0 }  ,  { 2 , 1 }  ,  { 2 , 2 }  ,  { 3 , -1 }  ,  { 3 , 1 }  };

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

	//std::cerr << out( numCurrFrame ) << std::endl;

	uint8_t runNum = 0;
	uint8_t lastRunNum = 0;

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
				//prevPixel.print( std::cout );
				runNum --;
				currFrame.setPixelPixel( prevPixel, i, j );
				continue;
			}
			if( lastRunNum ){
				Pixel currP = lastFrame.getPixel( i, j );
				//currP.print( std::cout );

				lastRunNum --;

				currFrame.setPixelPixel( currP, i, j );
				prevPixel = currP;

				continue;
			}

			uint8_t info;
			fin.read( (char*) &info, sizeof( info ) );

			uint8_t cpInfo = info;
			uint8_t type = cpInfo >> 5;
			if( info != (uint8_t) 255 ) cnt[type] ++;
			else cnt[8] ++;
			if( type == 0 ){
				runNum = info & ( ( 1 << 5 ) -1 ); // last 5 bits
				//prevPixel.print( std::cout );
				runNum --;
				currFrame.setPixelPixel( prevPixel, i, j );
				continue;
			}

			if( type == 3 ){
				lastRunNum = info & ( ( 1 << 5 ) -1 );
				lastRunNum --;
				Pixel currP = lastFrame.getPixel( i, j );
				//currP.print( std::cout );
				currFrame.setPixelPixel(  currP, i, j );
				prevPixel = currP;
				continue;
			}

			if( info == (uint8_t)255 ){

				uint8_t _r, _g, _b;
				fin.read( (char*) &_r, sizeof( _r ) );
				newPixel.setR( _r );

				fin.read( (char*) &_g, sizeof( _g ) );
				newPixel.setG( _g );

				fin.read( (char*) &_b, sizeof( _b ) );
				newPixel.setB( _b );

				//newPixel.print( std::cout );
				currFrame.setPixelPixel( newPixel, i, j );
				prevPixel = newPixel;

				continue;
			}

			if( type == 1 ){
				int8_t dg = info & ( ( 1 << 5 ) -1 );
				dg -= 15;

				uint8_t info2;
				fin.read( (char*) &info2, sizeof( info2 ) );

				int8_t dgr = ( info2 >> 4 );
				dgr -= 7;
				// dgr = dg - dr  dr = dg - dgr
				int8_t dr = dg - dgr;

				int8_t dgb = info2 & ( ( 1 << 4 ) -1 );
				dgb -= 7;
				int8_t db = dg - dgb;

				newPixel.setR( prevPixel.getR() - dr );
				newPixel.setG( prevPixel.getG() - dg );
				newPixel.setB( prevPixel.getB() - db );
			}


  			if( type == 2 ){
  				int8_t dg  = ( info & ( ( 1 << 5 ) -1 ) ) >> 3;
  				int8_t dgr = ( info & ( ( 1 << 3 ) -1 ) ) >> 1;
  				int8_t dgb = info & 1;
  
  				dg -= 1;
  				dgr -= 1;
  
  				int8_t dr = dg - dgr;
  				int8_t db = dg - dgb;
  
  				newPixel.setR( prevPixel.getR() - dr );
  				newPixel.setG( prevPixel.getG() - dg );
  				newPixel.setB( prevPixel.getB() - db );
  			}

//    			if( type == 6 ){
//  
//    				int8_t dg  = ( info & ( ( 1 << 5 ) -1 ) ) >> 3;
//    				int8_t dgr = ( info & ( ( 1 << 3 ) -1 ) ) >> 1;
//    				int8_t dgb = info & 1;
//    
//    				dg -= 1;
//    				dgr -= 1;
//    
//    				int8_t dr = dg - dgr;
//    				int8_t db = dg - dgb;
//    
//    				Pixel currP = lastFrame.getPixel( i, j );
//    				newPixel.setR( currP.getR() - dr );
//    				newPixel.setG( currP.getG() - dg );
//    				newPixel.setB( currP.getB() - db );
//    			}
//  
//    			if( type == 7 ){
//    				int8_t dg = info & ( ( 1 << 5 ) -1 );
//    				dg -= 15;
//    
//    				uint8_t info2;
//    				fin.read( (char*) &info2, sizeof( info2 ) );
//    
//    				int8_t dgr = ( info2 >> 4 );
//    				dgr -= 7;
//    				// dgr = dg - dr  dr = dg - dgr
//    				int8_t dr = dg - dgr;
//    
//    				int8_t dgb = info2 & ( ( 1 << 4 ) -1 );
//    				dgb -= 7;
//    				int8_t db = dg - dgb;
//    
//    				Pixel currP = lastFrame.getPixel( i, j );
//    				newPixel.setR( currP.getR() - dr );
//    				newPixel.setG( currP.getG() - dg );
//    				newPixel.setB( currP.getB() - db );
//    			}

  			if( type == 4 || type == 5 ){
  
  				int ind = info & ( ( 1 << 5 ) -1 );
  				if( type == 5 ) ind += 32;

  				int currX = lastFrameDelta[ind].first + i;
  				int currY = lastFrameDelta[ind].second + j;

				assert( currX >= 0 );
				assert( currX < IMG_H );
				assert( currY >= 0 );
				assert( currY < IMG_W );
				newPixel.setPixel( lastFrame.getPixel( currX, currY ) );
  			}


  			if( type == 6 || type == 7 ){
  
  			bool indFrame = info & ( 1 << 4 );
  				uint8_t ind = info & ( ( 1 << 4 ) -1 );
  				if( type == 7 ) ind += 16;
  
  				int currX = prevFrameDelta[ind].first + i;
  				int currY = prevFrameDelta[ind].second + j;
  
  			assert( haveFrame[indFrame] );
  			assert( currX >= 0 and currX < IMG_H );
  			assert( currY >= 0 and currY < IMG_W );
  			newPixel.setPixel( prevFrame[indFrame].getPixel( currX, currY ) );
  			}

			//newPixel.print( std::cout );

			currFrame.setPixelPixel( newPixel, i, j );
			prevPixel.setPixel( newPixel );

			//currFrame.setPixelPixel( newPixel.getPixel(), i, j );
		}
	}

	if( !firstTime ){
		currDiff = currFrame.getDiff( lastFrame );
	}

	if( !haveFrame[0] || currDiff >= MAX_DIFF / 2 ){
		prevFrame[0].setFrame( currFrame );
		haveFrame[0] = true;
		haveFrame[2] = haveFrame[3] = false;
	}else if( !haveFrame[1] || currDiff >= MAX_DIFF / 10 ){
		prevFrame[1].setFrame( currFrame );
		haveFrame[1] = true;
	}

	Mat currMatImage( IMG_H, IMG_W, CV_8UC3, Scalar(0,0,0));
	for( int i=0 ; i < IMG_H ; i++ ){
		for( int j=0 ; j < IMG_W ; j++ ){

			uint8_t &b = currMatImage.at< cv::Vec3b>( i, j )[0];
			uint8_t &g = currMatImage.at< cv::Vec3b>( i, j )[1];
			uint8_t &r = currMatImage.at< cv::Vec3b>( i, j )[2];

			r = currFrame.getPixel( i, j ).getR();
			g = currFrame.getPixel( i, j ).getG();
			b = currFrame.getPixel( i, j ).getB();

		}
	}

	images.push_back( currMatImage );

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

	return 0;

	unsigned long long sum = 0;
	for( int i=0 ; i < 9 ; i++ ) sum += cnt[i];
	for( int i=0 ; i < 9 ; i++ ){
		std::cerr << out( i ) << out( cnt[i] ) << std::endl;
		//double curr = sum / cnt[i];
		//std::cerr << i << " : " << curr << std::endl;
	}
	std::cerr << std::endl;

	Size S = Size( IMG_W, IMG_H );
	//VideoWriter record("Ananasko.avi", cv::VideoWriter::fourcc('p', 'n', 'g', ' '), 24, S, true);
	//VideoWriter record("Ananasko.avi", cv::VideoWriter::fourcc( 'F', 'F', 'V', '1'), 24, S, true);
	VideoWriter record("Ananasko.avi", cv::VideoWriter::fourcc( 'H', 'F', 'Y', 'U'), 24, S, true);

	if( !record.isOpened() ){
		std::cerr << " bruhmomento " << std::endl;
	}
	for( int i=0 ; i < (int)images.size() ; i++ ){
		record << images[i];
		if (images[i].empty()) {
			std::cerr << "problem" << std::endl;
			break;
		}

	}
	record.release();


#ifdef TIME
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cerr << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
#endif


	return 0;
}
