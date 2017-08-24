/*
 * main.cpp
 *
 *  Created on: 24 Aug 2017
 *      Author: Carlos Miguens
 */


#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

using namespace cv;
using namespace std;

Mat getImageCannyBorders( Mat src);
vector<Point> findBordersPoints(Mat src);
Mat fourPointsTransform(Mat src, vector<Point>);
vector<Point> orderPoints(vector<Point> points);

/** @function getBorders */
Mat getImageCannyBorders( Mat src)
{
	Mat border;
	/// Convert it to gray
	cvtColor( src, border, CV_BGR2GRAY );
	GaussianBlur( border, border, Size(5,5), 0, 0, BORDER_DEFAULT );
	// Canny edge detector
	Canny(border, border, 75, 200);
	return border;
}

/** @function findTicket **/
vector<Point> findBordersPoints(Mat src)
{
	vector<vector<Point> > contours;	/// Find contours
	findContours( src, contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

	vector<vector<Point> > contours_poly( contours.size() );

	for(unsigned int i = 0; i < contours.size(); i++ )
	{
		double peri = arcLength(contours[i], true);
		approxPolyDP( Mat(contours[i]), contours_poly[i], 0.02 * peri, true );
		if (contours_poly[i].size() == 4)
		{
			return contours_poly[i];
		}
	}

	return contours_poly[0];
}

vector<Point> orderPoints(vector<Point> points)
{
	vector<Point> order;
	Point tl(std::numeric_limits<int>::max(), 0);
	Point tr(0,std::numeric_limits<int>::max());
	Point bl(std::numeric_limits<int>::max(),std::numeric_limits<int>::max());
	Point br(std::numeric_limits<int>::min(), std::numeric_limits<int>::min());

	for (int i = 0; i < 4;i++)
	{
		int x = points[i].x;
		int y = points[i].y;
		if (x + y < tl.x + tl.y) tl = points[i];
		if (x - y > tr.x - tr.y) tr = points[i];
		if (x - y < bl.x - bl.y) bl = points[i];
		if (x + y > br.x + br.y) br = points[i];
	}
	order.push_back(tl);
	order.push_back(tr);
	order.push_back(bl);
	order.push_back(br);
	return order;
}

Mat fourPointsTransform(Mat src, vector<Point> corners)
{
	Point2f points[4];
	vector<Point> pv = orderPoints(corners);
	std::copy(pv.begin(), pv.end(), points);
	Point tl = points[0];
	Point tr = points[1];
	Point bl = points[2];
	Point br = points[3];

	// compute the width of the new image, which will be the
	// maximum distance between bottom-right and bottom-left
	// x-coordiates or the top-right and top-left x-coordinates
	float widthA, widthB, maxWidth;
	float heightA, heightB, maxHeight;

	widthA = sqrt((pow((br.x - bl.x), 2)) + (pow((br.y - bl.y), 2)));
	widthB = sqrt((pow((tr.x - tl.x), 2)) + (pow((tr.y - tl.y), 2)));
	maxWidth = max(int(widthA), int(widthB));

	// compute the height of the new image, which will be the
	// maximum distance between the top-right and bottom-right
	// y-coordinates or the top-left and bottom-left y-coordinates
	heightA = sqrt((pow((tr.x - br.x), 2)) + (pow((tr.y - br.y), 2)));
	heightB = sqrt((pow((tl.x - bl.x), 2)) + (pow((tl.y - bl.y), 2)));
	maxHeight = max(int(heightA), int(heightB));

	// now that we have the dimensions of the new image, construct
	// the set of destination points to obtain a "birds eye view",
	// (i.e. top-down view) of the image, again specifying points
	// in the top-left, top-right, bottom-right, and bottom-left order
	Point2f dts[4];
	dts[0] = Point(0,0);
	dts[1] = Point(maxWidth-1,0);
	dts[2] = Point(0, maxHeight-1);
	dts[3] = Point(maxWidth-1,maxHeight-1);

	Mat warpMatrix = getPerspectiveTransform(points, dts);

	Mat rotated;
	Size size(maxWidth, maxHeight);
	warpPerspective(src, rotated, warpMatrix, size, INTER_LINEAR, BORDER_CONSTANT);
	return rotated;
}

/** @function main */
int main( int argc, char** argv )
{
	Mat src, graysrc, ticketImage;
	char* window_name = "Ticket Scanner";
	/// Load an image
	src = imread( "ticket.png" );
	if( !src.data )
		return -1;
	// Process image
	graysrc = getImageCannyBorders(src);
	vector<Point> borders = findBordersPoints(graysrc);
	ticketImage = fourPointsTransform(src, borders);

	cvtColor( ticketImage, graysrc, CV_BGR2GRAY );
	//GaussianBlur( graysrc, graysrc, Size(5,5), 0, 0, BORDER_DEFAULT );

	threshold(graysrc,ticketImage, 127, 255,THRESH_TOZERO);
	//GaussianBlur( ticketImage, ticketImage, Size(5,5), 0, 0, BORDER_DEFAULT );
	//adaptiveThreshold(ticketImage,ticketImage, 255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,11,2);


	namedWindow( window_name, CV_WINDOW_AUTOSIZE );
	imshow( window_name, ticketImage);
	waitKey(0);
	return 0;
}
