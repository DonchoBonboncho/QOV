#include <bits/stdc++.h>

bool grig[20][20];

int main (){

	int n;
	std::cin >> n;

	int mid = ( n >> 1 );
	for( int i=0 ; i < n ; i++ ){
		for( int j=0 ; j < n ; j++ ) std::cin >> grig[i][j];
	}

	int num = 0;
	std::cout << " { " ;
	for( int i=0 ; i < n ; i++ ){
		for( int j=0 ; j < n ; j++ ){
			if( grig[i][j] ){
				std::cout << " { " << i - mid << " , " << j - mid << " } ";
				num ++;
				if( num != 64 ) std::cout << " , ";
			}
		}
	}
	std::cout << " } " << std::endl;

	std::cerr << num << std::endl;


	return 0;
}
