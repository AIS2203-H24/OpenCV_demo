
#include <opencv2/opencv.hpp>

using namespace cv;

int main() {

    VideoCapture capture(0);
    if (!capture.isOpened()) {
        std::cerr << "Unable to open camera.." << std::endl;
        return 1;
    }

    const std::string windowTitle{"Display image"};
    namedWindow(windowTitle, WINDOW_AUTOSIZE);

    Mat image;
    bool stop{false};
    while (!stop) {
        capture >> image;

        imshow(windowTitle, image);
        const auto key = waitKey(1);
        if (key == 'q') {
            stop = true;
        }
    }

    return 0;
}
