#pragma once
#include <opencv2/opencv.hpp>
#include <algorithm>


template < typename T = long, typename ... Args >
T max(T const & a, T const & b, Args ... rest)
{
	return max(max(a, b), rest ...);
}

template < typename T >
T max(T const & a, T const & b)
{
	return a < b ? b : a;
}


void show(std::string image)
{
	const cv::Mat input = cv::imread(image, 1);	
	cv::imshow(image, input);
}


template <class T = cv::Mat_< cv::Vec3b >>
void display_mosaiced_image(T& image_to_write, const T& image_to_read, const std::string& name)
{
	for ( int y = 0; y < image_to_read.rows; ++y )
	{
		for ( int x = 0; x < image_to_read.cols; ++x )
		{
			image_to_write(y, x) = image_to_read(y, x);
		}
	}

	cv::namedWindow(name, cv::WINDOW_AUTOSIZE);
	cv::imshow(name, image_to_write);
}


template <class T = cv::Mat_< cv::Vec3b >, class U = cv::Point>
void mosaic_with_homography(const T& rgb_image1, const T& rgb_image2, const U& points1, const U& points2)
{
	// 画像1を画像2の空間に投影
	auto homography = cv::findHomography(points1, points2, CV_RANSAC);
	cv::Mat_< cv::Vec3b > result;
	cv::warpPerspective(rgb_image1, result, homography, cv::Size(( ( int ) rgb_image1.cols * 2 ), rgb_image1.rows * 2));
	display_mosaiced_image(result, rgb_image2, "Homography");
}


template <class T = cv::Mat_< cv::Vec3b >, class U = cv::Point>
void mosaic_with_affine(const T& rgb_image1, const T& rgb_image2, const U& points1, const U& points2)
{
	auto warp_dst = cv::Mat::zeros(rgb_image1.rows, rgb_image1.cols, rgb_image1.type());
	cv::Point2f srcTri[3];
	cv::Point2f dstTri[3];

	for ( size_t i = 0; i < 3; ++i )
	{
		srcTri[i] = cv::Point2f(points1[i][0], points1[i][1]);
		dstTri[i] = cv::Point2f(points2[i][0], points2[i][1]);
	}

	cv::Mat warp_mat = getAffineTransform(srcTri, dstTri);
	cv::Mat_< cv::Vec3b > result;
	warpAffine(rgb_image1, result, warp_mat, cv::Size(( ( int ) rgb_image1.cols * 2 ), rgb_image1.rows * 2));
	display_mosaiced_image(result, rgb_image2, "Affine");
}


cv::Mat_<double> least_mean_square_helmat(std::vector< cv::Vec2f > source, std::vector< cv::Vec2f > distination, size_t new_size = 5)
{
	source.resize(new_size);
	distination.resize(new_size);

	const auto size = source.size();
	auto A = cv::Mat_<double>(size * 2, 4);
	auto L = cv::Mat_<double>(size * 2, 1);

	for ( size_t i = 0; i < size; ++i )
	{
		A(i, 0) = source[i][0];
		A(i, 1) = source[i][1];
		A(i, 2) = 1;
		A(i, 3) = 0;

		auto j = i + size;
		A(j, 0) = source[i][1];
		A(j, 1) = -source[i][0];
		A(j, 2) = 0;
		A(j, 3) = 1;

		L(0, i) = distination[i][0];
		L(0, j) = distination[i][1];
	}

	cv::Mat_<double> K = ( A.t() * A ).inv() * A.t() * L;
	return  cv::Mat_<double>(2, 3) << K(0, 0), K(0, 1), K(0, 2), -K(0, 1), K(0, 0), K(0, 3);
}


template<class T = cv::Vec3b>
double rgb_euclid_distance(const T& rhs, const T& lhs)
{
	double result = 0.;
	for ( size_t i = 0; i < 3; ++i )
	{
		result += pow(rhs[i] - lhs[i], 2);
	}
	return sqrt(result);
}


template <class T = cv::Mat_< cv::Vec3b >, class U = std::vector< cv::Vec2f >>
void sieve_with_RGB_distance(const T& rgb_source, const T& rgb_distination, U& source_keypoints, U& distination_keypoints, double threshold = 100)
{
	auto max_cols = max(rgb_source.cols, rgb_distination.cols);
	auto max_rows = max(rgb_source.rows, rgb_distination.rows);

	auto check_width = [&] (double x)
	{
		return max_cols - 1 < floorl(x)
			? max_cols - 1 : max<long>(floorl(x), 0);
	};

	auto check_height = [&] (double y)
	{
		return max_rows - 1 < floorl(y)
			? max_rows - 1 : max<long>(floorl(y), 0);
	};

	const auto size = source_keypoints.size();

	U new_source_keypoints, new_distination_keypoints;
	new_source_keypoints.reserve(size);
	new_distination_keypoints.reserve(size);

	for ( size_t i = 0; i < size; ++i )
	{
		const auto rhs = rgb_source.template at<cv::Vec3b>(check_height(source_keypoints[i][1]), check_width(source_keypoints[i][0]));
		const auto lhs = rgb_distination.template at<cv::Vec3b>(check_height(distination_keypoints[i][1]), check_width(distination_keypoints[i][0]));
		auto d = rgb_euclid_distance(rhs, lhs);

		if ( threshold > d )
		{
			new_source_keypoints.push_back(source_keypoints[i]);
			new_distination_keypoints.push_back(distination_keypoints[i]);
		}
	}

	source_keypoints = ( new_source_keypoints );
	distination_keypoints = ( new_distination_keypoints );
}


