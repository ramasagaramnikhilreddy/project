#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>
#include <chrono>
#include <thread>

const std::string AIO_KEY = "aio_nGQM985xkxp2UmgSu2Y44XdaGiKl";  // Same Adafruit IO Key
const std::string FEED_NAME = "led-blink";  // Feed name for number input

// Function to export GPIO if not already exported
void exportGPIO(int gpio) {
    std::ifstream gpioFile("/sys/class/gpio/gpio" + std::to_string(gpio) + "/value");
    if (!gpioFile.good()) {
        std::ofstream exportFile("/sys/class/gpio/export");
        if (exportFile.is_open()) {
            exportFile << gpio;
            exportFile.close();
        } else {
            std::cerr << "Error exporting GPIO" << gpio << std::endl;
        }
    }
}

// Function to set GPIO direction (output)
void setGPIODirection(int gpio, const std::string& direction) {
    std::ofstream directionFile("/sys/class/gpio/gpio" + std::to_string(gpio) + "/direction");
    if (directionFile.is_open()) {
        directionFile << direction;
        directionFile.close();
    } else {
        std::cerr << "Error setting direction for GPIO" << gpio << std::endl;
    }
}

// Function to write GPIO value
void writeGPIOValue(int gpio, int value) {
    std::ofstream valueFile("/sys/class/gpio/gpio" + std::to_string(gpio) + "/value");
    if (valueFile.is_open()) {
        valueFile << value;
        valueFile.close();
    } else {
        std::cerr << "Error writing value to GPIO" << gpio << std::endl;
    }
}

// Helper function for libcurl to write response into a std::string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// Function to get blink count from Adafruit IO feed
int getBlinkCountFromFeed() {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        std::string url = "https://io.adafruit.com/api/v2/nikhil_27/feeds/" + FEED_NAME + "/data/last";
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, ("X-AIO-Key: " + AIO_KEY).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        curl_global_cleanup();
    }

    // Example response: {"id":"xxxx","value":"5","created_at":"..."}
    size_t pos = readBuffer.find("\"value\":\"");
    if (pos != std::string::npos) {
        size_t start = pos + 9; // length of "value":"
        size_t end = readBuffer.find("\"", start);
        std::string countStr = readBuffer.substr(start, end - start);
        try {
            return std::stoi(countStr);
        } catch (...) {
            std::cerr << "Error converting feed value to integer!" << std::endl;
            return 0;
        }
    }

    return 0; // Return 0 on error
}

int main() {
    const int ledGPIO = 60; // Same onboard LED pin

    // Export and set direction for LED GPIO
    exportGPIO(ledGPIO);
    setGPIODirection(ledGPIO, "out");

    std::cout << "Starting LED blink control based on feed..." << std::endl;

    while (true) {
        int blinkCount = getBlinkCountFromFeed();

        if (blinkCount <= 0) {
            std::cerr << "Invalid blink count (" << blinkCount << "). Retrying..." << std::endl;
        } else {
            std::cout << "Blinking LED " << blinkCount << " times..." << std::endl;
            for (int i = 0; i < blinkCount; ++i) {
                writeGPIOValue(ledGPIO, 1); // LED ON
                std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 500 ms ON
                writeGPIOValue(ledGPIO, 0); // LED OFF
                std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 500 ms OFF
            }
        }

        // Small delay before next check
        std::this_thread::sleep_for(std::chrono::seconds(5)); // 5 seconds delay
    }

    return 0;
}
