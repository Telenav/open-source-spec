#include <folly/futures/Future.h>
#include <folly/portability/GTest.h>

#include <chrono>
#include <map>
#include <vector>

using namespace folly;

namespace {

struct Leaf {
  Leaf(int value): v(value) {}
  bool operator< (const Leaf& lhs) const {
    return v < lhs.v;
  }
  int v;
};

struct LeafReq {
  LeafReq(int value) : v(value) {}
  int v;
};

struct LeafResponse {
  LeafResponse(int value) : v(value) {}
  int v;
};

struct Request {};

struct Response {
  Response(int value) : v(value) {}
  int v;
};

class RPCServer {
public:
  folly::Future<LeafResponse> futureLeafRPC(const LeafReq &req) {
    return folly::makeFuture<LeafResponse>(LeafResponse(req.v));
  }
};

static std::shared_ptr<RPCServer> rpc = nullptr;
std::shared_ptr<RPCServer> getClient(const Leaf &leaf) {
  if (!rpc) {
    rpc = std::make_shared<RPCServer>();
  }
  return rpc;
}

folly::Future<std::vector<LeafResponse>>
fanout(const std::map<Leaf, LeafReq> &leafToReqMap,
       std::chrono::milliseconds timeout) {
  std::vector<folly::Future<LeafResponse>> leafFutures;

  for (const auto &kv : leafToReqMap) {
    const auto &leaf = kv.first;
    const auto &leafReq = kv.second;

    leafFutures.push_back(
        // Get the client for this leaf and do the async RPC
        getClient(leaf)
            ->futureLeafRPC(leafReq)
            // If the request times out, use an empty response and move on.
            .onTimeout(timeout, [=] { return LeafResponse(0); }));
  }

  // Collect all the individual leaf requests into one Future
  // Collect()takes a collection of Future<T> and returns a Future<vector<T>>
  // that will complete when all those futures are complete
  return folly::collect(leafFutures);
}

// Some sharding function; possibly dependent on previous responses.
std::map<Leaf, LeafReq>
buildLeafToReqMap(const Request &request,
                  const std::vector<LeafResponse> &responses) {
  std::map<Leaf, LeafReq> m;
  m.insert(std::pair<Leaf, LeafReq>(Leaf(0), LeafReq(1)));
  m.insert(std::pair<Leaf, LeafReq>(Leaf(1), LeafReq(2)));
  m.insert(std::pair<Leaf, LeafReq>(Leaf(2), LeafReq(3)));
  return m;
}

// This function assembles our final response.
Response
assembleResponse(const Request &request,
                 const std::vector<LeafResponse> &firstFanoutResponses,
                 const std::vector<LeafResponse> &secondFanoutResponses) {
  Response response(0);
  for (const auto& f : firstFanoutResponses) {
    response.v += f.v;
  }

  for (const auto& s : secondFanoutResponses) {
    response.v += s.v;
  }
  return response;
}

folly::Future<Response> twoStageFanout(std::shared_ptr<Request> request) {
  constexpr auto FIRST_FANOUT_TIMEOUT_MS = std::chrono::milliseconds(3000);
  constexpr auto SECOND_FANOUT_TIMEOUT_MS = std::chrono::milliseconds(3000);

  // Stage 1: first fanout
  return fanout(buildLeafToReqMap(*request, {}), FIRST_FANOUT_TIMEOUT_MS)
      // Stage 2: With the first fanout completed, initiate the second fanout.
      .then([=](Try<std::vector<LeafResponse>> &&response) {
        auto firstFanoutResponses = std::move(response.value());

        // This time, sharding is dependent on the first fanout.
        return fanout(buildLeafToReqMap(*request, firstFanoutResponses),
                      SECOND_FANOUT_TIMEOUT_MS)
            // Stage 3: Assemble and return the final response.
            .then([=](Try<std::vector<LeafResponse>> &&secondFanoutResponses) {
              return assembleResponse(*request, firstFanoutResponses,
                                      secondFanoutResponses.value());
            });
      });
}

} // namespace

TEST(TestFuture, testCompile) { EXPECT_EQ(true, true); }

TEST(TestFuture, testExample) 
{
  auto req = std::make_shared<Request>(Request());
  auto result = twoStageFanout(req);
  EXPECT_EQ(12, result.value().v);
}
