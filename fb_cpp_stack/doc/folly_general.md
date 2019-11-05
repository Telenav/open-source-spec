
- [Facebook folly](#facebook-folly)
  - [Get started](#get-started)
  - [folly::Future](#follyfuture)
    - [Example 1](#example-1)
    - [Example 2](#example-2)
    - [Example 3](#example-3)
  - [folly::Executors](#follyexecutors)
  - [folly::fiber](#follyfiber)
  - [folly::experimental::coro](#follyexperimentalcoro)


# Facebook folly

Folly (Facebook Open Source Library) contains a collection of reusable C++ library artifacts developed and used at Facebook.  Its the fundamental of lots of Facebook's open source libraries, such as Fizz/Wangle/MCrouter.

## Get started
Start from [README](https://github.com/facebook/folly/blob/master/README.md) and take a look at folder structure inside /folly can give you a initial opinion about folly.  
If you are new to C++11, here is an good article about [Learn from Folly source code the new C++11 features](http://www.javadepend.com/Blog/?p=431). 
Here is presentation from Facebook engineer about folly: Experiences with Facebook's C++ library [video](https://www.youtube.com/watch?v=GDxb21kEthM)

## folly::Future

Futures represent the asynchronous result of an already running operation.  future->then provide a powerful ability to chain callbacks, which std::future doesn't support yet(Until the end of 2019, still `std::experimental::future<T>::then` from cppreference).  Here is a [tech-blog from facebook](https://engineering.fb.com/developer-tools/futures-for-c-11-at-facebook/) describes folly::futures.  Highly recommended.

### Example 1
```C++
auto f = folly::makeFuture(std::string("first"))
                    .then([=](folly::Try<std::string>&& t){ return folly::makeFuture(t.value() + " second ");} )
                    .then([=](folly::Try<std::string>&& t){ return folly::makeFuture(t.value() + " third ");} )
```

### Example 2
This example comes from Facebook's blog, it shows how Facebook construct its news feed.  It has a two-stage leaf-aggregate pattern, where a request is broken into many leaf requests that shard to different leaf servers, and then they do the same thing but shard differently based on the results of the first aggregate. Finally, take both sets of results and reduce them to a single response.  

[![from_face_book](https://engineering.fb.com/wp-content/uploads/2015/06/GHcGrgDwh2hOxzMFADm5eE4AAAAAbj0JAAAB.jpg)](https://engineering.fb.com/wp-content/uploads/2015/06/GHcGrgDwh2hOxzMFADm5eE4AAAAAbj0JAAAB.jpg)


Each leaf request could be heavy and executed with different duration.  In the function of `fanout()`, it will wait for all features completed by folly::collect().  The first `then()` will be executed and come to second round of `fanout()`/`then()`.  After all is done will finally aggregate all result by `assembleResponse()`.

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
  ...
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
Here is the complete code for reference.[todo]

### Example 3

This example is from Instagram, you could read their tech blog here: [C++ Futures at Instagram](https://instagram-engineering.com/c-futures-at-instagram-9628ff634f49), then [notes about C++ Futures at Instagram](./cpp_futures_instagram_notes.md) for more information.

## folly::Executors
folly::Executors works similar as thread pool, it helps to run your concurrent code in a preferment way.  For more information you could go to [here](https://github.com/facebook/folly/blob/master/folly/docs/Executors.md).

```C++
auto f = someFutureFunction().via(getCPUExecutor()).then(...)
```
You could find more useful examples in [via()'s unit test](https://github.com/facebook/folly/blob/master/folly/futures/test/ViaTest.cpp#L64).

## folly::fiber
[folly::fiber](https://github.com/facebook/folly/tree/master/folly/fibers) is lightweight application thread, you can treat it as golang's goroutine(see [here](https://stackoverflow.com/a/35996436) for difference).

```C++
fiberManager.addTask([]() {
  ...
});
```
You could find more useful examples here in [unit test](https://github.com/facebook/folly/blob/05490a16e98b2b7c4857d39b2c2b9d89f33a5b1d/folly/fibers/test/FibersTestApp.cpp#L27).


## folly::experimental::coro
[folly::coro](https://github.com/facebook/folly/tree/master/folly/experimental/coro) is a developer-friendly asynchronous C++ framework based on [Coroutines TS](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1477r0.pdf).  For more information about coroutine, you could go to [cpp_coroutine_notes](./cpp_coroutine_notes.md).



