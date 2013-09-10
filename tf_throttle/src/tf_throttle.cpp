// Throttle /tf bandwidth
// Author: Max Schwarz <max.schwarz@uni-bonn.de>

#include <ros/init.h>
#include <ros/node_handle.h>
#include <tf/transform_listener.h>

#include <boost/foreach.hpp>

boost::scoped_ptr<tf::TransformListener> g_tf;
ros::Publisher pub;

void sendTransforms()
{
	std::vector<std::string> frames;
	g_tf->getFrameStrings(frames);

	tf::tfMessage msg;
	msg.transforms.reserve(frames.size());

	ros::Time now = ros::Time::now();

	BOOST_FOREACH(std::string frame, frames)
	{
		std::string parentFrame;
		if(!g_tf->getParent(frame, now, parentFrame))
			continue;

		tf::StampedTransform transform;
		try
		{
			g_tf->lookupTransform(
				parentFrame,
				frame,
				ros::Time(0),
				transform
			);
		}
		catch(tf::TransformException&)
		{
			continue;
		}

		geometry_msgs::TransformStamped m;
		tf::transformStampedTFToMsg(transform, m);

		msg.transforms.push_back(m);
	}

	pub.publish(msg);
}

int main(int argc, char** argv)
{
	ros::init(argc, argv, "tf_throttle");

	ros::NodeHandle nh("~");

	g_tf.reset(new tf::TransformListener(nh, ros::Duration(4.0)));

	ros::Timer timer = nh.createTimer(ros::Duration(0.5), boost::bind(&sendTransforms));
	pub = nh.advertise<tf::tfMessage>("tf", 1);

	ros::spin();

	return 0;
}