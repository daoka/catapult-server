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
#include "RootNamespaceHistory.h"
#include "catapult/io/Stream.h"

namespace catapult { namespace state {

	/// Policy for saving and loading root namespace history data without historical information.
	struct RootNamespaceHistoryNonHistoricalSerializer {
		/// Saves \a history to \a output.
		static void Save(const RootNamespaceHistory& history, io::OutputStream& output);

		/// Loads a single value from \a input.
		static RootNamespaceHistory Load(io::InputStream& input);
	};

	/// Policy for saving and loading root namespace history data.
	struct RootNamespaceHistorySerializer {
		/// Saves \a history to \a output.
		static void Save(const RootNamespaceHistory& history, io::OutputStream& output);

		/// Loads a single value from \a input.
		static RootNamespaceHistory Load(io::InputStream& input);
	};
}}
