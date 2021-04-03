/**
 *  @author Derendyaev Ilya, ilyajob05@gmail.com
 *  @date    1.1.2019
 *  TCP receiver and sender
 **/


#include <iostream>
#include <opencv2/opencv.hpp>
#include "tcpserver.h"

using namespace std;
using namespace cv;


int main(int argc, char *argv[])
{
    cv::VideoCapture camera;
    camera.open(0);

    if(!camera.isOpened())
    {
        cout << "not camera connect" << endl;
        return 0;
    }

    TCPServer serverInputConnection("127.0.0.1", 8080);
    serverInputConnection.createListener();

    ////////////////////// reconnect network cycle //////////////////////
    while(1)
    {
        if(serverInputConnection.waitConnect() == TCPServer::STATE_ERR)
        {
            cout << "reconnect...\n" << endl;
            sleep(2);
            continue;
        }

        // read HTTP header
        serverInputConnection.readMsg();
        cout << "Client header:\n" << serverInputConnection.inMsg.data() << endl; // show http header
        serverInputConnection.inMsg.clear();

        Mat inputImage = imread("../there-is-no-connected-camera-mac.jpg");

        cout << "start stream" << endl;

        // send http header
        string headImg;
        headImg += "HTTP/1.0 200 OK\r\n";
        headImg += "Cache-Control: no-cache\r\n";
        headImg += "Pragma: no-cache\r\n";
        headImg += "Connection: close\r\n";
        headImg += "Content-Type: multipart/x-mixed-replace; boundary=jpegboundary\r\n\r\n";


        namedWindow("camera image", WINDOW_NORMAL);
        ////////////////////// send video flow //////////////////////
        bool videoFlow = true;
        while(videoFlow)
        {
            if(camera.read(inputImage))
            {
                cout << "frame not read" << endl;
                waitKey(100);
            }

            imshow("camera image", inputImage);
            waitKey(10);

            vector<unsigned char> jpgBuff;
            jpgBuff.clear();
            std::vector<int> params;
            params.push_back(IMWRITE_JPEG_QUALITY);
            params.push_back(50);

            imencode(".jpg", inputImage, jpgBuff, params);

            // send http separation
            headImg += "--jpegboundary\r\n";
            headImg += "Content-type: image/jpeg\r\n";
            headImg += "Content-length: " + to_string(jpgBuff.size()) + "\r\n\r\n";
            std::vector<char> strBuff(headImg.begin(), headImg.end());


            headImg, serverInputConnection.Msg = strBuff;
            if(serverInputConnection.sendMsg()!= TCPServer::STATE_OK)
            {
                break;
            }

            headImg.clear();

            // send image
            if(serverInputConnection.sendMsg(jpgBuff) != TCPServer::STATE_OK)
            {
                break;
            }

            // delay
            char key = static_cast<char>(waitKey(50));
            if(key == 'q' || key == 'Q')
            {
                videoFlow = false;
            }
        }

        if(!videoFlow)
        {
            break;
        }
        //serverInputConnection.disconnect();
    }
    ////////////////////// reconnect network cycle //////////////////////

    camera.release();

    cout << "close srv" << endl;
    return 0;
}


