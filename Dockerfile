FROM ubuntu:latest

COPY build/bangserver /usr/local/bin/bangserver

EXPOSE 47654

ENTRYPOINT [ "/usr/local/bin/bangserver", "-v" ]
