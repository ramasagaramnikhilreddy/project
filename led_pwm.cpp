#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>

const std::string AIO_KEY = "aio_DFcd99sXlMLaIW8IODbRoGiHuWbg";
const std::string FEED_URL = "https://io.adafruit.com/api/v2/nikhil_27/feeds/led-pwm/data/last";
const std::string PWM_CHIP = "/sys/class/pwm/pwmchip1";
const std::string PWM_PATH = "/sys/class/pwm/pwm-0:0";
const int PERIOD = 1000000; // 1ms = 1kHz

int getBrightness() {
    FILE* pipe = popen(("curl -s -H \"X-AIO-Key: " + AIO_KEY + "\" " + FEED_URL).c_str(), "r");
    if (!pipe) return 0;
    char buffer[128];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe)) result += buffer;
    pclose(pipe);

    size_t pos = result.find("\"value\":\"");
    if (pos == std::string::npos) return 0;
    pos += 9;
    size_t end = result.find("\"", pos);
    if (end == std::string::npos) return 0;
    return std::stoi(result.substr(pos, end - pos));
}

void writeToFile(const std::string& path, const std::string& value) {
    std::ofstream fs(path);
    if (fs.is_open()) {
        fs << value;
        fs.close();
    } else {
        std::cerr << "Failed to write to " << path << std::endl;
    }
}

int main() {
    system("config-pin P9_14 pwm");
    writeToFile(PWM_CHIP + "/unexport", "0");
    sleep(1);
    writeToFile(PWM_CHIP + "/export", "0");
    sleep(1);

    writeToFile(PWM_PATH + "/period", std::to_string(PERIOD));
    writeToFile(PWM_PATH + "/duty_cycle", "0");
    writeToFile(PWM_PATH + "/enable", "1");

    std::cout << "Monitoring Adafruit IO Slider to control LED Brightness...\n";

    while (true) {
        int brightness = getBrightness();
        int duty = (brightness * PERIOD) / 100;
        writeToFile(PWM_PATH + "/duty_cycle", std::to_string(duty));
        std::cout << "Brightness: " << brightness << "%\n";
        sleep(1);
    }

    return 0;
}
