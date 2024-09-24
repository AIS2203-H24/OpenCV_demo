
#include "simple_socket/TCPSocket.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "network_helper.hpp"


std::string readFile(const std::filesystem::path &filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Unable to open file");
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string buffer(size, '\0');
    if (!file.read(&buffer[0], size)) {
        throw std::runtime_error("Error reading file");
    }

    return buffer;
}

int main() {

    std::filesystem::path imagePath = "data/images/Lenna.png";
    const auto data = readFile(imagePath);
    const auto size = data.size();
    const auto bytes = int_to_bytes(size);

    std::vector<unsigned char> toSend(data.begin(), data.end());
    toSend.insert(toSend.begin(), bytes.begin(), bytes.end());

    std::cout << "Waiting to connections" << std::endl;
    simple_socket::TCPServer server(8080);
    const auto conn = server.accept();

    conn->write(toSend);

    std::cout << "Done." << std::endl;
}
