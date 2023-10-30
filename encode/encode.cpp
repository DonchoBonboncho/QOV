/*

	OPERATIONS:

	_ _ _ | _ _ _ _ _
	TYPE     INFO

	1 1 1 1 1 1 1 1 + 3 B - new 
	0 0 1 | _ _ _ _ _	  - index
	0 1 0 | _ _ _ _ _     - diff up
	0 1 1 | _ _ _ _ _     - diff left

*/
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

class Pixel{
	private:

	int r;
	int g;
	int b;

	public:

	Pixel( ){
		r = b = g = 0;
	}
  
  	Pixel( int _r, int _g, int _b ){
  		r = _r;
  		g = _g;
  		b = _b;
  	}

	void setRGB( int _r, int _g, int _b ){
		r = _r;
		g = _g;
		b = _b;
	}

	// getters
	const int& getR( ) const{
		return r;
	}

	const int& getG( ) const{
		return g;
	}

	const int& getB( ) const{
		return b;
	}

	bool operator ==( const Pixel& currP ){
		if( r != currP.r ) return false;
		if( g != currP.g ) return false;
		if( b != currP.b ) return false;
		return true;
	}

};

Pixel valHash[modHash];

class Frame{
	private:

	int ind;
	int FrameH, FrameW;
	Pixel** pixels;

	public:

	Frame( int H, int W ){
		FrameH = H;
		FrameW = W;
		init();
	}

	void init( ){
		pixels = new Pixel* [FrameH];
		for( int i=0 ; i < FrameH ; i++ ) pixels[i] = new Pixel [FrameW];
	}

	void setPixelPixel( const Pixel& p, const int& x, const int& y ){
		pixels[x][y] = p;
	}

	void setPixelRGB( const int& r, const int& b, const int& g, const int& x, const int& y ){
		pixels[x][y].setRGB( r, g, b );
	}

};




std::string getFramNum( int numFrame , int sz = 4 ){

	std::string nas = std::to_string( numFrame );
	while( nas.size() < sz ) nas = "0" + nas;

	return nas;
}


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

	const int IMG_H = img.rows;
	const int IMG_W = img.cols;
	fout.write( (char *)&IMG_H, sizeof( IMG_H ) );
	fout.write( (char *)&IMG_W, sizeof( IMG_W ) );

	Frame currFrame( IMG_H, IMG_W );
	Pixel prev;
	int cntRun = 0;
	for(int i=0; i < IMG_H ; i++) {
		for(int j=0; j < IMG_W; j++) {

			int b = img.at< cv::Vec3b>( i, j )[0];
			int g = img.at< cv::Vec3b>( i, j )[1];
			int r = img.at< cv::Vec3b>( i, j )[2];

			Pixel curr( r, g, b );

			currFrame.setPixelPixel( curr, i, j );


			//run operation
			// 0 0 0 | _ _ _ _ _
			if( ( i || j ) and ( i != IMG_H and j != IMG_W ) and curr == prev ){
				cntRun ++;
				if( cntRun < ( 1 << 6 )-1 ) continue;
			}
			if( cntRun ){
				int type = 5;
				int info;
				//uint8_t info;
				info = ( type << 5 );
				info |= cntRun;
				std::cerr << out( cntRun ) << std::endl;
				cntRun = 0;
				fout.write( (char*) &info, sizeof( info ) );
				std::cout << 0 << std::endl;

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

				std::cout << 1 << std::endl;
				prev = curr;
				continue;
			}

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


			int type = 3;
			//uint8_t infoType = ( type << 5 );
			//uint8_t infoR = curr.getR();
			//uint8_t infoG = curr.getG();
			//uint8_t infoB = curr.getB();
			int infoType = ( type << 5 );
			int infoR = curr.getR();
			int infoG = curr.getG();
			int infoB = curr.getB();
			
			fout.write( (char *) &infoType, sizeof( infoType ) );
			fout.write( (char *) &infoR, sizeof( infoR ) );
			fout.write( (char *) &infoG, sizeof( infoG ) );
			fout.write( (char *) &infoB, sizeof( infoB ) );

			if( numHash < modHash ){
				hashTable[ currHash ] = numHash ++;
				valHash[ currHash ] = curr;
			}

			std::cout << 3 << std::endl;
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
