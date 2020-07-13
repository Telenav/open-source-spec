# .osrm.restrictions
Conditional restrictions are saved to disk, later to be parsed by `osrm-contract` or `osrm-customize` for validity and application to a given processing time.
 

## List

```bash
$ tar -xvf nevada-latest.osrm.restrictions
osrm_fingerprint.meta
/common/conditional_restrictions.meta
/common/conditional_restrictions
```

## osrm_fingerprint.meta
- [osrm_fingerprint.meta](./fingerprint.md)

## /common/conditional_restrictions, /common/conditional_restrictions.meta
- `/common/conditional_restrictions.meta` stores how many bytes totally used to store conditional restrictions in `/common/conditional_restrictions`, previously the `.meta` will used to store count of objects instead of bytes of data.    
- `/common/conditional_restrictions` stores parsed and penality calculated conditional restrictions. Only run `osrm-extract` with `--parse-conditional-restrictions` will generate such data.      

### Layout
![](./graph/map.osrm.restrictions.common.conditional_restrictions.png)

### Implementation
Once run `osrm-extract` with option `--parse-conditional-restrictions`, the conditional restrictions will be parsed, penality calculated and stored into the `.osrm.restrictions` file.    
The most important difference between non-conditional restriction and conditonal restriction is that conditional restriction will plus a condition that based on some kind of time range specification. OSRM defined [struct OpeningHours](https://github.com/Telenav/osrm-backend/blob/f45ab75cf9eb57cb9c857ea564beb95be0523968/include/util/opening_hours.hpp#L24) to represent the time range specification(only show fields below):                  
```c++
// Helper classes for "opening hours" format http://wiki.openstreetmap.org/wiki/Key:opening_hours
// Grammar https://wiki.openstreetmap.org/wiki/Key:opening_hours/specification
// Supported simplified features in CheckOpeningHours:
// - Year/Month/Day ranges
// - Weekday ranges
// - Time ranges
// Not supported:
// - Week numbers
// - Holidays, events, variables dates
// - Day offsets and periodic ranges
struct OpeningHours
{
    enum Modifier
    {
        unknown,
        open,
        closed,
        off,
        is24_7
    };

    struct Time
    {
        enum Event : unsigned char
        {
            invalid,
            none,
            dawn,
            sunrise,
            sunset,
            dusk
        };

        Event event;
        std::int32_t minutes;
    }

    struct TimeSpan
    {
        Time from, to;
    }

    struct WeekdayRange
    {
        int weekdays, overnight_weekdays;
    }

    struct Monthday
    {
        int year;
        char month;
        char day;
    }

    struct MonthdayRange
    {
        Monthday from, to;
    }

    std::vector<TimeSpan> times;
    std::vector<WeekdayRange> weekdays;
    std::vector<MonthdayRange> monthdays;
    Modifier modifier;
};

```

Refer to https://github.com/Telenav/osrm-backend/issues/90#issuecomment-652149723 for the conditional restrictions processing phases.     

Eventually when dump to `.osrm.restrictions` file, the conditional restrictions will be [re-connectted to their respective edge-based edges](https://github.com/Telenav/osrm-backend/blob/0b461183b97de493983ba44749c772719849fd3e/src/extractor/edge_based_graph_factory.cpp#L1175-L1198): use `turn_offset` instead of `from,to NodeID` to represent a turn. The `turn_offset` is actually the [`turn_id` in edge based graph](https://github.com/Telenav/osrm-backend/blob/f45ab75cf9eb57cb9c857ea564beb95be0523968/include/extractor/edge_based_edge.hpp#L34).     
```c++
    // re-hash conditionals to connect to their respective edge-based edges. Due to the ordering, we
    // do not really have a choice but to index the conditional penalties and walk over all
    // edge-based-edges to find the ID of the edge
    auto const indexed_conditionals = IndexConditionals(std::move(conditionals));
    util::Log() << "Writing " << indexed_conditionals.size() << " conditional turn penalties...";
    extractor::files::writeConditionalRestrictions(conditional_penalties_filename,
                                                   indexed_conditionals);
```

## Conditional restrictions in OSM map data
The [`restriction:conditional`](https://github.com/Telenav/osrm-backend/blob/f45ab75cf9eb57cb9c857ea564beb95be0523968/src/extractor/restriction_parser.cpp#L70) is the primary key that OSRM supports for conditional restrictions. However, it has a very small amount size in OSM data statistics: totally **7946** for worldwide([taginfo](https://taginfo.openstreetmap.org/keys) @20200713), which may explains why it's NOT a so important problem for open source projects.      
See more in https://github.com/Telenav/osrm-backend/issues/90#issuecomment-657334486.         

## Reference 
- [Conditional restrictions](https://wiki.openstreetmap.org/wiki/Conditional_restrictions)
- [Key:opening hours/specification](https://wiki.openstreetmap.org/wiki/Key:opening_hours/specification)
- [taginfo](https://taginfo.openstreetmap.org/keys)
- [Time/Conditional Restriction #90](https://github.com/Telenav/osrm-backend/issues/90)