FROM ubuntu:latest

COPY build/bin/bangserver /usr/local/bin

EXPOSE 47654

CMD [ "/usr/local/bin/bangserver" ]