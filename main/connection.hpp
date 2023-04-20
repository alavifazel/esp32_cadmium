#ifndef CADMIUM_CONNECTION_HPP_
#define CADMIUM_CONNECTION_HPP_

#include <include/cadmium/modeling/devs/atomic.hpp>
#include <include/esp32/communication.hpp>
#include <iostream>

using std::to_string;
using std::string;

namespace cadmium::iot {
    struct ConnectionState {
		double clock;
        double sigma;
        Phase phase;
        std::string recievedMessage;
        bool dataAvailable;
        void setDataAvailable(bool b) { this->dataAvailable = b; }
        bool getDataAvailable() { return this->dataAvailable; }

		ConnectionState(): clock(), sigma(1), phase(Phase::WAITING_FOR_DATA), recievedMessage{} {}
	};

	std::ostream& operator<<(std::ostream& out, const ConnectionState& s) {
		return out;
	}

class Connection : public Atomic<ConnectionState> {
	
	public:
        ESP32COM <Connection> esp32COM;

		Port<std::string> outData;
        ConnectionState& getState() { return state; }
		Connection(const std::string& id)
            : Atomic<ConnectionState>(id, ConnectionState()), esp32COM("ESP")
        {
                outData = addOutPort<std::string>("outData");

                esp32COM.setupConnection(this, "Device", "my_ssid", "", 5000);

		}


        // bool TCPDataIsAvailable() const { return tcpServerState.getDataAvailability(); }
        // std::string getTCPServerData() const { return tcpServerState.getData(); }

		void internalTransition(ConnectionState& s) const override {
            if(s.getDataAvailable()) {
                s.phase = DATA_RECEIVED;
                s.setDataAvailable(false);
            } else {
                s.phase = WAITING_FOR_DATA;
            }
            s.sigma = .1;
		}

		void externalTransition(ConnectionState& s, double e) const override {}

		void output(const ConnectionState& s) const override {
            switch(s.phase) {
                case DATA_RECEIVED:
                    outData->addMessage(esp32COM.tcpServerState.getData());
                    break;
                default:
                    break;
            }
		}

		[[nodiscard]] double timeAdvance(const ConnectionState& s) const override {
			return s.sigma;
		}
	};
    
}

#endif