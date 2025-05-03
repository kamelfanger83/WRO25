# Git Hub Documentation

# Repository overview

The repository is divided into two folders:
* arduino
* rpi

Which contain the code which is run on the Arduino and the Raspberry Pi respectively. Additionally, there are some non code parts: 
* Plan.md containing a plan on what has to be done and in which order. It is in Swiss German but has an englisch translation just below.
* wro robot finish.f3d contains the 3d modeled robot on fusion. There are all parts which can be printed separetly. We know that the file has to be converted before to be sent to a 3d-printer but to keep information about how exatly we modelled, we left it in the fusion format.
* field.blend which is a blender file that was used to test and debug our camera code. It contains the most important features of the field and a camera in the position we had the Raspberry pi camera.
* This readme

## Arduino folder:
  
* The arduino folder consists of one file: sketch_servo_listen_motor. The code on this file handles 2 things. For one it is resposible for the blind positioning code and secondly it handles the communication between the raspberry pi and the drive/servo motors. 

## Rpi folder 
* CMakeLists.txt contains the CMake configuration we use to build our project. It detects whether it is run on the Raspberry or another device and then either compiles with libcamera or the emulated camera which reads frames from disk.
* driver.cpp contains code which does the positioning and implements the mode system where modes can request waypoints (xy positions) on the field which are then visited in order.
* obstacleRound.cpp / openChallenge.cpp contain the modes for the obstacle Challenge / open Challenge respectively 
* serial.cpp contains code handling the connection to the arduino over a serial connection.
* structs.h declares all the important structs we use throughout our code.
* test.cpp declares an executable which we use for testing mostly things revolving around the camera.
* utils.cpp contains some general utility functions
* wasd.cpp contains code for steering the robot with W A S D controls which can be fun to drive around and test the hardware.

### rpi/camera:
* camera.h declares the interface for the camera.
* camera.cpp contains code for the Raspberry camera. It is for example responsible for inizalizing it, cleaning… but has no code of analyzing anything. This is mainly copied from https://git.libcamera.org/libcamera/simple-cam.git/tree. Additionally, there is code to emulate a camera in case the project is ran on a device other than the Raspberry by reading frames from disk.
* draw_line.cpp This file was only created to help debugging. It enables the coder to draw the calculated line on the frame in order to visualize what the code produced.
* find_color.cpp On one hand this file contains the "isColor"-functions which check the color of a pixel. On the other hand there are also some functions which are dedicated to the gradient analysis and the border detection
* find_line.cpp This file is the continuation of the previous one and detects lines with the color found with the functions in "color_find.cpp". There is also some additional functionality for detecting border points and lots of debug functions which draw stuff on frames so that we can save the frame to disk and inspect what was going on.
* obstacles.cpp We use obstacles as synonym for traffic lights. The function in this file project the points of a position of such traffic light and says if there really is one and the color it has.
* save_image.cpp it obivously contains code to save an image but also to convert the picture's color space.
* stb_image.h/stb_image_write.h is code to read and write images to disk which we use for debugging. They are open source on the internet.

### rpi/geo
* camera.cpp contains lots of 3D geometry and camera projection and unprojection code. The main thing that is declared here and used elsewhere is the `optimizePose` function which does the visual positioning. There are lots of functions needed for this `optimizePose` but also projection functions such as `projectPoint` and `projectLine` which are used both by `optimizePose` and elsewhere in the code directly. Additionally, there are some tests.
* constants.cpp contains camera parameter constants such as the field of view, angle by which it is tiled down on the robot and so on.
* coordinates.cpp and coordinatesTrafficlights.cpp contain hardcoded information about the location of the borders of the playing field, the orange and blue lines on the playing field, and a lot of useful functions to transform coordinates, return a certain position.
* utils.cpp contains a few handy functionalities for Operations on 3d Vectors. 
* p3p.py is a file with python code that can be used to calibrrate camera parameters 

### rpi/getCommands

* the getCommands folder contains instructions on how to convert the current pose of the robot as well as a target waypoint to actual commands for the arduino. It also contains a file made only for testing purposes. 
* getCommands.cpp creates commands based on a PD-Controller system, where the proportional part ensures that the robot is moving towards the target point while the differential part dampens the steering and prevents oscillation. The parameters for the different parts have to be adjusted to the vehicle.
* testCommands.cpp allows for manual coordinate inputs to check how the system behaves before testing on our actual robot.
