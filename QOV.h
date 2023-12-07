#include <iostream>
#include <bits/stdc++.h>
#include <algorithm>

class Pixel{

	private:
		uint8_t r;
		uint8_t g;
		uint8_t b;

	public:
	Pixel( ){
		reset();
	}
  
  	Pixel( uint8_t _r, uint8_t _g, uint8_t _b ){
  		r = _r;
  		g = _g;
  		b = _b;
  	}

	// setters
	void setRGB( uint8_t _r, uint8_t _g, uint8_t _b ){
		r = _r;
		g = _g;
		b = _b;
	}

	void setPixel( const Pixel& a ){
		r = a.r;
		g = a.g;
		b = a.b;
	}

	void reset( ){
		r = 0;
		g = 0;
		b = 0;
	}

	void setR( uint8_t _r ){
		r = _r;
	}

	void setG( uint8_t _g ){
		g = _g;
	}
	void setB( uint8_t _b ){
		b = _b;
	}

	// getters
	uint8_t getR( ) const{
		return r;
	}

	uint8_t getG( ) const{
		return g;
	}

	uint8_t getB( ) const{
		return b;
	}

	bool isZero( ) const{
		if( r || g || b ) return false;
		return true;
	}

	bool operator ==( const Pixel& currP ) const{
		if( r != currP.r ) return false;
		if( g != currP.g ) return false;
		if( b != currP.b ) return false;
		return true;
	}

	void print( std::ostream& os )const{
		os << (int)r << " " << (int)g << " " << (int)b << std::endl;
	}

};

class Frame{
	private:

	int ind;
	int FrameH, FrameW;
	Pixel** pixels;

	public:

	Frame( ){
		FrameH = FrameW = -1;
	}

	Frame( int H, int W ){
		FrameH = H;
		FrameW = W;
		init();
	}

	Pixel getPixel( int x, int y ) const{
		return pixels[x][y];
	}

	long long getDiff( const Frame& a ){
		long long diff = 0;
		for( int i=0 ; i < a.FrameH ; i ++ ){
			for( int j=0 ; j < a.FrameW ; j ++ ){
				diff += abs( a.pixels[i][j].getR() - pixels[i][j].getR() );
				diff += abs( a.pixels[i][j].getG() - pixels[i][j].getG() );
				diff += abs( a.pixels[i][j].getB() - pixels[i][j].getB() );
			}
		}
		return diff;
	}


	void init( ){
		pixels = new Pixel* [FrameH];
		for( int i=0 ; i < FrameH ; i++ ) pixels[i] = new Pixel [FrameW];
	}

	void setPixelPixel( const Pixel& p, int x, int y ){
		pixels[x][y] = p;
	}

	void setPixelRGB( uint8_t r, uint8_t b, uint8_t g, int x, int y ){
		pixels[x][y].setRGB( r, g, b );
	}

	void setFrame( const Frame& a ){
		if( FrameH == -1 || FrameW == -1 ){
			FrameH = a.FrameH;
			FrameW = a.FrameW;
			init();
		}
		FrameH = a.FrameH;
		FrameW = a.FrameW;
		for( int i=0 ; i < FrameH ; i++ ){
			for( int j=0 ; j < FrameW ; j++ ){
				pixels[i][j].setPixel( a.getPixel( i, j ) );
			}
		}
	}

	~Frame(){
		for( int i=0 ; i<FrameH ; i++ ){
			delete pixels[i];
		}
		delete[] pixels;
	}

};

std::string inBinary( int curr ){
	std::string nas = "";
	int cp = curr;
	while( cp ){
		nas += (char)('0' + (cp&1 ) );
		cp >>= 1;
	}

	while( (int)nas.size() < 8 ) nas = nas + "0";
	
	std::reverse( nas.begin(), nas.end() );
	return nas;
}


std::string getFramNum( int numFrame , int sz = 4, int start = 0 ){

	std::string nas = std::to_string( numFrame + start );
	while( (int)nas.size() < sz ) nas = "0" + nas;

	return nas;
}
