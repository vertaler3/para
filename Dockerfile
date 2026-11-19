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
# Источники apt → зеркало mirror.yandex.ru (archive/security.ubuntu.com из сети
# владельца недоступны — таймауты; журнал 2026-11-19-fix-dockerfile-apt-mirror.md).
# Идемпотентно, оба формата: sources.list (jammy) и *.sources (deb822, noble).
RUN set -eux; \
    for f in /etc/apt/sources.list /etc/apt/sources.list.d/*.sources; do \
      [ -f "$f" ] && sed -i 's|http://archive.ubuntu.com/ubuntu|http://mirror.yandex.ru/ubuntu|g; s|http://security.ubuntu.com/ubuntu|http://mirror.yandex.ru/ubuntu|g; s|http://ru.archive.ubuntu.com/ubuntu|http://mirror.yandex.ru/ubuntu|g' "$f" || true; \
    done; \
    apt-get update \
 && apt-get install -y --no-install-recommends ninja-build ccache libargon2-dev \
      git zlib1g-dev fonts-dejavu-core \
 # libharu v2.4.6 из исходников (02 §8.1; лицензия zlib/libPNG, pin — Р-12):
 # build-стадия видит пин коммита тега; файл шрифта — в рантайм-слой ниже
 && git clone -q --depth 1 --branch v2.4.6 \
      https://github.com/libharu/libharu.git /tmp/libharu \
 && test "$(git -C /tmp/libharu rev-parse HEAD)" \
      = "3467749fd1c0ab6ca6ed424d053b1ea53c1bf67c" \
 || { echo "libharu v2.4.6 pin mismatch — остановлено (Р-12)"; exit 1; } \
 && cmake -S /tmp/libharu -B /tmp/libharu/build -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=OFF -DLIBHPDF_EXAMPLES=OFF \
 && cmake --build /tmp/libharu/build -j"$(nproc)" \
 && cmake --install /tmp/libharu/build \
 && cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build -j"$(nproc)"

# ---- рантайм ----
FROM --platform=linux/amd64 ubuntu:24.04
# Источники apt → зеркало (см. build-стадию выше).
RUN set -eux; \
    for f in /etc/apt/sources.list /etc/apt/sources.list.d/*.sources; do \
      [ -f "$f" ] && sed -i 's|http://archive.ubuntu.com/ubuntu|http://mirror.yandex.ru/ubuntu|g; s|http://security.ubuntu.com/ubuntu|http://mirror.yandex.ru/ubuntu|g; s|http://ru.archive.ubuntu.com/ubuntu|http://mirror.yandex.ru/ubuntu|g' "$f" || true; \
    done; \
    apt-get update \
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
# TTF с кириллицей для PDF-справок (Э5, Р-12): из apt-пакета fonts-dejavu-core
# build-стадии; лицензия bitstream-vera (пермиссивная) — Р-12.
COPY --from=build /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf \
     /opt/para/fonts/DejaVuSans.ttf
COPY config.json /etc/app/config.json
RUN mkdir -p /opt/para/data && chown -R 10001:10001 /opt/para/data
WORKDIR /opt/para
USER 10001
EXPOSE 8080
# --seed: демо-контур сеет фикстуры при каждом старте — идемпотентно
# (маркер 'fixtures' в schema_migrations, docs/decisions.md Р-5/Р-8;
# повторный старт на томе ничего не дублирует). Прод-контур без демо-данных
# (Э9) запускается без флага.
ENTRYPOINT ["/opt/para/para", "--config", "/etc/app/config.json", "--seed"]
