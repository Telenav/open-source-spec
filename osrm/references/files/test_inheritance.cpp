#include <iostream>
#include <memory>

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

class CHAlgorithmDataFacadeInterface
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

class MLDAlgorithmSharedDataFacade : public MLDAlgorithmDataFacadeInterface
{
public:
    int getMLDData1() override final { return 4; };
    int getMLDData2() override final { return 5; };
};

class CHAlgorithmSharedDataFacade : public CHAlgorithmDataFacadeInterface
{
public:
    int getCHData1() override final { return 6; };
    int getCHData2() override final { return 7; };
};

// -----------------------------------------------------------------------------

struct CHRoutingData { int some_data; };
struct MLDRoutingData { int some_data; };

// ------------------------------ CH ------------------------------------------

class CHShortestPath
{
public:
    CHShortestPath(CHRoutingData &data) : data(data) {};

    int operator()(BaseDataFacadeInterface &base_facade, CHAlgorithmDataFacadeInterface &ch_facade)
    {
        data.some_data += 1;
        return data.some_data;
    }

    CHRoutingData &data;
};

class CHDistanceTable
{
public:
    CHDistanceTable(CHRoutingData &data) : data(data) {};

    int operator()(BaseDataFacadeInterface &base_facade, CHAlgorithmDataFacadeInterface &ch_facade)
    {
        data.some_data += 5;
        return data.some_data;
    }

    CHRoutingData &data;
};

// ------------------------------ MLD ------------------------------------------

class MLDShortestPath
{
public:
    MLDShortestPath(MLDRoutingData &data) : data(data) {};

    int operator()(BaseDataFacadeInterface &base_facade, MLDAlgorithmDataFacadeInterface &mld_facade)
    {
        data.some_data += 1;
        return data.some_data;
    }

    MLDRoutingData &data;
};

class MLDDistanceTable
{
public:
    MLDDistanceTable(MLDRoutingData &data) : data(data) {};

    int operator()(BaseDataFacadeInterface &base_facade, MLDAlgorithmDataFacadeInterface &mld_facade)
    {
        throw std::runtime_error("Not implemented");
        return 0;
    }

    MLDRoutingData &data;
};

// -----------------------------------------------------------------------------

class CHRoutePlugin
{
public:
    CHRoutePlugin()
        : shortest_path(data)
    {
    }

    void HandleRequest(std::shared_ptr<BaseDataFacadeInterface> base_facade, std::shared_ptr<CHAlgorithmDataFacadeInterface> ch_facade, int &result)
    {
        result = shortest_path(*base_facade, *ch_facade);
    }

    CHRoutingData data;
    CHShortestPath shortest_path;
};

class CHTablePlugin
{
public:
    CHTablePlugin()
        : distance_table(data)
    {
    }

    void HandleRequest(std::shared_ptr<BaseDataFacadeInterface> base_facade, std::shared_ptr<CHAlgorithmDataFacadeInterface> ch_facade, int &result)
    {
        result = distance_table(*base_facade, *ch_facade);
    }

    CHRoutingData data;
    CHDistanceTable distance_table;
};

class MLDRoutePlugin
{
public:
    MLDRoutePlugin()
        : shortest_path(data)
    {
    }

    void HandleRequest(std::shared_ptr<BaseDataFacadeInterface> base_facade, std::shared_ptr<MLDAlgorithmDataFacadeInterface> mld_facade, int &result)
    {
        result = shortest_path(*base_facade, *mld_facade);
    }

    MLDRoutingData data;
    MLDShortestPath shortest_path;
};

// -----------------------------------------------------------------------------

struct EngineConfig
{
    bool use_shared_memory;
    bool use_mld;
};

class EngineInterface
{
public:
    virtual void Route(int &ret) = 0;
    virtual void Table(int &ret) = 0;
};

class CHEngine : public EngineInterface
{
public:
    CHEngine(const EngineConfig &config)
    {
        if (config.use_shared_memory)
        {
            base_facade = std::make_shared<BaseSharedDataFacade>();
            ch_facade = std::make_shared<CHAlgorithmSharedDataFacade>();
        }
        else
        {
            throw std::runtime_error("Not implemented");
        }
    }

    void Route(int &ret) final override
    {
        route_plugin.HandleRequest(base_facade, ch_facade, ret);
    }

    void Table(int &ret) final override
    {
        table_plugin.HandleRequest(base_facade, ch_facade, ret);
    }

private:
    CHRoutePlugin route_plugin;
    CHTablePlugin table_plugin;
    std::shared_ptr<BaseDataFacadeInterface> base_facade;
    std::shared_ptr<CHAlgorithmDataFacadeInterface> ch_facade;
};

class MLDEngine : public EngineInterface
{
public:
    MLDEngine(const EngineConfig &config)
    {
        if (config.use_shared_memory)
        {
            base_facade = std::make_shared<BaseSharedDataFacade>();
            mld_facade = std::make_shared<MLDAlgorithmSharedDataFacade>();
        }
        else
        {
            throw std::runtime_error("Not implemented");
        }
    }

    void Route(int &ret) final override
    {
        route_plugin.HandleRequest(base_facade, mld_facade, ret);
    }

    void Table(int &ret) final override
    {
        throw std::runtime_error("Not implemented");
    }

private:
    MLDRoutePlugin route_plugin;
    std::shared_ptr<BaseDataFacadeInterface> base_facade;
    std::shared_ptr<MLDAlgorithmDataFacadeInterface> mld_facade;
};

// ------------------------- Public (pre 6) ------------------------------------

class OSRM
{
public:
    OSRM(const EngineConfig &config);

    void Route(int &ret);
    void Table(int &ret);

private:
    std::unique_ptr<EngineInterface> engine;
};


//// --------------------------- in .cpp file -----------------------------------

OSRM::OSRM(const EngineConfig &config)
{
    if (config.use_mld)
    {
        engine = std::make_unique<MLDEngine>(config);
    }
    else
    {
        engine = std::make_unique<CHEngine>(config);
    }
}

void OSRM::Route(int &ret)
{
    engine->Route(ret);
}

void OSRM::Table(int &ret)
{
    engine->Table(ret);
}

// -----------------------------------------------------------------------------

int main(int argc, char** argv)
{
    OSRM mld_osrm { EngineConfig{true, true} };
    OSRM ch_osrm { EngineConfig{true, false} };

    int mld_ret;
    mld_osrm.Route(mld_ret);

    int ch_ret;
    ch_osrm.Route(ch_ret);

    std::cout << "CH: " << ch_ret << std::endl;
    std::cout << "MLD: " << mld_ret << std::endl;

    return EXIT_SUCCESS;
}