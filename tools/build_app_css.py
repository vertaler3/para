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

FIXES = '''[data-submitted-block]{margin-bottom:1.5rem}
[data-submitted-block] .stack{margin-top:1rem;gap:.5rem;border:0;border-radius:0;background:var(--clear);overflow:visible}
[data-document-block] .document-paper{margin-top:.75rem}
[data-document-block] .document-paper .detail-list{margin-top:1rem}
@media print{body[data-page=application] .content-form>:not([data-document-block]),body[data-page=application] [data-submitted-block],body[data-page=application] [data-document-block]>:not(.document-paper),#offline-banner{display:none!important}body[data-page=application] .content-form{max-width:none;padding:0}}
.app-card[hidden],.app-list[hidden]{display:none!important}
.document-flow .section-title .button{flex:none}
.ob-footer .button{white-space:nowrap}
.htmx-indicator.skeleton{display:none}
.htmx-request .htmx-indicator.skeleton,.htmx-request.htmx-indicator.skeleton{display:block;opacity:1}
a.notification,a.app-card,a.next-class,a.lesson,a.staff-change{text-decoration:none}
.para-error-art__grid{stroke:var(--border)}
.para-error-art__nodes,.para-error-art__handles path+path{fill:var(--surface)}
@media(min-width:1100px){body:not(.entry-page):not([data-page="not-found"])>.footer{color:var(--on-chrome);border-top-color:var(--chrome-hover);margin-top:1.5rem}
body:not(.entry-page):not([data-page="not-found"])>.footer .lang-switch button,body:not(.entry-page):not([data-page="not-found"])>.footer .lang-switch span{color:var(--on-chrome)}
body:not(.entry-page):not([data-page="not-found"])>.footer .lang-switch button[aria-pressed=true]{color:var(--on-chrome);text-decoration:underline;text-underline-offset:4px}}
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
.campus-scene{cursor:grab}
.campus-scene.is-dragging{cursor:grabbing}
.campus-scene:focus-visible{outline:3px solid var(--link);outline-offset:-3px}
.campus-svg{display:block;width:100%;height:100%;touch-action:none;user-select:none;-webkit-user-select:none}
.campus-labels{position:absolute;inset:0;z-index:2;pointer-events:none}
.campus-labels .campus-building-label{width:max-content;display:flex;align-items:center;justify-content:center;pointer-events:auto;cursor:pointer}
.campus-ground{fill:var(--surface);stroke:var(--border);stroke-width:1}
.campus-block{cursor:pointer;outline:none}
.campus-block polygon{stroke:var(--primary);stroke-width:1;stroke-linejoin:round}
.campus-roof{fill:var(--surface)}
.campus-wall-l{fill:var(--surface-2)}
.campus-wall-r{fill:var(--border)}
.campus-block.is-empty polygon{stroke-dasharray:4 3}
.campus-block:hover>.campus-roof{fill:var(--surface-2)}
.campus-block:focus-visible polygon{stroke:var(--link);stroke-width:3}
.campus-block.is-selected>.campus-roof,.campus-slab.is-active .campus-roof{fill:var(--primary)}
.campus-block.is-selected>.campus-wall-l,.campus-slab.is-active .campus-wall-l{fill:var(--primary-hover)}
.campus-block.is-selected>.campus-wall-r,.campus-slab.is-active .campus-wall-r{fill:var(--primary-pressed)}
.campus-slab.is-active polygon{stroke:var(--surface)}
.campus-loading[hidden]{display:none}
.campus-state{flex-direction:column;gap:.75rem;text-align:center;padding:1.5rem}
.campus-state h3{font-size:1rem;color:var(--text)}
.campus-skeleton{width:min(420px,80%);height:12rem;margin:0}
.map-plan--floor{display:flex;flex-direction:column;gap:.75rem;min-height:0}
.map-side{display:grid;grid-template-columns:repeat(auto-fill,minmax(92px,1fr));gap:.75rem}
.map-plan--floor .corridor{padding:.6rem 0;border-top:1px dashed var(--control-border);border-bottom:1px dashed var(--control-border)}
.map-room{display:flex;flex-direction:column;align-items:center;justify-content:center;gap:.2rem;min-height:44px;font:inherit;font-weight:600;cursor:pointer}
.map-room .caption{font-weight:400;color:var(--text-2)}
.map-room.is-found{border:2px solid var(--primary);animation:map-pulse 1.2s ease-in-out 3}
@keyframes map-pulse{50%{box-shadow:0 0 0 6px var(--primary-soft-bg)}}
@media(prefers-reduced-motion:reduce){.map-room.is-found{animation:none}}
.map-search-results{margin-top:.5rem;flex-wrap:wrap}
#room-search-form .row input{flex:1;min-width:0}
@media(min-width:1200px){.week-board .lesson{grid-row:var(--event-row)/span var(--event-span,3)}.week-board .day-group.is-today .schedule-list{border:0;border-radius:0}.week-changes{max-width:600px}}
.lesson-subject a{color:inherit;text-decoration:none}
.lesson-subject a:hover{text-decoration:underline;text-underline-offset:4px}
.next-schedule{display:inline-flex;align-items:center;gap:.35rem;min-height:44px;margin-top:.5rem;font-size:.8rem;font-weight:550;color:var(--link);text-decoration:none}
.next-schedule .icon{width:16px;height:16px}
.next-schedule:hover{text-decoration:underline;text-underline-offset:4px}
.grade-tags{display:flex;flex-wrap:wrap;align-items:center;gap:.4rem .75rem}
.discipline-row .small.red{margin-top:.4rem}
.discipline-row .row strong{white-space:nowrap}
input[type=range]{min-height:44px!important;padding:0!important;border:0!important;background:none!important;accent-color:var(--primary);cursor:pointer}
.field label output{font-weight:600;font-variant-numeric:tabular-nums}
[data-admission-plan] .field label{display:block;margin-bottom:.25rem}
.notice.danger .button,.retake-block .button{margin-top:.75rem}
.retake-block p{margin-top:.25rem}
#session .actions{margin-top:1rem}
.ranking-row:focus{outline:2px solid var(--primary);outline-offset:2px}
#rating-root .enh-toolbar{align-items:flex-end}
#rating-root .enh-toolbar>.caption{max-width:320px}
.notice.danger>p+.button{display:inline-flex}
#assignment+section,.content-narrow>.notice.section+section{margin-top:2rem}
.notification .button.small,.news-item .button.small{min-height:44px;padding:.5rem .875rem;margin-top:.5rem;font-size:.875rem}
.notification h2 .red{color:var(--danger-text)}
#notification-count{margin:0 0 .25rem}
.news-item:not(.is-open) .news-body{display:-webkit-box;-webkit-line-clamp:3;-webkit-box-orient:vertical;overflow:hidden}
.news-item.is-open .news-body{color:var(--text);font-size:.9375rem;line-height:1.65}
.news-item .news-full{margin-top:.75rem}
.news-item .news-full .actions{margin-top:.5rem}
.news-item .news-full .actions>.button{display:inline-flex;width:auto;flex:0 0 auto}
.profile-head{margin-bottom:.5rem;max-width:640px}
.profile-head .library-row{max-width:600px}
.ics-card h3{font-size:1.125rem}
.ics-link-row{display:flex;gap:.5rem;align-items:stretch}
.ics-link-row input{flex:1;font-size:.875rem!important}
.ics-link-row .button{flex:none}
.ics-steps{padding-top:1rem;border-top:1px solid var(--border)}
.ics-steps h4{font-size:.9375rem;margin-bottom:.5rem}
.ics-card .actions .button{flex:1 1 auto}
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
