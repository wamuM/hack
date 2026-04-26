# Generalized Trable

A geography-based puzzle game written in C. Given a start region and a goal region, the player must find the correct route by naming the intermediate regions that connect them. The game works at any administrative level — countries, provinces, comarques, districts — by fetching real boundary data from the OpenStreetMap Overpass API.

## How it works

The game builds a graph where each node is an administrative region and each edge connects two regions that share a physical border. A BFS search finds the shortest path between the start and goal. As the player types region names, each guess is classified in real time:

- **Correct** — the region is on the optimal path.
- **Close** — the region is not on the path but borders at least one region that is.
- **Wrong** — the region has no direct connection to the path.

Border data is fetched once from the Overpass API and cached locally, so subsequent runs are instant.

## How I run it

As an example execute the next scriptin the game home folder
```
ui/build$ ./ui ISO3166-1 FR 2 4 en
```

You can also try other examples:
```
ISO3166-1 ES 2 4 ca
ISO3166-1 CN 2 4 ca
ISO3166-2 ES-CT 4 6 ca
ISO3166-2 ES-CT 4 7 ca
```

For consulting if the region exist go to https://overpass-turbo.eu/ and use the next script:
```
[out:json][timeout:180];relation[%REGION%]['boundary'='administrative']['admin_level'='%REGION ADMIN LEVEL%'];
      map_to_area->.countryArea;relation['boundary'='administrative']['admin_level'='REGION SUB-ADMIN LEVEL'](area.countryArea);
      out geom;
```

Level guides for all the countries: https://wiki.openstreetmap.org/wiki/Tag:boundary%3Dadministrative#Table_:_Admin_level_for_all_countries

## Project structure

You will find the source code directly in the home and some brief test that you can run to check that everything it's ok. 

```
.
├── main.c                  # Entry point
├── graph.c / graph.h       # Graph construction from Overpass JSON
├── node.h                  # node and graph struct definitions
├── path.c / path.h         # BFS shortest-path algorithm
├── deviation.c / deviation.h  # Player guess classification
├── challenge_generator.c/h # Random start/goal pair generation
├── auto_completion.h       # Region name autocomplete interface
├── fetcher.c / fetcher.h   # Overpass API HTTP fetcher + local cache
├── cJSON.c / cJSON.h       # Bundled JSON parser (cJSON)
├── Makefile
└── tests/
    ├── Makefile
    ├── test_path.c         # BFS unit tests
    ├── test_deviation.c    # Deviation classification unit tests
    └── test_graph.c        # Graph integration test (requires network)
```

## Dependencies

-------------------------------------------------------------------------------------------------------
| Dependency | Purpose                                      | Install                                 |
|------------|----------------------------------------------|-----------------------------------------|
|  `libcurl` |      HTTP requests to the Overpass API       | `sudo apt install libcurl4-openssl-dev` |
|  `cJSON`   |  JSON parsing (bundled, no install needed)   |                    —                    |
|  `cmake`   | You need cmake to build the game from source |         `sudo apt install cmake`        |
-------------------------------------------------------------------------------------------------------

## Building

```bash
# After cloning the repository, go to the directory, create the build folder and configure it for the next steps
mkdir build-001
cd build-001/
cmake -DCMAKE_BUILD_TYPE=Release .. 

# Compile the source code and install it if everything goes as planned 
cmake --build .
cmake --install .

# Build the main game binary
make

# Clean compiled binaries
make clean
```

## Running the tests

Tests live in the `tests/` directory and are run from there.

```bash
cd tests

make test_path       		  # BFS / path unit tests       
make test_deviation  		  # Deviation logic unit tests   
make test_graph      		  # Graph integration test       
make test_challenge_generator # Random start test

make test            # Run all four suites
make clean           # Remove test binaries
```

`test_path` and `test_deviation` build an in-memory graph and run fully offline. `test_graph` fetches real data from the Overpass API on the first run and caches the result in `cache/` — subsequent runs use the cache and need no network.

## Data source and query format

Region data is fetched from the [Overpass API](https://overpass-api.de) using `get_json(region, admin_level, sub_level)`:

```c
// Fetch all admin_level 7 regions (comarques) inside Spain (admin_level 2)
cJSON *obj = get_json("'ISO3166-1'='ES'", 2, 7);
```

The first argument is an OSM tag filter identifying the parent area, the second is its administrative level, and the third is the level of the sub-regions to load as graph nodes.

Common administrative levels in OSM (example: in Spain):

| Level | Typical meaning |
|---|---|
| 2 | Country |
| 4 | Region / autonomous community |
| 6 | Province |
| 7 | comarca / district |
| 8 | Municipality |

Fetched responses are stored in `cache/` as JSON files named after the query parameters. Delete the `cache/` directory to force a fresh fetch.

## Module overview

### `graph.c`
Parses the Overpass JSON response and builds an undirected graph. Two regions are connected by an edge if their boundary ways share at least one OSM way reference (i.e. they physically share a border segment).

### `path.c`
Implements BFS on the graph to find the shortest path by hop count between a start and a goal node. Exposes `bfs()`, `bfs_free_path()`, and `get_neighbors()`.

### `deviation.c`
Classifies a player's guessed region against the solution path:

```
PATH_NODE        — the guess is on the optimal path
ALMOST_PATH_NODE — the guess borders a path node (distance 1)
INCORRECT_NODE   — the guess has no direct connection to the path
```

The main entry point for game logic is `evaluate_user_regions()`, which takes an array of name strings and writes a `Deviation_case` result for each one.

### `challenge_generator.c`
Picks a random start/goal pair that guarantees a minimum path length, using repeated BFS attempts with a configurable retry limit.

### `fetcher.c`
Handles HTTP fetching via libcurl and local JSON caching. Responses are stored in `cache/<query>.json`. If the cache file already exists it is loaded directly without a network request.

### `auto_completion.h`
Interface for region name autocompletion as the player types. Returns up to `MAX_NUM_SUGGEST` (3) suggestions matching the current input prefix.

## Known limitations

- The cache never expires automatically. If OSM boundary data changes, delete `cache/` to re-fetch.
- `challenge_generator` uses `rand()` — call `srand(time(NULL))` in `main` before generating challenges, otherwise every run produces the same start/goal pair.
- Regions with no shared border ways (e.g. islands) will appear as isolated nodes in the graph. The BFS returns an empty path for these, and `challenge_generator` will skip them.
- The Overpass API has rate limits. If many regions are loaded at once, consider adding a delay or using a local Overpass instance.
