#ifndef CADMIUM_EXAMPLE_TOP_LEVEL_HPP_
#define CADMIUM_EXAMPLE_TOP_LEVEL_HPP_

#include <include/cadmium/modeling/devs/coupled.hpp>
#include "device.hpp"
#include "connection.hpp"

namespace cadmium::iot {

	struct TopLevelModel : public Coupled {

		TopLevelModel(const std::string& id) : Coupled(id) {
			auto conn = addComponent<Connection>("connection");
			auto dev = addComponent<Device>("device");
			addCoupling(conn->outData, dev->inData);
		}
	};
}

#endif
