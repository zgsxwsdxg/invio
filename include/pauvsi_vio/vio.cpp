/*
 * vio.cpp
 *
 *  Created on: Sep 19, 2016
 *      Author: kevinsheridan
 */

#include "vio.h"


/*
 * starts all state vectors at 0.0
 */
VIO::VIO()
{
	this->readROSParameters();

	//feature tracker pass it its params
	this->feature_tracker.setParams(FEATURE_SIMILARITY_THRESHOLD, MIN_EIGEN_VALUE,
			KILL_BY_DISSIMILARITY, NUM_FEATURES, MIN_EIGEN_VALUE);

	//set up image transport
	image_transport::ImageTransport it(nh);
	this->cameraSub = it.subscribeCamera(this->getCameraTopic(), 1, &VIO::cameraCallback, this);

	//setup imu sub
	this->imuSub = nh.subscribe(this->getIMUTopic(), 100, &VIO::imuCallback, this);

	this->broadcastWorldToOdomTF();
}

VIO::~VIO()
{

}

void VIO::cameraCallback(const sensor_msgs::ImageConstPtr& img, const sensor_msgs::CameraInfoConstPtr& cam)
{
	ros::Time start = ros::Time::now();
	cv::Mat temp = cv_bridge::toCvShare(img, "mono8")->image.clone();

	//set the K and D matrices
	this->setK(get3x3FromVector(cam->K));
	this->setD(cv::Mat(cam->D, false));

	//undistort the image using the fisheye model
	ROS_ASSERT(cam->distortion_model == "fisheye");
	cv::fisheye::undistortImage(temp, temp, this->K, this->D, this->K);

	// set the current frame
	this->setCurrentFrame(temp, cv_bridge::toCvCopy(img, "mono8")->header.stamp);

	// process the frame correspondences
	this->run();

	//get the run time
	ROS_DEBUG_STREAM_THROTTLE(0.5, (ros::Time::now().toSec() - start.toSec()) * 1000 << " milliseconds runtime");

	this->viewImage(this->getCurrentFrame());
}

void VIO::imuCallback(const sensor_msgs::ImuConstPtr& msg)
{
	//ROS_DEBUG_STREAM_THROTTLE(0.1, "accel: " << msg->linear_acceleration);
	this->inertial_motion_estimator.addIMUMessage(*msg);
	//ROS_DEBUG_STREAM("time compare " << ros::Time::now().toNSec() - msg->header.stamp.toNSec());
}

cv::Mat VIO::get3x3FromVector(boost::array<double, 9> vec)
{
	cv::Mat mat = cv::Mat(3, 3, CV_32F);
	for(int i = 0; i < 3; i++)
	{
		mat.at<float>(i, 0) = vec.at(3 * i + 0);
		mat.at<float>(i, 1) = vec.at(3 * i + 1);
		mat.at<float>(i, 2) = vec.at(3 * i + 2);
	}

	ROS_DEBUG_STREAM_ONCE("K = " << mat);
	return mat;
}

/*
 * shows cv::Mat
 */
void VIO::viewImage(cv::Mat img){
	cv::imshow("test", img);
	cv::waitKey(30);
}

/*
 * draws frame with its features
 */
void VIO::viewImage(Frame frame){
	cv::Mat img;
	cv::drawKeypoints(frame.image, frame.getKeyPointVectorFromFeatures(), img, cv::Scalar(0, 0, 255));
	this->viewImage(img);

}

/*
 * sets the current frame and computes important
 * info about it
 * finds corners
 * describes corners
 */
void VIO::setCurrentFrame(cv::Mat img, ros::Time t)
{
	if(currentFrame.isFrameSet())
	{
		//first set the last frame to current frame
		lastFrame = currentFrame;
	}

	currentFrame = Frame(img, t, lastFrame.nextFeatureID); // create a frame with a starting ID of the last frame's next id
}

/*
 * runs:
 * feature detection, ranking, flowing
 * motion estimation
 * feature mapping
 */
