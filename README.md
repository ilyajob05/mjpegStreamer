[![CMake](https://github.com/ilyajob05/mjpegStreamer/actions/workflows/cmake.yml/badge.svg)](https://github.com/ilyajob05/mjpegStreamer/actions/workflows/cmake.yml)
[![Docker Image CI](https://github.com/ilyajob05/mjpegStreamer/actions/workflows/docker-image.yml/badge.svg)](https://github.com/ilyajob05/mjpegStreamer/actions/workflows/docker-image.yml)

# mjpegStreamer
Simple application for create and translation mjpeg video stream to http webpage use openCV
Multithread video streaming

# Requirements
openCV

# Usage
Set number youre camera 

`mkdir build`

`cd build`

`cmake ..`

`./mjpegStreamer -src 0 -port 8080 -quality [1-10] -fps 1`

or

`./bin/mjpegStreamer -src 0 -port 8080 -quality [1-10] -fps 1`


Open browser and connect 127.0.0.1:8080




