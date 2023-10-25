#include <opencv2/core.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>
#include <chrono>

#define out(x) #x << " = " << x << "  "

using namespace cv;

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

	fout.write( (char *)&img.rows, sizeof( img.rows ) );
	fout.write( (char *)&img.cols, sizeof( img.cols ) );

	for(int i=0; i<img.rows; i++) {
		for(int j=0; j<img.cols; j++) {

			int b = img.at< cv::Vec3b>( i, j )[0];
			int g = img.at< cv::Vec3b>( i, j )[1];
			int r = img.at< cv::Vec3b>( i, j )[2];

			int type = 9;
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
