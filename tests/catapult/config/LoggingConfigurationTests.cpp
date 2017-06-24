#include "catapult/config/LoggingConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

	namespace {
		struct LoggingConfigurationTraits {
			using ConfigurationType = LoggingConfiguration;
			using ComponentLevelsMap = utils::ConfigurationBag::KeyValueMap<utils::LogLevel>;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {{
						"console",
						{
							{ "sinkType", "Async" },
							{ "level", "Warning" },
							{ "colorMode", "AnsiBold" }
						}
				}, {
						"console.component.levels",
						{
							{ "net", "Trace" },
							{ "random", "Fatal" }
						}
				}, {
						"file",
						{
							{ "sinkType", "Sync" },
							{ "level", "Fatal" },
							{ "directory", "foo" },
							{ "filePattern", "bar%4N.log" },
							{ "rotationSize", "123KB" },
							{ "maxTotalSize", "10MB" },
							{ "minFreeSpace", "987KB" }
						}
				}, {
						"file.component.levels",
						{
							{ "io", "Info" },
							{ "net", "Warning" },
							{ "?", "Info" }
						}
				}};
			}

			static bool IsSectionOptional(const std::string& section) {
				return "console.component.levels" == section || "file.component.levels" == section;
			}

			static void AssertZero(const BasicLoggerConfiguration& config) {
				// Assert:
				EXPECT_EQ(utils::LogSinkType::Sync, config.SinkType);
				EXPECT_EQ(utils::LogLevel::Trace, config.Level);
				EXPECT_TRUE(config.ComponentLevels.empty());
			}

			static void AssertZero(const LoggingConfiguration& config) {
				// Assert:
				// - console
				AssertZero(config.Console);
				EXPECT_EQ(utils::LogColorMode::Ansi, config.Console.ColorMode);

				// - file
				AssertZero(config.File);
				EXPECT_EQ("", config.File.Directory);
				EXPECT_EQ("", config.File.FilePattern);
				EXPECT_EQ(utils::FileSize::FromBytes(0), config.File.RotationSize);
				EXPECT_EQ(utils::FileSize::FromBytes(0), config.File.MaxTotalSize);
				EXPECT_EQ(utils::FileSize::FromBytes(0), config.File.MinFreeSpace);
			}

			static void AssertCustom(const LoggingConfiguration& config) {
				// Arrange:
				ComponentLevelsMap expectedConsoleComponentLevels = {
					{ "net", utils::LogLevel::Trace },
					{ "random", utils::LogLevel::Fatal },
				};

				ComponentLevelsMap expectedFileComponentLevels = {
					{ "io", utils::LogLevel::Info },
					{ "net", utils::LogLevel::Warning },
					{ "?", utils::LogLevel::Info }
				};

				// Assert:
				// - console (basic)
				EXPECT_EQ(utils::LogSinkType::Async, config.Console.SinkType);
				EXPECT_EQ(utils::LogLevel::Warning, config.Console.Level);
				EXPECT_EQ(expectedConsoleComponentLevels, config.Console.ComponentLevels);

				// - console (specific)
				EXPECT_EQ(utils::LogColorMode::AnsiBold, config.Console.ColorMode);

				// - file (basic)
				EXPECT_EQ(utils::LogSinkType::Sync, config.File.SinkType);
				EXPECT_EQ(utils::LogLevel::Fatal, config.File.Level);
				EXPECT_EQ(expectedFileComponentLevels, config.File.ComponentLevels);

				// - file (specific)
				EXPECT_EQ("foo", config.File.Directory);
				EXPECT_EQ("bar%4N.log", config.File.FilePattern);
				EXPECT_EQ(utils::FileSize::FromKilobytes(123), config.File.RotationSize);
				EXPECT_EQ(utils::FileSize::FromMegabytes(10), config.File.MaxTotalSize);
				EXPECT_EQ(utils::FileSize::FromKilobytes(987), config.File.MinFreeSpace);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(LoggingConfigurationTests, Logging)

	// region logger configuration -> logger options

	namespace {
		LoggingConfiguration LoadCustomConfiguration() {
			auto bag = utils::ConfigurationBag(LoggingConfigurationTraits::CreateProperties());
			return LoggingConfiguration::LoadFromBag(bag);
		}
	}

	TEST(LoggingConfigurationTests, CanMapToConsoleLoggerOptions) {
		// Arrange:
		auto config = LoadCustomConfiguration();

		// Act:
		auto options = GetConsoleLoggerOptions(config.Console);

		// Assert:
		EXPECT_EQ(utils::LogSinkType::Async, options.SinkType);
		EXPECT_EQ(utils::LogColorMode::AnsiBold, options.ColorMode);
	}

	TEST(LoggingConfigurationTests, CanMapToFileLoggerOptions) {
		// Arrange:
		auto config = LoadCustomConfiguration();

		// Act:
		auto options = GetFileLoggerOptions(config.File);

		// Assert:
		EXPECT_EQ(utils::LogSinkType::Sync, options.SinkType);
		EXPECT_EQ(utils::LogColorMode::None, options.ColorMode);

		EXPECT_EQ("foo", options.Directory);
		EXPECT_EQ("bar%4N.log", options.FilePattern);
		EXPECT_EQ(123u * 1024, options.RotationSize);
		EXPECT_EQ(10u * 1024 * 1024, options.MaxTotalSize);
		EXPECT_EQ(987u * 1024, options.MinFreeSpace);
	}

	// endregion
}}