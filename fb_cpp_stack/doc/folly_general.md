# Facebook folly

Folly (Facebook Open Source Library) contains a collection of reusable C++ library artifacts developed and used at Facebook.  Its the fundamental of lots of Facebook's open source libraries, such as Fizz/Wangle/MCrouter.

## Start
Start from [README](https://github.com/facebook/folly/blob/master/README.md) and take a look at folder structure inside /folly can give you a initial opinion about folly.  
If you are new to C++11, here is an good article about [Learn from Folly source code the new C++11 features](http://www.javadepend.com/Blog/?p=431). 
Here is presentation from Facebook engineer about folly: Experiences with Facebook's C++ library [video](https://www.youtube.com/watch?v=GDxb21kEthM)

## Future

Futures represent the asynchronous result of an already running operation.  Future::then provide a powerful ability to chain callbacks, which std::future doesn't support yet(Until the end of 2019, still `std::experimental::future<T>::then` from cppreference).  Here is a [tech-blog from facebook](https://engineering.fb.com/developer-tools/futures-for-c-11-at-facebook/) describes folly::futures.  Highly recommended.

### Example 1
```C++
auto f = folly::makeFuture(std::string("first"))
                    .then([=](folly::Try<std::string>&& t){ return folly::makeFuture(t.value() + " second ");} )
                    .then([=](folly::Try<std::string>&& t){ return folly::makeFuture(t.value() + " third ");} )
```

### Example 2
instegram's example

### Example 3

```C++
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

```

## Executers


## Fiber

## Coroutine



