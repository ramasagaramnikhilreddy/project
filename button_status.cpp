#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <chrono>
#include <thread>

const std::string AIO_KEY = "aio_UVAf80DCHDGn48LjP7aLODRHpMFN";
const std::string FEED_NAME = "button-status";
const int GPIO_NUM = 46; // P8_16
const std::string GPIO_PATH = "/sys/class/gpio/gpio" + std::to_string(GPIO_NUM);

// Run a shell command
void runCommand(const std::string& command) {
    int ret = system(command.c_str());
    if (ret != 0) {
        std::cerr << "Command failed: " << command << std::endl;
    }
}

// Export GPIO if not already exported
void setupGPIO() {
    std::ifstream check(GPIO_PATH + "/value");
    if (!check.good()) {
        std::ofstream exportFile("/sys/class/gpio/export");
        exportFile << GPIO_NUM;
        exportFile.close();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::ofstream dirFile(GPIO_PATH + "/direction");
    dirFile << "in";
    dirFile.close();

    // Configure pin mode to gpio with pull-up
    runCommand("config-pin P8_16 gpio_pu");
}

// Read GPIO value
int readGPIO() {
    std::ifstream valueFile(GPIO_PATH + "/value");
    int value = -1;
    if (valueFile.is_open()) {
        valueFile >> value;
        valueFile.close();
    }
    return value;
}

// Send to Adafruit IO
void sendToAdafruitIO(const std::string& value) {
    std::string command = "curl -s -X POST "
                          "https://io.adafruit.com/api/v2/nikhil_27/feeds/" + FEED_NAME +
                          "/data "
                          "-H 'Content-Type: application/json' "
                          "-H 'X-AIO-Key: " + AIO_KEY + "' "
                          "-d '{\"value\": \"" + value + "\"}'";
    runCommand(command);
}

int main() {
    setupGPIO();
    std::cout << "Monitoring push button to detect window status..." << std::endl;

    int lastValue = -1;

    while (true) {
        int value = readGPIO();

        if (value != lastValue && (value == 0 || value == 1)) {
            if (value == 0) {
                std::cout << "[Window Closed] Button Pressed" << std::endl;
                sendToAdafruitIO("0");
            } else if (value == 1) {
                std::cout << "[Window Open] Button Released" << std::endl;
                sendToAdafruitIO("1");
            }
            lastValue = value;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    return 0;
}
