#ifndef CADMIUM_EXAMPLE_GPT_Device_HPP_
#define CADMIUM_EXAMPLE_GPT_Device_HPP_

#include <include/cadmium/modeling/devs/atomic.hpp>
#include <include/esp32/communication.hpp>
#include <iostream>
#include <string>

using std::to_string;
using std::string;


enum DevicePhase {
	IDLE,
	ROTATING_LEFT,
	ROTATING_RIGHT
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
		ESP32COM<Device> esp32COM;
		
        DeviceState& getState() { return state; }

		Device(const std::string& id): 
					Atomic<DeviceState>(id, DeviceState(DevicePhase::IDLE)),
					esp32COM(this, "Device", "my_ssid", "", 5000)

		{
			esp32COM.log("State: IDLE");
		}

		void internalTransition(DeviceState& s) const override {
                                esp32COM.log("State: IDLE");
                    s.phase = DevicePhase::IDLE;
                    s.sigma = 10;
            // switch(s.phase) {
            //     case IDLE:
            //         esp32COM.log("State: IDLE");
            //         s.phase = DevicePhase::IDLE;
            //         s.sigma = 1;
            //         break;
            //     case ROTATING_LEFT:
            //         esp32COM.log("State: ROTATING_LEFT. Duration = 5s");
            //         break;
            //     case ROTATING_RIGHT:
            //         esp32COM.log("State: ROTATING_RIGHT. Duration = 5s");
            //         s.sigma = 5;
            //         break;                    


            // }
		}
		
		void externalTransition(DeviceState& s, double e) const override {
            std::string data = esp32COM.tcpServerState.getData();
            if(data == "ROTATE_LEFT") {
                s.phase = ROTATING_LEFT;
                s.sigma = 5;

            } else if (data == "ROTATE_RIGHT") {
                s.phase = ROTATING_RIGHT;
                s.sigma = 5;
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