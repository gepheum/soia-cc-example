// Unit tests examples
//
// Run with:
//   bazel run :example.test

#include <gtest/gtest.h>

#include "absl/time/time.h"
#include "gmock/gmock.h"
#include "soiagen/user.h"
#include "soiagen/user.testing.h"

TEST(ExampleTest, TestStruct) {
  using ::soiagen_user::User;
  using ::testing::soiagen::StructIs;

  const User john = {
      .name = "John Doe",
      .pets =
          {
              {.height_in_meters = 1.67, .name = "Cheeta", .picture = "🐒"},
          },
      .quote = "Life is like a box of chocolates.",
      .user_id = 42,
  };

  EXPECT_THAT(john, (StructIs<User>{
                        // Only the specified fields are tested
                        .pets = testing::ElementsAre(StructIs<User::Pet>{
                            .height_in_meters = testing::FloatNear(1.7, 0.1),
                        }),
                        .quote = testing::StartsWith("Life is"),
                        .user_id = 42,
                    }));
}

TEST(ExampleTest, TestEnum) {
  using ::soiagen_user::User;
  using ::testing::soiagen::IsTrialStartTime;

  User::SubscriptionStatus john_status = soiagen::kFree;
  User::SubscriptionStatus jade_status =
      soiagen::wrap_trial_start_time(absl::FromUnixMillis(1743682787000));

  EXPECT_THAT(john_status, testing::Eq(soiagen::kFree));

  EXPECT_THAT(jade_status, IsTrialStartTime());
  EXPECT_THAT(jade_status, IsTrialStartTime(testing::Gt(absl::UnixEpoch())));
}
