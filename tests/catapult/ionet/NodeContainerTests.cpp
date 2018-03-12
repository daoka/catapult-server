#include "catapult/ionet/NodeContainer.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/nodeps/LockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

#define TEST_CLASS NodeContainerTests

	namespace {
		using BasicNodeDataContainer = test::BasicNodeDataContainer;

		void Add(
				NodeContainer& container,
				const Key& identityKey,
				const std::string& nodeName,
				NodeSource nodeSource,
				NodeRoles roles = NodeRoles::None) {
			container.modifier().add(test::CreateNamedNode(identityKey, nodeName, roles), nodeSource);
		}

		auto SeedThreeNodes(NodeContainer& container) {
			auto keys = test::GenerateRandomDataVector<Key>(3);
			Add(container, keys[0], "bob", NodeSource::Dynamic);
			Add(container, keys[1], "alice", NodeSource::Local);
			Add(container, keys[2], "charlie", NodeSource::Dynamic);
			return keys;
		}

		auto SeedFiveNodes(NodeContainer& container) {
			auto keys = SeedThreeNodes(container);
			keys.push_back(test::GenerateRandomData<Key_Size>());
			keys.push_back(test::GenerateRandomData<Key_Size>());
			Add(container, keys[3], "dolly", NodeSource::Dynamic);
			Add(container, keys[4], "ed", NodeSource::Static);
			return keys;
		}

		void AssertEmpty(const NodeContainer& container) {
			auto view = container.view();
			auto pairs = test::CollectAll(view);

			// Assert:
			EXPECT_EQ(0u, pairs.size());
			EXPECT_TRUE(pairs.empty());
		}
	}

	// region constructor

	TEST(TEST_CLASS, ContainerIsInitiallyEmpty) {
		// Act:
		NodeContainer container;

		// Assert:
		AssertEmpty(container);
	}

	// endregion

	// region contains

	TEST(TEST_CLASS, ContainsReturnsTrueWhenNodeIsKnown) {
		// Arrange:
		NodeContainer container;
		auto keys = test::GenerateRandomDataVector<Key>(5);

		// - seed 10 keys
		for (const auto& key : keys) {
			Add(container, key, "", NodeSource::Dynamic);
			Add(container, test::GenerateRandomData<Key_Size>(), "", NodeSource::Dynamic);
		}

		// Sanity:
		const auto& view = container.view();
		EXPECT_EQ(10u, view.size());

		// Act + Assert:
		for (const auto& key : keys)
			EXPECT_TRUE(view.contains(key));
	}

	TEST(TEST_CLASS, ContainsReturnsFalseWhenNodeIsUnknown) {
		// Arrange:
		NodeContainer container;

		// - seed 10 keys
		for (const auto& key : test::GenerateRandomDataVector<Key>(10))
			Add(container, key, "", NodeSource::Dynamic);

		// Sanity:
		const auto& view = container.view();
		EXPECT_EQ(10u, view.size());

		// Act + Assert:
		for (const auto& key : test::GenerateRandomDataVector<Key>(5))
			EXPECT_FALSE(view.contains(key));
	}

	// endregion

	// region add

	TEST(TEST_CLASS, CanAddSingleNode) {
		// Arrange:
		NodeContainer container;
		auto key = test::GenerateRandomData<Key_Size>();

		// Act:
		Add(container, key, "bob", NodeSource::Dynamic);

		// Assert:
		const auto& view = container.view();
		EXPECT_EQ(1u, view.size());

		auto expectedContents = BasicNodeDataContainer{ { key, "bob", NodeSource::Dynamic } };
		EXPECT_EQ(expectedContents, test::CollectAll(view));
	}

	TEST(TEST_CLASS, CanAddMultipleNodes) {
		// Arrange:
		NodeContainer container;
		auto keys = test::GenerateRandomDataVector<Key>(3);

		// Act:
		Add(container, keys[0], "bob", NodeSource::Dynamic);
		Add(container, keys[1], "alice", NodeSource::Local);
		Add(container, keys[2], "charlie", NodeSource::Dynamic);

		// Assert:
		const auto& view = container.view();
		EXPECT_EQ(3u, view.size());

		auto expectedContents = BasicNodeDataContainer{
			{ keys[0], "bob", NodeSource::Dynamic },
			{ keys[1], "alice", NodeSource::Local },
			{ keys[2], "charlie", NodeSource::Dynamic }
		};
		EXPECT_EQ(expectedContents, test::CollectAll(view));
	}

	TEST(TEST_CLASS, CanPromoteNodeSource) {
		// Arrange:
		NodeContainer container;
		auto key = test::GenerateRandomData<Key_Size>();

		// Act: promote from remote to local
		Add(container, key, "bob", NodeSource::Dynamic);
		Add(container, key, "bob2", NodeSource::Local);

		// Assert: promotion is allowed
		const auto& view = container.view();
		EXPECT_EQ(1u, view.size());

		auto expectedContents = BasicNodeDataContainer{ { key, "bob2", NodeSource::Local } };
		EXPECT_EQ(expectedContents, test::CollectAll(view));
	}

	TEST(TEST_CLASS, CannotDemoteNodeSource) {
		// Arrange:
		NodeContainer container;
		auto key = test::GenerateRandomData<Key_Size>();

		// Act: demote from local to remote
		Add(container, key, "bob", NodeSource::Local);
		Add(container, key, "bob2", NodeSource::Dynamic);

		// Assert: demotion is not allowed
		const auto& view = container.view();
		EXPECT_EQ(1u, view.size());

		auto expectedContents = BasicNodeDataContainer{ { key, "bob", NodeSource::Local } };
		EXPECT_EQ(expectedContents, test::CollectAll(view));
	}

	TEST(TEST_CLASS, NewerDataFromSameSourcePreemptsOlderData) {
		// Arrange:
		NodeContainer container;
		auto key = test::GenerateRandomData<Key_Size>();

		// Act: push a name change from the same source
		Add(container, key, "bob", NodeSource::Static);
		Add(container, key, "bob2", NodeSource::Static);

		// Assert: data from the new source is selected
		const auto& view = container.view();
		EXPECT_EQ(1u, view.size());

		auto expectedContents = BasicNodeDataContainer{ { key, "bob2", NodeSource::Static } };
		EXPECT_EQ(expectedContents, test::CollectAll(view));
	}

	// endregion

	// region getNodeInfo

	TEST(TEST_CLASS, NodeInfoIsInaccessibleForUnknownNode) {
		// Arrange:
		NodeContainer container;
		SeedThreeNodes(container);
		auto otherKey = test::GenerateRandomData<Key_Size>();

		// Act + Assert:
		EXPECT_THROW(container.view().getNodeInfo(otherKey), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, NodeInfoIsInitializedWhenNodeIsAdded) {
		// Arrange:
		NodeContainer container;
		auto key = test::GenerateRandomData<Key_Size>();

		// Act:
		Add(container, key, "bob", NodeSource::Dynamic);

		// Assert:
		const auto& view = container.view();
		EXPECT_EQ(1u, view.size());

		const auto& nodeInfo = view.getNodeInfo(key);
		EXPECT_EQ(NodeSource::Dynamic, nodeInfo.source());
		EXPECT_EQ(0u, nodeInfo.numConnectionStates());
	}

	TEST(TEST_CLASS, NodeInfoStateIsPreservedWhenSourceIsPromoted) {
		// Arrange:
		NodeContainer container;
		auto key = test::GenerateRandomData<Key_Size>();

		// - add an aged connection
		Add(container, key, "bob", NodeSource::Dynamic);
		container.modifier().provisionConnectionState(ServiceIdentifier(123), key).Age = 17;

		// Act: promote the node source
		Add(container, key, "bob", NodeSource::Static);

		// Assert:
		const auto& view = container.view();
		EXPECT_EQ(1u, view.size());

		const auto& nodeInfo = view.getNodeInfo(key);
		EXPECT_EQ(NodeSource::Static, nodeInfo.source());
		EXPECT_EQ(1u, nodeInfo.numConnectionStates());
		EXPECT_EQ(17u, nodeInfo.getConnectionState(ServiceIdentifier(123))->Age);
	}

	// endregion

	// region addConnectionStates

	TEST(TEST_CLASS, AddConnectionStatesHasNoEffectWhenNoExistingNodesHaveRequiredRole) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedThreeNodes(container);

		// Act: add connection states *after* adding nodes
		container.modifier().addConnectionStates(ServiceIdentifier(123), NodeRoles::Api);

		// Assert:
		const auto& view = container.view();
		EXPECT_FALSE(!!view.getNodeInfo(keys[0]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[1]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[2]).getConnectionState(ServiceIdentifier(123)));
	}

	TEST(TEST_CLASS, AddConnectionStatesHasNoEffectWhenNoAddedNodesHaveRequiredRole) {
		// Arrange:
		NodeContainer container;

		// Act: add connection states *before* adding nodes
		container.modifier().addConnectionStates(ServiceIdentifier(123), NodeRoles::Api);
		auto keys = SeedThreeNodes(container);

		// Assert:
		const auto& view = container.view();
		EXPECT_FALSE(!!view.getNodeInfo(keys[0]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[1]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[2]).getConnectionState(ServiceIdentifier(123)));
	}

	namespace {
		auto SeedFiveNodesWithVaryingRoles(NodeContainer& container) {
			auto keys = test::GenerateRandomDataVector<Key>(5);
			Add(container, keys[0], "bob", NodeSource::Dynamic, NodeRoles::Api);
			Add(container, keys[1], "alice", NodeSource::Local, NodeRoles::Peer);
			Add(container, keys[2], "charlie", NodeSource::Dynamic, NodeRoles::None);
			Add(container, keys[3], "dolly", NodeSource::Dynamic, NodeRoles::Api | NodeRoles::Peer);
			Add(container, keys[4], "ed", NodeSource::Static, NodeRoles::Peer);
			return keys;
		}
	}

	TEST(TEST_CLASS, AddConnectionStatesAddsConnectionStatesToExistingNodesThatHaveRequiredRole) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedFiveNodesWithVaryingRoles(container);

		// Act: add connection states *after* adding nodes
		container.modifier().addConnectionStates(ServiceIdentifier(123), NodeRoles::Api);

		// Assert:
		const auto& view = container.view();
		EXPECT_TRUE(!!view.getNodeInfo(keys[0]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[1]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[2]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_TRUE(!!view.getNodeInfo(keys[3]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[4]).getConnectionState(ServiceIdentifier(123)));
	}

	TEST(TEST_CLASS, AddConnectionStatesAddsConnectionStatesToAddedNodesThatHaveRequiredRole) {
		// Arrange:
		NodeContainer container;

		// Act: add connection states *before* adding nodes
		container.modifier().addConnectionStates(ServiceIdentifier(123), NodeRoles::Api);
		auto keys = SeedFiveNodesWithVaryingRoles(container);

		// Assert:
		const auto& view = container.view();
		EXPECT_TRUE(!!view.getNodeInfo(keys[0]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[1]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[2]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_TRUE(!!view.getNodeInfo(keys[3]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!view.getNodeInfo(keys[4]).getConnectionState(ServiceIdentifier(123)));
	}

	TEST(TEST_CLASS, AddConnectionStatesAddsConnectionStatesToAddedNodesThatHaveUpgradedAndChangedRole) {
		// Arrange:
		NodeContainer container;

		// - add a service for api roles
		container.modifier().addConnectionStates(ServiceIdentifier(123), NodeRoles::Api);

		// - add a node that does not have matching roles
		auto key = test::GenerateRandomData<Key_Size>();
		Add(container, key, "bob", NodeSource::Dynamic, NodeRoles::Peer);

		// Sanity: the connection state is not present
		EXPECT_FALSE(!!container.view().getNodeInfo(key).getConnectionState(ServiceIdentifier(123)));

		// Act: promote the node with a changed (matching) role
		Add(container, key, "bob", NodeSource::Static, NodeRoles::Api);

		// Sanity: the connection state was added by promotion
		EXPECT_TRUE(!!container.view().getNodeInfo(key).getConnectionState(ServiceIdentifier(123)));
	}

	TEST(TEST_CLASS, AddConnectionStatesCanAddMultipleConnectionStatesToAddedMatchingNodes) {
		// Arrange:
		NodeContainer container;

		// - add multiple services
		container.modifier().addConnectionStates(ServiceIdentifier(123), NodeRoles::Api);
		container.modifier().addConnectionStates(ServiceIdentifier(124), NodeRoles::Peer);
		container.modifier().addConnectionStates(ServiceIdentifier(125), NodeRoles::None);
		container.modifier().addConnectionStates(ServiceIdentifier(126), NodeRoles::Api);

		// Act: add a node with matching roles
		auto key = test::GenerateRandomData<Key_Size>();
		Add(container, key, "bob", NodeSource::Dynamic, NodeRoles::Api);

		// Assert: connection states are present for the matching services (None matches everything)
		const auto& view = container.view();
		const auto& nodeInfo = view.getNodeInfo(key);
		EXPECT_TRUE(!!nodeInfo.getConnectionState(ServiceIdentifier(123)));
		EXPECT_FALSE(!!nodeInfo.getConnectionState(ServiceIdentifier(124)));
		EXPECT_TRUE(!!nodeInfo.getConnectionState(ServiceIdentifier(125)));
		EXPECT_TRUE(!!nodeInfo.getConnectionState(ServiceIdentifier(126)));
	}

	// endregion

	// region provisionConnectionState

	TEST(TEST_CLASS, ProvisionConnectionStateFailsWhenNodeIsUnknown) {
		// Arrange:
		NodeContainer container;
		SeedThreeNodes(container);
		auto otherKey = test::GenerateRandomData<Key_Size>();

		// Act + Assert:
		EXPECT_THROW(container.modifier().provisionConnectionState(ServiceIdentifier(123), otherKey), catapult_invalid_argument);
	}

	TEST(TEST_CLASS, ProvisionConnectionStateAddsStateIfNotPresent) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedThreeNodes(container);

		// Act:
		const auto& connectionState = container.modifier().provisionConnectionState(ServiceIdentifier(123), keys[1]);

		// Assert:
		test::AssertZeroed(connectionState);
	}

	TEST(TEST_CLASS, ProvisionConnectionStateReturnsExistingStateIfPresent) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedThreeNodes(container);
		const auto& originalConnectionState = container.modifier().provisionConnectionState(ServiceIdentifier(123), keys[1]);

		// Act:
		const auto& connectionState = container.modifier().provisionConnectionState(ServiceIdentifier(123), keys[1]);

		// Assert:
		EXPECT_EQ(&originalConnectionState, &connectionState);
	}

	TEST(TEST_CLASS, ProvisionConnectionStateReturnsUniqueConnectionStatePerNode) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedThreeNodes(container);

		// Act:
		const auto& connectionState1 = container.modifier().provisionConnectionState(ServiceIdentifier(123), keys[0]);
		const auto& connectionState2 = container.modifier().provisionConnectionState(ServiceIdentifier(123), keys[2]);

		// Assert:
		EXPECT_NE(&connectionState1, &connectionState2);
	}

	// endregion

	// region ageConnections

	TEST(TEST_CLASS, AgeConnectionsAgesZeroAgedMatchingConnections) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedThreeNodes(container);
		{
			auto modifier = container.modifier();

			// Act:
			modifier.ageConnections(ServiceIdentifier(123), { keys[0], keys[2] });
		}

		// Assert: nodes { 0, 2 }  should have new state entries for id(123)
		auto view = container.view();
		EXPECT_EQ(1u, view.getNodeInfo(keys[0]).getConnectionState(ServiceIdentifier(123))->Age);
		EXPECT_FALSE(!!view.getNodeInfo(keys[1]).getConnectionState(ServiceIdentifier(123)));
		EXPECT_EQ(1u, view.getNodeInfo(keys[2]).getConnectionState(ServiceIdentifier(123))->Age);
	}

	TEST(TEST_CLASS, AgeConnectionsAgesNonzeroAgedMatchingConnections) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedThreeNodes(container);
		{
			auto modifier = container.modifier();
			modifier.provisionConnectionState(ServiceIdentifier(123), keys[0]).Age = 1;
			modifier.provisionConnectionState(ServiceIdentifier(123), keys[1]).Age = 2;
			modifier.provisionConnectionState(ServiceIdentifier(123), keys[2]).Age = 3;

			// Act:
			modifier.ageConnections(ServiceIdentifier(123), { keys[0], keys[2] });
		}

		// Assert: nodes { 0, 2 } are aged, node { 1 } is cleared
		auto view = container.view();
		EXPECT_EQ(2u, view.getNodeInfo(keys[0]).getConnectionState(ServiceIdentifier(123))->Age);
		EXPECT_EQ(0u, view.getNodeInfo(keys[1]).getConnectionState(ServiceIdentifier(123))->Age);
		EXPECT_EQ(4u, view.getNodeInfo(keys[2]).getConnectionState(ServiceIdentifier(123))->Age);
	}

	TEST(TEST_CLASS, AgeConnectionsOnlyAffectsConnectionStatesWithMatchingIdentifiers) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedThreeNodes(container);
		{
			auto modifier = container.modifier();
			modifier.provisionConnectionState(ServiceIdentifier(123), keys[0]).Age = 1;
			modifier.provisionConnectionState(ServiceIdentifier(123), keys[1]).Age = 2;
			modifier.provisionConnectionState(ServiceIdentifier(123), keys[2]).Age = 3;

			// Act:
			modifier.ageConnections(ServiceIdentifier(124), { keys[0], keys[2] });
		}

		// Assert:
		auto view = container.view();
		const auto& nodeInfo1 = view.getNodeInfo(keys[0]);
		const auto& nodeInfo2 = view.getNodeInfo(keys[1]);
		const auto& nodeInfo3 = view.getNodeInfo(keys[2]);

		// - nodes { 0, 2 }  should have new state entries for id(124)
		EXPECT_EQ(1u, nodeInfo1.getConnectionState(ServiceIdentifier(124))->Age);
		EXPECT_FALSE(!!nodeInfo2.getConnectionState(ServiceIdentifier(124)));
		EXPECT_EQ(1u, nodeInfo3.getConnectionState(ServiceIdentifier(124))->Age);

		// - no id(123) ages were changed
		EXPECT_EQ(1u, nodeInfo1.getConnectionState(ServiceIdentifier(123))->Age);
		EXPECT_EQ(2u, nodeInfo2.getConnectionState(ServiceIdentifier(123))->Age);
		EXPECT_EQ(3u, nodeInfo3.getConnectionState(ServiceIdentifier(123))->Age);

		// - each info has the correct number of states
		EXPECT_EQ(2u, nodeInfo1.numConnectionStates());
		EXPECT_EQ(1u, nodeInfo2.numConnectionStates());
		EXPECT_EQ(2u, nodeInfo3.numConnectionStates());
	}

	// endregion

	// region FindAllActiveNodes

	TEST(TEST_CLASS, FindAllActiveNodesReturnsEmptySetWhenNoNodesAreActive) {
		// Arrange:
		NodeContainer container;
		SeedFiveNodes(container);

		// Act:
		auto nodes = FindAllActiveNodes(container.view());

		// Assert:
		EXPECT_TRUE(nodes.empty());
	}

	TEST(TEST_CLASS, FindAllActiveNodesReturnsAllNodesWithAnyActiveConnection) {
		// Arrange:
		NodeContainer container;
		auto keys = SeedFiveNodes(container);
		{
			auto modifier = container.modifier();
			modifier.provisionConnectionState(ServiceIdentifier(111), keys[0]).Age = 1;
			modifier.provisionConnectionState(ServiceIdentifier(333), keys[2]).Age = 3;
			modifier.provisionConnectionState(ServiceIdentifier(111), keys[3]).Age = 0;
			modifier.provisionConnectionState(ServiceIdentifier(111), keys[4]).Age = 1;
		}

		// Act:
		auto nodes = FindAllActiveNodes(container.view());
		auto identities = test::ExtractNodeIdentities(nodes);

		// Assert:
		// - 0 => id(111)
		// - 1 => inactive
		// - 2 => id(333)
		// - 3 => inactive
		// - 4 => id(111)
		EXPECT_EQ(utils::KeySet({ keys[0], keys[2], keys[4] }), identities);
	}

	// endregion

	// region synchronization

	namespace {
		auto CreateLockProvider() {
			return std::make_unique<NodeContainer>();
		}
	}

	DEFINE_LOCK_PROVIDER_TESTS(TEST_CLASS)

	// endregion
}}