#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>
#include <chrono>
#include <thread>

const std::string AIO_KEY = "aio_WHlO21ha2TKfWd5cOnw3Pdhyvhxt";  // Replace with your Adafruit IO Key
const std::string FEED_NAME = "led-toggle";  // Feed name

// Function to send data to Adafruit IO
void sendToAdafruitIO(const std::string& value) {
    std::string command = "curl -s -X POST "
                          "https://io.adafruit.com/api/v2/nikhil_27/feeds/" + FEED_NAME +
                          "/data "
                          "-H 'Content-Type: application/json' "
                          "-H 'X-AIO-Key: " + AIO_KEY + "' "
                          "-d '{\"value\": \"" + value + "\"}'";

    system(command.c_str());
}

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

// Function to get temperature value from Adafruit IO feed
float getTemperatureFromFeed() {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        std::string url = "https://io.adafruit.com/api/v2/nikhil_27/feeds/room-temperature/data/last";
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

    // Example response: {"id":"xxxx","value":"26.5","created_at":"..."}
    size_t pos = readBuffer.find("\"value\":\"");
    if (pos != std::string::npos) {
        size_t start = pos + 9; // length of "value":"
        size_t end = readBuffer.find("\"", start);
        std::string tempStr = readBuffer.substr(start, end - start);
        return std::stof(tempStr);
    }

    return -1000.0f; // Return an impossible temperature on error
}

int main() {
    const int ledGPIO = 60; // GPIO pin for LED
    const int threshold = 85; // Temperature threshold in Celsius

    // Export and set direction for LED GPIO
    exportGPIO(ledGPIO);
    setGPIODirection(ledGPIO, "out");

    std::cout << "Starting LED control based on values..." << std::endl;

    while (true) {
        // Replace hardcoded value with reading from Adafruit
        float temperature = getTemperatureFromFeed();

        if (temperature == -1000.0f) {
            std::cerr << "Failed to read temperature. Retrying..." << std::endl;
        } else {
            if (temperature < threshold) {
                std::cout << "Temperature (" << temperature << "C) below threshold -> LED ON" << std::endl;
                writeGPIOValue(ledGPIO, 1);   // Turn ON LED
                sendToAdafruitIO("ON");       // Also send status to Adafruit
            } else {
                std::cout << "Temperature (" << temperature << "C) above threshold -> LED OFF" << std::endl;
                writeGPIOValue(ledGPIO, 0);   // Turn OFF LED
                sendToAdafruitIO("OFF");      // Also send status to Adafruit
            }
        }

        // Delay
        std::this_thread::sleep_for(std::chrono::seconds(2)); // 2 seconds delay
    }

    return 0;
}
