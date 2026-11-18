#!/usr/bin/env python3
"""fe_rebuild (Р-31): сборка static/css/app.css дизайна v3.

app.css = CSS эталона design-drafts/v3/assets/prototype.css (без блока токенов —
они в tokens.css; шрифты → /static/fonts/; русские подписи из CSS → attr();
селекторы по href прототипа → data-*) + прод-правила, которых нет в v2
(design-drafts/assets/app.css): хуки htmx/состояний/5.17/5.18/бронь/стол и т.п.
Прод-правила, перекрывающие каркас прототипа (таб-бар, nav-link, тост, скелетон,
focus), отбрасываются — их место занимает версия прототипа.

Запуск (одноразовый, из корня): python3 -I tools/build_app_css.py <старый app.css> > static/css/app.css
"""
import re, sys, pathlib
ROOT = pathlib.Path(__file__).resolve().parents[1]

def strip_comments(s): return re.sub(r'/\*.*?\*/', '', s, flags=re.S)

def parse(s, ctx=''):
    s = strip_comments(s); i = 0; n = len(s); out = []
    while i < n:
        j = s.find('{', i)
        if j < 0: break
        pre = s[i:j].strip(); d = 1; k = j + 1
        while d and k < n:
            d += {'{': 1, '}': -1}.get(s[k], 0); k += 1
        body = s[j + 1:k - 1]
        if pre.startswith(('@media', '@supports')):
            p = re.sub(r'\s+', ' ', pre)
            p = re.sub(r'@media\s*\(', '@media(', p).replace(': ', ':')
            out += parse(body, p)
        else:
            out.append((ctx, re.sub(r'\s+', ' ', pre).replace(', ', ','),
                        re.sub(r'\s+', ' ', body.strip()).rstrip(';').replace('; ', ';')))
        i = k
    return out

CHROME_SEL = ('body:not(.entry-page)', '.site-header', '.header-actions .icon-button',
              '.header-date', '.staff-nav a', '.campus-nav')

def chrome(css):
    """Рамка навигации (шапка 360px, сайдбар ≥1100px) — токены --chrome* вместо
    --primary/--on-primary (Р-31(а): иначе тёмная тема инвертирует сайдбар в светлый).
    Вход (body[data-page=login], .entry-page) не трогается."""
    def fix(m):
        sel, body = m.group(1), m.group(2)
        if not any(k in sel for k in CHROME_SEL) or 'data-page=login' in sel or sel.lstrip().startswith('.entry-page'):
            return m.group(0)
        if '[aria-current]' in sel and 'primary-soft-bg' in body:
            body = body.replace('var(--primary-soft-bg)', 'var(--chrome-active-bg)').replace('color:var(--primary)', 'color:var(--chrome-active-text)')
        body = (body.replace('var(--on-primary)', 'var(--on-chrome)').replace('var(--primary-hover)', 'var(--chrome-hover)')
                    .replace('background:var(--primary)', 'background:var(--chrome)').replace('border-bottom-color:var(--primary)', 'border-bottom-color:var(--chrome)'))
        return sel + '{' + body + '}'
    return re.sub(r'([^{}@]+)\{([^{}]*)\}', fix, css)

FIXES = '''.ob-footer .button{white-space:nowrap}
.htmx-indicator.skeleton{display:none}
.htmx-request .htmx-indicator.skeleton,.htmx-request.htmx-indicator.skeleton{display:block;opacity:1}
a.notification,a.app-card,a.next-class,a.lesson,a.staff-change{text-decoration:none}
.para-error-art__grid{stroke:var(--border)}
.para-error-art__nodes,.para-error-art__handles path+path{fill:var(--surface)}
.schedule-skeleton{display:none}
.schedule-skeleton.htmx-request{display:grid;gap:.75rem;margin-bottom:1.5rem}
.lesson[data-event]{cursor:pointer}
.lesson.moved{box-shadow:none}
.lesson.moved .lesson-detail{color:var(--text-2)}
button.change-detail{display:inline-flex;align-items:center;min-height:44px;padding:0;border:0;background:var(--clear);font:inherit;font-size:.875rem;color:var(--danger-text);text-decoration:underline;text-underline-offset:3px;cursor:pointer}
.lesson-dialog .change-block{margin-top:1rem}
.week-board .day-group.is-today>.section-title h2{color:inherit}
.week-board .day-group.is-today .schedule-list{border:1px solid var(--border);border-radius:13px}
.week-board .lesson.cancelled{background:var(--surface-2)}
.week-board .lesson.cancelled .lesson-room s{color:var(--text-2)}
.week-changes{margin:1.5rem 0}
.week-changes .change-block{margin-top:.75rem}
@media(min-width:1200px){.week-board .lesson{grid-row:var(--event-row)/span var(--event-span,3)}.week-board .day-group.is-today .schedule-list{border:0;border-radius:0}.week-changes{max-width:600px}}
'''

DROP = {'@font-face', ':focus-visible', '.primary-nav', '.nav-link', '.skeleton'}

def main():
    old = pathlib.Path(sys.argv[1]).read_text(encoding='utf-8')
    proto = (ROOT / 'design-drafts/v3/assets/prototype.css').read_text(encoding='utf-8')
    v2 = set(parse((ROOT / 'design-drafts/assets/app.css').read_text(encoding='utf-8')))
    # 1) эталон: токены — в tokens.css
    proto = re.sub(r':root\s*\{\s*color-scheme:light;.*?\}', '', proto, count=1, flags=re.S)
    proto = re.sub(r':root\s*\{\s*--ease-workspace:[^}]*\}', '', proto, count=1)
    proto = proto.replace('url(fonts/', 'url(/static/fonts/')
    proto = proto.replace("content:'МОЁ ПРОСТРАНСТВО'", 'content:attr(data-heading)')
    proto = proto.replace("content:'УЧЕБНЫЙ ОФИС'", 'content:attr(data-heading)')
    proto = proto.replace('[href="profile.html"]', '[data-nav=profile]')
    proto = chrome(proto)
    assert 'МОЁ' not in proto and 'profile.html' not in proto
    # 2) прод-дополнения
    extra = []
    for ctx, sel, body in parse(old):
        if (ctx, sel, body) in v2: continue
        if ctx == '' and sel in DROP: continue
        if ctx == '' and sel == '.toast' and 'position:fixed' in body: continue
        if sel == '.toast-action': body = body.replace('color:var(--bg)', 'color:var(--link)')
        extra.append((ctx, sel, body))
    out = ['/* Пара · app.css — дизайн v3 (navy), fe_rebuild, Р-31.\n'
           '   Сгенерирован tools/build_app_css.py: эталон design-drafts/v3/assets/prototype.css\n'
           '   (без токенов — static/css/tokens.css) + прод-правила (htmx, состояния, 5.17/5.18,\n'
           '   бронь, стол сотрудника). Цвета — только var(--токен) (AGENTS §5.6). */\n',
           proto.strip(), '\n\n/* ---- прод-правила (хуки и экраны прода поверх эталона v3) ---- */\n']
    cur = None
    for ctx, sel, body in extra:
        if ctx != cur:
            if cur: out.append('}\n')
            if ctx: out.append(ctx + '{\n')
            cur = ctx
        out.append(f'{sel}{{{body}}}\n')
    if cur: out.append('}\n')
    out.append('\n/* ---- стыки эталона v3 и прод-разметки (fe_rebuild) ---- */\n' + FIXES)
    sys.stdout.write(''.join(out))

if __name__ == '__main__':
    main()
