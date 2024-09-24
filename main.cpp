
#include <future>
#include <opencv2/opencv.hpp>
#include <thread>

using namespace cv;

namespace {
    bool windowClosed(const std::string &windowTitle) {
        return getWindowProperty(windowTitle, WND_PROP_VISIBLE) < 1;
    }

    template<typename R>
    bool isReady(std::future<R> const &f) { return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }

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

    class FaceDetector {
    public:
        void start() {
            thread_ = std::jthread(&FaceDetector::run, this);
        }

        void stop() {
            stop_ = true;
            cv_.notify_one();
        }

        std::future<std::vector<Rect>> process(const Mat &image) {

            std::lock_guard lock(m_);
            image_ = image.clone();
            imageToProcess = true;

            cv_.notify_one();

            promise_ = std::promise<std::vector<Rect>>();
            return promise_.get_future();
        }

    private:
        Mat image_;
        std::mutex m_;

        std::condition_variable cv_;
        std::atomic_bool stop_{false};
        std::atomic_bool imageToProcess{false};

        std::promise<std::vector<Rect>> promise_;

        std::jthread thread_;

        void run() {
            while (!stop_) {

                std::unique_lock lock(m_);
                cv_.wait(lock, [this] {
                    return imageToProcess.load() || stop_.load();
                });

                if (stop_) break;

                const auto faces = faceDetect(image_);
                promise_.set_value(faces);

                imageToProcess = false;
            }
        }
    };

}// namespace

int main() {

    VideoCapture capture(0);
    if (!capture.isOpened()) {
        std::cerr << "Unable to open camera.." << std::endl;
        return 1;
    }

    const std::string windowTitle{"Display image"};
    namedWindow(windowTitle, WINDOW_AUTOSIZE);

    FaceDetector faceDetector;
    faceDetector.start();

    Mat image;
    bool stop{false};
    std::future<std::vector<Rect>> faces;
    std::vector<Rect> faceResult_;
    while (!stop) {

        capture >> image;

        if (!faces.valid()) {
            faces = faceDetector.process(image);
        }

        if (isReady(faces)) {
            const auto result = faces.get();
            if (!result.empty()) faceResult_ = result;
            faces = {};
        }

        drawFaces(image, faceResult_);

        imshow(windowTitle, image);

        const auto key = waitKey(1);
        if (windowClosed(windowTitle) || key == 'q') {
            stop = true;
            faceDetector.stop();
        }
    }

    return 0;
}
