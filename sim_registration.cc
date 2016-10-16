#include "sim_registration.h"
#include "Core.h"
#include <map>
#include "iris18.h"
#include "iris16.h"
#include "iris17.h"
#include "iris19.h"

namespace iris {
	static std::map<std::string, std::function<Core*()>> cores = {
		{ "iris19", iris19::newCore },
		{ "iris18", iris18::newCore },
		{ "iris17", iris17::newCore },
		{ "iris16", iris16::newCore },
	};
    Core* getCore(const std::string& name) {
		auto loc = cores.find(name);
		if (loc != cores.end()) {
			return loc->second();
		} else {
			std::stringstream stream;
			stream << "Tried to create a non-existent core: " << name << "!!!";
			throw iris::Problem(stream.str());
		}
    }
    void forEachCoreName(std::function<void(const std::string&)> fn) {
        for (auto const& entry : cores) {
            fn(entry.first);
        }
    }
}
