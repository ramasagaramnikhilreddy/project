#include <iostream>
#include <unistd.h>
#include "AnalogIn.h" // Molloy's AnalogIn class

int main() {
            AnalogIn tempSensor(0); // AIN0 = P9_39
                while (true) {
                                float tempValue = tempSensor.getVoltage() * 100; // LM35 gives 10mV per °C
                                        std::string command = "curl -X POST -F \"value=" + std::to_string(tempValue) +
                                                                              "\" https://io.adafruit.com/api/v2/nikhil_27/feeds/room-temperature/data?X-AIO-Key=aio_YdXq19sTNJSf9UXAhRr8ghgv9dZF";
                                                system(command.c_str());
                                                        sleep(5);
                                                            }
                    return 0;
}
