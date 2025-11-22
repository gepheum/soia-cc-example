#include <string>
#include <type_traits>
#include <vector>

#include "absl/strings/ascii.h"
#include "absl/types/optional.h"
#include "soia.h"


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

inline void CapitalizeStrings(std::string& s) { absl::AsciiStrToUpper(&s); }

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
  void operator()(soia::reflection::enum_wrapper_field<Option, Value>) {
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
