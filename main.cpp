#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>

using namespace cv;
using namespace std;

class CHistogram {
protected:
	cv::Mat	bins;
public:
	CHistogram() { };
	~CHistogram() { };
	virtual int calculate( cv::Mat &image ) = 0;
	virtual void normalize( void ) = 0;
	virtual cv::Mat render( void ) = 0;
};

class CHistogramGray : public CHistogram {
	void clear() {
		bins.setTo( Scalar( 0.f ) );
	};
public:
	CHistogramGray() {
		bins = Mat( 1, 256, CV_32F, Scalar( 0.f ) );
	}
	int calculate( Mat& image ) {
		if( image.channels() != 1 || image.type() != CV_8U ) { return 1; }
		this->clear();
		Mat_< uchar >::iterator it = image.begin< uchar >();
		Mat_< uchar >::iterator itend = image.end< uchar >();
		for( ; it != itend; ++it ) {
			bins.at< float >( 0, *it ) += 1.f;
		}
		return 0;
	}
	void normalize( void ) {
		Mat_< float >::iterator it = bins.begin< float >();
		Mat_< float >::iterator itend = bins.end< float >();
		float max = 1.f;
		for( ; it != itend; ++it ) {
			if( *it > max ) max = *it;
		}
		bins /= max;
	}
	Mat render( void ) {
		Mat result( 100, 256, CV_8U, Scalar( 0 ) );
		Point start( 0, 0 ), end( 0, 0 );
		for( int i = 0; i < 256; i++ ) {
			start.x = end.x = i;
			end.y = cvRound( 100.f * bins.at< float >( i ) );
			line( result, start, end, Scalar( 192 ) );
		}
		flip( result, result, 0 );

		return result;
	}
};

class CHistogramRGB: public CHistogram
{
	void clear() {
		bins.setTo( Scalar( 0.f ) );
	};
public:
	CHistogramRGB() {
		bins = Mat( 3, 256, CV_32F, Scalar( 0.f ) );
	}

	int calculate( Mat& image ) {
		if( image.type() != CV_8UC3 ) { return 1; }

		this->clear();

		Mat_< Vec3b >::iterator it = image.begin< Vec3b >();
		Mat_< Vec3b >::iterator itend = image.end< Vec3b >();

		for( ; it != itend; ++it ) {
			bins.at< float >( 0, (*it)[0] ) += 1.f;
			bins.at< float >( 1, (*it)[1] ) += 1.f;
			bins.at< float >( 2, (*it)[2] ) += 1.f;
		}

		return 0;
	}

	void normalize( void ) {
		Mat_< float >::iterator it = bins.begin< float >();
		Mat_< float >::iterator itend = bins.end< float >();

		float max = 1.f;

		for( ; it != itend; ++it ) {
			if( *it > max ) max = *it;
		}

		bins /= max;
	}

	Mat render( void ) {
	    int w_hist = 256;
	    int h_hist = 100;

		Mat result( h_hist, w_hist, CV_8UC3, Scalar( 0 ) );
		Point start( 0, h_hist - 1 ), end( 0, h_hist - 1 );

		for( int i = 0; i < w_hist; i++ ) {
			start.x = end.x = i;

			end.y = h_hist - cvRound( h_hist * bins.at< float >( 0, i ) );
			line( result, start, end, Scalar( 150, 40, 40 ) );

			end.y = h_hist - cvRound( h_hist * bins.at< float >( 1, i ) );
			line( result, start, end, Scalar( 40, 150, 40 ) );

			end.y = h_hist - cvRound( h_hist * bins.at< float >( 2, i ) );
			line( result, start, end, Scalar( 40, 40, 160 ) );
		}

        for( int i = 1; i < w_hist; i++ ) {
            /// Contour line

			line( result,   Point( i - 1,   h_hist - cvRound( h_hist * bins.at< float >(0, i - 1    ) ) ),
                            Point( i,       h_hist - cvRound( h_hist * bins.at< float >(0, i        ) ) ),
                            Scalar( 255, 0, 0 ) );

            line( result,   Point( i - 1,   h_hist - cvRound( h_hist * bins.at< float >(1, i - 1    ) ) ),
                            Point( i,       h_hist - cvRound( h_hist * bins.at< float >(1, i        ) ) ),
                            Scalar( 0, 255, 0) );

            line( result,   Point( i - 1,   h_hist - cvRound( h_hist * bins.at< float >(2, i - 1    ) ) ),
                            Point( i,       h_hist - cvRound( h_hist * bins.at< float >(2, i        ) ) ),
                            Scalar( 0, 0, 255) );
        }

		return result;
	}
};

int main( int argc, char **argv ) {
	if( argc != 2 ) {
		cerr << "Wrong parameters" << endl;
		return -1;
	}
//
//	Mat img = imread( argv[ 1 ], 1 ); // Flag 0 forces to load image in grayscale mode
//	if( !img.data ) {
//		cerr << "Can't load " << argv[ 1 ] << " image file" << endl;
//		return -1;
//	}
//
    cv::VideoCapture capture( argv[ 1 ] );
	if (!capture.isOpened()) {
		cerr << "Can't read " << argv[ 1 ] << " video file" << endl;
		return -1;
	}

	CHistogramRGB histogram;
	double rate = capture.get( CV_CAP_PROP_FPS );
	int delay = 1000 / rate;
    Mat frame;

    for (;;)
	{
	    if ( !capture.read( frame ) ) break;
		imshow( "Extracted Frame", frame );
		waitKey (delay);

	    if( histogram.calculate (frame) == 0 ) {
            histogram.normalize ();
            Mat display = histogram.render ();

            namedWindow( "Histogram" );
            imshow( "Histogram", display );
        } else {
            cerr << "Can't calculate histogram" << endl;
        }
	}
	capture.release();

	return 0;
}
