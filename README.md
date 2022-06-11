[![CMake](https://github.com/ilyajob05/mjpegStreamer/actions/workflows/cmake.yml/badge.svg)](https://github.com/ilyajob05/mjpegStreamer/actions/workflows/cmake.yml)


# mjpegStreamer
Simple application for create and translation mjpeg video stream to http webpage use openCV

# Requirements
openCV

# Usage
Set number youre camera 
```
int main(int argc, char *argv[])
{
    cv::VideoCapture camera;
    camera.open(0);

```

`mkdir build`

`cd build`

`cmake ..`

`./mjpegStreamer`



Open browser and connect 127.0.0.1:8080



```mermaid
flowchart TD
subgraph Socket
    A[Start app] --> B{Init server}
    B --> |Ok| C[Wait connect] --> CRS[Create socket] --> PDF[Push socket to dataflow pipe]
    CRS --> C
    B --> |Err| D[Delay] --> B
    end

subgraph Data flow
    SL[Get socket to list]
    F[Frame]
    SL & F --> S[Wait frames]
    S --> CFL[Check and update socket list] --> SF[Send Frame]
    end

PDF --> |Data flow msg| SL 
```



