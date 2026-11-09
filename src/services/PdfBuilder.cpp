#include "services/PdfBuilder.h"

#include <cmath>
#include <string>

#include <drogon/drogon.h>

#include <hpdf.h>

namespace para
{
namespace
{

// Ошибки libharu: в серверный лог (02 §5.1 — наружу без деталей); флаг
// читает certificate после построения.
void onHpdfError(HPDF_STATUS errorNo, HPDF_STATUS detailNo, void* /*user*/)
{
    LOG_ERROR << "PdfBuilder: ошибка libharu " << std::hex << errorNo << "/"
              << detailNo << std::dec;
}

// Линия блока «Кому» (шрифт и размер выставлены вызывающим).
void textOut(HPDF_Page page, HPDF_REAL x, HPDF_REAL y, const std::string& s)
{
    HPDF_Page_TextOut(page, x, y, s.c_str());
}

// «Ключ: значение» — значение может отсутствовать (профиль без поля —
// вымышленные фикстуры полные, но защитный «—» вместо пустоты).
std::string kv(const std::string& key, const std::string& value)
{
    return key + ": " + (value.empty() ? "—" : value);
}

// Гейт Э5 раунд 2: libharu пишет строки Info-словаря литералом с
// OCTAL-эскейпами UTF-8 («\320\224…») — в сыром поиске по байтам «ДЕМО»
// (UTF-8) не находится. Заменяем значение /Keywords на raw UTF-8 С
// СОХРАНЕНИЕМ ДЛИНЫ (ASCII-паддинг): смещения в xref не сдвигаются, PDF
// остаётся валидным; в сырых байтах появляются настоящие UTF-8 байты
// «ДЕМО · вымышленные данные» (в литеральной строке PDF сырые байты
// ≥0x80 допустимы синтаксисом; в метаданных-панели просмотрщика значение
// — не spec-TextString, для демо принято, видимый штамп — на листе).
void revealKeywordsUtf8(std::string& bytes)
{
    const std::string key = "/Keywords (";
    const auto open = bytes.find(key);
    if (open == std::string::npos)
        return;
    const auto close = bytes.find(')', open + key.size());
    if (close == std::string::npos)
        return;
    const std::size_t innerOld = close - (open + key.size());
    std::string value =
        PdfBuilder::kStampText;  // raw UTF-8 «ДЕМО · … DEMO-FAKE-DATA»
    if (value.size() > innerOld)
        return;  // короче не делаем — xref дороже
    while (value.size() + 2 <= innerOld)
        value += " ·";
    if (value.size() < innerOld)
        value.append(innerOld - value.size(), '.');
    if (value.size() != innerOld)
        return;  // страховка: при несовпадении длины НЕ трогаем байты
                 // (сдвиг xref сломал бы PDF; правки по гейту Э5 раунд 2)
    // Точная длина — замена на месте, размер файла не меняется.
    std::copy(value.begin(), value.end(),
              bytes.begin() + static_cast<std::ptrdiff_t>(open + key.size()));
}

}  // namespace

PdfBuilder::Result PdfBuilder::certificate(const CertificateData& data,
                                           const std::string& fontPath)
{
    Result r;
    if (fontPath.empty())
    {
        r.error = "не задан путь к TTF-шрифту";
        return r;
    }

    HPDF_Doc pdf = HPDF_New(onHpdfError, nullptr);
    if (pdf == nullptr)
    {
        r.error = "HPDF_New";
        return r;
    }

    {
        // Регистрирует энкодер «UTF-8» (тип DOUBLE_BYTE, hpdf_encoder_utf.c
        // пина v2.4.6) — его берёт HPDF_GetFont ниже. Info-словарь от этого
        // энкодера НЕ зависит: строки Info пишет pdf->cur_encoder (базовый
        // StandardEncoding — HPDF_SetCurrentEncoder никто не вызывает),
        // литералом с октал-эскейпами UTF-8, как в пробе Э5 (Р-12) —
        // revealKeywordsUtf8 ниже рассчитан именно на это.
        HPDF_UseUTFEncodings(pdf);
        // Потоки без сжатия (правки по гейту Э5 раунд 2): при сжатии контент
        // — FlateDecode, поиск по нему невозможен. Демо-объём не критичен.
        HPDF_SetCompressionMode(pdf, HPDF_COMP_NONE);
        const char* fontName =
            HPDF_LoadTTFontFromFile(pdf, fontPath.c_str(), HPDF_TRUE);
        if (fontName == nullptr || HPDF_GetError(pdf) != HPDF_OK)
        {
            r.error = "HPDF_LoadTTFontFromFile: " + fontPath;
            HPDF_Free(pdf);
            return r;
        }

        HPDF_Page page = HPDF_AddPage(pdf);
        HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
        // ФИКС БАГА 6 (кириллица — мусорные глифы на устройствах): энкодер
        // обязателен. Здесь раньше был nullptr — HPDF_GetFont без имени
        // энкодера берёт StandardEncoding (hpdf_doc.c пина: «encoding-name
        // is not specified → default-encoding»), однобайтовый BasicEncoder →
        // простой шрифт /Subtype TrueType (HPDF_TTFont_New принимает только
        // SINGLE_BYTE) и запись текста литералом (WriteEscapeText): сырые
        // UTF-8 байты кириллицы мапились в глифы по одному байту — мусор
        // на рантаймах вне контура. Энкодер «UTF-8» (симметрично вызову
        // HPDF_UseUTFEncodings выше, согласован с записью строк UTF-8)
        // переводит шрифт в Type0-путь (hpdf_font_cid.c: HPDF_Type0Font_New
        // для TRUETYPE fontdef): /Encoding Identity-H, /ToUnicode,
        // CIDFontType2 с CIDToGIDMap, дескриптор с /FontFile2; текст в
        // контент-стриме — hex-CID (UTF-8→Unicode→CID в
        // UTF8_Encoder_EncodeText_Func, hex-конвертация — WriteBinary),
        // глифы читаемы на любом устройстве. Косинусная матрица штампа 45°
        // (SetTextMatrix) — уровнем графики, от типа шрифта не зависит;
        // /FontFile2 и самодостаточность файла сохраняются.
        HPDF_Font font = HPDF_GetFont(pdf, fontName, "UTF-8");
        if (font == nullptr || HPDF_GetError(pdf) != HPDF_OK)
        {
            r.error = "HPDF_GetFont";
            HPDF_Free(pdf);
            return r;
        }

        // Шапка справки.
        HPDF_Page_SetFontAndSize(page, font, (HPDF_REAL)22);
        HPDF_Page_BeginText(page);
        textOut(page, (HPDF_REAL)60, (HPDF_REAL)770, data.typeTitle);
        HPDF_Page_SetFontAndSize(page, font, (HPDF_REAL)11);
        textOut(page,
                (HPDF_REAL)60,
                (HPDF_REAL)740,
                "Заявка № " + std::to_string(data.applicationId) + " · от " +
                    data.createdAt + " · статус: " + data.statusTitle);

        // Блок «Кому» — данные профиля владельца заявки (02 §3.2 поток 5).
        HPDF_Page_SetFontAndSize(page, font, (HPDF_REAL)14);
        textOut(page, (HPDF_REAL)60, (HPDF_REAL)680,
                kv("Кому", data.fio));
        textOut(page, (HPDF_REAL)60, (HPDF_REAL)652,
                kv("Факультет", data.faculty));
        textOut(page, (HPDF_REAL)60, (HPDF_REAL)624,
                kv("Программа", data.program));
        textOut(page, (HPDF_REAL)60, (HPDF_REAL)596,
                "Курс: " + (data.course.empty() ? "—" : data.course) +
                    ", группа: " +
                    (data.group.empty() ? "—" : data.group));
        textOut(page, (HPDF_REAL)60, (HPDF_REAL)568,
                kv("Студенческий билет №", data.studentCard));

        HPDF_Page_EndText(page);

        // Диагональный штамп 45° (гайд BE §7; 01 §4.4) — серым, поверх.
        const HPDF_REAL c = static_cast<HPDF_REAL>(0.70710678118654752);
        HPDF_Page_SetGrayFill(page, (HPDF_REAL)0.55);
        HPDF_Page_SetFontAndSize(page, font, (HPDF_REAL)30);
        HPDF_Page_BeginText(page);
        HPDF_Page_SetTextMatrix(page, c, c, -c, c, (HPDF_REAL)110,
                                (HPDF_REAL)110);
        HPDF_Page_TextOut(page, (HPDF_REAL)60, (HPDF_REAL)360, kStampText);
        HPDF_Page_SetTextMatrix(page, 1, 0, 0, 1, 0, 0);
        HPDF_Page_EndText(page);
        HPDF_Page_SetGrayFill(page, (HPDF_REAL)0);

        // Подвал — бейдж концепта (AGENTS.md §5.1).
        HPDF_Page_SetFontAndSize(page, font, (HPDF_REAL)10);
        HPDF_Page_BeginText(page);
        textOut(page, (HPDF_REAL)60, (HPDF_REAL)70,
                "«Пара» — цифровой деканат. Концепт для хакатона; "
                "все данные вымышленные.");
        HPDF_Page_EndText(page);

        // Маркер штампа — KEYWORDS Info-словаря; ниже revealKeywordsUtf8
        // переписывает его значение сырыми UTF-8 байтами (гейт Э5 раунд 2).
        HPDF_SetInfoAttr(pdf, HPDF_INFO_KEYWORDS, kStampText);
        HPDF_SetInfoAttr(pdf, HPDF_INFO_TITLE,
                         (data.typeTitle + " (ДЕМО)").c_str());
        // Проба свежести сборки (гейт Э3 раунд 2: старый образ — по этой
        // строке в PDF видно, чей бинарь собрал файл).
        HPDF_SetInfoAttr(pdf, HPDF_INFO_SUBJECT, kBuildTag);

        if (HPDF_GetError(pdf) != HPDF_OK)
        {
            r.error = "построение страницы";
            HPDF_Free(pdf);
            return r;
        }

        HPDF_SaveToStream(pdf);
        const HPDF_UINT size = HPDF_GetStreamSize(pdf);
        if (size == 0 || HPDF_GetError(pdf) != HPDF_OK)
        {
            r.error = "HPDF_SaveToStream";
            HPDF_Free(pdf);
            return r;
        }
        std::string bytes;
        bytes.resize(size);
        HPDF_UINT got = size;
        HPDF_ReadFromStream(pdf, reinterpret_cast<HPDF_BYTE*>(&bytes[0]),
                            &got);
        HPDF_Free(pdf);
        bytes.resize(got);
        if (got == 0 || bytes.compare(0, 5, "%PDF-") != 0)
        {
            r.error = "поток без %PDF-";
            return r;
        }
        // Сырые UTF-8 «ДЕМО…» в KEYWORDS — та же длина, xref не страдает.
        revealKeywordsUtf8(bytes);
        if (bytes.find(kStampMarker) == std::string::npos ||
            bytes.find("ДЕМО") == std::string::npos)
        {
            r.error = "маркер штампа не найден после построения";
            return r;
        }
        r.bytes = std::move(bytes);
        r.ok = true;
        return r;
    }
}

}  // namespace para
