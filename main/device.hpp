#ifndef CADMIUM_EXAMPLE_GPT_Device_HPP_
#define CADMIUM_EXAMPLE_GPT_Device_HPP_

#include <include/cadmium/modeling/devs/atomic.hpp>
#include <include/esp32/communication.hpp>
#include <iostream>
#include <string>

using std::to_string;
using std::string;

static const char *TAG = "example";

#define BLINK_GPIO CONFIG_BLINK_GPIO

static uint8_t s_led_state = 0;

static led_strip_handle_t led_strip;

static void blink_led(void)
{
    /* If the addressable LED is enabled */
    if (s_led_state) {
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        led_strip_set_pixel(led_strip, 0, 16, 16, 16);
        /* Refresh the strip to send data */
        led_strip_refresh(led_strip);
    } else {
        /* Set all LED off to clear all pixels */
        led_strip_clear(led_strip);
    }
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

enum DevicePhase {
	AWAITING_REQUEST,
	TESTING_LOG
	// TESTING_AP
};

namespace cadmium::example::gpt {
	//! Class for representing the Device DEVS model state.
	struct DeviceState {
		double clock;
		double sigma;
		DevicePhase phase;

		// Testing Log
		int testingLogDuration;
		std::string testingLogData;
		DeviceState(DevicePhase phase): clock(), sigma(), phase(phase) {}
	};

	std::ostream& operator<<(std::ostream& out, const DeviceState& s) {
		return out;
	}

	class Device : public Atomic<DeviceState> {

	 public:
		Port<std::string> inData;
		ESP32COM<Device> esp32COM;

		Device(const std::string& id): 
					Atomic<DeviceState>(id, DeviceState(DevicePhase::AWAITING_REQUEST)),
					esp32COM("tag", "Init message", this)
		{
				inData = addInPort<std::string>("inData");
    			gpio_reset_pin((gpio_num_t)BLINK_GPIO);
  			    gpio_set_direction((gpio_num_t)BLINK_GPIO, GPIO_MODE_OUTPUT);
		}

		void handleCommand(DeviceState& s, double e, std::string command, std::string parameters) const {
			if(command == "LOG") {
				s.phase = DevicePhase::TESTING_LOG;
				size_t interval = std::stoi(parameters.substr(0, parameters.find("/")));
				std::string msg = parameters.substr(parameters.find("/") + 1, parameters.size());
				s.testingLogDuration = interval;
				s.sigma = 1;
				s.testingLogData = msg;
			} else if (false) {
				// ..
			} else {
				esp32COM.log("<< ERROR: Invalid command! >> ");
			}
		}

		void internalTransition(DeviceState& s) const override {
			switch(s.phase) {
				case DevicePhase::TESTING_LOG:
					if(s.testingLogDuration == 0) { 
						s.phase = DevicePhase::AWAITING_REQUEST;
						esp32COM.log("Done testing. Waiting for a new request.");

					} else {
						s.testingLogDuration -= 1;
						esp32COM.log(s.testingLogData.c_str());
					}
					s.sigma = 1;
					break;
				default:
					s.sigma = 1;
					break;
			}
		}
		
		void externalTransition(DeviceState& s, double e) const override {
			if (!inData->empty()) {
				std::string data = inData->getBag().back();
				std::string command, parameters;
				command = data.substr(0, data.find(':'));
				parameters = data.substr(data.find(':')+1, data.size());
				esp32COM.log("<< Command received >> ");
				esp32COM.log(command.c_str());
				esp32COM.log("<< Parameters >> ");
				esp32COM.log(parameters.c_str());
				handleCommand(s, e, command, parameters);
			}
		}

		void output(const DeviceState& s) const override {
			const char * msg = "Turning the LED";
        	blink_led();
			s_led_state = !s_led_state;
			vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
		}

		[[nodiscard]] double timeAdvance(const DeviceState& s) const override {
			return s.sigma;
		}
	};
}

#endif
