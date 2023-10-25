#include <opencv2/core.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>
#include <chrono>

using namespace cv;

int main(){

#ifdef TIME
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#endif

	std::ifstream fin( "encodeBytes.dat", std::ios::out | std::ios::binary );
	if( !fin ){
		std::cerr << " nqma otvarqne " << std::endl;
		return 1;
	}

	int currH, currW;
	fin.read((char *)&currH, sizeof( currH ) );
	fin.read((char *)&currW, sizeof( currW ) );

	std::cerr << " ! " << currH << " " << currW << std::endl;

	//std::cout << "P3" << std::endl;
	//std::cout << currW << " " << currH << " 255 " << std::endl;

	for( int i=0 ; i < currH ; i++ ){
		for( int j=0 ; j < currW ; j++ ){

			int type;
			fin.read( (char *) &type, sizeof( type ) );


			if( type == 9 ){

				int currR;
				fin.read( (char * )&currR, sizeof( currR ) );
				int currG;
				fin.read( (char * )&currG, sizeof( currG ) );
				int currB;
				fin.read( (char * )&currB, sizeof( currB ) );
			}
		}
	}

	fin.close();
	if( !fin.good() ){
		std::cerr << " bruhmomento " << std::endl;
		return 1;
	}


#ifdef TIME
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cerr << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
#endif

    return 0;
}
