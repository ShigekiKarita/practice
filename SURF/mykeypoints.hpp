#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <algorithm>

class MyKeyPoints
{
public:
	template <class T = std::string>
	explicit MyKeyPoints(
		T left_file_name, 
		T right_file_name,
		bool cross_check = true,
		T detector_name = "SIFT",
		T descriptor_name = "SIFT",
		T matcher_name = "FlannBased"
	) :
		left_file_name(left_file_name),
		right_file_name(right_file_name),
		cross_check(cross_check)
	{
	        //cv::initModule_nonfree();

		this->detector = cv::FeatureDetector::create(detector_name);
		this->extractor = cv::DescriptorExtractor::create(descriptor_name);
		this->matcher = cv::DescriptorMatcher::create(matcher_name);
	}

	void init()
	{		
		std::vector<cv::DMatch> matches;
		std::vector< cv::KeyPoint > keys1, keys2;

		detect_features(matches, keys1, keys2);
		
		std::sort(matches.begin(), matches.end());

		aquire_keypoints(matches, keys1, keys2);
	}

	void fast_init()
	{
		std::vector<cv::DMatch> matches;
		std::vector< cv::KeyPoint > keys1, keys2;
		
		this->detector = new cv::DynamicAdaptedFeatureDetector(new cv::SurfAdjuster(10, true), 5000, 10000, 10);
		this->extractor = cv::DescriptorExtractor::create("SURF");

		detect_features(matches, keys1, keys2);
		
		std::sort(matches.begin(), matches.end());

		aquire_keypoints(matches, keys1, keys2);
	}

	std::vector< cv::Vec2f > points1, points2;

private:
	std::string left_file_name, right_file_name;
	cv::Ptr<cv::FeatureDetector> detector;
	cv::Ptr<cv::DescriptorExtractor> extractor;
	cv::Ptr<cv::DescriptorMatcher> matcher;
	bool cross_check;

	template< class T = std::vector< cv::DMatch >, class U = std::vector< cv::KeyPoint > >
	void detect_features(T& matches, U&  keys1, U& keys2)
	{		
		const cv::Mat  image1 = cv::imread(this->left_file_name, 0);
		const cv::Mat  image2 = cv::imread(this->right_file_name, 0);

		this->detector->detect(image1, keys1);
		this->detector->detect(image2, keys2);

		cv::Mat descriptor1, descriptor2;
		this->extractor->compute(image1, keys1, descriptor1);
		this->extractor->compute(image2, keys2, descriptor2);

		match_features(descriptor1, descriptor2, matches);
	}

	template<class T = cv::Mat, class U = std::vector<cv::DMatch>>
	void match_features(const T& descriptor1, const T& descriptor2, U& matches)
	{
		if ( this->cross_check )
		{
			std::vector<cv::DMatch> match12, match21;
			
			matcher->match(descriptor1, descriptor2, match12);
			matcher->match(descriptor2, descriptor1, match21);

			for ( size_t i = 0; i < match12.size(); i++ )
			{
				const auto& forward = match12[i];
				const auto& backward = match21[forward.trainIdx];

				if ( backward.trainIdx == forward.queryIdx )
				{
					matches.push_back(forward);
				}
			}
		}
		else
		{
			matcher->match(descriptor1, descriptor2, matches);
		}
	}

	template<class T = std::vector<cv::DMatch>>
	void sieve_matches(const double sieve_rate, const size_t minimum_size, T& matches)
	{
		size_t shoten_size = matches.size() * sieve_rate;

		matches.resize(shoten_size > minimum_size ? shoten_size : minimum_size);
	}

	void good_matches(std::vector< cv::KeyPoint >& keypoints_1, std::vector< cv::KeyPoint >& keypoints_2, std::vector< cv::DMatch >& good_matches2)
	{
		using namespace std;
		using namespace cv;

		const cv::Mat  leftImageGrey = cv::imread(this->left_file_name, 0);
		const cv::Mat  rightImageGrey = cv::imread(this->right_file_name, 0);

		Ptr<FeatureDetector> detector;
        detector = new DynamicAdaptedFeatureDetector(new FastAdjuster(10, true), 5000, 10000, 10);
        detector->detect(leftImageGrey, keypoints_1);
        detector->detect(rightImageGrey, keypoints_2);

		
        Ptr<DescriptorExtractor> extractor = DescriptorExtractor::create("SIFT");
		
        cv::Mat descriptors_1, descriptors_2;
        extractor->compute(leftImageGrey, keypoints_1, descriptors_1);
        extractor->compute(rightImageGrey, keypoints_2, descriptors_2);

		
        vector< vector<DMatch> > matches;
		Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce");
        matcher->knnMatch(descriptors_1, descriptors_2, matches, 500);

		double tresholdDist = 0.25 * sqrt(double(leftImageGrey.size().height*leftImageGrey.size().height + leftImageGrey.size().width*leftImageGrey.size().width));

		good_matches2.reserve(matches.size());
		for ( size_t i = 0; i < matches.size(); ++i )
		{
			for ( size_t j = 0; j < matches[i].size(); j++ )
			{
				Point2f from = keypoints_1[matches[i][j].queryIdx].pt;
				Point2f to = keypoints_2[matches[i][j].trainIdx].pt;

				double dist = sqrt(( from.x - to.x ) * ( from.x - to.x ) + ( from.y - to.y ) * ( from.y - to.y ));

                if ( dist < tresholdDist && abs(from.y - to.y)<5 )
				{
					good_matches2.push_back(matches[i][j]);
					j = matches[i].size();
				}
			}
		}
	}

	template<class T = cv::Mat, class U = std::vector< cv::KeyPoint >>
	void aquire_keypoints(const T& matches, const U& keys1, const U& keys2)
	{
		std::vector< cv::Vec2f > points1(matches.size());
		std::vector< cv::Vec2f > points2(matches.size());
		for ( size_t i = 0; i < matches.size(); ++i )
		{
			points1[i][0] = keys1[matches[i].queryIdx].pt.x;
			points1[i][1] = keys1[matches[i].queryIdx].pt.y;

			points2[i][0] = keys2[matches[i].trainIdx].pt.x;
			points2[i][1] = keys2[matches[i].trainIdx].pt.y;
		}
		this->points1 = std::move(points1);
		this->points2 = std::move(points2);
	}
};
