# ngx_data_vs_time

This is an nginx module for providing simulation data to datavstime.com. In fact, it's the backend used by the demo on the website. 

Currently, that is all it can do. In the future, it will may get extended to enable proxying of data from common data sources.

## Docker

### Building

First, the custom nginx needs to be built.

    ./docker-build-up.sh

then:

    cd /repo
    ./fetch-nginx.sh
    ./build.sh

### Running

Now can run:

    ./docker-up.sh

note that this calls provision-up.sh which installs the previously build nginx in an appropriate location.