template <class T = cv::Mat_< cv::Vec3b >, class U = std::vector< cv::Vec2f >>
cv::Mat_<double> RGB_sieve_LMS_helmat(const T& rgb_source, const T& rgb_distination, U source_keypoints, U distination_keypoints)
{
	sieve_with_RGB_distance(rgb_source, rgb_distination, source_keypoints, distination_keypoints, 20);

	const auto size = source_keypoints.size();

	auto A = cv::Mat_<double>(size * 2, 4);
	auto L = cv::Mat_<double>(size * 2, 1);

	for ( size_t i = 0; i < size; ++i )
	{
		A(i, 0) = source_keypoints[i][0];
		A(i, 1) = source_keypoints[i][1];
		A(i, 2) = 1;
		A(i, 3) = 0;

		auto j = i + size;
		A(j, 0) = source_keypoints[i][1];
		A(j, 1) = -source_keypoints[i][0];
		A(j, 2) = 0;
		A(j, 3) = 1;

		L(0, i) = distination_keypoints[i][0];
		L(0, j) = distination_keypoints[i][1];
	}

	cv::Mat_<double> K = ( A.t() * A ).inv() * A.t() * L;
	return  cv::Mat_<double>(2, 3) << K(0, 0), K(0, 1), K(0, 2), -K(0, 1), K(0, 0), K(0, 3);
}


template <class T = cv::Mat_< cv::Vec3b >, class U = std::vector< cv::Vec2f >>
cv::Mat_<double> RGB_LMS_helmat(const T& rgb_source, const T& rgb_distination, U source_keypoints, U distination_keypoints, const size_t new_size = 10)
{
	if ( source_keypoints.size() > new_size )
	{
		source_keypoints.resize(new_size);
		distination_keypoints.resize(new_size);
	}

	const auto size = source_keypoints.size();

	cv::Mat_<double> A = cv::Mat_<double>(size * 2, 4);
	cv::Mat_<double> L = cv::Mat_<double>(size * 2, 1);
	cv::Mat_<double> R = cv::Mat_<double>::zeros(size * 2, size * 2);

	auto check_width = [&] (double x)
	{
		return rgb_source.cols - 1 < floorl(x)
			? rgb_source.cols - 1 : max<long>(floorl(x), 0);
	};

	auto check_height = [&] (double y)
	{
		return rgb_source.rows - 1 < floorl(y)
			? rgb_source.rows - 1 : max<long>(floorl(y), 0);
	};

	for ( size_t i = 0; i < size; ++i )
	{
		A(i, 0) = source_keypoints[i][0];
		A(i, 1) = source_keypoints[i][1];
		A(i, 2) = 1;
		A(i, 3) = 0;

		auto j = i + size;
		A(j, 0) = source_keypoints[i][1];
		A(j, 1) = -source_keypoints[i][0];
		A(j, 2) = 0;
		A(j, 3) = 1;

		L(0, i) = distination_keypoints[i][0];
		L(0, j) = distination_keypoints[i][1];

		const auto rhs = rgb_source.template at<cv::Vec3b>(check_height(source_keypoints[i][1]), check_width(source_keypoints[i][0]));
		const auto lhs = rgb_distination.template at<cv::Vec3b>(check_height(distination_keypoints[i][1]), check_width(distination_keypoints[i][0]));

		R(i, i) = rgb_euclid_distance(rhs, lhs);
		R(j, j) = R(i, i);
	}

	A = R * A;
	L = R * L;

	cv::Mat_<double> K = ( A.t() * A ).inv() * A.t() * L;

	return  cv::Mat_<double>(2, 3) << K(0, 0), K(0, 1), K(0, 2), -K(0, 1), K(0, 0), K(0, 3);
}


template <class T = cv::Mat_< cv::Vec3b >, class U = std::vector< cv::Vec2f >>
void mosaic_with_helmat(const T& rgb_source, const T& rgb_distination, U& source_keypoints, U& distination_keypoints, bool is_display = true)
{
	cv::Mat_<double> helmat = least_mean_square_helmat(source_keypoints, distination_keypoints);
	//const auto& helmat = RGB_sieve_LMS_helmat(rgb_source, rgb_distination, source_keypoints, distination_keypoints);

	cv::Mat_< cv::Vec3b > result;
	warpAffine(rgb_source, result, helmat, cv::Size(( ( int ) rgb_source.cols * 2 ), rgb_source.rows * 2));

	if ( is_display )
	{
		display_mosaiced_image(result, rgb_distination, "Helmert");
	}
}
