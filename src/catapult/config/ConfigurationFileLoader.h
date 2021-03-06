/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "PeersConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include <boost/filesystem.hpp>
#include <iostream>

namespace catapult { namespace config {

	/// Loads configuration from \a path using \a loader.
	template<
			typename TConfigurationLoader,
			typename TConfiguration = typename std::result_of<TConfigurationLoader(const std::string&)>::type
	>
	TConfiguration LoadConfiguration(const boost::filesystem::path& path, TConfigurationLoader loader) {
		if (!boost::filesystem::exists(path)) {
			auto message = "aborting load due to missing configuration file";
			CATAPULT_LOG(fatal) << message << ": " << path;
			CATAPULT_THROW_EXCEPTION(catapult_runtime_error(message));
		}

		std::cout << "loading configuration from " << path << std::endl;
		return loader(path.generic_string());
	}

	/// Loads ini configuration from \a path.
	template<typename TConfiguration>
	TConfiguration LoadIniConfiguration(const boost::filesystem::path& path) {
		return LoadConfiguration(path, [](const auto& filePath) {
			return TConfiguration::LoadFromBag(utils::ConfigurationBag::FromPath(filePath));
		});
	}

	/// Loads peers configuration from \a path for network \a networkIdentifier.
	inline
	std::vector<ionet::Node> LoadPeersConfiguration(const boost::filesystem::path& path, model::NetworkIdentifier networkIdentifier) {
		return LoadConfiguration(path, [networkIdentifier](const auto& filePath) {
			return LoadPeersFromPath(filePath, networkIdentifier);
		});
	}
}}
