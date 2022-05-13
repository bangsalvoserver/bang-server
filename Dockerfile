FROM alpine:latest

RUN apk update && apk add jsoncpp
RUN apk add --no-cache --virtual .build_deps \
    g++ cmake ninja pkgconf \
    fmt-dev jsoncpp-dev boost-dev asio-dev

COPY . /usr/src/bang
WORKDIR /usr/src/bang/build

RUN cmake -G Ninja -DCMAKE_BUILD_TYPE=Release .. && cmake --build .
RUN cmake --install . && ldconfig /usr/local/lib

WORKDIR /
RUN rm -rf /usr/src/bang && apk del .build_deps

EXPOSE 47654

CMD [ "/usr/local/bin/bangserver" ]