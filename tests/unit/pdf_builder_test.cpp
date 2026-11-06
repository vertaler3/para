// Юнит-тесты PdfBuilder (Э5 шаг 4; гайд BE §7 «PDF начинается с %PDF- и
// содержит штамп»; 02 §3.2 поток 5):
//  - справка собирается: %PDF- в начале, кириллица встроенным TTF
//    (/FontFile2 — HPDF_LoadTTFontFromFile), %%EOF в конце;
//  - МАРКЕР ШТАМПА в сырых байтах: kStampMarker (OCTAL-эскейпы UTF-8 «ДЕМО»
//    из KEYWORDS Info-словаря — поведение libharu v2.4.6, Р-12) найден
//    в потоке И в файле, сохранённом на диск и прочитанном обратно
//    (постановка шага: «сохранить PDF во временный файл и найти маркер»);
//  - битый путь к шрифту → ok=false (fail без исключений).
// Шрифт: env PARA_TEST_FONT → /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf
// (пакет fonts-dejavu-core build-стадии Dockerfile, Р-12). Отсутствие
// шрифта — честный FAIL с подсказкой, не «тихий» пропуск.
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

#include <doctest.hpp>

#include "services/PdfBuilder.h"

namespace fs = std::filesystem;
using para::PdfBuilder;

namespace
{

fs::path tempFontDir()
{
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch()
                           .count();
    static int counter = 0;
    auto path = fs::temp_directory_path() /
                ("para-pdf-" + std::to_string(stamp) + "-" +
                 std::to_string(++counter));
    fs::create_directories(path);
    return path;
}

std::string fontPath()
{
    // Переопределяется env (контейнеры с другим расположением шрифта).
    if (const char* env = std::getenv("PARA_TEST_FONT"))
        return std::string(env);
    return "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
}

PdfBuilder::CertificateData sample()
{
    PdfBuilder::CertificateData d;
    d.applicationId = 1;
    d.typeTitle = "Академическая справка";
    d.createdAt = "2026-11-02T09:12:00Z";
    d.statusTitle = "Готова";
    d.fio = "Хакимова Дина Ринатовна";
    d.faculty = "ФМО";
    d.program = "Мировая экономика";
    d.course = "1";
    d.group = "1-ФМО";
    d.studentCard = "24-1-004";
    return d;
}

}  // namespace

TEST_CASE("PdfBuilder: справка — %PDF-, встроенный TTF, маркер штампа в байтах")
{
    const std::string font = fontPath();
    if (!fs::exists(font))
    {
        FAIL("шрифт не найден: ", font,
             " (установи fonts-dejavu-core или задай PARA_TEST_FONT)");
        return;
    }

    const auto r = PdfBuilder::certificate(sample(), font);
    REQUIRE(r.ok);
    REQUIRE(r.bytes.size() > 2000);
    // %PDF- в начале потока (гайд BE §7) и %%EOF в конце.
    CHECK(r.bytes.compare(0, 5, "%PDF-") == 0);
    CHECK(r.bytes.find("%%EOF") != std::string::npos);
    // Кириллица — встроенным TTF (/FontFile2), не стандартным шрифтом.
    CHECK(r.bytes.find("/FontFile2") != std::string::npos);
    // МАРКЕР ШТАМПА (ASCII-хвост — выживает в сырых байтах и в любой
    // декодированной форме; правки по гейту Э5 раунд 1, Р-12).
    const std::string marker = PdfBuilder::kStampMarker;
    CHECK(marker == "DEMO-FAKE-DATA");
    CHECK(r.bytes.find(marker) != std::string::npos);
    // Кириллическая часть штампа — в сырых байтах как OCTAL-эскейпы UTF-8.
    CHECK(r.bytes.find("\\320\\224\\320\\225\\320\\234\\320\\236") !=
          std::string::npos);
}

TEST_CASE("PdfBuilder: маркер находится в файле, сохранённом на диск")
{
    const std::string font = fontPath();
    REQUIRE(fs::exists(font));

    const auto r = PdfBuilder::certificate(sample(), font);
    REQUIRE(r.ok);

    // Сохранить во временный файл и прочитать обратно — маркер обязан
    // присутствовать и в файле (постановка шага 4).
    auto dir = tempFontDir();
    const auto file = dir / "spravka.pdf";
    {
        std::ofstream out(file, std::ios::binary);
        REQUIRE(out.good());
        out.write(r.bytes.data(), static_cast<std::streamsize>(r.bytes.size()));
        REQUIRE(out.good());
    }
    {
        std::ifstream in(file, std::ios::binary);
        REQUIRE(in.good());
        std::string content((std::istreambuf_iterator<char>(in)),
                            std::istreambuf_iterator<char>());
        REQUIRE(content.size() == r.bytes.size());
        CHECK(content.compare(0, 5, "%PDF-") == 0);
        const std::string marker = PdfBuilder::kStampMarker;
        CHECK(content.find(marker) != std::string::npos);
        // Позиция маркера — в Info-словаре, рядом с "Keywords".
        const auto pos = content.find("/Keywords");
        REQUIRE(pos != std::string::npos);
        CHECK(content.find(marker, pos) != std::string::npos);
        // Кириллическая часть (октал-эскейпы) — тоже в файле.
        CHECK(content.find("\\320\\224\\320\\225\\320\\234\\320\\236",
                           pos) != std::string::npos);
    }
    std::error_code ec;
    fs::remove_all(dir, ec);
}

TEST_CASE("PdfBuilder: пустой/битый путь шрифта — ok=false, без исключений")
{
    const auto r1 = PdfBuilder::certificate(sample(), "");
    CHECK_FALSE(r1.ok);
    CHECK(r1.bytes.empty());
    CHECK_FALSE(r1.error.empty());

    const auto r2 = PdfBuilder::certificate(sample(), "/nonexistent/Font.ttf");
    CHECK_FALSE(r2.ok);
    CHECK(r2.bytes.empty());
    CHECK_FALSE(r2.error.empty());
}
