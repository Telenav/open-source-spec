# Tegola Quick experiment

[Tegola](https://github.com/go-spatial/tegola) is a Mapbox Vector Tile server written in Go.  Tegola supports [Mapbox vector Tile v2 specification](https://github.com/mapbox/vector-tile-spec) and provides native geometry processing(simplification, clipping, make valid, intersection, contains, scaling, translation).  For more features please go to [here](https://github.com/go-spatial/tegola#features).  

## Target
Generate a workable version based on sample data.

## Experiment
Postgis -> Tegola -> Frontend(webgl js, tegola frontend)  
For experiments below, I follow the wiki page [getting-started](https://tegola.io/documentation/getting-started/) and [tegola with mapbox](https://tegola.io/tutorials/tegola-with-mapbox/).  I setup those experiments on OSX.

### Postgis
I directely download bonn postgis data and then import this dumped file into postgis.
```sql
# step 1:
create database bonn;
# You could follow links here to set extensions: http://postgis.net/install/

# step 2:
psql bonn < bonn_osm.sql

# step 3:
psql -c "CREATE USER tegola;"
psql -d bonn -c "GRANT SELECT ON ALL TABLES IN SCHEMA public TO tegola;"
```

#### Issues
- Please double check your PostGIS port has been opened.  
The expect result is:
```bash
# netstat -antl | grep 5432
tcp4       0      0  127.0.0.1.5432         127.0.0.1.50017        ESTABLISHED
tcp4       0      0  *.5432                 *.*                    LISTEN  
```
If you don't find that, please go to postgresql.conf and modify config file with expected IP and port:  

```bash
# find the location of postgresql: find / -name "postgresql.conf"
# vim /usr/local/var/postgres/postgresql.conf
listen_addresses = 'localhost'          # what IP address(es) to listen on;
                                        # comma-separated list of addresses;
                                        # defaults to 'localhost'; use '*' for all
                                        # (change requires restart)
port = 5432                             # (change requires restart)
max_connections = 100                   # (change requires restart)
```  
If you changed conf file, please restart postgis
```bash
brew services stop postgresql
brew services start postgresql
# or
pg_ctl -D /usr/local/var/postgres -l /usr/local/var/postgres/server.log restart
```
Test your local connection
```bash
psql postgres -p 5432 -h localhost -l
```

### Tegola
If you use docker to start tegola, you could try with following command
```bash
docker run -v /Users/ngxuser/Desktop/git/tegloa/docker-config/:/opt/tegola_config --net=host -p 8080 gospatial/tegola serve --config /opt/tegola_config/config.toml 
```
If you checkout source code and build your own binary, please make sure $GOPATH has been set to avoid compilation issue.
```bash
go build
```

#### Isssues
- How to setup GOPATH
```bash
1. Install go
2. Set go path
export GOPATH=/usr/local/Cellar/go/1.12.5/
export PATH=$PATH:$GOPATH/bin
3. Check out code at 
  /usr/local/Cellar/go/1.12.5/libexec/src/github.com/go-spatial/tegola
4. Get dependency
go get github.com/go-spatial/tegola
5. Build
   go build
```
For more information, you go to these links: [issue-88](https://github.com/go-spatial/tegola/issues/88#issuecomment-288139027) or [faq-gopath](https://github.com/alco/gostart#faq0) or [Working with $GOPATH on Github forks](http://www.personal.psu.edu/bam49/notebook/gopath-github-fork/)



### Frontend
- You could use this URL to verify bonn's data
```url
http://localhost:8080/maps/bonn/11/7/50.vector.pbf?
```
- Tegola frontend
Use following URL
```url
http://localhost:8080/#11.8/50.6966/7.1308
```
You will see  

<img src="../resource/pictures/tegola_frontend.png" alt="tegola_frontend" style="width:400px;"/>

- Mapbox frontend

```js
<!doctype html>
<html lang="en">
  <head>
    <style>
      body {
        margin: 0;
        padding: 0;
      }

      #map {
        position: absolute;
        top: 0;
        bottom: 0;
        width: 100%;
      }
    </style>
    <script src='https://api.tiles.mapbox.com/mapbox-gl-js/v0.42.2/mapbox-gl.js'></script>
    <link href='https://api.tiles.mapbox.com/mapbox-gl-js/v0.42.2/mapbox-gl.css' rel='stylesheet' />
    <title>Mapbox example</title>
  </head>
  <body>
    <div id="map"></div>
    <script type="text/javascript">
      // [Perry] Enter your mapbox gl access token on the line below
      mapboxgl.accessToken = 'pk.eyJ1IjoibGl1eHVuODAxIiwiYSI6ImNqd2lheHc4dTAxaW00YW1tdHl6cG82d3EifQ.mboz0D3eklR15-Z0rtrFZA'
      var map = new mapboxgl.Map({
        container: 'map',
        style: 'mapbox://styles/mapbox/light-v9',
        center: [7.0982, 50.7374],
        zoom: 11
      });

      var nav = new mapboxgl.NavigationControl();
      map.addControl(nav, 'top-right');

      map.on('load', function () {
        map.addSource('bonn', {
          'type': 'vector',
          "tiles": ["http://localhost:8080/maps/bonn/{z}/{x}/{y}.vector.pbf?"],
          "tolerance": 0
        });

        map.addLayer({
          // [Perry] You could try with main_roads or lakes for id and source-layer
          "id": "road",
          "source": "bonn",
          "source-layer": "road",
          "type": "line",
          "paint": {
            "line-color": "#FF0000",
            "line-width": 1
          }
        });
      });


    </script>
  </body>
</html>
```
You will see:  
<img src="../resource/pictures/tegola_road_layers.png" alt="tegola_road_layers" style="width:400px;"/>



