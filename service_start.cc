// Starts a soia service on http://localhost:8787/?myapi
//
// Run with:
//   bazel run :service_start
//
// Send RPCs to the service with:
//   bazel run :service_client

#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "httplib.h"
#include "soia.h"
#include "soiagen/service.h"
#include "soiagen/user.h"

class ServiceImpl {
 public:
  using methods =
      std::tuple<soiagen_service::GetUser, soiagen_service::AddUser>;

  absl::StatusOr<soiagen_service::GetUserResponse> operator()(
      soiagen_service::GetUser, soiagen_service::GetUserRequest request,
      const soia::service::HttpHeaders& request_headers,
      soia::service::HttpHeaders& response_headers) {
    const int64_t user_id = request.user_id;
    if (user_id == 0) {
      return absl::UnknownError("invalid user id: 0");
    }
    soiagen_service::GetUserResponse response;
    const auto it = id_to_user_.find(request.user_id);
    if (it != id_to_user_.cend()) {
      response.user = it->second;
    }
    return response;
  }

  absl::StatusOr<soiagen_service::AddUserResponse> operator()(
      soiagen_service::AddUser, soiagen_service::AddUserRequest request,
      const soia::service::HttpHeaders& request_headers,
      soia::service::HttpHeaders& response_headers) {
    soiagen_user::User& user = request.user;
    const int64_t user_id = user.user_id;
    id_to_user_[user_id] = std::move(user);
    return soiagen_service::AddUserResponse{};
  }

 private:
  absl::flat_hash_map<int64_t, soiagen_user::User> id_to_user_;
};

int main() {
  constexpr int kPort = 8787;

  httplib::Server server;

  auto service_impl = std::make_shared<ServiceImpl>();
  soia::service::InstallServiceOnHttplibServer(server, "/myapi", service_impl);

  server.Get("/stop", [&](const httplib::Request& req, httplib::Response& res) {
    server.stop();
  });

  std::cout << "Starting server on http://localhost:" << kPort
            << "\nGo to http://localhost:" << kPort
            << "/myapi?list to see all methods\n";

  server.listen("localhost", kPort);
}
