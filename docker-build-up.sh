#!/bin/bash

docker run --rm -it -p 8883:80 \
  -v /git/ngx_data_vs_time/:/repo \
  -v /git/ngx_data_vs_time/:/var/www \
  mhowlett/nginx-build-base /bin/bash
