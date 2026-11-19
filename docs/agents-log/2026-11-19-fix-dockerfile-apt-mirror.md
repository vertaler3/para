# Журнал: Dockerfile — источники apt на зеркало mirror.yandex.ru — 2026-11-19

- **Дата:** 19.11.2026 · **Ветка:** `fe_rebuild` (main только читался для сравнения: `git diff main -- Dockerfile` до правки пуст)
- **Дорожка:** инфраструктура (Dockerfile; код приложения не менялся)

## Промпт (суть)

`docker compose up -d --build` падает на `apt-get update` — «Connection failed» /
«Failed to fetch http://archive.ubuntu.com/...» и security.ubuntu.com: из сети владельца эти
серверы недоступны (таймауты), а mirror.yandex.ru/ubuntu и ru.archive.ubuntu.com отвечают 200.
Нужно: в КАЖДОЙ стадии Dockerfile с `apt-get update` перед ним идемпотентно подменить источники
на зеркало (оба формата — `/etc/apt/sources.list` и `/etc/apt/sources.list.d/*.sources`);
ничего больше не менять — пин libharu v2.4.6 / rev-parse 3467749…, `--platform`, базовые
образы, ENTRYPOINT `--seed`. docker-compose.yml и скрипты не трогать. Проверка: build без
«Failed to fetch»/«Unable to locate package», up + `/healthz` → 200, тесты — 100%; push в fe_rebuild.

## Что сделал

- Воспроизвёл с хоста: `curl http://archive.ubuntu.com/ubuntu/dists/noble/Release` → таймаут
  (000); `http://mirror.yandex.ru/ubuntu/dists/{jammy,noble}/Release` → 200.
- `Dockerfile`: в двух стадиях с apt — build (`drogonframework/drogon:v1.9.14`, Ubuntu jammy,
  формат `sources.list`) и рантайм (`ubuntu:24.04`, формат deb822
  `sources.list.d/ubuntu.sources`) — тот же `RUN` начинается с `set -eux;` и цикла `sed` по
  обоим форматам (archive / security / ru.archive → `http://mirror.yandex.ru/ubuntu`; файла
  нет — пропуск, повторный запуск ничего не меняет), затем прежняя цепочка `apt-get update && …`
  без изменений. Пин libharu, `--platform`, теги образов, COPY и ENTRYPOINT не тронуты
  (`git diff` — только вставки перед `apt-get update` + две строки комментария).
- Других вызовов apt в репозитории нет (`docker-compose.yml`, `tools/` — пусто по grep).

## Проверка (фактические выводы)

`docker compose build --progress=plain` → exit 0, «Image para-para Built». Строк «Failed to
fetch» / «Unable to locate package» / «Connection failed» в логе — **0**. Все загрузки apt
(64 строки Get/Hit) идут с **mirror.yandex.ru**:
```
#11 6.808 Fetched 34.0 MB in 6s (5333 kB/s)
#11 10.02 Fetched 3930 kB in 0s (10.1 MB/s)
#12 7.519 Fetched 49.5 MB in 7s (6,981 kB/s)
#12 11.20 Fetched 4,847 kB in 0s (15.4 MB/s)
```
`docker compose up -d` → `para-para-1 Started`; `curl -s -o /dev/null -w '%{http_code}'
localhost:8080/healthz` → **200**.
`docker compose --profile tests run --rm tests`:
```
1/2 Test #1: para-tests .......................   Passed
2/2 Test #2: security_tests ...................   Passed
100% tests passed, 0 tests failed out of 2
```
(в логе тестов «Failed to fetch» / «Unable to locate» — тоже 0).

## Примечания

- Перед `up` остановлен мой локальный контейнер `para-final-para-1` (проверочный, занимал
  :8080); тома и данные не трогались. `up` идёт на штатный проект `para` и том `para_para-data`.
- `set -u` безопасен для существующей цепочки (переменных, кроме `$f` и `$(nproc)`, нет);
  проверка пина libharu через `|| { …; exit 1; }` работает как раньше: `set -e` не срабатывает
  внутри списков `&&`/`||`.
