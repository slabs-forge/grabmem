FROM alpine:latest
COPY grabmem /bin
COPY grabmore /bin
RUN mkdir /run/grabmem
CMD /bin/grabmem
