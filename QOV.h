#include <iostream>
#include <bits/stdc++.h>
#include <algorithm>

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
	int getR( ) const{
		return r;
	}

	int getG( ) const{
		return g;
	}

	int getB( ) const{
		return b;
	}

	bool operator ==( const Pixel& currP ){
		if( r != currP.r ) return false;
		if( g != currP.g ) return false;
		if( b != currP.b ) return false;
		return true;
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

	Pixel getPixel( int x, int y ) const{
		return pixels[x][y];
	}

	void init( ){
		pixels = new Pixel* [FrameH];
		for( int i=0 ; i < FrameH ; i++ ) pixels[i] = new Pixel [FrameW];
	}

	void setPixelPixel( const Pixel& p, int x, int y ){
		pixels[x][y] = p;
	}

	void setPixelRGB( int r, int b, int g, int x, int y ){
		pixels[x][y].setRGB( r, g, b );
	}

};

std::string inBinary( int curr ){
	std::string nas = "";
	int cp = curr;
	while( cp ){
		nas += (char)('0' + (cp&1 ) );
		cp >>= 1;
	}

	while( nas.size() < 8 ) nas = nas + "0";
	
	std::reverse( nas.begin(), nas.end() );
	return nas;
}


std::string getFramNum( int numFrame , int sz = 4 ){

	std::string nas = std::to_string( numFrame );
	while( nas.size() < sz ) nas = "0" + nas;

	return nas;
}