void VIO::run()
{
	// if there is a last frame, flow features and estimate motion
	if(lastFrame.isFrameSet())
	{
		if(lastFrame.features.size() > 0)
		{
			feature_tracker.flowFeaturesToNewFrame(lastFrame, currentFrame);
			currentFrame.cleanUpFeaturesByKillRadius(this->KILL_RADIUS);
			//this->checkFeatureConsistency(currentFrame, this->FEATURE_SIMILARITY_THRESHOLD);
		}

		//MOTION ESTIMATION

		double certainty = this->estimateMotion();
	}

	//check the number of 2d features in the current frame
	//if this is below the required amount refill the feature vector with
	//the best new feature. It must not be redundant.

	//ROS_DEBUG_STREAM("feature count: " << currentFrame.features.size());

	if(currentFrame.features.size() < this->NUM_FEATURES)
	{
		//add n new unique features
		//ROS_DEBUG("low on features getting more");
		currentFrame.getAndAddNewFeatures(this->NUM_FEATURES - currentFrame.features.size(), this->FAST_THRESHOLD, this->KILL_RADIUS, this->MIN_NEW_FEATURE_DISTANCE);
		//currentFrame.describeFeaturesWithBRIEF();
	}

	this->broadcastWorldToOdomTF();

	//ROS_DEBUG_STREAM("imu readings: " << this->imuMessageBuffer.size());
}

/*
 * gets parameters from ROS param server
 */
void VIO::readROSParameters()
{
	//CAMERA TOPIC
	ROS_WARN_COND(!ros::param::has("~cameraTopic"), "Parameter for 'cameraTopic' has not been set");
	ros::param::param<std::string>("~cameraTopic", cameraTopic, DEFAULT_CAMERA_TOPIC);
	ROS_DEBUG_STREAM("camera topic is: " << cameraTopic);

	//IMU TOPIC
	ROS_WARN_COND(!ros::param::has("~imuTopic"), "Parameter for 'imuTopic' has not been set");
	ros::param::param<std::string>("~imuTopic", imuTopic, DEFAULT_IMU_TOPIC);
	ROS_DEBUG_STREAM("IMU topic is: " << imuTopic);

	ros::param::param<std::string>("~imu_frame_name", imu_frame, DEFAULT_IMU_FRAME_NAME);
	ros::param::param<std::string>("~camera_frame_name", camera_frame, DEFAULT_CAMERA_FRAME_NAME);
	ros::param::param<std::string>("~odom_frame_name", odom_frame, DEFAULT_ODOM_FRAME_NAME);
	ros::param::param<std::string>("~center_of_mass_frame_name", CoM_frame, DEFAULT_COM_FRAME_NAME);
	ros::param::param<std::string>("~world_frame_name", world_frame, DEFAULT_WORLD_FRAME_NAME);

	ros::param::param<int>("~fast_threshold", FAST_THRESHOLD, DEFAULT_FAST_THRESHOLD);

	ros::param::param<float>("~feature_kill_radius", KILL_RADIUS, DEFAULT_2D_KILL_RADIUS);

	ros::param::param<int>("~feature_similarity_threshold", FEATURE_SIMILARITY_THRESHOLD, DEFAULT_FEATURE_SIMILARITY_THRESHOLD);
	ros::param::param<bool>("~kill_by_dissimilarity", KILL_BY_DISSIMILARITY, false);

	ros::param::param<float>("~min_eigen_value", MIN_EIGEN_VALUE, DEFAULT_MIN_EIGEN_VALUE);

	ros::param::param<int>("~num_features", NUM_FEATURES, DEFAULT_NUM_FEATURES);

	ros::param::param<int>("~min_new_feature_distance", MIN_NEW_FEATURE_DISTANCE, DEFAULT_MIN_NEW_FEATURE_DIST);

	ros::param::param<double>("~starting_gravity_mag", GRAVITY_MAG, DEFAULT_GRAVITY_MAGNITUDE);
}

/*
 * broadcasts the world to odom transform
 */
void VIO::broadcastWorldToOdomTF()
{
	static tf::TransformBroadcaster br;
	tf::Transform transform;
	transform.setOrigin(this->position);

	//ROS_DEBUG_STREAM(this->pose.pose.orientation.w << " " << this->pose.pose.orientation.x);
	transform.setRotation(this->orientation);
	br.sendTransform(tf::StampedTransform(transform, ros::Time::now(), this->world_frame, this->odom_frame));
}

