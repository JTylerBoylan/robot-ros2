from setuptools import find_packages, setup

package_name = 'robot_joysticks'

setup(
    name=package_name,
    version='1.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='JTB',
    maintainer_email='jtylerboylan@outlook.com',
    description='Robot Joysticks Package',
    license='Apache License 2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'swim_tuning_joystick = robot_joysticks.swim_tuning_joystick:main',
            'starq_gait_joystick = robot_joysticks.starq_gait_joystick:main',
            'legged_mpc_joystick = robot_joysticks.legged_mpc_joystick:main',
        ],
    },
)
