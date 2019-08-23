#include <iostream>
#include <memory>

//
// Interfaces
//

struct CH {};
struct MLD {};
struct Request {};
struct Response { int r; };

class BaseDataFacadeInterface
{
public:
    virtual int getData1() = 0;
    virtual int getData2() = 0;
};


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

// ----------------------------------------------------------------------------


//
// SharedDataFacade implementation
//

class BaseSharedDataFacade : public BaseDataFacadeInterface
{
public:
    int getData1() override final { return 1; };
    int getData2() override final { return 2; };
};

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

template<typename AlgorithmT>
class SharedDataFacade: public BaseSharedDataFacade, public AlgorithmSharedDataFacade<AlgorithmT> { };

using SharedDataFacadeCH = SharedDataFacade<CH>;

// -----------------------------------------------------------------------------

template<typename AlgorithmT>
struct RoutingData
{
    int some_data;
};

template<typename AlgorithmT>
class ShortestPath;

template<typename AlgorithmT>
class DistanceTable;

// ------------------------------ CH ------------------------------------------

template<>
class ShortestPath<CH>
{
public:
    ShortestPath(RoutingData<CH> &data) : data(data) {};

    template<template <typename A> class FacadeT>
    Response operator()(FacadeT<CH> &facade, const Request& r)
    {
        Response result;
        result.r += facade.getCHData1();
        result.r += facade.getData1();
        return result;
    }

    RoutingData<CH> &data;
};

template<>
class DistanceTable<CH>
{
public:
    DistanceTable(RoutingData<CH> &data) : data(data) {};

    template<template <typename A> class FacadeT>
    Response operator()(FacadeT<CH> &facade, const Request& r)
    {
        Response result;
        return result;
    }

    RoutingData<CH> &data;
};

// ------------------------------ MLD ------------------------------------------

template<>
class ShortestPath<MLD>
{
public:
    ShortestPath(RoutingData<MLD> &data) : data(data) {};

    template<template <typename A> class FacadeT>
    Response operator()(FacadeT<MLD> &facade, const Request& r)
    {
        Response result;
        result.r += facade.getMLDData2();
        result.r += facade.getData2();
        return result;
    }

    RoutingData<MLD> &data;
};

template<>
class DistanceTable<MLD>
{
public:
    DistanceTable(RoutingData<MLD> &data) : data(data) {};

    template<template <typename A> class FacadeT>
    Response operator()(FacadeT<MLD> &facade, const Request& r)
    {
        throw std::runtime_error("Not implemented");
        Response result;
        return result;
    }

    RoutingData<MLD> &data;
};

// -----------------------------------------------------------------------------

template<typename AlgorithmT>
class RoutePlugin
{
public:
    RoutePlugin()
        : shortest_path(data)
    {
    }

    template<template <typename A> class FacadeT>
    Response HandleRequest(std::shared_ptr<FacadeT<AlgorithmT>> facade, const Request &r)
    {
        return shortest_path(*facade, r);
    }

    RoutingData<AlgorithmT> data;
    ShortestPath<AlgorithmT> shortest_path;
};

template<typename AlgorithmT>
class TablePlugin
{
public:
    TablePlugin()
        : distance_table(data)
    {
    }

    template<template <typename A> class FacadeT>
    Response HandleRequest(std::shared_ptr<FacadeT<AlgorithmT>> facade, const Request &r)
    {
        return distance_table(*facade, r);
    }

    RoutingData<AlgorithmT> data;
    DistanceTable<AlgorithmT> distance_table;
};

// -----------------------------------------------------------------------------

class EngineInterface
{
public:
    virtual Response Route(const Request &ret) = 0;
    virtual Response Table(const Request &ret) = 0;
};

template<typename AlgorithmT, template <typename A> class FacadeT>
class Engine : public EngineInterface
{
public:
    Response Route(const Request &ret) final override
    {
        return route_plugin.HandleRequest(facade, ret);
    }

    Response Table(const Request &ret) final override
    {
        return table_plugin.HandleRequest(facade, ret);
    }

private:
    RoutePlugin<AlgorithmT> route_plugin;
    TablePlugin<AlgorithmT> table_plugin;
    std::shared_ptr<FacadeT<AlgorithmT>> facade;
};

// ------------------------- Public  -------------------------------------------

struct EngineConfig
{
    bool use_shared_memory;
    bool use_mld;
};

class OSRM
{
public:
    OSRM(const EngineConfig &config);

    Response Route(const Request &ret);
    Response Table(const Request &ret);

private:
    std::unique_ptr<EngineInterface> engine;
};


//// --------------------------- in .cpp file -----------------------------------

OSRM::OSRM(const EngineConfig &config)
{
    if (config.use_shared_memory)
    {
        if (config.use_mld)
        {
            engine = std::make_unique<Engine<MLD, SharedDataFacade>>();
        }
        else
        {
            engine = std::make_unique<Engine<CH, SharedDataFacade>>();
        }
    }
    else
    {
        throw std::runtime_error("Not implemented");
    }
}

Response OSRM::Route(const Request &ret)
{
    return engine->Route(ret);
}

Response OSRM::Table(const Request &ret)
{
    return engine->Table(ret);
}

// -----------------------------------------------------------------------------

int main(int argc, char** argv)
{
    OSRM mld_osrm { EngineConfig{true, true} };
    OSRM ch_osrm { EngineConfig{true, false} };

    Request mld_ret;
    Response mld_result = mld_osrm.Route(mld_ret);

    Request ch_ret;
    Response ch_result = ch_osrm.Route(ch_ret);

    std::cout << "CH: " << ch_result.r << std::endl;
    std::cout << "MLD: " << mld_result.r << std::endl;

    return EXIT_SUCCESS;
}