/*
 * broadcasts the odom to tempIMU trans
 */
ros::Time VIO::broadcastOdomToTempIMUTF(double roll, double pitch, double yaw, double x, double y, double z)
{
	static tf::TransformBroadcaster br;
	tf::Transform transform;
	transform.setOrigin(tf::Vector3(x, y, z));
	tf::Quaternion q;
	q.setRPY(roll, pitch, yaw);
	//ROS_DEBUG_STREAM(q.getW() << ", " << q.getX() << ", " << q.getY() << ", " << q.getZ());
	transform.setRotation(q);
	ros::Time sendTime = ros::Time::now();
	br.sendTransform(tf::StampedTransform(transform, sendTime, this->camera_frame, "temp_imu_frame"));
	return sendTime;
}

void VIO::correctOrientation(tf::Quaternion q, double certainty)
{
	//Takes orientation and rotates it towards q.
	orientation = orientation.slerp(q, certainty);
}
/*
 * returns the certainty
 * predicts the new rotation and position of the camera.
 * transfroms it to the odometry frame
 * and publishes a pose estimate
 *
 * FOR BACK UP
 * ROS_DEBUG_STREAM("velo: " << inertialVelocityChange.getX() << ", " << inertialVelocityChange.getY() << ", " << inertialVelocityChange.getZ());
 * ROS_DEBUG_STREAM("pos: " << inertialPositionChange.getX() << ", " << inertialPositionChange.getY() << ", " << inertialPositionChange.getZ());
 * ROS_DEBUG_STREAM("angle: " << inertialAngleChange.getX() << ", " << inertialAngleChange.getY() << ", " << inertialAngleChange.getZ());
 * ROS_ASSERT(inertialVelocityChange.getX() == inertialVelocityChange.getX() && inertialAngleChange.getX() == inertialAngleChange.getX());
 */
double VIO::estimateMotion()
{
//	tf::Vector3 inertialAngleChange, inertialPositionChange, inertialVelocityChange; // change in angle and pos from imu
//	tf::Vector3 visualAngleChange, visualPositionChange;
//	tf::Vector3 finalAngleChange(0.0, 0.0, 0.0), finalVelocityChange(0.0, 0.0, 0.0), finalPositionChange(0.0, 0.0, 0.0);
//	double visualMotionCertainty;
//	double averageMovement;
//
//	tf::Vector3 lastVelocity(this->velocity.vector.x, this->velocity.vector.y, this->velocity.vector.z);
//	tf::Vector3 lastAngularVelocity(this->angular_velocity.vector.x, this->angular_velocity.vector.y, this->angular_velocity.vector.z);
//
//	// get motion estimate from the IMU
//	//this->getInertialMotionEstimate(this->pose.header.stamp, this->currentFrame.timeImageCreated, lastVelocity, lastAngularVelocity, inertialAngleChange, inertialPositionChange, inertialVelocityChange);
//
//	finalAngleChange = inertialAngleChange;
//	finalPositionChange = inertialPositionChange;
//	finalVelocityChange = inertialVelocityChange;
//
//	//infer motion from images
//	tf::Vector3 unitVelocityInference;
//	bool visualMotionInferenceSuccessful = false;
//
//	//get motion inference from visual odometry
//	visualMotionInferenceSuccessful = this->visualMotionInference(lastFrame, currentFrame, inertialAngleChange,
//			visualAngleChange, unitVelocityInference, averageMovement);
//
//
//	//set the time stamp of the pose to the time of current frame
//	this->pose.header.stamp = this->currentFrame.timeImageCreated;
//
//	this->assembleStateVectors(finalPositionChange, finalAngleChange, finalVelocityChange);
//
//	ROS_DEBUG_STREAM("unit Velo " << unitVelocityInference.getX() << ", " << unitVelocityInference.getY() << ", " << unitVelocityInference.getZ());
//	this->broadcastOdomToTempIMUTF(0.0, 0.0, 0.0, unitVelocityInference.getX(), unitVelocityInference.getY(), unitVelocityInference.getZ());

}



