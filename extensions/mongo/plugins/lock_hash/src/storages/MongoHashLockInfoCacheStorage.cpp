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

#include "MongoHashLockInfoCacheStorage.h"
#include "src/mappers/HashLockInfoMapper.h"
#include "mongo/plugins/lock_shared/src/storages/MongoLockInfoCacheStorageTraits.h"
#include "plugins/txes/lock_hash/src/cache/HashLockInfoCache.h"

namespace catapult { namespace mongo { namespace plugins {

	namespace {
		struct HashLockCacheTraits : public storages::BasicMongoCacheStorageTraits<cache::HashLockInfoCacheDescriptor> {
			static constexpr const char* Collection_Name = "hashLockInfos";
			static constexpr const char* Id_Property_Name = "lock.hash";
		};
	}

	DEFINE_MONGO_FLAT_CACHE_STORAGE(HashLockInfo, MongoLockInfoCacheStorageTraits<HashLockCacheTraits>)
}}}
