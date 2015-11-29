# ngx_data_vs_time

This is an nginx module that exposes data in a format suitable for use by the datavstime Proxy Adapter. In fact, it provides data for the main demo.

I built it because DvT can put considerable strain on the data source it's connected to so I wanted something very performant for the demo. I expect ngx_data_vs_time is probably at least a couple of orders of magnitude quicker than connecting to a real database (it's doing far less work).

Now I've created this, I remember how much more effort it is to write something in C compared to a higher
level (but still quite performant) language such as go or C#. With that in mind, if you would like to create
your own custom data provider for DvT, I would recommend you don't start here, rather check out one of 
the [higher level language examples](https://github.com/mhowlett/datavstime-examples).

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
