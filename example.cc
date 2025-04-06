// Code snippets showing how to use soia-generated data classes.
//
// Run with:
//   bazel run :example

#include <iostream>
#include <type_traits>

#include "absl/container/flat_hash_set.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/time/time.h"
#include "absl/types/optional.h"
#include "soia.h"
#include "soiagen/user.h"

// CapitalizeStrings recursively capitalizes all the strings found within a
// soia value.

template <typename T>
typename std::enable_if_t<!soia::reflection::IsRecord<T>()>
CapitalizeStrings(T&);
void CapitalizeStrings(std::string& s);
template <typename T>
void CapitalizeStrings(std::vector<T>& vector);
template <typename T, typename GetKey>
void CapitalizeStrings(soia::keyed_items<T, GetKey>& vector);
template <typename T>
void CapitalizeStrings(absl::optional<T>& optional);
template <typename T>
typename std::enable_if_t<soia::reflection::IsRecord<T>()>
CapitalizeStrings(T& record);

template <typename T>
typename std::enable_if_t<!soia::reflection::IsRecord<T>()>
CapitalizeStrings(T&) {}

void CapitalizeStrings(std::string& s) { absl::AsciiStrToUpper(&s); }

template <typename T>
void CapitalizeStrings(std::vector<T>& vector) {
  for (T& value : vector) {
    CapitalizeStrings(value);
  }
}

template <typename T, typename GetKey>
void CapitalizeStrings(soia::keyed_items<T, GetKey>& vector) {
  for (T& value : vector) {
    CapitalizeStrings(value);
  }
}

template <typename T>
void CapitalizeStrings(absl::optional<T>& optional) {
  if (optional.has_value()) {
    CapitalizeStrings(*optional);
  }
}

template <typename T>
struct CapitalizeStringsVisitor {
  T& record;

  template <typename Getter, typename Value>
  void operator()(soia::reflection::struct_field<Getter, Value>) {
    CapitalizeStrings(Getter()(record));
  }

  template <typename Const>
  void operator()(soia::reflection::enum_const_field<Const>) {}

  template <typename Option, typename Value>
  void operator()(soia::reflection::enum_value_field<Option, Value>) {
    auto* value = Option::get_or_null(record);
    if (value != nullptr) {
      CapitalizeStrings(*value);
    }
  }
};

template <typename T>
typename std::enable_if_t<soia::reflection::IsRecord<T>()>
CapitalizeStrings(T& record) {
  soia::reflection::ForEachField<T>(CapitalizeStringsVisitor<T>{record});
}

