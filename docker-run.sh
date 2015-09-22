#!/bin/bash

docker run --name ngx-dvt -d -p 8883:80 \
  -v /git/ngx_data_vs_time/:/repo \
  -v /git/ngx_data_vs_time/:/var/www \
  mhowlett/nginx-build-base /repo/provision-run.sh
