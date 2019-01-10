FROM toshi38/esp32-freertos

RUN mkdir -p /gaus-esp32-freertos
WORKDIR /gaus-esp32-freertos

COPY . .

RUN make defconfig && make
