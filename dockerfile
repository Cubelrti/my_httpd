FROM gcc:4.9
COPY . /usr/src/myapp
WORKDIR /usr/src/myapp/src
RUN make
CMD ["./my_http"]