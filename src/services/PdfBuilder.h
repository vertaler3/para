#pragma once

#include <string>

namespace para
{

// PDF-справка заявки (Э5 шаг 4; 02 §3.2 поток 5, §2.2/§8.1: libharu v2.4.6
// из исходников — см. docs/decisions.md Р-12). Кириллица — встроенным TTF
// DejaVu Sans (HPDF_LoadTTFontFromFile с встраиванием — шрифт попадает
// в файл как /FontFile2); шрифт берётся с энкодером «UTF-8»
// (HPDF_UseUTFEncodings + HPDF_GetFont(..., "UTF-8") — фикс бага 6):
// libharu собирает шрифт как Type0/CIDFontType2 (Identity-H + /ToUnicode +
// CIDToGIDMap), текст страницы в контент-стриме — hex-CID, глифы читаемы
// на любом устройстве (nullptr-энкодер давал простой TrueType/
// StandardEncoding — UTF-8 байты мапились в глифы по одному, мусор);
// диагональный штамп 45° «ДЕМО · вымышленные данные» (требование 01 §4.4 /
// гайда BE §7) + тот же текст в KEYWORDS Info-словаря.
//
// МАРКЕР ШТАМПА В ФАЙЛЕ (юнит-тест pdf_builder_test.cpp на сохранённом
// файле): Info-словарь от энкодера шрифта НЕ зависит — строки Info пишет
// pdf->cur_encoder (базовый StandardEncoding): ЛИТЕРАЛОМ с OCTAL-эскейпами
// UTF-8 (не hex UTF-16BE), «ДЕМО» = \320\224\320\225\320\234\320\236;
// revealKeywordsUtf8 переписывает значение /Keywords сырыми UTF-8 байтами
// с сохранением длины (xref не сдвигается). После фикса бага 6 текст
// страницы — hex-CID, поэтому ASCII-маркер «DEMO-FAKE-DATA» и сырые
// UTF-8 «ДЕМО» живут в байтах файла только в /Keywords (тесты ищут по
// всему файлу и «рядом с /Keywords» — совместимо; ASCII проходит любое
// декодирование без изменений). Последовательность детерминирована пином
// библиотеки v2.4.6 (Р-12) и проверяется тестом на каждом прогоне —
// смена поведения libharu сломает тест, а не промолчит.
class PdfBuilder
{
  public:
    struct CertificateData
    {
        int64_t applicationId = 0;
        std::string typeTitle;   // «Академическая справка» (каталог models)
        std::string createdAt;   // created_at заявки (DDL-формат ISO)
        std::string statusTitle; // «Готова» / «Выдана» (models)
        std::string fio;
        std::string faculty;
        std::string program;
        std::string course;
        std::string group;
        std::string studentCard;
    };

    struct Result
    {
        bool ok = false;
        std::string bytes;  // сырой PDF (HPDF_SaveToStream → ReadFromStream)
        std::string error;  // причина при !ok — только в серверный лог
    };

    // Текст штампа: на листе диагональю 45° и в KEYWORDS Info-словаря
    // (кириллическая часть — требование гайда BE §7/01 §4.4; ASCII-хвост —
    // машинный маркер).
    static constexpr const char* kStampText =
        "ДЕМО · вымышленные данные · DEMO-FAKE-DATA";

    // Машинный маркер штампа: гарантированно есть в сырых байтах файла И
    // в любой декодированной форме текста (см. выше).
    static constexpr const char* kStampMarker = "DEMO-FAKE-DATA";

    // Тег сборки в Info-словаре (/Subject) — проба свежести образа для
    // гейта: если в PDF нет этой строки, проверка шла по бинарю старой
    // сборки (прецедент Э3, раунд 2).
    static constexpr const char* kBuildTag = "para-e5-gate-r2";

    // Собрать справку; fontPath — файл TTF с кириллицей (DejaVu Sans).
    // Любая ошибка libharu → ok=false (контроллер отвечает 500 без деталей,
    // 02 §5.1); память освобождает HPDF_Free (RAII-обёртка внутри).
    static Result certificate(const CertificateData& data,
                              const std::string& fontPath);
};

}  // namespace para