int main() {
  // Every generated symbol lives in a namespace called `soiagen_${path}`,
  // where ${path} is the path to the .soia file relative from the root of the
  // soia source directory, with the ".soia" extension removed, and slashes
  // replaced with underscores.
  using ::soiagen_user::User;
  using ::soiagen_user::UserRegistry;

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
      .subscription_status = soiagen::kPremium,
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
      .subscription_status = soiagen::kFree,
      .user_id = 44,
  };

  // CONSTRUCTING ENUMS

  // Use soiagen::${kFieldName} for constant variants.
  User::SubscriptionStatus john_status = soiagen::kFree;
  User::SubscriptionStatus jane_status = soiagen::kPremium;

  // Compilation error: MONDAY is not a field of the SubscriptionStatus enum.
  // User::SubscriptionStatus sara_status = soiagen::kMonday;

  // Use soiagen::wrap_${field_name} for data variants.
  User::SubscriptionStatus jade_status =
      soiagen::wrap_trial_start_time(absl::FromUnixMillis(1743682787000));

  // The ${kFieldName} and wrap_${field_name} symbols are also defined in the
  // generated class.
  User::SubscriptionStatus lara_status = User::SubscriptionStatus::kFree;

  // CONDITIONS ON ENUMS

  if (john_status == soiagen::kFree) {
    std::cout << "John, would you like to upgrade to premium?\n";
  }

  // Call is_${field_name}() to check if the enum holds a data variant.
  if (jade_status.is_trial_start_time()) {
    // as_${field_name}() returns the value held by the enum
    const absl::Time trial_start_time = jade_status.as_trial_start_time();
    std::cout << "Jade's trial started on " << trial_start_time << "\n";
  }

  // One way to do an exhaustive switch on an enum.
  switch (lara_status.kind()) {
    case User::SubscriptionStatus::kind_type::kConstUnknown:
      // UNKNOWN is the default value for an uninitialized SubscriptionStatus.
      // ...
      break;
    case User::SubscriptionStatus::kind_type::kConstFree:
      // ...
      break;
    case User::SubscriptionStatus::kind_type::kConstPremium:
      // ...
      break;
    case User::SubscriptionStatus::kind_type::kValTrialStartTime: {
      const absl::Time& trial_start_time = lara_status.as_trial_start_time();
      std::cout << "Lara's trial started on " << trial_start_time << "\n";
    }
  }

  // Another way to do an exhaustive switch using the visitor pattern.
  struct Visitor {
    void operator()(soiagen::k_unknown) const {
      std::cout << "Lara's subscription status is UNKNOWN\n";
    }
    void operator()(soiagen::k_free) const {
      std::cout << "Lara's subscription status is FREE\n";
    }
    void operator()(soiagen::k_premium) const {
      std::cout << "Lara's subscription status is PREMIUM\n";
    }
    void operator()(
        User::SubscriptionStatus::wrap_trial_start_time_type& w) const {
      const absl::Time& trial_start_time = w.value;
      std::cout << "Lara's trial started on " << trial_start_time << "\n";
    }
  };
  lara_status.visit(Visitor());

  // SERIALIZATION

  // Serialize a soia value to JSON with ToDenseJson or ToReadableJson.
  std::cout << soia::ToDenseJson(john) << "\n";
  // [42,"John Doe"]

  std::cout << soia::ToReadableJson(john) << "\n";
  // {
  //   "user_id": 42,
  //   "name": "John Doe"
  // }

  // The dense flavor is the flavor you should pick if you intend to
  // deserialize the value in the future. Soia allows fields to be renamed, and
  // because fields names are not part of the dense JSON, renaming a field does
  // not prevent you from deserializing the value.
  // You should pick the readable flavor mostly for debugging purposes.

  // The binary format is not human readable, but it is a bit more compact than
  // JSON, and serialization/deserialization can be a bit faster.
  // Only use it when this small performance gain is likely to matter, which
  // should be rare.
  std::cout << soia::ToBytes(john).as_string() << "\n";
  // soia�+Jane Doe����Fluffy�cat��Rex�dog

  // DESERIALIZATION

  // Use Parse to deserialize a soia value from JSON or binary format.
  absl::StatusOr<User> maybe_john = soia::Parse<User>(soia::ToDenseJson(john));
  assert(maybe_john.ok() && *maybe_john == john);

  // KEYED ARAYS

  // A soia::keyed_items<T, get_key> is a container that stores items of type T
  // and allows for fast lookups by key using a hash table

  UserRegistry user_registry;
  soia::keyed_items<User, soiagen::get_user_id<>>& users = user_registry.users;
  users.push_back(john);
  users.push_back(jane);
  users.push_back(lyla);

  assert(users.size() == 3);
  assert(users[0] == john);

  const User* maybe_jane = users.find_or_null(43);
  assert(maybe_jane != nullptr && *maybe_jane == jane);

  assert(users.find_or_default(44).name == "Lyla Doe");
  assert(users.find_or_default(45).name == "");

  // EQUALITY AND HASHING

  // Soia structs and enums are equality comparable and hashable.
  absl::flat_hash_set<User> user_set;
  user_set.insert(john);
  user_set.insert(jane);
  user_set.insert(jane);
  user_set.insert(lyla);

  assert(user_set.size() == 3);

  // CONSTANTS

  const User& tarzan = soiagen_user::k_tarzan();
  assert(tarzan.name == "Tarzan");

  // DYNAMIC REFLECTION

  using ::soia::reflection::GetTypeDescriptor;
  using ::soia::reflection::TypeDescriptor;

  // A TypeDescriptor describes a soia type. It contains the definition of all
  // the structs and enums referenced from the type.
  const TypeDescriptor& user_descriptor = GetTypeDescriptor<User>();

  // TypeDescriptor can  be serialized/deserialized to/from JSON.

  absl::StatusOr<TypeDescriptor> reserialized_type_descriptor =
      TypeDescriptor::FromJson(user_descriptor.AsJson());
  assert(reserialized_type_descriptor.ok());

  // STATIC REFLECTION

  // Static reflection allows you to inspect and modify values of generated
  // soia types in a typesafe maneer.

  User tarzan_copy = soiagen_user::k_tarzan();
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
  //   .subscription_status: ::soiagen::wrap_trial_start_time(absl::FromUnixMillis(1743592409000 /* 2025-04-02T11:13:29+00:00 */)),
  // }
}
