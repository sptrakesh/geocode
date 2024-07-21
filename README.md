# geocode
A library of utility functions related to geo-coordinates used across a few different projects.

Provides functions for performing common activities such as:
* Calculate distance between two coordinates using Vincenty's formula.
* Check if a point falls within a bounding polygon.
* Look up the street address for a specified geo-coordinate using [positionstack](https://positionstack.com/).
* Look up the geo-coordinate for a specified street address using [positionstack](https://positionstack.com/).
* Cluster a set of coordinates using [k-means](https://en.wikipedia.org/wiki/K-means_clustering) algorithm.
* Convert coordinates into **Open Location Code**.

## Build
Build the library using [cmake](https://cmake.org):

```shell
git clone https://github.com/sptrakesh/geocode.git
cd geocode
cmake -DCMAKE_PREFIX_PATH="/usr/local/boost;/usr/local/cpr" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr/local/spt \
  -DOPENSSL_ROOT_DIR=/opt/homebrew/opt/openssl \
  -S . -B build
cmake --build build -j12
sudo cmake --install build
```

## Acknowledgements
This software has been developed mainly using work other people/projects have contributed.
The following are the components used to build this software:

* **[Boost](https://boost.org)** - Boost libraries.
* **[vincentys-formula](https://github.com/dariusarnold/vincentys-formula)** - Vincenty's formula implementation for distance computation.
* **[Open Location Code](https://github.com/google/open-location-code)** - Google implementation of the open location code standard.
* **[cpr](https://github.com/libcpr/cpr)** - Curl requests for C++.
* **[geofence](https://github.com/chrberger/geofence)** - Geofence library.
* **[positionstack](https://positionstack.com/)** - API for address lookup/translation.
* **[Reasonable Deviations](https://reasonabledeviations.com/2019/10/02/k-means-in-cpp/)** - k-means algorithm implementation.
* **[Catch2](https://github.com/catchorg/Catch2)** - Unit testing framework.