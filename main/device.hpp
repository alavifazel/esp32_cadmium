#ifndef CADMIUM_EXAMPLE_GPT_Device_HPP_
#define CADMIUM_EXAMPLE_GPT_Device_HPP_

#include <include/cadmium/modeling/devs/atomic.hpp>
#include <include/esp32/communication.hpp>
#include <iostream>
#include <string>

using std::to_string;
using std::string;

enum DevicePhase {
	AWAITING_REQUEST,
	TESTING_LOG
	// TESTING_AP
};

namespace cadmium::iot {
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

		}

		[[nodiscard]] double timeAdvance(const DeviceState& s) const override {
			return s.sigma;
		}
	};
}

#endif
