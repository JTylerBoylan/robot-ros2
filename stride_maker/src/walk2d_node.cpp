#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include "stride_maker/stride_maker.hpp"

#include <thread>
#include <mutex>

static inline rclcpp::QoS qos_reliable()
{
    return rclcpp::QoS(rclcpp::KeepLast(10)).reliable();
}

class Walk2DNode : public rclcpp::Node
{
private:
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr _cmd_vel_sub;
    std::vector<rclcpp::Publisher<robot_msgs::msg::LegTrajectory>::SharedPtr> _leg_traj_pubs;

    std::vector<double> _hip_positions;
    double _robot_width = 1.0;
    double _body_height = 0.2;
    double _center_shift = 0.0;
    int _stride_resolution = 100;
    double _step_height = 0.05;
    double _duty_factor = 0.5;
    double _max_stance_length = 0.15;

    std::mutex _mutex;
    std::vector<robot_msgs::msg::LegTrajectory> _trajectories;
    double _frequency = 10.0;

    void map_des2cmd(const double des_v, const double des_w, double &cmd_v, double &cmd_w)
    {
        // mapping obtained from walk stride sweep experiment
        const double a = 64 * des_v * des_v - 192 * des_v * des_w - 80 * des_v + 144 * des_w * des_w - 120 * des_w + 25;
        if (a < 0)
        {
            cmd_v = std::numeric_limits<double>::quiet_NaN();
            cmd_w = std::numeric_limits<double>::quiet_NaN();
            return;
        }

        const double sqrta = sqrt(a);
        cmd_v = 0.5 * des_v - 0.75 * des_w - 0.0625 * sqrta + 0.3125;
        cmd_w = 4.0 * des_w + (-8.0 * des_v - sqrta + 5.0) / 3.0;
    }

    void cmd_vel(const geometry_msgs::msg::Twist::SharedPtr msg)
    {
        double vel_x;
        double omega_z;
        map_des2cmd(msg->linear.x, msg->angular.z, vel_x, omega_z);

        if (std::isnan(vel_x) || std::isnan(omega_z))
        {
            RCLCPP_WARN(this->get_logger(), "Invalid velocity command.");
            return;
        }

        if (vel_x == 0.0 && omega_z == 0.0)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _trajectories.clear();
            _frequency = 10.0;
            return;
        }

        const double vel_left = vel_x - 0.5 * _robot_width * omega_z;
        const double vel_right = vel_x + 0.5 * _robot_width * omega_z;
        const double frequency = std::max(std::abs(vel_left), std::abs(vel_right)) / _max_stance_length * _duty_factor;

        double stance_length_left, stance_length_right;
        if (std::abs(vel_left) > std::abs(vel_right))
        {
            stance_length_left = vel_left > 0 ? _max_stance_length : -_max_stance_length;
            stance_length_right = vel_right * _duty_factor / frequency;
        }
        else
        {
            stance_length_right = vel_right > 0 ? _max_stance_length : -_max_stance_length;
            stance_length_left = vel_left * _duty_factor / frequency;
        }

        const auto traj_FL = make_walk_stride(_stride_resolution, frequency, _duty_factor,
                                              _center_shift, stance_length_left, _body_height,
                                              _step_height, 0.25);

        const auto traj_RL = make_walk_stride(_stride_resolution, frequency, _duty_factor,
                                              _center_shift, stance_length_left, _body_height,
                                              _step_height, 0.75);

        const auto traj_RR = make_walk_stride(_stride_resolution, frequency, _duty_factor,
                                              _center_shift, stance_length_right, _body_height,
                                              _step_height, 0.25);

        const auto traj_FR = make_walk_stride(_stride_resolution, frequency, _duty_factor,
                                              _center_shift, stance_length_right, _body_height,
                                              _step_height, 0.75);

        std::lock_guard<std::mutex> lock(_mutex);
        _trajectories = {traj_FL, traj_RL, traj_RR, traj_FR};
        _frequency = frequency;
    }

    void run_walk()
    {
        while (rclcpp::ok())
        {
            double frequency;
            std::vector<robot_msgs::msg::LegTrajectory> trajectories;
            {
                std::lock_guard<std::mutex> lock(_mutex);
                trajectories = _trajectories;
                frequency = _frequency;
            }

            if (trajectories.empty())
            {
                this->get_clock()->sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            assert(trajectories.size() == _leg_traj_pubs.size());
            for (size_t i = 0; i < _leg_traj_pubs.size(); i++)
            {
                _leg_traj_pubs[i]->publish(trajectories[i]);
            }

            this->get_clock()->sleep_for(std::chrono::milliseconds(static_cast<long>(1000.0 / frequency)));
        }
    }

public:
    Walk2DNode() : Node("walk2d_node")
    {
        std::vector<std::string> leg_names = {"FL", "RL", "RR", "FR"};
        this->declare_parameter("leg_names", leg_names);
        this->get_parameter("leg_names", leg_names);

        this->declare_parameter("hip_positions", _hip_positions);
        this->get_parameter("hip_positions", _hip_positions);
        assert(_hip_positions.size() == 2 * leg_names.size());

        this->declare_parameter("robot_width", _robot_width);
        this->get_parameter("robot_width", _robot_width);

        this->declare_parameter("body_height", _body_height);
        this->get_parameter("body_height", _body_height);

        this->declare_parameter("center_shift", _center_shift);
        this->get_parameter("center_shift", _center_shift);

        this->declare_parameter("stride_resolution", _stride_resolution);
        this->get_parameter("stride_resolution", _stride_resolution);

        this->declare_parameter("step_height", _step_height);
        this->get_parameter("step_height", _step_height);

        this->declare_parameter("duty_factor", _duty_factor);
        this->get_parameter("duty_factor", _duty_factor);

        this->declare_parameter("max_stance_length", _max_stance_length);
        this->get_parameter("max_stance_length", _max_stance_length);

        _cmd_vel_sub = this->create_subscription<geometry_msgs::msg::Twist>("cmd_vel", qos_reliable(), std::bind(&Walk2DNode::cmd_vel, this, std::placeholders::_1));

        for (const auto &leg_name : leg_names)
        {
            _leg_traj_pubs.push_back(this->create_publisher<robot_msgs::msg::LegTrajectory>("leg" + leg_name + "/trajectory", qos_reliable()));
        }

        std::thread thread(&Walk2DNode::run_walk, this);
        thread.detach();

        RCLCPP_INFO(this->get_logger(), "Walk2D Node Initialized.");
    }
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Walk2DNode>());
    rclcpp::shutdown();
    return 0;
}