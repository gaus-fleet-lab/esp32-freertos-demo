FROM toshi38/esp32-freertos

RUN mkdir -p /gaus-esp32-freertos
WORKDIR /gaus-esp32-freertos

COPY . .

#Provide a fake certificate so the build works:
RUN echo "FAKE_CERT" >> main/ca_cert.pem

RUN make defconfig && make
