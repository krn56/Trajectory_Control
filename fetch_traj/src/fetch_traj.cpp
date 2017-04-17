#include "fetch_traj/fetch_traj.h"

TrajectoryFollow::TrajectoryFollow()
{
    arm_client = new armClient("arm_controller/follow_joint_trajectory", true);
    base_client = new baseClient("move_base", true);
    outputs = MatrixXf::Random(6,6);
}

TrajectoryFollow::~TrajectoryFollow()
{
     delete arm_client;
     delete base_client;
}

void TrajectoryFollow::startTrajectory(control_msgs::FollowJointTrajectoryGoal& arm_goal)
{
    arm_goal.trajectory.header.stamp = ros::Time::now() + ros::Duration(1.0);
    arm_client->sendGoal(arm_goal);
}

void TrajectoryFollow::startMoveBase(move_base_msgs::MoveBaseGoal& base_goal)
{
    base_goal.target_pose.header.stamp = ros::Time::now() + ros::Duration(1.0);
    ROS_INFO_STREAM(base_goal.target_pose.pose.position.x << std::endl);
    base_client->sendGoal(base_goal);
}

actionlib::SimpleClientGoalState TrajectoryFollow::getState()
{
    return base_client->getState();
}

void TrajectoryFollow::jointsCallback(const sensor_msgs::JointState& state)
{
    ROS_INFO_STREAM("t:" << std::endl);
    joint_states.clear();
    ROS_INFO_STREAM("t1:" << std::endl);
    for(unsigned int i = 6; i < 12; i++)
    {
        joint_states.push_back(state.position[i]);
    }
}

MatrixXf TrajectoryFollow::trajectory(const MatrixXf& joint_positions, float tf)
{
    MatrixXf M(6,6);
    M << 1, 0, 0, 0, 0, 0,
         0, 1, 0, 0, 0, 0,
         0, 0, 2, 0, 0, 0,
         1, pow(tf,1), pow(tf,2), pow(tf,3), pow(tf,4), pow(tf,5),
         0, 1, 2*tf, 3*pow(tf,2), 4*pow(tf,3), 5*pow(tf,4),
         0, 0, 2, 6*tf, 12*pow(tf,2),  20*pow(tf,3);
    //ROS_INFO_STREAM("M:" << M << std::endl);
    MatrixXf inputs(6,6);
    inputs << joint_positions(0,0), 0, 0, joint_positions(1,0), 0, 0,
              joint_positions(0,1), 0, 0, joint_positions(1,1), 0, 0,
              joint_positions(0,2), 0, 0, joint_positions(1,2), 0, 0,
              joint_positions(0,3), 0, 0, joint_positions(1,3), 0, 0,
              joint_positions(0,4), 0, 0, joint_positions(1,4), 0, 0,
              joint_positions(0,5), 0, 0, joint_positions(1,5), 0, 0;

    inputs.transposeInPlace();
    std::cout << "miNVERSE:" << M.inverse() << std::endl;
    std::cout << "inputs:" << inputs << std::endl;
    MatrixXf outputs(6,6);
    outputs = (M.inverse())*inputs;
    std::cout << "outputs:" << outputs << std::endl;
    return outputs;
}

