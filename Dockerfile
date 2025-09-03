FROM alpine:3.19 AS build
RUN apk add --no-cache build-base cmake ninja openssl-dev
WORKDIR /src
COPY . .
RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DACX_WITH_OPENSSL=ON         && cmake --build build --target acx         && strip build/acx
FROM alpine:3.19
RUN adduser -D -u 10001 acx
USER acx
WORKDIR /home/acx
COPY --from=build /src/build/acx /usr/local/bin/acx
ENTRYPOINT ["/usr/local/bin/acx"]
CMD ["help"]


LABEL org.opencontainers.image.title="ACX Enterprise Contact CLI"               org.opencontainers.image.description="Auditable contact CLI with elite OSS governance & security"               org.opencontainers.image.licenses="Apache-2.0"


HEALTHCHECK --interval=30s --timeout=3s --retries=3 CMD ["/app/acx","--version"] || exit 1
