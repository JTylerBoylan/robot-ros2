from launch import LaunchDescription
from launch_ros.actions import Node

def Bar100Node():
    return Node(
        package='bar100_ros2',
        executable='bar100_node',
        name='bar100_node',
        output='screen',
        parameters=[{
            'i2c-bus': 7,
            'frequency': 20.0,
            'fluid_density': 997.0474,
            'gravity': 9.80665,
            'surface_pressure': 1.0,
        }],
    )

def generate_launch_description():
    return LaunchDescription([
        Bar100Node(),
    ])