
# Map Matching Basic

## What is map matching
Give a sequence of GPS signals, find the most probable sequence of road segments.

<img src="../resource/pictures/mm_what_is_mm.png" alt="mm_what_is_mm" width="200"/><br/>
(Picture from the paper of [HMMM Through Noise and Sparseness](https://www.microsoft.com/en-us/research/publication/hidden-markov-map-matching-noise-sparseness/))

Usually, we call GPS data collected from each of our user as GPS probe data.

## Different type of map matching

### Onboard map matching
Real time map matching, always happened on embedded device.  When user is driving, map matching component try to snap user's GPS trace to map data.  Onboard map matching always have richer information compare to offboard map matching, such as information from gyro(driving angle), information from vehicle(speed, gps accuracy).
Onboard map matching always retrieve map data from local device.  During map matching, based on exsisting gps trace, map matching component recommends the most probable route for user.  It might maintain several route candidate during calculation and re-adjust probobility based on coming gps points.  Its comain the probability of different candidate jump back and forth, but map matching system must avoid providing such experience for end user.

During navigation, detect whether user is following the route now is the main purpose of onboard nevigation.  We expect which to trigger re-routing when we find user is off suggested route(deviation).

### Offboard map matching
Post processing, always happened at server side with full user gps trip.  GPS points will be recorded once every three seconds or 20 meters.
It could share the same algorithm with onboard map matching, but data cleaning would be a big part or most important part of work.


## What's the challenge of map matching

<img src="../resource/pictures/mm_noise_example.png" alt="mm_noise_example" width="200"/><br/>

GPS points always be noisy and sparseness.  Especially when start your device, at parking lot, passing through skyscraper.  

<img src="../resource/pictures/mm_missing_gps.png" alt="mm_missing_gps" width="200"/><br/>
User's single trip might be represented by several GPS probe trace, deviation and lost gps data is a common situation.

<img src="../resource/pictures/mm_sfo_airport.png" alt="mm_sfo_airport" width="200"/><br/>
Map matching would spend a lot of effort at complex graph, such as airport, multiple layer of road networks.  
Add also, map data might not accurately represent the real world, map matching is extremely hard at such situation.

## Why map matching is useful

<img src="../resource/pictures/mm_historical_data_eta.png" alt="mm_historical_data_eta" width="200"/><br/>
Provide accurate arriving time estimation is the target of every nevigation system.






