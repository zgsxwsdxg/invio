/*
 * Frame.cpp
 *
 *  Created on: Aug 1, 2017
 *      Author: kevin
 */

#include "Frame.h"

Frame::Frame() {
	this->keyframe = false; // initially we are not a keyframe
	avgFeatureDepthSet = false;
}

Frame::Frame(cv::Mat _img, cv::Mat_<float> _k, ros::Time _t)
{
	this->keyframe = false; // initially we are not a keyframe
	this->img = _img;
	this->K = _k;
	this->t = _t;

	avgFeatureDepth = false;
}

Frame::~Frame() {
	// TODO Auto-generated destructor stub
}

double Frame::getAverageFeatureDepth()
{
	if(this->avgFeatureDepthSet)
	{
		return avgFeatureDepth;
	}
	else
	{
		// compute the average feature depth of this frame
		int maturePointCount = 0;
		for(auto e : this->features)
		{
			if(e.isImmature())
			{
				maturePointCount++;


			}
		}
	}
}

void Frame::setPose(tf::Transform tf){
	this->poseEstimate = tf;
	this->poseEstimate_inv = this->poseEstimate.inverse();
}

void Frame::setPose_inv(tf::Transform tf){
	this->poseEstimate_inv = tf;
	this->poseEstimate = this->poseEstimate_inv.inverse();
}

/*
 * uses the current pose and current pixels to determine the 3d position of the objects
 * assumes that all pixels lie on a plane
 */

/*void updateObjectPositions(cv::Mat_<float> K)
{
	for(int i = 0; i < features.size(); i++)
	{
		tf::Vector3 pixel = tf::Vector3((features.at(i).px.x - K(2)) / K(0), (features.at(i).px.y - K(5)) / K(4), 1.0);

		tf::Vector3 dir = currentPose * pixel - currentPose.getOrigin();

		double dt = (-currentPose.getOrigin().z() / dir.z());

		if(dt <= 0)
		{
			ROS_DEBUG("removing feature from planar odom because it is not on the xy plane");
			//remove this feature it is not on the plane
			features.erase(features.begin() + i);
			i--;
		}
		else
		{
			features.at(i).obj = currentPose.getOrigin() + dir * dt;
		}
	}

	if(features.size() <= 4)
	{
		ROS_WARN("planar odometry has too few features!");
	}
}*/
