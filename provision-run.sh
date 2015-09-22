#!/bin/bash

cd /repo/nginx-1.9.3/
make install
/usr/sbin/nginx -c /repo/nginx.conf
