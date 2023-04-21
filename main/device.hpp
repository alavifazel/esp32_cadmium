#ifndef CADMIUM_EXAMPLE_GPT_Device_HPP_
#define CADMIUM_EXAMPLE_GPT_Device_HPP_

#include <include/cadmium/modeling/devs/atomic.hpp>
#include <include/esp32/communication.hpp>
#include <iostream>
#include <string>

using std::string;
using std::to_string;

enum DevicePhase
{
    IDLE,
    ROTATING_CW,
    ROTATING_CCW
};

namespace cadmium::iot
{
    struct DeviceState
    {
        double clock;
        double sigma;
        DevicePhase phase;
        int testingLogDuration;
        std::string testingLogData;
        DeviceState(DevicePhase phase) : clock(), sigma(), phase(phase) {}
    };

    std::ostream &operator<<(std::ostream &out, const DeviceState &s)
    {
        return out;
    }

    class Device : public Atomic<DeviceState>
    {
    public:
        ESP32COM esp32COM;
        Port<std::string> inData;
        DeviceState &getState() { return state; }

        Device(const std::string &id) : Atomic<DeviceState>(id, DeviceState(DevicePhase::IDLE)), esp32COM("Device")

        {
			inData = addInPort<std::string>("inData");            
            esp32COM.log("State: IDLE");
        }

        void internalTransition(DeviceState &s) const override
        {
            esp32COM.log("State: IDLE");
            s.phase = IDLE;
            s.sigma = 1;
        }

        void externalTransition(DeviceState &s, double e) const override
        {

            std::string data = inData->getBag().back();
            if (data == "ROTATE_CW")
            {
                s.phase = ROTATING_CW;
                esp32COM.log("State: ROTATING_CW");
                s.sigma = 10;
            }
            if (data == "ROTATE_CCW")
            {
                esp32COM.log("State: ROTATING_CCW");
                s.phase = ROTATING_CCW;
                s.sigma = 10;
            }

        }

        void output(const DeviceState &s) const override {}

        [[nodiscard]] double timeAdvance(const DeviceState &s) const override
        {
            return s.sigma;
        }
    };
}

#endif