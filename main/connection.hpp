#ifndef CADMIUM_CONNECTION_HPP_
#define CADMIUM_CONNECTION_HPP_

#include <include/cadmium/modeling/devs/atomic.hpp>
#include <include/esp32/communication.hpp>
#include <iostream>

using std::to_string;
using std::string;

namespace cadmium::example::gpt {
    struct ConnectionState {
		double clock;
        double sigma;
        Phase phase;
        std::string recievedMessage;
		ConnectionState(): clock(), sigma(1), phase(Phase::WAITING_FOR_DATA), recievedMessage{} {}
	};

	std::ostream& operator<<(std::ostream& out, const ConnectionState& s) {
		return out;
	}

class Connection : public Atomic<ConnectionState>, ESP32COM<Connection> {
	
	public:
		Port<std::string> outData;
        ConnectionState& getState() { return state; }
		Connection(const std::string& id)
            : Atomic<ConnectionState>(id, ConnectionState()),
              ESP32COM("tag", "Init message", this)
        {
				WifiAPSetup("my_ssid", "");
				TCPServerSetup();
                outData = addOutPort<std::string>("outData");
		}


        // bool TCPDataIsAvailable() const { return tcpServerState.getDataAvailability(); }
        // std::string getTCPServerData() const { return tcpServerState.getData(); }

		void internalTransition(ConnectionState& s) const override {
            
            switch(s.phase) {
                case WAITING_FOR_DATA:
                    break;
                case DATA_RECEIVED:
                    s.phase = Phase::WAITING_FOR_DATA;
                    break;
            }
            s.sigma = 1;
		}

		void externalTransition(ConnectionState& s, double e) const override {
            log(std::to_string(e).c_str());


        }

		void output(const ConnectionState& s) const override {
            switch(s.phase) {
                case DATA_RECEIVED:
                    outData->addMessage(tcpServerState.getData());
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
