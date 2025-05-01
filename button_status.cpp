#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>  // For system()
#include <chrono>
#include <thread>

const std::string AIO_KEY = "aio_mcAE67q6usCLUqrpf9Yhn3J4053V";  // Your AIO key
const std::string FEED_NAME = "button-status";  // Adafruit IO Feed name
const int buttonGPIO = 46;  // GPIO number for button (P8_16)

// Function to send status to Adafruit IO
void sendToAdafruitIO(const std::string& value) {
    std::string command = "curl -s -X POST "
                          "https://io.adafruit.com/api/v2/nikhil_27/feeds/" + FEED_NAME +
                          "/data "
                          "-H 'Content-Type: application/json' "
                          "-H 'X-AIO-Key: " + AIO_KEY + "' "
                          "-d '{\"value\": \"" + value + "\"}'";
    system(command.c_str());
}

// Function to export GPIO if needed
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

// Function to read GPIO value
int readGPIOValue(int gpio) {
    std::ifstream valueFile("/sys/class/gpio/gpio" + std::to_string(gpio) + "/value");
    int value = -1;
    if (valueFile.is_open()) {
        valueFile >> value;
        valueFile.close();
    } else {
        std::cerr << "Error reading GPIO" << gpio << std::endl;
    }
    return value;
}

int main() {
    exportGPIO(buttonGPIO);
    setGPIODirection(buttonGPIO, "in");

    std::cout << "Monitoring button press (window sensor) every 0.2 seconds..." << std::endl;

    int lastValue = -1;  // To track last sent value to avoid repeated sending

    while (true) {
        int value = readGPIOValue(buttonGPIO);

        if (value != lastValue) {  // Only send if value changed
            if (value == 0) {
                std::cout << "[Window Closed] Button Pressed" << std::endl;
                sendToAdafruitIO("0");
            } else if (value == 1) {
                std::cout << "[Window Open] Button Released" << std::endl;
                sendToAdafruitIO("1");
            } else {
                std::cerr << "Invalid GPIO value!" << std::endl;
            }
            lastValue = value;
        } else {
            // Show the current status even if not changed
            if (value == 0) {
                std::cout << "[Still Closed]" << std::endl;
            } else if (value == 1) {
                std::cout << "[Still Open]" << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));  // Wait 0.2 seconds
    }

    return 0;
}
