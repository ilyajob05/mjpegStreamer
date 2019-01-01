# mjpegStreamer
Simple application for create and translation mjpeg video stream to http webpage use openCV

# Requirements
openCV

# Usage
Set number youre camera in system 
```
int main(int argc, char *argv[])
{
    cv::VideoCapture camera;
    camera.open(0);

```

Open browser and connect 127.0.0.1:8080

![](screen2.png)



