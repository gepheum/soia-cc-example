// Calls a soia API on a remote server.
//
// Run with:
//   bazel run :api_client
//
// Assumes you are already running in another process:
//   bazel run :api_server

#include <iostream>
#include <memory>

#include "absl/status/statusor.h"
#include "absl/types/optional.h"
#include "httplib.h"
#include "soia.h"
#include "soiagen/api.h"

int main() {
  constexpr int kPort = 8787;

  httplib::Client client("localhost", kPort);
  std::unique_ptr<soia::api::ApiClient> api_client =
      soia::api::MakeHttplibApiClient(&client, "/myapi");

  const std::vector<soiagen_user::User> users = {
      {.name = "Jane", .user_id = 41},
      {.name = "John", .user_id = 42},
      {.name = "Dora", .user_id = 43},
  };

  for (const soiagen_user::User& user : users) {
    // Add user to the server.
    soiagen_api::AddUserRequest add_user_request;
    add_user_request.user = user;

    std::cout << "About to add user " << user.name << "\n";
    absl::StatusOr<soiagen_api::AddUserResponse> add_user_response =
        InvokeRemote(*api_client, soiagen_api::AddUser(), add_user_request);
    std::cout << "  status: " << add_user_response.status() << "\n";
  }

  soiagen_api::GetUserRequest get_user_request;
  get_user_request.user_id = 43;

  std::cout << "About to get user with id " << get_user_request.user_id << "\n";
  absl::StatusOr<soiagen_api::GetUserResponse> get_user_response =
      InvokeRemote(*api_client, soiagen_api::GetUser(), get_user_request);
  std::cout << "  status: " << get_user_response.status() << "\n";
  if (get_user_response.ok()) {
    const absl::optional<soiagen_user::User>& user = get_user_response->user;
    if (user.has_value()) {
      std::cout << "Found user: " << *user << "\n";
    } else {
      std::cout << "Not found\n";
    }
  }
}
