# Abstract version of OSRM Facade

## Why need facade
OSRM routing engine has two kind of requirements:
- Support different routing algorithm to calculate route, like CH, CRP
- Support different strategy to load data, such as load all data into memory, use mmap to load, use shared memory to load  

Facade is the layer provides data access interfaces to support different algorithm(routing, table, map matching) achieve their goal.    

## A simple case based on inheritance

Suppose we need to implement a class to provide routing service

```C++
class OSRM
{
public:
    OSRM(const EngineConfig& config)
    {
        if (config.use_mld) 
        {
            engine = std::make_unique<MLDEngine>(config);
        }
        else
        {
            engine = std::makde_unique<CHEngine>(config);
        }
    }

    Response Route(const Request& r)
    {
        return engine->Route(r);
    }

private:
    std::unique_ptr<EngineInterface> engine;
};
```
EngineConfig defines which algorithm to use and how to load data
```C++
struct EngineConfig
{
    // enum for how to load data, could be shared mem, mmap or memory
    
    // enum for which algorithm to use
};
```
We would init engine based on different value in EngineConfig.

EngineInterface defines the interface of Route and MLDEngine/CHEngine implements it.
```c++
class EngineInterface
{
public:
    virtual void Route(int &ret) = 0;
};

// [Perry] OSRM abstract two kind of facade
//         base_facade used to retrieve normal graph data, like node, edge, name
//         Algorithm used to retrieve algorithm specific data, like cell in mld
class MLDEngine : public EngineInterface
{
public:
    MLDEngine(const EngineConfig& config)
    {
        if (config.shared_memory)
        {
            base_facade = std::make_shared<BaseSharedDataFacade>();
            mld_facade = std::make_shared<MLDAlgorithmSharedDataFacade>();
        }
        else
        {
            // other mode
        }
    }

    Response Route(Request& r) final override
    {
        return route_plugin.HandleRequest(base_facade, mld_facade, r);
    }

private:
    MLDRoutePlugin route_plugin;
    std::shared_ptr<BaseDataFacadeInterface> base_facade;
    std::shared_ptr<MLDAlgorithmDataFacadeInterface> mld_facade;
};
```

route_plugin needs to know which data facade to pass to which algorithm and whether that algorithm exists for the specified algorithm.  For example, MLDRoutePlugin contains algorithm finally used to calculate shortest_path and pass facade to this algorithm.

```C++
class MLDRoutePlugin
{
public:
    Response HandleRequest(std::shared_ptr<BaseDataFacadeInterface> base_facade, std::shared_ptr<MLDAlgorithmDataFacadeInterface> mld_facade,
    Request& r)
    {
        return shortest_path(*base_facade, *mld_facade, r);
    }

    MLDShortestPath shortest_path;
};
```

The place for route calculation happened in MLDShortestPath

```C++
class MLDShortestPath
{
public:

    Response operator()(BaseDataFacadeInterface &base_facade, MLDAlgorithmDataFacadeInterface &mld_facade, Request& r)
    {
        // [Perry] Calculate route by retrieving data from base_facade and mld_facade
        return response;
    }
};

```

BaseDataFacadeInterface and AlgorithmDataFacade defines the interface to retrieve data, and for different mode there will be such class to enable those interfaces.  

```C++
//
// Interfaces
//

class BaseDataFacadeInterface
{
public:
    virtual int getData1() = 0;
    virtual int getData2() = 0;
};


class MLDAlgorithmDataFacadeInterface
{
public:
    virtual int getMLDData1() = 0;
    virtual int getMLDData2() = 0;
};

//
// SharedDataFacade implementation
//
class BaseSharedDataFacade : public BaseDataFacadeInterface
{
public:
    int getData1() override final { return 1; };
    int getData2() override final { return 2; };
};

class MLDAlgorithmSharedDataFacade : public MLDAlgorithmDataFacadeInterface
{
public:
    int getMLDData1() override final { return 4; };
    int getMLDData2() override final { return 5; };
};
```

Those facade will be created in related engines and finally passing to algorithm.  For more information, you could go to [here](../references/files/test_inheritance.cpp).


## A simple case based on template
The upper version could help you easily understand how OSRM stores data, but in current version, OSRM implements them by C++ template.  You could find related sample code [here](../references/files/test_template.cpp).  

For algorithm data facade:
```C++
struct CH {};
struct MLD {};

template<typename AlgorithmT>
class AlgorithmDataFacadeInterface;

template<>
class AlgorithmDataFacadeInterface<MLD>
{
public:
    virtual int getMLDData1() = 0;
    virtual int getMLDData2() = 0;
};

template<>
class AlgorithmDataFacadeInterface<CH>
{
public:
    virtual int getCHData1() = 0;
    virtual int getCHData2() = 0;
};

```
Please beware of that: member function of a template class cannot be virtual, only specialized version could have template function.  More information: [Can a class member function template be virtual?](https://stackoverflow.com/questions/2354210/can-a-class-member-function-template-be-virtual).  

Here is the implementation:  
```C++
template<typename AlgorithmT>
class AlgorithmSharedDataFacade;

template<>
class AlgorithmSharedDataFacade<MLD> : public AlgorithmDataFacadeInterface<MLD>
{
public:
    int getMLDData1() override final { return 4; };
    int getMLDData2() override final { return 5; };
};

template<>
class AlgorithmSharedDataFacade<CH> : public AlgorithmDataFacadeInterface<CH>
{
public:
    int getCHData1() override final { return 6; };
    int getCHData2() override final { return 7; };
};

```

For base data, we could still use inheritance:
```C++
class BaseDataFacadeInterface
{
public:
    virtual int getData1() = 0;
    virtual int getData2() = 0;
};

class BaseSharedDataFacade : public BaseDataFacadeInterface
{
public:
    int getData1() override final { return 1; };
    int getData2() override final { return 2; };
};
```

By inherit interface from base and algorithm facade, we could have the ability to visit all facade api:
```C++
template<typename AlgorithmT>
class SharedDataFacade: public BaseSharedDataFacade, public AlgorithmSharedDataFacade<AlgorithmT> { };
```

Here is the implementation of Algorithm:
```C++

    template<template <typename A> class FacadeT>
    Response operator()(FacadeT<MLD> &facade, const Request& r)
    {
        Response result;
        result.r += facade.getMLDData2();
        result.r += facade.getData2();
        return result;
    }
```
Please pay attention to facade defined below:
```C++
template<typename AlgorithmT, template <typename A> class FacadeT>
class Engine : public EngineInterface
{
public:
    Response Route(const Request &ret) final override
    {
        return route_plugin.HandleRequest(facade, ret);
    }
private:
    RoutePlugin<AlgorithmT> route_plugin;
    std::shared_ptr<FacadeT<AlgorithmT>> facade;
};
```
Here is how to initialize specific Engine:
```C++
OSRM::OSRM(const EngineConfig &config)
{
    if (config.use_shared_memory)
    {
        if (config.use_mld)
        {
            engine = std::make_unique<Engine<MLD, SharedDataFacade>>();
        }
        //...
    }
    //...
}

std::unique_ptr<EngineInterface> engine;
```


