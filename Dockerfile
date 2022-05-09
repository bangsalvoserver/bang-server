FROM ubuntu

RUN apt-get update && apt-get install -y --no-install-recommends \
    g++ cmake ninja-build pkgconf libfmt-dev libjsoncpp-dev \
    libboost-dev libboost-system-dev libasio-dev

COPY . /home/src

RUN mkdir -p /home/src/build && cd /home/src/build && \
    cmake -G Ninja .. && cmake --build . -j6 && cmake --install . && ldconfig

RUN rm -rf /home/src

EXPOSE 47654

CMD [ "/usr/local/bin/bangserver" ]