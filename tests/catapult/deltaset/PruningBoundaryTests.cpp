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

#include "catapult/deltaset/PruningBoundary.h"
#include "tests/TestHarness.h"

namespace catapult { namespace deltaset {

#define TEST_CLASS PruningBoundaryTests

	TEST(TEST_CLASS, CanCreateUnsetPruningBoundary) {
		// Act:
		PruningBoundary<int> boundary;

		// Assert:
		EXPECT_FALSE(boundary.isSet());
	}

	TEST(TEST_CLASS, CanCreateUnsetPruningBoundary_SharedPtr) {
		// Act:
		PruningBoundary<std::shared_ptr<int>> boundary;

		// Assert:
		EXPECT_FALSE(boundary.isSet());
	}

	TEST(TEST_CLASS, CanCreatePruningBoundaryWithValue) {
		// Act:
		PruningBoundary<int> boundary(17);

		// Assert:
		EXPECT_TRUE(boundary.isSet());
		EXPECT_EQ(17, boundary.value());
	}

	TEST(TEST_CLASS, CanCreatePruningBoundaryWithValue_SharedPtr) {
		// Act:
		PruningBoundary<std::shared_ptr<int>> boundary(std::make_shared<int>(17));

		// Assert:
		EXPECT_TRUE(boundary.isSet());
		EXPECT_EQ(17, *boundary.value());
	}
}}
