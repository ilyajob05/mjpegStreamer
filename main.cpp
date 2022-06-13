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


static void show_usage(std::string name)
{
    std::cout << "Usage: \n"
              << "\tmjpegStreamer -src 0 -port 8080 -quality [1-10] -fps 10"
              << std::endl;
}

int main(int argc, char* argv[])
{
    int src_cam, port, fps, quality, size_img, h_img, w_img;
    src_cam = 0;
    port = 8080;
    quality = 5;
    h_img = 0;
    w_img = 0;
    fps = 20;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if ((arg == "-h") || (arg == "--help")) {
            show_usage(argv[0]);
            return 0;
        } else if (arg == "-src") {
            if (i + 1 < argc) {
                //                i++;
                src_cam = stoi(argv[++i]);
            } else {
                std::cerr << "--destination option requires one argument." << std::endl;
                return 1;
            }
        } else if (arg == "-port") {
            if (i + 1 < argc) {
                port = stoi(argv[++i]);
            } else {
                std::cerr << "--destination option requires one argument." << std::endl;
                return 1;
            }
        } else if (arg == "-quality") {
            if (i + 1 < argc) {
                quality = stoi(argv[++i]);
            } else {
                std::cerr << "--destination option requires one argument." << std::endl;
                return 1;
            }
        } else if (arg == "-size_img") {
            if (i + 1 < argc) {
                h_img = stoi(argv[++i]);
                w_img = stoi(argv[++i]);
            } else {
                std::cerr << "--destination option requires one argument." << std::endl;
                return 1;
            }
        } else if (arg == "-fps") {
            if (i + 1 < argc) {
                fps = stoi(argv[++i]);
            } else {
                std::cerr << "--destination option requires one argument." << std::endl;
                return 1;
            }
        }
    }

    int frame_delay_ms = 1000 / fps;
    cv::VideoCapture camera;
    camera.open(src_cam);

    if (!camera.isOpened()) {
        cout << "not camera connect" << endl;
        return 0;
    }

    TCPServer serverInputConnection("127.0.0.1", port);
    serverInputConnection.createListener();

    ////////////////////// reconnect network cycle //////////////////////
    while (1) {
        if (serverInputConnection.waitConnect() == TCPServer::STATE_ERR) {
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

        while (videoFlow) {
            if (camera.read(inputImage)) {
                waitKey(1);
            }

            imshow("camera image", inputImage);
            waitKey(10);
            vector<unsigned char> jpgBuff;
            jpgBuff.clear();
            std::vector<int> params;
            params.push_back(IMWRITE_JPEG_QUALITY);
            params.push_back(quality * 10);
            imencode(".jpg", inputImage, jpgBuff, params);
            // send http separation
            headImg += "--jpegboundary\r\n";
            headImg += "Content-type: image/jpeg\r\n";
            headImg += "Content-length: " + to_string(jpgBuff.size()) + "\r\n\r\n";
            std::vector<char> strBuff(headImg.begin(), headImg.end());
            serverInputConnection.Msg = strBuff;

            if (serverInputConnection.sendMsg() != TCPServer::STATE_OK) {
                break;
            }

            headImg.clear();

            // send image
            if (serverInputConnection.sendMsg(jpgBuff) != TCPServer::STATE_OK) {
                break;
            }

            // delay
            char key = static_cast<char>(waitKey(frame_delay_ms));

            if (key == 'q' || key == 'Q') {
                videoFlow = false;
            }
        }

        if (!videoFlow) {
            break;
        }

        //serverInputConnection.disconnect();
    }

    ////////////////////// reconnect network cycle //////////////////////
    camera.release();
    cout << "close srv" << endl;
    return 0;
}


