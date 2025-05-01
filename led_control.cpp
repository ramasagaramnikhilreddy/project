#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <cstdlib>

const std::string AIO_USERNAME = "nikhil_27";  // Your Adafruit IO Username
const std::string AIO_KEY = "aio_yyrs39totqd70LKKjYwYAcmeZegf";  // Your AIO Key
const std::string FEED_NAME = "control-led";  // Your Adafruit IO Feed name
const int ledGPIO = 60;  // GPIO number (P9_12)

// Function to export GPIO if not already exported
void exportGPIO(int gpio) {
    std::ifstream gpioFile("/sys/class/gpio/gpio" + std::to_string(gpio) + "/value");
    if (!gpioFile.good()) {
        std::ofstream exportFile("/sys/class/gpio/export");
        if (exportFile.is_open()) {
            exportFile << gpio;
            exportFile.close();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } else {
            std::cerr << "Error exporting GPIO" << gpio << std::endl;
        }
    }
}

// Function to set GPIO direction
void setGPIODirection(int gpio, const std::string& direction) {
    std::ofstream directionFile("/sys/class/gpio/gpio" + std::to_string(gpio) + "/direction");
    if (directionFile.is_open()) {
        directionFile << direction;
        directionFile.close();
    } else {
        std::cerr << "Error setting direction for GPIO" << gpio << std::endl;
    }
}

// Function to write value to GPIO
void writeGPIOValue(int gpio, int value) {
    std::ofstream valueFile("/sys/class/gpio/gpio" + std::to_string(gpio) + "/value");
    if (valueFile.is_open()) {
        valueFile << value;
        valueFile.close();
    } else {
        std::cerr << "Error writing value to GPIO" << gpio << std::endl;
    }
}

// Function to read from Adafruit IO feed
std::string readFromAdafruitIO() {
    std::string command = "curl -s "
                          "https://io.adafruit.com/api/v2/" + AIO_USERNAME + "/feeds/" + FEED_NAME + "/data/last "
                          "-H 'X-AIO-Key: " + AIO_KEY + "'";
    std::string result;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Failed to run curl command." << std::endl;
        return "";
    }
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);

    // Parse result to extract the value field
    size_t pos = result.find("\"value\":\"");
    if (pos != std::string::npos) {
        pos += 9; // length of "\"value\":\""
        size_t endPos = result.find("\"", pos);
        if (endPos != std::string::npos) {
            return result.substr(pos, endPos - pos);
        }
    }
    return "";
}

int main() {
    exportGPIO(ledGPIO);
    setGPIODirection(ledGPIO, "out");

    std::cout << "Monitoring Adafruit IO virtual button to control LED..." << std::endl;

    while (true) {
        std::string value = readFromAdafruitIO();

        if (value == "1") {
            std::cout << "Button ON -> Turning LED OFF" << std::endl;
            writeGPIOValue(ledGPIO, 0);  // 0 = LED ON (active low)
        } else if (value == "0") {
            std::cout << "Button OFF -> Turning LED ON" << std::endl;
            writeGPIOValue(ledGPIO, 1);  // 1 = LED OFF (active low)
        } else {
            std::cerr << "Invalid response from Adafruit IO: " << value << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait 2 seconds before checking again
    }

    return 0;
}
