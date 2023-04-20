/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include <include/cadmium/simulation/rt_root_coordinator.hpp>
#include <include/cadmium/simulation/rt_clock/chrono.hpp>

#include <iostream>
#include <limits>
#include <string>
#include "top.hpp"

using namespace cadmium::iot;

extern "C" void app_main()
{
    auto model = std::make_shared<TopLevelModel>("top");
    cadmium::ChronoClock clock;
    auto realTimeRootCoordinator = cadmium::RealTimeRootCoordinator<cadmium::ChronoClock<std::chrono::steady_clock>>(model, clock);
    // And finally, we run the simulation
    realTimeRootCoordinator.start();
    realTimeRootCoordinator.simulate(std::numeric_limits<double>::infinity());
    realTimeRootCoordinator.stop();
}
