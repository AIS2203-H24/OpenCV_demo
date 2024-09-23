
#include <opencv2/opencv.hpp>
#include <thread>

using namespace cv;

namespace {
    bool windowClosed(const std::string &windowTitle) {
        return getWindowProperty(windowTitle, WND_PROP_VISIBLE) < 1;
    }

    std::vector<Rect> faceDetect(const Mat &image) {
        Mat gray;
        cvtColor(image, gray, COLOR_BGR2GRAY);
        CascadeClassifier faceCascade;
        if (!faceCascade.load("data/haarcascades/haarcascade_frontalface_default.xml")) {
            throw std::runtime_error("failed to load face-cascade classifier");
        }
        std::vector<Rect> faces;
        faceCascade.detectMultiScale(
                gray,
                faces,
                1.1,// scaleFactor (Specifies how much the image size is reduced at each scale (e.g., 1.1 means 10% reduction))
                5,  // minNeighbors (Determines how many neighbors each candidate rectangle should have to retain it. Higher values result in fewer detections but with higher quality.)
                0 | CASCADE_SCALE_IMAGE,
                Size(30, 30)// minSize (Minimum possible object size. Objects smaller than this are ignored.)
        );

        return faces;
    }

    void drawFaces(Mat &image, const std::vector<Rect> &faces) {
        for (const auto &face: faces) {
            const Point top_left(face.x, face.y);
            const Point bottom_right(face.x + face.width, face.y + face.height);
            rectangle(image, top_left, bottom_right, Scalar(0, 255, 0), 2);
        }
    }

}// namespace

int main() {

    VideoCapture capture(0);
    if (!capture.isOpened()) {
        std::cerr << "Unable to open camera.." << std::endl;
        return 1;
    }

    const std::string windowTitle{"Display image"};
    namedWindow(windowTitle, WINDOW_AUTOSIZE);

    std::mutex imageMutex, facesMutex;

    Mat image;
    std::atomic_bool stop{false};
    std::vector<Rect> faces;
    std::atomic_bool faceDetected{false};

    // WARNING, while this works and seems to be thread safe, it is not a pretty solution

    std::jthread detectionThread([&] {
        while (!stop) {

            std::unique_lock lck(imageMutex);
            if (image.empty()) continue;
            Mat faceImage = image.clone();
            lck.unlock();

            const auto faceResult = faceDetect(faceImage);
            std::lock_guard lck2(facesMutex);
            faces = faceResult;
            faceDetected = true;
        }
    });

    std::vector<Rect> facesCopy;

    while (!stop) {

        std::unique_lock lck(imageMutex);
        capture >> image;
        lck.unlock();

        if (faceDetected) {
            std::unique_lock lck2(facesMutex);
            facesCopy = faces;
            lck2.unlock();

            faceDetected = false;
        }

        drawFaces(image, facesCopy);

        imshow(windowTitle, image);

        const auto key = waitKey(1);
        if (windowClosed(windowTitle) || key == 'q') {
            stop = true;
        }
    }

    return 0;
}