/*
 * uses epipolar geometry from two frames to
 * estimate relative motion of the frame;
 */
bool VIO::visualMotionInference(Frame frame1, Frame frame2, tf::Vector3 angleChangePrediction,
		tf::Vector3& rotationInference, tf::Vector3& unitVelocityInference, double& averageMovement)
{
	//first get the feature deltas from the two frames
	std::vector<cv::Point2f> prevPoints, currentPoints;
	feature_tracker.getCorrespondingPointsFromFrames(frame1, frame2, prevPoints, currentPoints);

	//undistort points using fisheye model
	//cv::fisheye::undistortPoints(prevPoints, prevPoints, this->K, this->D);
	//cv::fisheye::undistortPoints(currentPoints, currentPoints, this->K, this->D);

	//get average movement bewteen images
	averageMovement = feature_tracker.averageFeatureChange(prevPoints, currentPoints);

	//ensure that there are enough points to estimate motion with vo
	if(currentPoints.size() < 5)
	{
		return false;
	}

	cv::Mat mask;

	//calculate the essential matrix
	cv::Mat essentialMatrix = cv::findEssentialMat(prevPoints, currentPoints, this->K, cv::RANSAC, 0.999, 1.0, mask);

	//ensure that the essential matrix is the correct size
	if(essentialMatrix.rows != 3 || essentialMatrix.cols != 3)
	{
		return false;
	}

	//recover pose change from essential matrix
	cv::Mat translation;
	cv::Mat rotation;

	//decompose matrix to get possible deltas
	cv::recoverPose(essentialMatrix, prevPoints, currentPoints, this->K, rotation, translation, mask);


	//set the unit velocity inference
	unitVelocityInference.setX(translation.at<double>(0, 0));
	unitVelocityInference.setY(translation.at<double>(1, 0));
	unitVelocityInference.setZ(translation.at<double>(2, 0));

	return true;
}

/*
 * map average pixel change from zero to threshold and then make that a value from 0 to 1
 * So, now you have this value if you multiply it times velocity
 */
void VIO::recalibrateState(double avgPixelChange, double threshold, bool consecutive)
{
	static double lastNormalize = 0;
	static sensor_msgs::Imu lastImu;
	double normalize = avgPixelChange/threshold;
	sensor_msgs::Imu currentImu = inertial_motion_estimator.getMostRecentImu();

	velocity = velocity * normalize;

	inertial_motion_estimator.gyroBiasX = (1-normalize)*currentImu.angular_velocity.x
											+ normalize*inertial_motion_estimator.gyroBiasX;
	inertial_motion_estimator.gyroBiasY = (1-normalize)*currentImu.angular_velocity.y
											+ normalize*inertial_motion_estimator.gyroBiasY;
	inertial_motion_estimator.gyroBiasZ = (1-normalize)*currentImu.angular_velocity.z
											+ normalize*inertial_motion_estimator.gyroBiasZ;
	//POTENTIAL BUG
	if(consecutive)
	{
		normalize = (normalize+lastNormalize)/2;
		double scale = sqrt(lastImu.angular_velocity.x*lastImu.angular_velocity.x + lastImu.angular_velocity.y*lastImu.angular_velocity.y
							+ lastImu.angular_velocity.z*lastImu.angular_velocity.z);
		if(scale != 0)
			inertial_motion_estimator.scaleAccelerometer = (1-normalize)*GRAVITY_MAG/scale;

		tf::Vector3 accel(lastImu.linear_acceleration.x*inertial_motion_estimator.scaleAccelerometer
					 	 , lastImu.linear_acceleration.y*inertial_motion_estimator.scaleAccelerometer
						 , lastImu.linear_acceleration.z*inertial_motion_estimator.scaleAccelerometer);
		tf::Vector3 gravity(0,0,GRAVITY_MAG);

		correctOrientation(inertial_motion_estimator.getDifferenceQuaternion(gravity, accel), (1-normalize));
	}


	lastImu = currentImu;
	lastNormalize = normalize;
	return;
}




