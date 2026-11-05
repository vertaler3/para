# «Пара» — цифровой деканат (концепт хакатона)
# Docker multi-stage по спецификации 02 §8.1 (docs/02-техническое-решение.md).
# Отклонения от спецификации — docs/decisions.md Р-4:
#   1) тег build-образа v1.9.14 (тега 1.9.14 на Docker Hub нет);
#   2) COPY config.json → /etc/app/config.json (его читает ENTRYPOINT).

# ---- сборка ----
# --platform=linux/amd64: образ drogon существует только как amd64; без явного
# пина рантайм-стадия ubuntu:24.04 резолвится в нативную arm64 (на arm64-хосте)
# и amd64-бинарь не стартует (rosetta error — docs/decisions.md Р-4 п.5)
FROM --platform=linux/amd64 drogonframework/drogon:v1.9.14 AS build
WORKDIR /src
COPY . .
RUN apt-get update \
 && apt-get install -y --no-install-recommends ninja-build ccache \
 && cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build -j"$(nproc)"

# ---- рантайм ----
FROM --platform=linux/amd64 ubuntu:24.04
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      libsqlite3-0 libssl3t64 libargon2-1 ca-certificates sqlite3 tzdata \
      libc-ares2 libjsoncpp25 libpq5 libmariadb3 \
 && rm -rf /var/lib/apt/lists/*
# Единственная зависимость, которой нет в ubuntu:24.04: libdrogon.a собран
# против hiredis 0.14 (soname libhiredis.so.0.14), а в noble hiredis 1.2
# с другим soname — копируем точную версию из build-образа (Р-4 п.6);
# soname-ссылку создаёт ldconfig. Остальные зависимости вшитого в бинарь
# ORM/DNS-кода закрыты apt-пакетами выше (soname'ы совпадают с noble).
COPY --from=build /usr/lib/x86_64-linux-gnu/libhiredis.so.0.14 /usr/lib/x86_64-linux-gnu/
RUN ldconfig
COPY --from=build /src/build/para      /opt/para/para
COPY --from=build /src/views           /opt/para/views
COPY --from=build /src/static          /opt/para/static
COPY --from=build /src/sql             /opt/para/sql
COPY config.json /etc/app/config.json
RUN mkdir -p /opt/para/data && chown -R 10001:10001 /opt/para/data
WORKDIR /opt/para
USER 10001
EXPOSE 8080
ENTRYPOINT ["/opt/para/para", "--config", "/etc/app/config.json"]
