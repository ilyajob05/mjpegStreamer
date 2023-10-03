[![CMake](https://github.com/ilyajob05/mjpegStreamer/actions/workflows/cmake.yml/badge.svg)](https://github.com/ilyajob05/mjpegStreamer/actions/workflows/cmake.yml)


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

or
```
docker pull icvision/mjpeg_streamer
docker run --rm --device=/dev/video0:/dev/video0 -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY -it \
  -p 8080:8080 mjpeg_streamer:latest -src 0 -port 8080 -quality 5 -fps 20
```

Open browser and connect 127.0.0.1:8080




