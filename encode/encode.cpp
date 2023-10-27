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

class Pixel{
	private:

	int r;
	int g;
	int b;

	public:

	void setRGB( int _r, int _g, int _b ){
		r = _r;
		g = _g;
		b = _b;
	}

	const int& getR( ) const{
		return r;
	}

	const int& getG( ) const{
		return g;
	}

	const int& getB( ) const{
		return b;
	}

};

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

	void setPixelVal( const int& r, const int& b, const int& g, const int& x, const int& y ){
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

	std::ofstream fout("../decode/encodeBytes.dat", std::ios::out | std::ios::binary);

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
	for(int i=0; i < IMG_H ; i++) {
		for(int j=0; j < IMG_W; j++) {

			int b = img.at< cv::Vec3b>( i, j )[0];
			int g = img.at< cv::Vec3b>( i, j )[1];
			int r = img.at< cv::Vec3b>( i, j )[2];

			currFrame.setPixelVal( r, g, b, i, j );
			
			//uint8_t type = 
			//int type = ( 1 << 9 ) -1;
			std::cerr << out( type ) << std::endl;
			fout.write( (char*) &type, sizeof( type ) );
			fout.write( (char*) &r, sizeof( r ) );
			fout.write( (char*) &g, sizeof( g ) );
			fout.write( (char*) &b, sizeof( b ) );

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
