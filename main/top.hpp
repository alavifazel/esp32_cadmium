#ifndef CADMIUM_EXAMPLE_TOP_LEVEL_HPP_
#define CADMIUM_EXAMPLE_TOP_LEVEL_HPP_

#include <include/cadmium/modeling/devs/coupled.hpp>
#include "device.hpp"

namespace cadmium::iot {

	struct TopLevelModel : public Coupled {

		TopLevelModel(const std::string& id) : Coupled(id) {
			auto dev = addComponent<Device>("device");
		}
	};
}

#endif
