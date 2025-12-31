// Code snippets showing how to use skir-generated data classes.
//
// Run with:
//   bazel run :example

#include <iostream>

#include "absl/container/flat_hash_set.h"
#include "absl/status/statusor.h"
#include "absl/time/time.h"
#include "skir.h"
#include "skirout/user.h"
#include "string_capitalizer.h"

int main() {
  // REFERRING TO GENERATED SYMBOLS

  // Every generated symbol lives in a namespace called `skirout_${path}`,
  // where ${path} is the path to the .skir file relative from the root of the
  // skir source directory, with the ".skir" extension removed, and slashes
  // replaced with underscores.
  using ::skirout_user::SubscriptionStatus;
  using ::skirout_user::User;
  using ::skirout_user::UserRegistry;

  // CONSTRUCTING STRUCTS

  // You can construct a struct like this:
  User john;
  john.user_id = 42;
  john.name = "John Doe";

  // Or you can use the designated initialized syntax:
  User jane = {
      // Keep fields in alphabetical order
      .name = "Jane Doe",
      .pets = {{
                   .name = "Fluffy",
                   .picture = "cat",
               },
               {
                   .name = "Rex",
                   .picture = "dog",
               }},
      .subscription_status = skirout::kPremium,
      .user_id = 43,
  };

  // ${StructName}::whole forces you to initialize all the fields of the struct.
  // You will get a compile-time error if you miss one.
  User lyla = User::whole{
      .name = "Lyla Doe",
      .pets =
          {
              User::Pet::whole{
                  .height_in_meters = 0.05f,
                  .name = "Tiny",
                  .picture = "🐁",
              },
          },
      .quote = "This is Lyla's world, you just live in it",
      .subscription_status = skirout::kFree,
      .user_id = 44,
  };

  // CONSTRUCTING ENUMS

  // Use skirout::${kFieldName} for constant variants.
  SubscriptionStatus john_status = skirout::kFree;
  SubscriptionStatus jane_status = skirout::kPremium;

  // Compilation error: MONDAY is not a field of the SubscriptionStatus enum.
  // SubscriptionStatus sara_status = skirout::kMonday;

  // Use skirout::wrap_${field_name} for wrapper variants.
  SubscriptionStatus jade_status =
      skirout::wrap_trial_start_time(absl::FromUnixMillis(1743682787000));

  // The ${kFieldName} and wrap_${field_name} symbols are also defined in the
  // generated class.
  SubscriptionStatus lara_status = SubscriptionStatus::kFree;

  // CONDITIONS ON ENUMS

  if (john_status == skirout::kFree) {
    std::cout << "John, would you like to upgrade to premium?\n";
  }

  // Call is_${field_name}() to check if the enum holds a wrapper variant.
  if (jade_status.is_trial_start_time()) {
    // as_${field_name}() returns the wrapped value
    const absl::Time trial_start_time = jade_status.as_trial_start_time();
    std::cout << "Jade's trial started on " << trial_start_time << "\n";
  }

  // One way to do an exhaustive switch on an enum.
  switch (lara_status.kind()) {
    case SubscriptionStatus::kind_type::kUnknown:
      // UNKNOWN is the default value for an uninitialized SubscriptionStatus.
      // ...
      break;
    case SubscriptionStatus::kind_type::kFreeConst:
      // ...
      break;
    case SubscriptionStatus::kind_type::kPremiumConst:
      // ...
      break;
    case SubscriptionStatus::kind_type::kTrialStartTimeWrapper: {
      const absl::Time& trial_start_time = lara_status.as_trial_start_time();
      std::cout << "Lara's trial started on " << trial_start_time << "\n";
    }
  }

  // Another way to do an exhaustive switch using the visitor pattern.
  struct Visitor {
    void operator()(skirout::k_unknown) const {
      std::cout << "Lara's subscription status is UNKNOWN\n";
    }
    void operator()(skirout::k_free) const {
      std::cout << "Lara's subscription status is FREE\n";
    }
    void operator()(skirout::k_premium) const {
      std::cout << "Lara's subscription status is PREMIUM\n";
    }
    void operator()(SubscriptionStatus::wrap_trial_start_time_type& w) const {
      const absl::Time& trial_start_time = w.value;
      std::cout << "Lara's trial started on " << trial_start_time << "\n";
    }
  };
  lara_status.visit(Visitor());

  // SERIALIZATION

  // Serialize a skir value to JSON with ToDenseJson or ToReadableJson.
  std::cout << skir::ToDenseJson(john) << "\n";
  // [42,"John Doe"]

  std::cout << skir::ToReadableJson(john) << "\n";
  // {
  //   "user_id": 42,
  //   "name": "John Doe"
  // }

  // The dense flavor is the flavor you should pick if you intend to
  // deserialize the value in the future. Skir allows fields to be renamed, and
  // because fields names are not part of the dense JSON, renaming a field does
  // not prevent you from deserializing the value.
  // You should pick the readable flavor mostly for debugging purposes.

  // The binary format is not human readable, but it is a bit more compact than
  // JSON, and serialization/deserialization can be a bit faster.
  // Only use it when this small performance gain is likely to matter, which
  // should be rare.
  std::cout << skir::ToBytes(john).as_string() << "\n";
  // skir�+Jane Doe����Fluffy�cat��Rex�dog

  // DESERIALIZATION

  // Use Parse to deserialize a skir value from JSON or binary format.
  absl::StatusOr<User> maybe_john = skir::Parse<User>(skir::ToDenseJson(john));
  assert(maybe_john.ok() && *maybe_john == john);

  // KEYED ARAYS

  // A skir::keyed_items<T, get_key> is a container that stores items of type T
  // and allows for fast lookups by key using a hash table.

  UserRegistry user_registry;
  skir::keyed_items<User, skirout::get_user_id<>>& users = user_registry.users;
  users.push_back(john);
  users.push_back(jane);
  users.push_back(lyla);

  assert(users.size() == 3);
  assert(users[0] == john);

  const User* maybe_jane = users.find_or_null(43);
  assert(maybe_jane != nullptr && *maybe_jane == jane);

  assert(users.find_or_default(44).name == "Lyla Doe");
  assert(users.find_or_default(45).name == "");

  // If multiple items have the same key, find_or_null and find_or_default
  // return the last one. Duplicates are allowed but generally discouraged.
  User evil_lyla = lyla;
  evil_lyla.name = "Evil Lyla";
  users.push_back(evil_lyla);
  assert(users.find_or_default(44).name == "Evil Lyla");

  // EQUALITY AND HASHING

  // Skir structs and enums are equality comparable and hashable.
  absl::flat_hash_set<User> user_set;
  user_set.insert(john);
  user_set.insert(jane);
  user_set.insert(jane);
  user_set.insert(lyla);

  assert(user_set.size() == 3);

  // CONSTANTS

  const User& tarzan = skirout_user::k_tarzan();
  assert(tarzan.name == "Tarzan");

  // DYNAMIC REFLECTION

  using ::skir::reflection::GetTypeDescriptor;
  using ::skir::reflection::TypeDescriptor;

  // A TypeDescriptor describes a skir type. It contains the definition of all
  // the structs and enums referenced from the type.
  const TypeDescriptor& user_descriptor = GetTypeDescriptor<User>();

  // TypeDescriptor can be serialized/deserialized to/from JSON.

  absl::StatusOr<TypeDescriptor> reserialized_type_descriptor =
      TypeDescriptor::FromJson(user_descriptor.AsJson());
  assert(reserialized_type_descriptor.ok());

  // STATIC REFLECTION

  // Static reflection allows you to inspect and modify values of generated
  // skir types in a typesafe manner.

  User tarzan_copy = skirout_user::k_tarzan();
  CapitalizeStrings(tarzan_copy);

  std::cout << tarzan_copy << "\n";
  // {
  //   .user_id: 123,
  //   .name: "TARZAN",
  //   .quote: "AAAAAAAAAAYAAAAAAAAAAYAAAAAAAAAA",
  //   .pets: {
  //     {
  //       .name: "CHEETA",
  //       .height_in_meters: 1.67,
  //       .picture: "🐒",
  //     },
  //   },
  //   .subscription_status:
  //   ::skirout::wrap_trial_start_time(absl::FromUnixMillis(1743592409000 /*
  //   2025-04-02T11:13:29+00:00 */)),
  // }

  return 0;
}
