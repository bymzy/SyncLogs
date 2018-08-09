#! /bin/bash

build/bin/SyncLog --peers 127.0.0.1:1001:1,127.0.0.1:1002:2,127.0.0.1:1003:3 --sid 1 --logdir /tmp/sid1/log --datadir /tmp/sid1/data &
build/bin/SyncLog --peers 127.0.0.1:1001:1,127.0.0.1:1002:2,127.0.0.1:1003:3 --sid 2 --logdir /tmp/sid2/log --datadir /tmp/sid2/data &
build/bin/SyncLog --peers 127.0.0.1:1001:1,127.0.0.1:1002:2,127.0.0.1:1003:3 --sid 3 --logdir /tmp/sid3/log --datadir /tmp/sid3/data &
