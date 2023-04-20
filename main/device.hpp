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
    ROTATING_LEFT,
    ROTATING_RIGHT
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
        bool handledSigma;
        DeviceState(DevicePhase phase) : clock(), sigma(), phase(phase), handledSigma(false) {}
    };

    std::ostream &operator<<(std::ostream &out, const DeviceState &s)
    {
        return out;
    }

    class Device : public Atomic<DeviceState>
    {
    public:
        ESP32COM<Device> esp32COM;
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
            if (data == "ROTATE_LEFT")
            {
                s.phase = ROTATING_LEFT;
                esp32COM.log("State: ROTATING_LEFT");
                s.sigma = 10;
            }
            if (data == "ROTATE_RIGHT")
            {
                esp32COM.log("State: ROTATING_RIGHT");
                s.phase = ROTATING_RIGHT;
                s.sigma = 10;
            }

        }

        void output(const DeviceState &s) const override
        {
        }

        [[nodiscard]] double timeAdvance(const DeviceState &s) const override
        {
            return s.sigma;
        }
    };
}

#endif