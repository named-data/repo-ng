#!/usr/bin/env bash
set -x
set -e

# Run tests
sudo rm -Rf ~/.ndn
mkdir ~/.ndn

sudo cp /usr/local/etc/ndn/nfd.conf.sample /usr/local/etc/ndn/nfd.conf

IS_OSX=$( python -c "print 'yes' if 'OSX' in '$NODE_LABELS'.strip().split(' ') else 'no'" )
IS_LINUX=$( python -c "print 'yes' if 'Linux' in '$NODE_LABELS'.strip().split(' ') else 'no'" )

if [ $IS_OSX = "yes" ]; then
    security unlock-keychain -p "named-data"
fi

./build/unit-tests -l test_suite
