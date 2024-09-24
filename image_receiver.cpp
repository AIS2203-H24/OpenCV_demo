
#include "simple_socket/TCPSocket.hpp"

#include "network_helper.hpp"
#include <opencv2/opencv.hpp>

int main() {

    simple_socket::TCPClientContext ctx;
    const auto conn = ctx.connect("127.0.0.1", 8080);

    std::vector<unsigned char> sizeBuffer(4);
    conn->readExact(sizeBuffer);

    const auto size = bytes_to_int(sizeBuffer);
    std::vector<unsigned char> data(size);
    conn->readExact(data);

    cv::Mat image = imdecode(data, cv::IMREAD_COLOR);

    imshow("image", image);
    cv::waitKey();
}