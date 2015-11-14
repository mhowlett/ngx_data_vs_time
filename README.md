# ngx_data_vs_time

This is an nginx module for providing simulation data to datavstime.com. In fact, it's the backend used by the demo on the website. 

I built this because DvT can put considerable strain on the datasource it's connected to, so I needed something very performant for the demo. I expect this is going to be at least a couple of orders of magnitude more efficient than connecting to a real database (it's doing far less work).

Currently, providing simulation data is all it can do. In the future, it may get extended to enable proxying of data from common data sources, though where possible this functionality will be included directly in the front end.


## Docker

### Building

First, the custom nginx needs to be built.

    ./docker-build-up.sh

then:

    cd /repo
    ./fetch-nginx.sh
    ./build.sh

### Running

Now you can run it:

    ./docker-up.sh

note that this calls provision-up.sh which installs the previously build nginx in an appropriate location.
