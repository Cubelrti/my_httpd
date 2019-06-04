# my_httpd

A deadly simple http server.

## Prerequisite

-   Docker
-   Bash

## Build

```bash
chmod +x ./build.sh
./build.sh
```

Initial build may take a long time since `docker` will pull the whole `gcc` image first if you don't have.

## Run

```bash
chmod +x ./run.sh
./run.sh 8080 # OR any port you love
```

## Test

Open browser and open `localhost:8080` to see if the server is working correctly.
