# PBF decoder in golang

## [qedus/osmpbf](https://github.com/qedus/osmpbf)

### How to use
You could follow the example [here](https://github.com/qedus/osmpbf/blob/f9408716cb01c9a60d2929bf510d9b9b4d893f58/example_test.go#L1) with following changes:
```go
    // [Perry] Use your own pbf
    f, err := os.Open("nv.osm.pbf")

    // ...

    // [Perry]Write your own paser to handle osm object
    for {
		if v, err := d.Decode(); err == io.EOF {
			break
		} else if err != nil {
			log.Fatal(err)
		} else {
			switch v := v.(type) {
			case *osmpbf.Node:
				// Process Node v.
				nc++
			case *osmpbf.Way:
				// Process Way v.
                wc++
                // [Perry] Print wayid, nodeids
				fmt.Printf("WayID: %d, ", v.ID)
				for _, node := range v.NodeIDs {
					fmt.Printf("%d,", node)
				}
				fmt.Printf("\n")
			case *osmpbf.Relation:
				// Process Relation v.
				rc++
			default:
				log.Fatalf("unknown type %T\n", v)
			}
		}

```

### Internal
Before I use go version of OSM decoder, I am the beliver of OSMIUM, its elegant design of reader->decoder->handler really influenced me a lot, I feel it might be the only pbf decoder you should use.  But after have a try with go version, I was suprised by its high performance and most importantly, simple implementation based on the power of go.  


