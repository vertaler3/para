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
        HPDF_UseUTFEncodings(pdf);  // TextOut/Info — строки UTF-8
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
        HPDF_Font font = HPDF_GetFont(pdf, fontName, nullptr);
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

        // Маркер штампа — KEYWORDS Info-словаря (сырые байты файла, см.
        // заголовок класса: литерал с OCTAL-эскейпами UTF-8 = kStampMarker).
        HPDF_SetInfoAttr(pdf, HPDF_INFO_KEYWORDS, kStampText);
        HPDF_SetInfoAttr(pdf, HPDF_INFO_TITLE,
                         (data.typeTitle + " (ДЕМО)").c_str());

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
        r.bytes = std::move(bytes);
        r.ok = true;
        return r;
    }
}

}  // namespace para
