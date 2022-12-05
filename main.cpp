/**
 *  @author Derendyaev Ilya, ilyajob05@gmail.com
 *  @date    1.1.2019
 *  TCP receiver and sender
 **/

#include <atomic>
#include <signal.h>
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


std::atomic<int> closeApp{0};
static void handler(int signo)
{
    closeApp = 1;
}


void* listener(TCPServer& srvLst)
{
    // register handler for interrupt "accept()"
    struct sigaction sigact = { 0 };
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigact.sa_flags = sigact.sa_flags | SA_NODEFER | SA_RESETHAND;
    sigact.sa_handler = &handler;

    if (sigaction(SIGUSR1, &sigact, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    //    raise(SIGSEGV);
    //  Mat inputImage = imread("../there-is-no-connected-camera-mac.jpg");
    Mat inputImage = imread("/home/ilya/PRJ/mjpegStreamer/there-is-no-connected-camera-mac.jpg");
    vector<unsigned char> jpgBuff;
    vector<int> params;
    params.push_back(IMWRITE_JPEG_QUALITY);
    params.push_back(50);
    imencode(".jpg", inputImage, jpgBuff, params);
    vector<char> jpgBuffChar(jpgBuff.begin(), jpgBuff.end());

    while (true) {
        auto sc = make_unique<TCPServer::SocketAndState>();
        int socket = srvLst.waitConnect(*sc);

        if (socket == TCPServer::kError) {
            if (closeApp == 1) {
                cout << "listener close\n" << endl;
                return;
            } else {
                cout << "reconnect...\n" << endl;
                sleep(2);
                continue;
            }
        }

        sc->socket = socket;
        sc->state = TCPServer::kSendHeader;
        // read HTTP header
        srvLst.readMsg(*sc);
        cout << "Client header:\n" << sc->inMsg.data() << endl; // show http header
        cout << "start stream" << endl;
        // send http header
        string headImg;
        headImg += "HTTP/1.0 200 OK\r\n";
        headImg += "Cache-Control: no-cache\r\n";
        headImg += "Pragma: no-cache\r\n";
        headImg += "Connection: close\r\n";
        headImg += "Content-Type: multipart/x-mixed-replace; boundary=jpegboundary\r\n\r\n";
        // send http separation
        headImg += "--jpegboundary\r\n";
        headImg += "Content-type: image/jpeg\r\n";
        headImg += "Content-length: " + to_string(jpgBuff.size()) + "\r\n\r\n";
        std::vector<char> strBuff(headImg.begin(), headImg.end());
        sc->outMsg = strBuff;

        if (srvLst.sendMsg(*sc) != TCPServer::kOk) {
            cout << "Err: sendMsg()" << endl;
            break;
        }

        headImg.clear();
        // send image
        sc->outMsg = jpgBuffChar;

        if (srvLst.sendMsg(*sc) != TCPServer::kOk) {
            cout << "Err: sendMsg()" << endl;
            break;
        }

        {
            unique_lock sLock(srvLst.sockListLock);
            srvLst.sockList.push_back(move(sc));
        }
    }
}


void readArgs(int argc, char* argv[], int& src_cam, int& port, int& fps, int& quality, int& size_img, int& h_img, int& w_img)
{
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if ((arg == "-h") || (arg == "--help")) {
            show_usage(argv[0]);
            return;
        } else if (arg == "-src") {
            if (i + 1 < argc) {
                //                i++;
                src_cam = stoi(argv[++i]);
            } else {
                std::cerr << "--destination option requires one argument." << std::endl;
                return;
            }
        } else if (arg == "-port") {
            if (i + 1 < argc) {
                port = stoi(argv[++i]);
            } else {
                std::cerr << "--destination option requires one argument." << std::endl;
                return;
            }
        } else if (arg == "-quality") {
            if (i + 1 < argc) {
                quality = stoi(argv[++i]);
            } else {
                std::cerr << "--destination option requires one argument." << std::endl;
                return;
            }
        } else if (arg == "-size_img") {
            if (i + 1 < argc) {
                h_img = stoi(argv[++i]);
                w_img = stoi(argv[++i]);
            } else {
                std::cerr << "--destination option requires one argument." << std::endl;
                return;
            }
        } else if (arg == "-fps") {
            if (i + 1 < argc) {
                fps = stoi(argv[++i]);
            } else {
                std::cerr << "--destination option requires one argument." << std::endl;
                return;
            }
        }
    }
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
    readArgs(argc, argv, src_cam, port, fps, quality, size_img, h_img, w_img);
    int frame_delay_ms = 1000 / fps;
    cv::VideoCapture camera;
    camera.open(src_cam);

    if (!camera.isOpened()) {
        cout << "not camera connect" << endl;
        return 0;
    }

    TCPServer serverInputConnection("127.0.0.1", port);
    serverInputConnection.emptyImage = cv::imread("../there-is-no-connected-camera-mac.jpg");
    serverInputConnection.createListener();
    thread thListener(listener, ref(serverInputConnection));
    cv::Mat inputImage;
    string headImg;

    while (1) {
        ////////////////////// send video stream //////////////////////
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
            serverInputConnection.sendMsgs(strBuff);
            headImg.clear();
            vector<char> jpgBuffChar(jpgBuff.begin(), jpgBuff.end());
            serverInputConnection.sendMsgs(jpgBuffChar);
            serverInputConnection.clearErrSockets();
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

    // interrupt listener thread
    serverInputConnection.closeSockets();
    pthread_kill(thListener.native_handle(), SIGUSR1);
    thListener.join();
    camera.release();
    cout << "close srv" << endl;
    return 0;
}


