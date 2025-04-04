// Starts an HTTP server with a soia API on http://localhost:8787/?myapi
//
// Run with:
//   bazel run :api_server
//
// Send RPCs to the server with:
//   bazel run :api_client

#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "httplib.h"
#include "soia.h"
#include "soiagen/api.h"

class ApiImpl {
 public:
  using methods = std::tuple<soiagen_api::GetUser, soiagen_api::AddUser>;

  absl::StatusOr<soiagen_api::GetUserResponse> operator()(
      soiagen_api::GetUser, soiagen_api::GetUserRequest request,
      const soia::api::HttpHeaders& request_headers,
      soia::api::HttpHeaders& response_headers) {
    const int64_t user_id = request.user_id;
    if (user_id == 0) {
      return absl::UnknownError("invalid user id: 0");
    }
    soiagen_api::GetUserResponse response;
    const auto it = id_to_user_.find(request.user_id);
    if (it != id_to_user_.cend()) {
      response.user = it->second;
    }
    return response;
  }

  absl::StatusOr<soiagen_api::AddUserResponse> operator()(
      soiagen_api::AddUser, soiagen_api::AddUserRequest request,
      const soia::api::HttpHeaders& request_headers,
      soia::api::HttpHeaders& response_headers) {
    soiagen_user::User& user = request.user;
    const int64_t user_id = user.user_id;
    id_to_user_[user_id] = std::move(user);
    return soiagen_api::AddUserResponse{};
  }

 private:
  absl::flat_hash_map<int64_t, soiagen_user::User> id_to_user_;
};

int main() {
  constexpr int kPort = 8787;

  httplib::Server server;

  auto api_impl = std::make_shared<ApiImpl>();
  soia::api::InstallApiOnHttplibServer(server, "/myapi", api_impl);

  server.Get("/stop", [&](const httplib::Request& req, httplib::Response& res) {
    server.stop();
  });

  std::cout << "Starting server on http://localhost:" << kPort
            << "\nGo to http://localhost:" << kPort
            << "/myapi?list to see all methods\n";

  server.listen("localhost", kPort);
}