control_msgs::FollowJointTrajectoryGoal TrajectoryFollow::armExtensionTrajectory(const MatrixXf& input)
{
    control_msgs::FollowJointTrajectoryGoal arm_goal;
    ROS_INFO("1");
    arm_goal.trajectory.joint_names.push_back("shoulder_pan_joint");
    arm_goal.trajectory.joint_names.push_back("shoulder_lift_joint");
    arm_goal.trajectory.joint_names.push_back("upperarm_roll_joint");
    arm_goal.trajectory.joint_names.push_back("elbow_flex_joint");
    arm_goal.trajectory.joint_names.push_back("forearm_roll_joint");
    arm_goal.trajectory.joint_names.push_back("wrist_flex_joint");
    arm_goal.trajectory.joint_names.push_back("wrist_roll_joint");

    arm_goal.trajectory.points.resize(no_of_iterations);
    float t = 0;

    for(unsigned int i = 0; i < no_of_iterations; i++)
    {
    arm_goal.trajectory.points[i].positions.resize(7);
    arm_goal.trajectory.points[i].positions[0] = input(0,0) + input(1,0)*t + input(2,0)*pow(t,2) + input(3,0)*pow(t,3) + input(4,0)*pow(t,4) + input(5,0)*pow(t,5);
    arm_goal.trajectory.points[i].positions[1] = input(0,1) + input(1,1)*t + input(2,1)*pow(t,2) + input(3,1)*pow(t,3) + input(4,1)*pow(t,4) + input(5,1)*pow(t,5);
    arm_goal.trajectory.points[i].positions[2] = input(0,2) + input(1,2)*t + input(2,2)*pow(t,2) + input(3,2)*pow(t,3) + input(4,2)*pow(t,4) + input(5,2)*pow(t,5);
    arm_goal.trajectory.points[i].positions[3] = input(0,3) + input(1,3)*t + input(2,3)*pow(t,2) + input(3,3)*pow(t,3) + input(4,3)*pow(t,4) + input(5,3)*pow(t,5);
    arm_goal.trajectory.points[i].positions[4] = input(0,4) + input(1,4)*t + input(2,4)*pow(t,2) + input(3,4)*pow(t,3) + input(4,4)*pow(t,4) + input(5,4)*pow(t,5);
    arm_goal.trajectory.points[i].positions[5] = input(0,5) + input(1,5)*t + input(2,5)*pow(t,2) + input(3,5)*pow(t,3) + input(4,5)*pow(t,4) + input(5,5)*pow(t,5);
    arm_goal.trajectory.points[i].positions[6] = 0;

    arm_goal.trajectory.points[i].velocities.resize(7);

    arm_goal.trajectory.points[i].velocities[0] = input(1,0) + 2*input(2,0)*pow(t,1) + 3*input(3,0)*pow(t,2) + 4*input(4,0)*pow(t,3) + 5*input(5,0)*pow(t,4);
    arm_goal.trajectory.points[i].velocities[1] = input(1,1) + 2*input(2,1)*pow(t,1) + 3*input(3,1)*pow(t,2) + 4*input(4,1)*pow(t,3) + 5*input(5,1)*pow(t,4);
    arm_goal.trajectory.points[i].velocities[2] = input(1,2) + 2*input(2,2)*pow(t,1) + 3*input(3,2)*pow(t,2) + 4*input(4,2)*pow(t,3) + 5*input(5,2)*pow(t,4);
    arm_goal.trajectory.points[i].velocities[3] = input(1,3) + 2*input(2,3)*pow(t,1) + 3*input(3,3)*pow(t,2) + 4*input(4,3)*pow(t,3) + 5*input(5,3)*pow(t,4);
    arm_goal.trajectory.points[i].velocities[4] = input(1,4) + 2*input(2,4)*pow(t,1) + 3*input(3,4)*pow(t,2) + 4*input(4,4)*pow(t,3) + 5*input(5,4)*pow(t,4);
    arm_goal.trajectory.points[i].velocities[5] = input(1,5) + 2*input(2,5)*pow(t,1) + 3*input(3,5)*pow(t,2) + 4*input(4,5)*pow(t,3) + 5*input(5,5)*pow(t,4);
    arm_goal.trajectory.points[i].velocities[6] = 0;

    arm_goal.trajectory.points[i].accelerations.resize(7);

    arm_goal.trajectory.points[i].accelerations[0] = 2*input(2,0) + 6*input(3,0)*pow(t,1) + 12*input(4,0)*pow(t,2) + 20*input(5,0)*pow(t,3);
    arm_goal.trajectory.points[i].accelerations[1] = 2*input(2,1) + 6*input(3,1)*pow(t,1) + 12*input(4,1)*pow(t,2) + 20*input(5,1)*pow(t,3);
    arm_goal.trajectory.points[i].accelerations[2] = 2*input(2,2) + 6*input(3,2)*pow(t,1) + 12*input(4,2)*pow(t,2) + 20*input(5,2)*pow(t,3);
    arm_goal.trajectory.points[i].accelerations[3] = 2*input(2,3) + 6*input(3,3)*pow(t,1) + 12*input(4,3)*pow(t,2) + 20*input(5,3)*pow(t,3);
    arm_goal.trajectory.points[i].accelerations[4] = 2*input(2,4) + 6*input(3,4)*pow(t,1) + 12*input(4,4)*pow(t,2) + 20*input(5,4)*pow(t,3);
    arm_goal.trajectory.points[i].accelerations[5] = 2*input(2,5) + 6*input(3,5)*pow(t,1) + 12*input(4,5)*pow(t,2) + 20*input(5,5)*pow(t,3);
    arm_goal.trajectory.points[i].accelerations[6] = 0;

    arm_goal.trajectory.points[i].time_from_start = ros::Duration(1 + 2*t);

    t = t + float(t_max)/no_of_iterations;

    ROS_INFO_STREAM("arm_goal" << arm_goal << std::endl);
    //ROS_INFO_STREAM("t:" << t << std::endl);
    }
    return arm_goal;
}

move_base_msgs::MoveBaseGoal TrajectoryFollow::baseMove()
{
    move_base_msgs::MoveBaseGoal base_goal;

    base_goal.target_pose.pose.position.x = 10.0;
    base_goal.target_pose.pose.orientation.w = 1.0;
    base_goal.target_pose.header.frame_id = 'first';
    base_goal.target_pose.header.stamp = ros::Time::now();

    return base_goal;
}
