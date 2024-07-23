FROM sptrakesh/cpr as cpr
FROM sptrakesh/boost as base

COPY --from=cpr /opt/local /opt/local
COPY CMakeLists.txt /opt/spt/geocode/
COPY GeocodeConfig.cmake /opt/spt/geocode/
COPY src /opt/spt/geocode/src
COPY docker/env.sh /opt/spt/geocode/docker/

WORKDIR /opt/spt/geocode/build
RUN apk add --no-cache readline-dev curl-dev \
  && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/opt/local -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=mold" -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=mold" .. \
  && make -j8 \
  && make install

FROM sptrakesh/cppbase

COPY --from=base /opt/local /opt/local
COPY --from=base /opt/spt/include /opt/spt/include
COPY --from=base /opt/spt/lib /opt/spt/lib
COPY docker/scripts/*.sh /opt/spt/bin/

WORKDIR /opt/spt
ENV LD_LIBRARY_PATH=/usr/lib \
  PATH=$PATH:/opt/spt/bin