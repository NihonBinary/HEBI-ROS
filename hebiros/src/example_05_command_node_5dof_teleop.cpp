#include "ros/ros.h"
#include "sensor_msgs/JointState.h"
#include "sensor_msgs/Joy.h"
#include "hebiros/AddGroupFromNamesSrv.h"

using namespace hebiros;


float curr_joy[5] = {0, 0, 0, 0, 0};

void joy_callback(const sensor_msgs::Joy::ConstPtr& joy)
{ 
  curr_joy[0] = joy->axes[0];
  curr_joy[1] = joy->axes[1];
  curr_joy[2] = joy->axes[3];
  curr_joy[3] = joy->axes[4];
  curr_joy[4] = joy->axes[5] - joy->axes[2];
  //std::cout << curr_joy[0] << ", " << curr_joy[1] << ", " << curr_joy[2] << ", " << curr_joy[3] << ", " << curr_joy[4] << std::endl;
  
}


//Global variable and callback function used to store feedback data
sensor_msgs::JointState feedback;

void feedback_callback(sensor_msgs::JointState data) 
{
  feedback = data;
}


int main(int argc, char **argv) 
{

  //Initialize ROS node
  ros::init(argc, argv, "example_04_command_node_5dof");
  ros::NodeHandle n;
  ros::Rate loop_rate(200);

  std::string group_name = "my_group";

  //Create a client which uses the service to create a group
  ros::ServiceClient add_group_client = n.serviceClient<AddGroupFromNamesSrv>(
    "/hebiros/add_group_from_names");

  //Create a subscriber to receive feedback from a group
  //Register feedback_callback as a callback which runs when feedback is received
  ros::Subscriber feedback_subscriber = n.subscribe(
    "/hebiros/"+group_name+"/feedback/joint_state", 100, feedback_callback);

  //Create a publisher to send desired commands to a group
  ros::Publisher command_publisher = n.advertise<sensor_msgs::JointState>(
    "/hebiros/"+group_name+"/command/joint_state", 100);

  // Create a joystick callback
  ros::Subscriber joy_subscriber = n.subscribe<sensor_msgs::Joy>("joy", 10, joy_callback);

  AddGroupFromNamesSrv add_group_srv;

  //Construct a group using 3 known modules
  add_group_srv.request.group_name = group_name;
  add_group_srv.request.names = {"base", "shoulder", "elbow", "wrist1", "wrist2"};
  add_group_srv.request.families = {"HEBI"};
  //Call the add_group_from_names service to create a group
  //Specific topics and services will now be available under this group's namespace
  add_group_client.call(add_group_srv);

  //Construct a JointState to command to the modules
  //This may potentially contain a name, position, velocity, and effort for each module
  sensor_msgs::JointState command_msg;
  command_msg.name.push_back("HEBI/base");
  command_msg.name.push_back("HEBI/shoulder");
  command_msg.name.push_back("HEBI/elbow");
  command_msg.name.push_back("HEBI/wrist1");
  command_msg.name.push_back("HEBI/wrist2");
  command_msg.position.resize(5);

  double scale = 0.01;

  while(ros::ok())
  {
    command_msg.position[0] += scale * curr_joy[0];
    command_msg.position[1] += scale * curr_joy[1];
    command_msg.position[2] += scale * curr_joy[2];
    command_msg.position[3] += scale * curr_joy[3];
    command_msg.position[4] += scale * curr_joy[4];
    command_publisher.publish(command_msg);
    
    std::cout << command_msg.position[0] << ", " 
              << command_msg.position[1] << ", " 
              << command_msg.position[2] << ", " 
              << command_msg.position[3] << ", " 
              << command_msg.position[4] << std::endl;

    ros::spinOnce();
    loop_rate.sleep();
  }

  return 0;
}









