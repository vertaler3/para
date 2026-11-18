/* «Пара» · каркас фронтенда (Э7 FE): vanilla JS, без библиотек и внешних CDN.
   Основа — design-drafts/assets/app.js (журнал html-дорожки 2026-11-07),
   мок-данные и локальная демонстрационная логика черновиков вырезаны:
   остались сеть, офлайн-баннер, бейдж колокольчика, демо-доступ и форма
   входа (экран views/login.csp). Контракт ошибок API — 02 §5.1
   {error:{code,message}}; офлайн-поведение — 03 §5.4; состояния входа —
   03 §5.1/§5.3. Данные только с сервера; секретов здесь нет (AGENTS §5.2). */
(() => {
  'use strict';

  const $ = (sel) => document.querySelector(sel);

  /* ---------- словарь интерфейса 5.18 (шаг 3) ------------------------- */
  /* Тот же серверный словарь (models/i18n.h), что рендерит CSP: контроллер
     кладёт его JSON-бандлом в view-данные ДО рендера (Р-16), footer.csp
     отдаёт <script type="application/json" id="para-i18n"> (значения —
     серверные константы, собирает и JSON-экранирует сервер — utils/
     I18nView.cpp). t(key, fallback): ключ есть — перевод, нет — русская
     строка-фолбэк на месте вызова (бандл отсутствует/повреждён — экран
     выглядит как до 5.18). LANG — код из <html lang> (head.csp, тот же
     резолв: cookie para_lang → Accept-Language → ru); LOCALE — для
     toLocaleDateString дат виджетов. */
  const I18N = (() => {
    try {
      const node = document.getElementById('para-i18n');
      return node ? (JSON.parse(node.textContent) || {}) : {};
    } catch (e) { return {}; }
  })();
  const t = (key, fallback) => {
    const v = I18N[key];
    return v || fallback || key;
  };
  const LANG = document.documentElement.lang || 'ru';
  const LOCALE = { ru: 'ru-RU', en: 'en-GB', zh: 'zh-CN' }[LANG] || 'ru-RU';

  /* ---------- тост (app.css .toast; позиция — над таб-баром, 03 §2.8) ---------- */
  /* Второй аргумент — действие справа (03 §2.8: «действие справа»):
     {label, href}. Цвет действия — --bg на инверсном тосте (пара
     ('bg','text') из baseline tools/contrast_check.py, 10.6:1/15.4:1 —
     PASS обеих тем); «цветом --primary» из 03 §2.8 на инверсном тосте НЕ
     проходит WCAG (замер: 2.0:1 светлая / 1.62:1 тёмная) — отклонение
     задокументировано в app.css и журнале экрана (правки по ревью). */
  const toastEl = $('#toast');
  let toastTimer = 0;
  const toast = (text, action) => {
    if (!toastEl) return;
    toastEl.replaceChildren();
    toastEl.append(document.createTextNode(text));
    if (action && action.label) {
      const a = document.createElement('a');
      a.className = 'toast-action';
      a.href = action.href || '#';
      a.textContent = action.label;
      toastEl.append(a);
    }
    toastEl.hidden = false;
    clearTimeout(toastTimer);
    toastTimer = setTimeout(() => { toastEl.hidden = true; }, 4000);
  };

  /* ---------- офлайн-баннер (03 §5.4: строка под шапкой, не модально) ---------- */
  /* Тап по баннеру / кнопка «Повторить» — проверка сети (03 §5.4): проба
     /healthz; сеть вернулась — баннер исчезает, данные обновляются. */
  const banner = $('#offline-banner');
  const bannerText = banner ? banner.querySelector('[data-offline-text]') : null;
  let lastSyncAt = null; // время последней успешной загрузки данных — метка кэша
  let probing = false;   // повторная проба, пока предыдущая не завершилась
  const applyOffline = (offline) => {
    if (banner) {
      banner.hidden = !offline;
      if (offline && bannerText) {
        bannerText.textContent = lastSyncAt
          ? t('offline.bannerStale', 'Нет сети. Показываем данные на момент').trimEnd() + ' ' + lastSyncAt
          : t('offline.banner', 'Нет сети. Проверьте подключение — и попробуйте ещё раз.');
      }
    }
    /* действия, требующие сети, блокируются disabled-primary с helper-
       текстом на самой кнопке (03 §5.4): у элемента с атрибутом
       data-offline-label на время офлайна подпись заменяется на его
       значение (например «Нужна сеть для входа»), исходная подпись
       запоминается в data-was-label и возвращается после reconnect;
       без атрибута — прежнее поведение (title виден при hover) */
    document.querySelectorAll('[data-network-action]').forEach((el) => {
      if (offline) {
        if (!el.hasAttribute('data-was-disabled')) {
          el.setAttribute('data-was-disabled', el.disabled ? '1' : '0');
        }
        if (el.hasAttribute('data-offline-label')) {
          if (!el.hasAttribute('data-was-label')) {
            el.setAttribute('data-was-label', el.textContent);
          }
          el.textContent = el.getAttribute('data-offline-label');
        }
        el.disabled = true;
        el.title = t('offline.needed', 'Нужна сеть для этого действия');
      } else if (el.hasAttribute('data-was-disabled')) {
        el.disabled = el.getAttribute('data-was-disabled') === '1';
        el.removeAttribute('data-was-disabled');
        el.removeAttribute('title');
        if (el.hasAttribute('data-was-label')) {
          el.textContent = el.getAttribute('data-was-label');
          el.removeAttribute('data-was-label');
        }
      }
    });
  };
  const retryNetwork = async () => {
    if (probing) return;
    probing = true;
    try {
      const resp = await fetch('/healthz', { cache: 'no-store' });
      if (resp.ok) {
        applyOffline(false);
        refreshUnreadBadge();
        toast(t('toast.netBack', 'Сеть вернулась — данные обновлены'));
      }
    } catch (e) { /* сети нет — баннер остаётся, пользователь может повторить */ }
    probing = false;
  };
  if (banner) banner.addEventListener('click', retryNetwork);
  window.addEventListener('offline', () => applyOffline(true));
  window.addEventListener('online', () => {
    applyOffline(false);
    refreshUnreadBadge();
    toast(t('toast.netBack', 'Сеть вернулась — данные обновлены'));
  });
  applyOffline(!navigator.onLine);

  /* ---------- CSRF double-submit (02 §6.4, Э8 шаг 2) ---------- */
  /* Токен живёт в cookie csrf (сервер выдаёт его и <meta name="csrf-token">
     в <head> каждой страницы — канон htmx). Перехватчик fetch добавляет
     заголовок X-CSRF-Token ко ВСЕМ не-GET запросам приложения
     (относительные URL того же origin); htmx-запросам — слушатель
     htmx:configRequest. GET-чтение токен не носит (02 §6.4: state-changing
     — только POST/PUT/DELETE). Расхождение/отсутствие сервер отвечает 403
     {error:{code:"csrf"}} — «обновите страницу» (тогда advice выдаст
     свежий токен). */
  const csrfToken = () => {
    const match = document.cookie.match(/(?:^|;\s*)csrf=([A-Fa-f0-9]+)/);
    if (match) return match[1];
    const meta = document.querySelector('meta[name="csrf-token"]');
    return meta ? meta.getAttribute('content') : '';
  };
  const nativeFetch = window.fetch.bind(window);
  window.fetch = (input, init) => {
    const url = typeof input === 'string' ? input : (input && input.url) || '';
    const method = String((init && init.method) || (input && input.method) || 'GET').toUpperCase();
    if (method !== 'GET' && method !== 'HEAD' && url.charAt(0) === '/') {
      const token = csrfToken();
      if (token) {
        const opts = init ? Object.assign({}, init) : {};
        opts.headers = Object.assign({}, opts.headers, { 'X-CSRF-Token': token });
        return nativeFetch(input, opts);
      }
    }
    return nativeFetch(input, init);
  };
  if (window.htmx) {
    /* htmx идёт поверх XHR (перехватчик fetch его не видит) — свой канал.
       htmx-запросы проекта сегодня только GET (фрагменты), но защита
       ставится на все будущие не-GET сразу. */
    document.body.addEventListener('htmx:configRequest', (event) => {
      const verb = String(event.detail.verb || 'get').toLowerCase();
      if (verb !== 'get') {
        const token = csrfToken();
        if (token) event.detail.headers['X-CSRF-Token'] = token;
      }
    });
  }

  /* ---------- единый доступ к /v1 API ---------- */
  /* Возвращает разобранный JSON или null. Не ходит в сеть офлайн; ошибки
     читает из тела {error:{code,message}} (02 §5.1); 5xx — тост, ошибки
     формы/данных оставляем вызывающему коду экрана. Сетевой сброс —
     переводит каркас в офлайн-режим. */
  const fetchJSON = async (url, options = {}) => {
    if (!navigator.onLine) {
      applyOffline(true);
      return null;
    }
    try {
      const resp = await fetch(url, Object.assign({ headers: { Accept: 'application/json' } }, options));
      if (!resp.ok) {
        let message = 'Ошибка ' + resp.status;
        try {
          const body = await resp.json();
          if (body && body.error && body.error.message) message = body.error.message;
        } catch (e) { /* тело не JSON — остаётся текст с кодом статуса */ }
        if (resp.status >= 500) toast(message);
        return null;
      }
      lastSyncAt = new Date().toLocaleTimeString('ru-RU', { hour: '2-digit', minute: '2-digit' });
      return await resp.json();
    } catch (e) {
      applyOffline(true);
      return null;
    }
  };

  /* ---------- бейдж колокольчика (шапка, topbar-частичный шаблон) ---------- */
  /* GET /v1/notifications?unread=1: счётчик — длина первой страницы
     (сервер отдаёт по 20 записей, AGENTS Э6), максимум «20+». Бейдж скрыт
     по умолчанию в разметке; 401 (гость/сессия истекла) и другие ошибки
     бейдж не показывают. */
  const badge = document.querySelector('[data-unread-badge]');
  const bellLink = badge ? badge.closest('a') : null;
  const renderBadge = (count) => {
    if (!badge) return;
    if (count > 0) {
      const label = count >= 20 ? '20+' : String(count);
      badge.textContent = label;
      badge.hidden = false;
      if (bellLink) bellLink.setAttribute('aria-label', t('a11y.notifications', 'Уведомления') + ': ' + label);
    } else {
      badge.hidden = true;
      if (bellLink) bellLink.setAttribute('aria-label', t('a11y.notifications', 'Уведомления'));
    }
  };
  const refreshUnreadBadge = async () => {
    if (!badge) return;
    const data = await fetchJSON('/v1/notifications?unread=1');
    if (data && Array.isArray(data.notifications)) renderBadge(data.notifications.length);
  };
  refreshUnreadBadge();

  /* ---------- тема и настройки (settings.theme — серверная настройка) ---------- */
  /* Тема применяется на КАЖДОМ экране: applyTheme ставит/снимает
     data-theme на <html> — так её читают токены (static/css/tokens.css:
     :root[data-theme=dark] и @media prefers-color-scheme с :root:not(
     [data-theme=light])); «system» — атрибут снимается (схема по системе).
     Локального хранилища нет — данные только с сервера: на старте каждой
     страницы настройки читаются из GET /v1/notifications/settings
     (401 у гостя — fetchJSON вернёт null, остаётся системная схема).
     Экран «Профиль» переиспользует applyTheme/putSettings для сегмента
     «Оформление» (смена применяется сразу, сохранение — PUT, 02 §5.2). */
  const applyTheme = (theme) => {
    if (theme === 'light' || theme === 'dark') {
      document.documentElement.setAttribute('data-theme', theme);
    } else {
      document.documentElement.removeAttribute('data-theme');
    }
  };
  const loadSettings = () => fetchJSON('/v1/notifications/settings');
  const putSettings = (patch) => fetchJSON('/v1/notifications/settings', {
    method: 'PUT',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(patch)
  });
  /* authedSettings: GET настроек удался → пользователь в сессии (гость
     получает 401 → null); язык гостя живёт только в cookie para_lang. */
  let authedSettings = false;
  loadSettings().then((s) => {
    if (!s) return;
    authedSettings = true;
    if (s.theme) applyTheme(s.theme);
    markFooterTheme(s.theme || 'system');
    markLang(s.lang);
  });

  /* ---------- язык интерфейса (5.18): сегмент в профиле + компактный
     переключатель в footer ---------- */
  /* Переводы рендерит СЕРВЕР (словарь i18n) — смена языка это ПЕРЕХОД,
     а не локальная правка DOM: клик → (авторизован: PUT {lang} — тот же
     путь, что тема; гость пропускает) → мини-роут /lang ставит cookie
     para_lang (app.js cookie НЕ пишет — конвенция каркаса: все cookie
     выдаёт сервер) и отвечает 302 назад — страница перезагружается на
     выбранном языке. Активная кнопка — aria-pressed: до ответа сервера
     её знает cookie para_lang (НЕ HttpOnly — читаем здесь), после входа
     уточняется GET-ом настроек. */
  const paraLang = () => {
    const m = document.cookie.match(/(?:^|;\s*)para_lang=(ru|en|zh)/);
    return m ? m[1] : '';
  };
  const markLang = (lang) => {
    if (!lang) return;
    document.querySelectorAll('[data-lang-choice]').forEach((b) => {
      b.setAttribute('aria-pressed', String(b.dataset.langChoice === lang));
    });
  };
  document.querySelectorAll('[data-lang-choice]').forEach((btn) => {
    btn.addEventListener('click', async () => {
      const lang = btn.dataset.langChoice;
      if (!/^(ru|en|zh)$/.test(lang)) return;
      if (!navigator.onLine) { applyOffline(true); return; }
      if (authedSettings) {
        const saved = await putSettings({ lang });
        if (!saved) { toast(t('toast.failLang', 'Не получилось сохранить язык')); return; }
      }
      const next = location.pathname + location.search;
      location.href = '/lang?lang=' + lang + '&next=' + encodeURIComponent(next);
    });
  });
  markLang(paraLang());

  /* ---------- каркас v3 (fe_rebuild шаг 4, Р-31): тема в footer, дата недели,
     аватар, активный пункт «В кампусе» ---------- */
  /* Тема из footer: та же механика, что сегмент профиля — applyTheme сразу,
     авторизованному PUT {theme}; гостю — только на текущей странице
     (локального хранилища нет — конвенция каркаса). */
  function markFooterTheme(theme) {
    document.querySelectorAll('[data-footer-theme]').forEach((b) => {
      b.setAttribute('aria-pressed', String(b.dataset.footerTheme === theme));
    });
  }
  markFooterTheme(document.documentElement.getAttribute('data-theme') || 'system');
  document.addEventListener('click', async (e) => {
    const btn = e.target.closest && e.target.closest('[data-footer-theme]');
    if (!btn) return;
    const theme = btn.dataset.footerTheme;
    if (!/^(light|dark|system)$/.test(theme)) return;
    applyTheme(theme);
    markFooterTheme(theme);
    document.querySelectorAll('[data-theme-choice]').forEach((b) => {
      b.setAttribute('aria-pressed', String(b.dataset.themeChoice === theme));
    });
    if (authedSettings) {
      const saved = await putSettings({ theme });
      toast(saved ? t('toast.themeSaved', 'Оформление сохранено')
                  : t('toast.failTheme', 'Не получилось сохранить тему'));
    }
  });
  /* Дата в шапке — текущая учебная неделя «пн–вс» в формате языка страницы
     (<html lang>, Intl; без строк словаря — месяцы локализует браузер). */
  (() => {
    const el = document.querySelector('[data-header-date]');
    if (!el || typeof Intl === 'undefined') return;
    const now = new Date();
    const mon = new Date(now); mon.setDate(now.getDate() - ((now.getDay() + 6) % 7));
    const sun = new Date(mon); sun.setDate(mon.getDate() + 6);
    const lang = document.documentElement.lang || 'ru';
    try {
      const f = new Intl.DateTimeFormat(lang, { day: 'numeric', month: 'long' });
      el.textContent = f.formatRange ? f.formatRange(mon, sun) : f.format(mon) + ' – ' + f.format(sun);
    } catch (_) { /* старый браузер — дата просто не показывается */ }
  })();
  /* Аватар — инициалы ФИО из GET /v1/me (гость/ошибка — кружок пустой,
     ссылка на профиль работает). */
  (() => {
    const av = document.querySelector('[data-avatar]');
    if (!av) return;
    fetchJSON('/v1/me').then((me) => {
      const fio = me && me.profile && me.profile.fio ? String(me.profile.fio) : '';
      const parts = fio.trim().split(/\s+/).filter(Boolean);
      const ini = parts.length > 1 ? parts[1][0] + parts[0][0] : (parts[0] || '').slice(0, 2);
      av.textContent = ini.toUpperCase();
    });
  })();
  /* Быстрый поиск «Куда перейти?» (эталон v3): фильтр серверного списка разделов по
     словам подписи/описания (ё→е, грубая основа слова — как в эталоне), Ctrl/⌘+K,
     ↓ — к первой ссылке, Enter — открыть первую. */
  (() => {
    const dlg = document.querySelector('[data-search-dialog]');
    if (!dlg || typeof dlg.showModal !== 'function') return;
    const input = dlg.querySelector('[data-search-input]');
    const items = Array.from(dlg.querySelectorAll('[data-search-item]'));
    const empty = dlg.querySelector('[data-search-empty]');
    const count = dlg.querySelector('[data-search-count]');
    const stem = (w) => w.toLowerCase().replace(/ё/g, 'е').replace(/(ами|ями|ого|ему|ов|ев|ия|ии|ую|а|ы|и|у|е)$/u, '');
    const plural = (n) => {
      const m10 = n % 10, m100 = n % 100;
      if (m10 === 1 && m100 !== 11) return count.dataset.one;
      if (m10 >= 2 && m10 <= 4 && (m100 < 12 || m100 > 14)) return count.dataset.few;
      return count.dataset.many;
    };
    const render = () => {
      const terms = input.value.trim().split(/\s+/).filter(Boolean).map(stem);
      let n = 0;
      items.forEach((li) => {
        const words = li.textContent.toLowerCase().split(/[\s,·]+/).map(stem);
        const ok = terms.every((t) => words.some((w) => w.indexOf(t) !== -1));
        li.hidden = !ok; if (ok) n += 1;
      });
      if (empty) empty.hidden = n > 0;
      if (count) count.textContent = n + ' ' + plural(n);
    };
    let prevFocus = null;
    const open = () => { prevFocus = document.activeElement; input.value = ''; render(); dlg.showModal(); input.focus(); };
    document.addEventListener('click', (e) => {
      if (e.target.closest && e.target.closest('[data-search-open]')) open();
      else if (e.target.closest && e.target.closest('[data-search-close]')) dlg.close();
      else if (e.target === dlg) dlg.close();
    });
    input.addEventListener('input', render);
    input.addEventListener('keydown', (e) => {
      const first = items.find((li) => !li.hidden);
      const link = first && first.querySelector('a');
      if (e.key === 'ArrowDown' && link) { e.preventDefault(); link.focus(); }
      if (e.key === 'Enter' && link) { e.preventDefault(); link.click(); }
    });
    dlg.addEventListener('close', () => { if (prevFocus && prevFocus.isConnected) prevFocus.focus(); });
    document.addEventListener('keydown', (e) => {
      if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'k') { e.preventDefault(); if (dlg.open) dlg.close(); else open(); }
    });
  })();
  document.querySelectorAll('[data-campus-link]').forEach((a) => {
    const href = a.getAttribute('href');
    if (href && (location.pathname === href || location.pathname.indexOf(href + '/') === 0)) {
      a.setAttribute('aria-current', 'page');
    }
  });

  /* ---------- онбординг С5 (03 §4.7/§4.9.4) ---------- */
  /* Состояние чеклиста — onboarding_json настроек: массив КЛЮЧЕЙ закрытых
     шагов (дефолт DDL «[]» — SettingsService.h/0004). Ключи и подписи —
     FE (постановка экрана «Профиль»): PUT onboarding_json — строка с JSON
     (SettingsService::applyPatch). «Вошёл» отмечается сам при первом
     показе чеклиста (пользователь уже в сессии); все 4 — чеклист исчезает
     навсегда (оверлей дашборда не показывается). */
  const ONBOARD_STEPS = [
    { key: 'login', label: () => t('onboarding.stepLogin', 'Вошёл') },
    { key: 'schedule', label: () => t('onboarding.stepSchedule', 'Посмотрел расписание') },
    { key: 'application', label: () => t('onboarding.stepApplication', 'Подал заявку') },
    { key: 'notifications', label: () => t('onboarding.stepNotifications', 'Включил уведомления') }
  ];
  const ONBOARD_KEYS = ONBOARD_STEPS.map((s) => s.key);
  /* только известные ключи, в каноничном порядке (тело PUT и рендер) */
  const obNormalize = (arr) =>
    ONBOARD_KEYS.filter((k) => Array.isArray(arr) && arr.indexOf(k) !== -1);
  const saveOnboarding = (steps) =>
    putSettings({ onboarding_json: JSON.stringify(steps) });
  const obEl = (tag, className, text) => {
    const node = document.createElement(tag);
    if (className) node.className = className;
    if (text != null) node.textContent = text;
    return node;
  };
  /* Рендер чеклиста в контейнер ([data-ob-steps]/[data-ob-count]/
     [data-ob-bar]): строки-кнопки ≥44px (тап — отметка → PUT settings;
     03 §6.2 «тап по всей строке»), счётчик N/4 и полоса прогресса
     (--value — ширина :before каркасного .progress, app.css). Возвращает
     актуальный массив шагов; onChange(steps) — после каждого сохранения. */
  const renderOnboarding = (root, steps, onChange) => {
    const stepsBox = root.querySelector('[data-ob-steps]');
    if (!stepsBox) return steps;
    stepsBox.replaceChildren();
    ONBOARD_STEPS.forEach((s) => {
      const done = steps.indexOf(s.key) !== -1;
      const row = obEl('button', 'ob-step');
      row.type = 'button';
      row.setAttribute('aria-pressed', String(done));
      row.append(obEl('span', 'ob-mark', done ? '✓' : ''), obEl('span', 'ob-label', s.label()));
      row.disabled = done; /* выполненный шаг — отметка уже сохранена */
      row.addEventListener('click', async () => {
        if (row.disabled || steps.indexOf(s.key) !== -1) return;
        if (!navigator.onLine) { applyOffline(true); return; }
        row.disabled = true;
        const saved = await saveOnboarding(obNormalize(steps.concat(s.key)));
        row.disabled = false;
        if (!saved) { toast(t('toast.failStep', 'Не получилось отметить шаг — проверьте интернет')); return; }
        steps = obNormalize(saved.onboarding);
        renderOnboarding(root, steps, onChange);
        if (onChange) onChange(steps);
      });
      stepsBox.append(row);
    });
    const countEl = root.querySelector('[data-ob-count]');
    if (countEl) countEl.textContent = steps.length + '/4';
    const bar = root.querySelector('[data-ob-bar]');
    if (bar) {
      bar.setAttribute('aria-valuenow', String(steps.length));
      bar.style.setProperty('--value', (steps.length * 25) + '%');
    }
    return steps;
  };

  /* ---------- демо-доступы на входе (03 §4.1) ---------- */
  /* Тап по строке демо-доступа ([data-demo-login="anya.demo"]) подставляет
     логин в поле #login и переводит фокус в #password. Само поле пароля
     не заполняется: единый демо-пароль публикуется в комментарии к работе,
     в разметку и JS его значение не подставляется (AGENTS §5.2). */
  document.querySelectorAll('[data-demo-login]').forEach((button) => {
    button.addEventListener('click', () => {
      const login = $('#login');
      if (!login) return;
      login.value = button.getAttribute('data-demo-login');
      const password = $('#password');
      if (password) password.focus();
      document.querySelectorAll('[data-demo-login]').forEach((x) => {
        x.setAttribute('aria-pressed', String(x === button));
      });
    });
  });

  /* ---------- show/hide пароля (03 §4.1: «Пароль — show/hide по тапу») ---------- */
  const togglePassword = $('#toggle-password');
  if (togglePassword) {
    togglePassword.addEventListener('click', () => {
      const password = $('#password');
      if (!password) return;
      const show = password.type === 'password';
      password.type = show ? 'text' : 'password';
      togglePassword.setAttribute('aria-pressed', String(show));
      togglePassword.setAttribute('aria-label', show ? 'Скрыть пароль' : 'Показать пароль');
    });
  }

  /* ---------- форма входа (views/login.csp; 03 §4.1, §5.1/§5.3) ---------- */
  /* POST /v1/auth/login JSON {login,password} (02 §5.2, Content-Type:
     application/json). Успех 200 — редирект на «Сегодня» (/). Ошибка —
     сообщение у формы из {error:{code,message}} (02 §5.1):
     invalid_credentials — нейтральный текст без деталей, что именно
     неверно (фокус в пароль, поля не очищаются — 03 §5.3);
     too_many_attempts — блокировка входа (03 §4.1/§5.3): паузу ведёт
     сервер (LoginLimiter 15→30→60… с), клиент читает Retry-After и
     показывает отсчёт на disabled-кнопке «Подождите N с» до её
     разблокировки; прочие — message из ответа. Пока идёт запрос —
     кнопка disabled «Входим…» (03 §5.1: кнопка с async-действием в
     состоянии loading). Офлайн-кнопка — helper «Нужна сеть для входа»
     (03 §5.4, через applyOffline + data-offline-label). Сетевой сброс —
     переход каркаса в офлайн-режим (03 §5.4). Демо-пароль в JS не
     подставляется (AGENTS §5.2). */
  const loginForm = $('#login-form');
  if (loginForm) {
    const loginInput = $('#login');
    const passwordInput = $('#password');
    const formError = $('#login-error');
    const submitButton = loginForm.querySelector('button[type="submit"]');
    const showFormError = (text) => {
      if (!formError) return;
      formError.textContent = text;
      formError.hidden = false;
    };
    /* отсчёт блокировки входа (03 §4.1: «Подождите N с» на кнопке) */
    let countdownTimer = 0;
    const stopCountdown = () => {
      if (countdownTimer) {
        clearInterval(countdownTimer);
        countdownTimer = 0;
      }
    };
    const startCountdown = (seconds) => {
      stopCountdown();
      if (!submitButton) return;
      let remain = seconds;
      submitButton.disabled = true;
      /* если офлайн-режим уже владеет подписью (data-was-label стоит,
         helper-текст на кнопке) — тики и старт отсчёта её не перебивают */
      if (!submitButton.hasAttribute('data-was-label')) {
        submitButton.textContent = 'Подождите ' + remain + ' с';
      }
      countdownTimer = setInterval(() => {
        remain -= 1;
        if (remain > 0) {
          if (!submitButton.hasAttribute('data-was-label')) {
            submitButton.textContent = 'Подождите ' + remain + ' с';
          }
          return;
        }
        stopCountdown();
        const offlineOwnsLabel = submitButton.hasAttribute('data-was-label');
        if (offlineOwnsLabel || !navigator.onLine) {
          /* отсчёт истёк в офлайне: подписью управляет офлайн-режим
             (helper-текст, после reconnect вернётся из data-was-label).
             Record ОБНОВЛЯЕТСЯ на «Войти» — иначе восстановится
             устаревший текст отсчёта, захваченный applyOffline в момент
             offline-события (гонка раунда 2 ревью, 03 §5.4) */
          submitButton.setAttribute('data-was-label', 'Войти');
          submitButton.setAttribute('data-was-disabled', '0');
          if (!offlineOwnsLabel) {
            submitButton.textContent = 'Войти';
          }
          return;
        }
        submitButton.textContent = 'Войти';
        submitButton.disabled = false;
      }, 1000);
    };
    loginForm.addEventListener('submit', async (event) => {
      event.preventDefault();
      if (countdownTimer) return; // блокировка входа ещё идёт (03 §4.1)
      if (submitButton && submitButton.disabled) return; // запрос уже идёт
      if (!navigator.onLine) {
        applyOffline(true);
        return;
      }
      if (formError) formError.hidden = true;
      const previousLabel = submitButton ? submitButton.textContent : '';
      let wentOffline = false; // сетевой сброс во время запроса (03 §5.4)
      let blockedFor = 0;      // Retry-After из 429 (03 §4.1/§5.3)
      if (submitButton) {
        submitButton.disabled = true;
        submitButton.textContent = t('login.busy', 'Входим…');
      }
      try {
        const resp = await fetch('/v1/auth/login', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({
            login: loginInput ? loginInput.value.trim() : '',
            password: passwordInput ? passwordInput.value : ''
          })
        });
        if (resp.ok) {
          window.location.href = '/';
          return;
        }
        let code = '';
        let message = 'Ошибка ' + resp.status;
        try {
          const body = await resp.json();
          if (body && body.error) {
            code = body.error.code || '';
            if (body.error.message) message = body.error.message;
          }
        } catch (e) { /* тело не JSON — остаётся текст со статусом */ }
        if (code === 'invalid_credentials') {
          message = 'Неверный логин или пароль';
          if (passwordInput) passwordInput.focus();
        } else if (code === 'too_many_attempts') {
          const raw = parseInt(resp.headers.get('Retry-After'), 10);
          blockedFor = Number.isFinite(raw) && raw > 0 ? raw : 15; // стартовая пауза 03 §4.1
          message = 'Слишком много попыток входа. Подождите ' + blockedFor + ' секунд и попробуйте снова.';
        }
        showFormError(message);
      } catch (e) {
        wentOffline = true;
        if (submitButton) {
          /* подпись вернуть ДО перехода в офлайн-режим: applyOffline
             запомнит «Войти» (а не busy-подпись «Входим…») в
             data-was-label и поставит helper-текст (03 §5.4) */
          submitButton.textContent = previousLabel;
          submitButton.setAttribute('data-was-label', previousLabel);
        }
        applyOffline(true);
      } finally {
        if (submitButton) {
          if (blockedFor > 0) {
            startCountdown(blockedFor);
          } else if (wentOffline || !navigator.onLine) {
            /* офлайн-режим управляет кнопкой (helper-текст уже на ней):
               остаётся disabled, после возврата сети каркас восстановит
               её из data-was-disabled/data-was-label (03 §5.4); '0' —
               кнопка была активной до запроса */
            submitButton.setAttribute('data-was-disabled', '0');
            submitButton.setAttribute('data-was-label', previousLabel);
          } else {
            submitButton.disabled = false;
            submitButton.textContent = previousLabel;
          }
        }
      }
    });
  }

  /* ---------- экраны «Успеваемость» (views/grades.csp, views/
     grades-discipline.csp; 03 §4.4/§4.9.3), «Зачётная книжка»
     (views/gradebook.csp) и «Рейтинг» (views/rating.csp); 03 §4.9.6 --- */
  /* Контент — SSR (тела GradesService/GradebookService/RatingService
     рендерит сервер); догрузки на клиенте нет — скелетон не нужен
     (03 §5.1), ошибку данных показывает сам шаблон (failed=1, состояние
     03 §5.3). Здесь только «Повторить»: реально перезапрашивает —
     перезагрузка страницы; офлайн обслуживает баннер каркаса (topbar +
     applyOffline, 03 §5.4). */
  if (document.body.dataset.page === 'grades' || document.body.dataset.page === 'discipline' || document.body.dataset.page === 'gradebook' || document.body.dataset.page === 'rating') {
    const gradesRetry = document.querySelector('[data-grades-retry], [data-gradebook-retry], [data-rating-retry]');
    if (gradesRetry) gradesRetry.addEventListener('click', () => window.location.reload());
  }

  /* ---------- экран «Сегодня» (views/dashboard.csp; 03 §4.2, §5) ---------- */
  /* Данные — один fetch GET /v1/dashboard (контракт DashboardService.h):
     виджеты «Следующая пара»/«Сегодня» (ScheduleService), «Дедлайны»
     (session_plan — GradesService::exams), «Заявки» (ApplicationsService,
     активные статусы), «Что нового» + unread_count (NotificationsFeedService);
     degraded — ключи источников, упавших с ошибкой БД: сбой виджета
     отличим от «нет данных» и показывается компактной строкой с
     «Повторить» (03 §4.2 «свой компактный ErrorState», §5.3).
     Каждый тап ведёт к объекту (03 §4.2): пара → /schedule (перенос/
     отмена → /schedule?date=<день> — канон deep_link фикстур Э6; отдельная
     карточка изменения §4.9.1 — экран Э7, который ещё не собран),
     дедлайн → /grades, заявка → /applications/{id}, новость → /notifications.
     Поведение 03 §4.2: порядок виджетов по времени суток (06–12 базовый —
     «Следующая пара» первой; 12–18 — остаток «Сегодня» вверх; 18–23 —
     сверху «Что сдать завтра»: пары из публичного GET /v1/schedule/week
     + дедлайны агрегата с датой завтра; 23–06 — «За ночь: N событий»
     вместо «Что нового», N — unread_count агрегата — накопившиеся
     непрочитанные); минутный отсчёт «Следующей пары»; счётчики «· N»
     в заголовках «Сегодня»/«Что нового». Даты виджетов — UTC проекта
     (события фикстур в «…Z»), часы режимов — локальные (как приветствие).
     Состояния (03 §5): скелетон — начальная разметка до ответа; пусто —
     тексты 03 §5.2 в разметке; ошибка — компактная в виджете / экранная
     с «Повторить» (повторный fetch); офлайн — баннер каркаса (fetchJSON
     сам переводит в офлайн-режим и возвращает null).
     Вставка — только createElement/textContent: данные с сервера никогда
     не попадают в innerHTML (02 §6.4). */
  if (document.body.dataset.page === 'dashboard') {
    const $$ = (sel) => document.querySelector(sel);
    const show = (sel, on) => { const n = $$(sel); if (n) n.hidden = !on; };

    /* приветствие — по локальному времени (03 §4.2); дата — локальная */
    const hour = new Date().getHours();
    const greetingEl = $$('[data-greeting]');
    if (greetingEl) {
      greetingEl.textContent = hour < 12 ? t('dash.greetingMorning', 'Доброе утро')
        : hour < 18 ? t('dash.greetingAfternoon', 'Добрый день')
        : t('dash.greetingEvening', 'Добрый вечер');
    }
    const dateEl = $$('[data-today-date]');
    if (dateEl) {
      dateEl.textContent = new Date().toLocaleDateString(LOCALE, { weekday: 'long', day: 'numeric', month: 'long' });
    }

    /* словари-константы словарей CHECK DDL (0002/0003/0004) — метки FE */
    /* метки словарей CHECK DDL — из словаря 5.18 (t: бандл сервера);
       фолбэк — прежние русские строки */
    const LESSON_KIND = {
      lecture: t('lesson.lecture', 'Лекция'),
      seminar: t('lesson.seminar', 'Семинар'),
      practice: t('lesson.practice', 'Практика')
    };
    const EXAM_KIND = {
      consultation: t('exam.consultation', 'Консультация'),
      exam: t('exam.exam', 'Экзамен'),
      credit: t('exam.credit', 'Зачёт'),
      retake: t('exam.retake', 'Пересдача')
    };
    const APP_STATUS = {
      draft: t('apps.statusDraft', 'Черновик'),
      submitted: t('apps.statusSubmitted', 'Подана'),
      accepted: t('apps.statusAccepted', 'Принята'),
      in_progress: t('apps.statusInProgress', 'В работе'),
      need_info: t('apps.statusNeedInfo', 'Требуется уточнение'),
      ready: t('apps.statusReady', 'Готова'),
      issued: t('apps.statusIssued', 'Выдана'),
      rejected: t('apps.statusRejected', 'Отклонена'),
      cancelled: t('apps.statusCancelled', 'Отменена')
    };
    const NEWS_TYPE = {
      schedule: t('notif.typeSchedule', 'Расписание'),
      deadline: t('notif.typeDeadline', 'Дедлайн'),
      application: t('notif.typeApplication', 'Заявка'),
      announcement: t('notif.typeAnnouncement', 'Объявление')
    };
    /* категории новостей 5.17 — словарь CHECK 0007, метки те же, что на
       сегменте «Новости» экрана уведомлений */
    const NEWS_CATEGORY = {
      official: t('news.categoryOfficial', 'Официальная'),
      conference: t('news.categoryConference', 'Конференция'),
      org: t('news.categoryOrg', 'Организация'),
      event: t('news.categoryEvent', 'Событие')
    };

    const el = (tag, className, text) => {
      const node = document.createElement(tag);
      if (className) node.className = className;
      if (text != null) node.textContent = text;
      return node;
    };
    const timeOf = (iso) => (typeof iso === 'string' && iso.length >= 16) ? iso.slice(11, 16) : '';
    const dateOf = (iso) => (typeof iso === 'string' && iso.length >= 10) ? iso.slice(0, 10) : '';
    const minutesUntil = (iso) => Math.max(0, Math.round((new Date(iso).getTime() - Date.now()) / 60000));
    const plural = (n, one, few, many) => {
      const m10 = n % 10, m100 = n % 100;
      if (m10 === 1 && m100 !== 11) return one;
      if (m10 >= 2 && m10 <= 4 && (m100 < 12 || m100 > 14)) return few;
      return many;
    };
    const dayDiff = (iso) => {
      const due = new Date(iso);
      const today = new Date();
      due.setHours(0, 0, 0, 0);
      today.setHours(0, 0, 0, 0);
      return Math.round((due.getTime() - today.getTime()) / 86400000);
    };
    const dayLabel = (iso) => {
      const diff = dayDiff(iso);
      if (diff === 0) return t('dash.todayAdv', 'сегодня');
      if (diff === 1) return t('dash.tomorrowAdv', 'завтра');
      return new Date(iso).toLocaleDateString(LOCALE, { day: 'numeric', month: 'long' });
    };
    const shortStamp = (iso) => {
      if (typeof iso !== 'string' || iso.length < 16) return '';
      const sameDay = dayDiff(iso) === 0;
      return sameDay ? timeOf(iso) : dayLabel(iso) + ', ' + timeOf(iso);
    };
    /* UTC-дата проекта со сдвигом в сутках (события фикстур в «…Z») */
    const utcDateOffset = (days) => new Date(Date.now() + days * 86400000).toISOString().slice(0, 10);

    /* --- состояние последнего ответа агрегата (для вечернего виджета) --- */
    let lastData = null;
    let countdownTimer = 0; // минутный отсчёт «Следующей пары» (03 §4.2)
    const stopCountdown = () => {
      if (countdownTimer) {
        clearInterval(countdownTimer);
        countdownTimer = 0;
      }
    };

    /* --- виджет «Следующая пара» (03 §5.2: после последней пары скрыт);
       отсчёт обновляется раз в минуту, пока пара не началась (03 §4.2) --- */
    const updateNextMeta = (next) => {
      const meta = $$('[data-next-meta]');
      if (!meta) return;
      const started = new Date(next.starts_at).getTime() <= Date.now();
      if (started) {
        meta.textContent = t('dash.inProgressNow', 'идёт сейчас') + ' · ' + t('dash.until', 'до') + ' ' + timeOf(next.ends_at);
        return;
      }
      const mins = minutesUntil(next.starts_at);
      meta.textContent = mins >= 1
        ? t('dash.inMinutes', 'через {n}').replace('{n}', mins + ' ' + plural(mins, t('plural.minuteOne', 'минуту'), t('plural.minuteFew', 'минуты'), t('plural.minuteMany', 'минут')))
        : t('dash.lessMinute', 'меньше минуты');
    };
    const renderNext = (next, failed) => {
      show('[data-next-loading]', false);
      show('[data-next-error]', Boolean(failed));
      stopCountdown();
      const widget = $$('[data-widget="next-class"]');
      const body = $$('[data-next-body]');
      if (!widget || !body) return;
      if (failed) { widget.hidden = false; body.hidden = true; return; }
      if (!next) { widget.hidden = true; return; }
      widget.hidden = false;
      body.hidden = false;
      updateNextMeta(next);
      if (countdownTimer === 0) countdownTimer = setInterval(() => updateNextMeta(next), 60000);
      const kind = $$('[data-next-kind]');
      if (kind) {
        const label = LESSON_KIND[next.kind];
        kind.hidden = !label;
        if (label) kind.textContent = label;
      }
      const time = $$('[data-next-time]');
      if (time) time.textContent = timeOf(next.starts_at) + '–' + timeOf(next.ends_at);
      const discipline = $$('[data-next-discipline]');
      if (discipline) discipline.textContent = next.discipline || '';
      const room = $$('[data-next-room]');
      if (room) room.textContent = next.room || '—';
    };

    /* --- виджет «Сегодня»: все пары дня, прошедшие — приглушённы,
       счётчик «· N пар» в заголовке (03 §4.2) --------------------------- */
    const renderToday = (events, failed) => {
      show('[data-today-loading]', false);
      show('[data-today-error]', Boolean(failed));
      const body = $$('[data-today-body]');
      const title = $$('[data-today-title]');
      if (!body) return;
      const todayLabel = t('nav.today', 'Сегодня');
      if (failed) {
        body.hidden = true;
        if (title) title.textContent = todayLabel;
        return;
      }
      if (title) {
        title.textContent = events.length
          ? todayLabel + ' · ' + events.length + ' ' + plural(events.length, t('plural.classOne', 'пара'), t('plural.classFew', 'пары'), t('plural.classMany', 'пар'))
          : todayLabel;
      }
      if (!events.length) { body.hidden = true; show('[data-today-empty]', true); return; }
      show('[data-today-empty]', false);
      body.hidden = false;
      body.replaceChildren();
      events.forEach((e) => {
        const row = el('a', 'lesson');
        /* тап → объект: обычная пара — /schedule; перенос/отмена — день
           расписания с этим переносом (канон deep_link фикстур Э6;
           карточка изменения §4.9.1 — экран Э7, который ещё не собран) */
        row.href = e.status && e.status !== 'normal' && dateOf(e.starts_at)
          ? '/schedule?date=' + dateOf(e.starts_at)
          : '/schedule';
        if (timeOf(e.ends_at) && new Date(e.ends_at).getTime() < Date.now()) row.classList.add('is-past');
        const time = el('div', 'lesson-time');
        time.append(el('strong', null, timeOf(e.starts_at)));
        if (timeOf(e.ends_at)) time.append(el('span', null, timeOf(e.ends_at)));
        const mid = el('div');
        mid.append(el('div', 'lesson-subject', e.discipline || ''));
        const detail = el('p', 'lesson-detail', LESSON_KIND[e.kind] || '');
        if (e.status === 'moved') detail.append(' ', el('span', 'badge', 'Перенос'));
        if (e.status === 'cancelled') detail.append(' ', el('span', 'badge danger', 'Отменена'));
        mid.append(detail);
        const room = el('div', 'lesson-room');
        if (e.status === 'cancelled') {
          room.append(el('s', null, e.room || '—'));
          room.append(el('small', null, 'отменена'));
        } else if (e.status === 'moved' && e.prev_room) {
          room.append(el('s', null, e.prev_room));
          room.append(document.createTextNode(' → ' + (e.room || '—')));
        } else {
          room.append(el('span', null, e.room || '—'));
        }
        row.append(time, mid, room);
        body.append(row);
      });
    };

    /* --- виджет «Дедлайны»: точка срочности (03 §4.2) ------------------- */
    const renderDeadlines = (items, failed) => {
      show('[data-deadlines-loading]', false);
      show('[data-deadlines-error]', Boolean(failed));
      const body = $$('[data-deadlines-body]');
      if (!body) return;
      if (failed) { body.hidden = true; return; }
      const future = items.filter((d) => dayDiff(d.starts_at) >= 0);
      if (!future.length) { body.hidden = true; show('[data-deadlines-empty]', true); return; }
      show('[data-deadlines-empty]', false);
      body.hidden = false;
      body.replaceChildren();
      future.forEach((d) => {
        const diff = dayDiff(d.starts_at);
        const urgency = diff <= 1 ? 'dot-danger' : diff <= 3 ? 'dot-warn' : 'dot-ok';
        const row = el('li');
        const link = el('a');
        link.href = '/grades';
        link.append(el('span', 'deadline-dot ' + urgency));
        link.append(document.createTextNode((EXAM_KIND[d.kind] || '') + ' · ' + (d.discipline || '')));
        const time = el('p', 'deadline-time calm', dayLabel(d.starts_at) + (timeOf(d.starts_at) ? ' · ' + timeOf(d.starts_at) : ''));
        row.append(link, time);
        body.append(row);
      });
    };

    /* --- виджет «Заявки»: активные → /applications/{id} ----------------- */
    const renderApplications = (items, failed) => {
      show('[data-applications-loading]', false);
      show('[data-applications-error]', Boolean(failed));
      const body = $$('[data-applications-body]');
      if (!body) return;
      if (failed) { body.hidden = true; return; }
      if (!items.length) { body.hidden = true; show('[data-applications-empty]', true); return; }
      show('[data-applications-empty]', false);
      body.hidden = false;
      body.replaceChildren();
      items.forEach((a) => {
        const row = el('div', 'compact-app');
        const link = el('a', null, a.type_title || a.type || t('notif.typeApplication', 'Заявка'));
        link.href = '/applications/' + a.id;
        const status = APP_STATUS[a.status] || a.status || '';
        const stamp = shortStamp(a.updated_at);
        row.append(link, el('p', 'caption', status + (stamp ? ' · ' + stamp : '')));
        body.append(row);
      });
    };

    /* --- виджет «Что нового» → /notifications; счётчик «· N»; ночью
       заголовок — «За ночь: N событий» (03 §4.2). 5.17: к уведомлениям
       агрегата ПОДМЕШИВАЮТСЯ 1–2 свежие новости (freshNews — первые
       строки GET /v1/news, он отсортирован published_at DESC; null —
       fetch не удался, виджет не роняем, остаются уведомления) —
       решение «как читает данные» за клиентским fetch: агрегат
       /v1/dashboard новостями не владеет (DashboardService — уведомления),
       а прецедент подмешивания поверх агрегата — renderTomorrow с его
       GET /v1/schedule/week. Строки-новости ведут на сегмент «Новости»
       (/notifications?segment=news — живая страница), счётчик «· N»
       считает показанные строки. ---------------------------------------- */
    const renderNews = (items, failed, unreadCount, freshNews) => {
      show('[data-news-loading]', false);
      show('[data-news-error]', Boolean(failed));
      const body = $$('[data-news-body]');
      const title = $$('[data-news-title]');
      if (!body) return;
      const mixed = Array.isArray(freshNews) ? freshNews.slice(0, 2) : [];
      const nightMode = (() => { const h = new Date().getHours(); return h >= 23 || h < 6; })();
      if (title) {
        const whatsNew = t('dash.whatsNew', 'Что нового');
        title.textContent = nightMode
          ? t('dash.overnight', 'За ночь') + ': ' + (unreadCount || 0) + ' ' + plural(unreadCount || 0, t('plural.eventOne', 'событие'), t('plural.eventFew', 'события'), t('plural.eventMany', 'событий'))
          : ((items.length + mixed.length) ? whatsNew + ' · ' + (items.length + mixed.length) : whatsNew);
      }
      if (failed) { body.hidden = true; return; }
      if (!items.length && !mixed.length) { body.hidden = true; show('[data-news-empty]', true); return; }
      show('[data-news-empty]', false);
      body.hidden = false;
      body.replaceChildren();
      mixed.forEach((n) => {
        const row = el('div', 'news-row');
        const link = el('a', null, n.title || '');
        link.href = '/notifications?segment=news';
        const caption = el('p', 'caption', (NEWS_CATEGORY[n.category] || t('news.one', 'Новость')) + (n.published_at ? ' · ' + shortStamp(n.published_at) : ''));
        row.append(link, caption);
        body.append(row);
      });
      items.forEach((n) => {
        const row = el('div', 'news-row');
        const link = el('a', null, n.title || '');
        link.href = '/notifications';
        const caption = el('p', 'caption', (NEWS_TYPE[n.type] || '') + (n.created_at ? ' · ' + shortStamp(n.created_at) : ''));
        if (!n.read_at) {
          const dot = el('span', 'unread-mark');
          dot.setAttribute('aria-hidden', 'true');
          caption.append(' ', dot);
        }
        row.append(link, caption);
        body.append(row);
      });
    };

    /* --- вечерний виджет «Что сдать завтра» (03 §4.2, 18:00–23:00):
       пары завтра — публичный GET /v1/schedule/week (offset=0; если завтра
       уже следующая неделя — offset=1), дедлайны завтра — из агрегата
       (session_plan); пусто → виджет скрывается. Даты — UTC проекта. --- */
    const renderTomorrow = async () => {
      const sec = $$('[data-widget="tomorrow"]');
      const body = $$('[data-tomorrow-body]');
      if (!sec || !body) return;
      const tomorrow = utcDateOffset(1);
      const week = await fetchJSON('/v1/schedule/week?offset=0');
      let events = week && Array.isArray(week.events) ? week.events : [];
      if (week && week.to && tomorrow > week.to) {
        const nextWeek = await fetchJSON('/v1/schedule/week?offset=1');
        events = nextWeek && Array.isArray(nextWeek.events) ? nextWeek.events : [];
      }
      const rows = [];
      events.forEach((e) => {
        if (dateOf(e.starts_at) === tomorrow) {
          rows.push({ time: timeOf(e.starts_at), kind: e.kind, title: e.discipline || '', status: e.status, href: '/schedule?date=' + tomorrow });
        }
      });
      ((lastData && Array.isArray(lastData.deadlines)) ? lastData.deadlines : []).forEach((d) => {
        if (dateOf(d.starts_at) === tomorrow) {
          rows.push({ time: timeOf(d.starts_at), kind: d.kind, title: d.discipline || '', status: '', href: '/grades' });
        }
      });
      rows.sort((a, b) => (a.time < b.time ? -1 : a.time > b.time ? 1 : 0));
      show('[data-tomorrow-loading]', false);
      if (!rows.length) { sec.hidden = true; return; }
      sec.hidden = false;
      body.hidden = false;
      body.replaceChildren();
      rows.forEach((r) => {
        const row = el('a', 'lesson');
        row.href = r.href;
        const time = el('div', 'lesson-time');
        time.append(el('strong', null, r.time));
        const mid = el('div');
        mid.append(el('div', 'lesson-subject', r.title));
        const detail = el('p', 'lesson-detail', EXAM_KIND[r.kind] || LESSON_KIND[r.kind] || '');
        if (r.status === 'moved') detail.append(' ', el('span', 'badge', t('schedule.sheetChange', 'Перенос')));
        if (r.status === 'cancelled') detail.append(' ', el('span', 'badge danger', t('lesson.cancelled', 'Отменена')));
        mid.append(detail);
        row.append(time, mid);
        body.append(row);
      });
    };

    /* --- порядок виджетов по времени суток (03 §4.2): 06–12 базовый
       («Следующая пара» первой), 12–18 — остаток «Сегодня» вверх,
       18–23 — сверху «Что сдать завтра», ночь — «За ночь…» в «Что нового»
       (переставляет renderNews). «Следующая пара» видима, пока есть пара
       (после последней скрывается в renderNext, 03 §5.2). --- */
    const applyDayMode = async () => {
      const mainCol = $$('[data-main-column]');
      const nextSec = $$('[data-widget="next-class"]');
      const todaySec = $$('[data-widget="today"]');
      const tomorrowSec = $$('[data-widget="tomorrow"]');
      if (!mainCol || !nextSec || !todaySec || !tomorrowSec) return;
      const h = new Date().getHours();
      if (h >= 12 && h < 18) {
        mainCol.insertBefore(todaySec, nextSec);
        tomorrowSec.hidden = true;
      } else if (h >= 18 && h < 23) {
        mainCol.insertBefore(tomorrowSec, mainCol.firstElementChild);
        await renderTomorrow();
      } else {
        mainCol.insertBefore(nextSec, todaySec);
        tomorrowSec.hidden = true;
      }
    };

    /* --- загрузка/повтор: скелетоны → данные | ошибки ------------------- */
    const loaders = ['[data-next-loading]', '[data-today-loading]', '[data-deadlines-loading]', '[data-applications-loading]', '[data-news-loading]'];
    const widgetErrors = ['[data-next-error]', '[data-today-error]', '[data-deadlines-error]', '[data-applications-error]', '[data-news-error]'];
    const loadDashboard = async () => {
      loaders.forEach((sel) => show(sel, true));
      show('[data-tomorrow-loading]', true);
      widgetErrors.forEach((sel) => show(sel, false));
      show('[data-dashboard-error]', false);
      /* тела списков очищаются (строки строит JS); «Следующая пара» —
         только прячется: её поля — статичная разметка карточки, их стирает
         нельзя (renderNext заполняет их по data-атрибутам) */
      ['[data-today-body]', '[data-tomorrow-body]', '[data-deadlines-body]', '[data-applications-body]', '[data-news-body]'].forEach((sel) => { const n = $$(sel); if (n) { n.hidden = true; n.replaceChildren(); } });
      show('[data-next-body]', false);
      show('[data-today-empty]', false);
      show('[data-deadlines-empty]', false);
      show('[data-applications-empty]', false);
      show('[data-news-empty]', false);
      const newsTitle = $$('[data-news-title]');
      if (newsTitle) newsTitle.textContent = t('dash.whatsNew', 'Что нового');
      const data = await fetchJSON('/v1/dashboard');
      if (!data) {
        /* офлайн (баннер уже показал каркас) или ошибка API — экранная
           ошибка с «Повторить»; скелетоны скрыть (03 §5.3) */
        loaders.forEach((sel) => show(sel, false));
        show('[data-dashboard-error]', true);
        return;
      }
      lastData = data;
      const name = data.profile && data.profile.short_name ? data.profile.short_name : '';
      const tail = $$('[data-greeting-tail]');
      if (tail) tail.hidden = !name;
      if (name) {
        const nameEl = $$('[data-account-short]');
        if (nameEl) nameEl.textContent = name;
      }
      const degraded = Array.isArray(data.degraded) ? data.degraded : [];
      const hit = (k) => degraded.indexOf(k) !== -1;
      renderNext(data.next_class || null, hit('next_class'));
      renderToday(Array.isArray(data.today) ? data.today : [], hit('today'));
      renderDeadlines(Array.isArray(data.deadlines) ? data.deadlines : [], hit('deadlines'));
      renderApplications(Array.isArray(data.applications) ? data.applications : [], hit('applications'));
      /* 5.17: свежие новости для «Что нового» — отдельный GET /v1/news
         (клиентский, как /v1/schedule/week у renderTomorrow); сбой —
         freshNews null, виджет остаётся на уведомлениях агрегата */
      let freshNews = null;
      const newsFeed = await fetchJSON('/v1/news');
      if (newsFeed && Array.isArray(newsFeed.news)) freshNews = newsFeed.news.slice(0, 2);
      renderNews(Array.isArray(data.news) ? data.news : [], hit('news'), data.unread_count, freshNews);
      await applyDayMode();
    };
    const retry = $$('[data-dashboard-retry]');
    if (retry) retry.addEventListener('click', () => { loadDashboard(); });
    /* компактные «Повторить» виджетов — перезапрашивают весь агрегат
       (03 §5.3: «кнопка „Повторить“ всегда реально перезапрашивает») */
    document.querySelectorAll('[data-widget-retry]').forEach((b) => b.addEventListener('click', () => { loadDashboard(); }));

    /* --- онбординг С5 (03 §4.7/§4.9.4): чеклист «4 шага» поверх «Сейчас
       важно», пока onboarding_json не завершён. Показ — [data-onboarding]
       (разметка над .dashboard-grid, «поверх карточек»); отметка шага —
       тап по строке (renderOnboarding → PUT settings); все 4 или
       «Пропустить» — карточка исчезает навсегда (ключи всех шагов в
       onboarding_json). Сбой настроек чеклист не показывает (баннер
       каркаса уже отреагировал) — виджеты агрегата не зависят от него. --- */
    const initOnboarding = async () => {
      const card = document.querySelector('[data-onboarding]');
      if (!card) return;
      const s = await loadSettings();
      if (!s) return;
      let steps = obNormalize(s.onboarding);
      if (steps.indexOf('login') === -1) {
        /* «Вошёл» выполнен самим фактом входа — отмечается при первом
           показе (проволочная рамка 03 §4.9.4: старт «1/4») */
        steps = obNormalize(steps.concat('login'));
        const saved = await saveOnboarding(steps);
        if (saved) steps = obNormalize(saved.onboarding);
      }
      if (steps.length >= ONBOARD_KEYS.length) return; /* завершён — никогда */
      card.hidden = false;
      renderOnboarding(card, steps);
      const skip = card.querySelector('[data-ob-skip]');
      if (skip) {
        skip.addEventListener('click', async () => {
          if (skip.disabled) return;
          if (!navigator.onLine) { applyOffline(true); return; }
          skip.disabled = true;
          const saved = await saveOnboarding(ONBOARD_KEYS.slice());
          skip.disabled = false;
          if (!saved) { toast(t('toast.failSave', 'Не получилось сохранить — проверьте интернет')); return; }
          card.hidden = true;
        });
      }
    };
    initOnboarding();
    loadDashboard();
  }

  /* ---------- экран «Расписание» (views/schedule.csp; 03 §4.3, §5) ---------- */
  /* Сегменты «Сегодня | Неделя» и стрелки недели ±1 — htmx (вендоренный
     static/vendor/htmx.min.js, лицензия — docs/decisions.md Р-18):
     hx-get на /schedule/segment?segment=today|week&offset=N, outerHTML-
     обмен контейнера #schedule-segment (его data-state="data|empty"
     приезжает с каждым фрагментом), скелетон — hx-indicator. Vanilla JS
     остаётся для sheet «Детали пары» и состояний «Ошибка/Офлайн»
     (03 §5.3/§5.4). Данные карточек — data-атрибуты, отрисованные
     сервером; вставка — только textContent (02 §6.4). */
  if (document.body.dataset.page === 'schedule') {
    const sheet = document.getElementById('lesson-sheet');
    const errBox = document.getElementById('schedule-error');
    const PEOPLE_ID_OFFSET = 1000; // профиль человека = teacher_id + 1000 (Р-15)
    /* метки — словарь 5.18 (t: бандл сервера), фолбэк — прежние строки */
    const KIND = {
      lecture: t('lesson.lecture', 'Лекция'),
      seminar: t('lesson.seminar', 'Семинар'),
      practice: t('lesson.practice', 'Практика')
    };
    const STATUS = {
      normal: t('lesson.normal', 'По расписанию'),
      moved: t('lesson.moved', 'Перенесена'),
      cancelled: t('lesson.cancelled', 'Отменена'),
      substituted: t('lesson.substituted', 'Замена')
    };
    const segment = () => document.getElementById('schedule-segment');

    /* --- диалог занятия (эталон .lesson-dialog/.detail-list, v3 шаг 3).
       Источник — data-атрибуты карточки [data-event] (сервер); вставка —
       только textContent/createElement (02 §6.4). Блок «было → стало» —
       эквивалент schedule-change.html (отдельной страницы в проде нет).
       Имя преподавателя — GET /v1/people/{teacher_id+1000} (Р-15). --- */
    const sheetPart = (sel) => (sheet ? sheet.querySelector(sel) : null);
    const setField = (sel, text) => { const n = sheetPart(sel); if (n) n.textContent = text; };
    const withBuilding = (room, building) => (building ? room + ' · ' + building : room);
    let sheetOpener = null;
    let sheetSeq = 0;
    const openSheet = async (card, opener) => {
      if (!sheet) return;
      const d = card.dataset;
      const seq = ++sheetSeq;
      sheetOpener = opener || null;
      const subject = card.querySelector('.lesson-subject');
      setField('[data-sheet-title]', subject ? subject.textContent.trim() : t('lesson.title', 'Пара'));
      setField('[data-sheet-time]', [d.start, d.end].filter(Boolean).join('–'));
      setField('[data-sheet-when]', d.dateLabel || d.date || '');
      setField('[data-sheet-room]', withBuilding(d.room || '—', d.building));
      const map = sheetPart('[data-sheet-map]');
      if (map) {
        map.href = '/map?q=' + encodeURIComponent(d.room || '');
        map.hidden = !d.room || d.status === 'cancelled';
      }
      setField('[data-sheet-kind]', KIND[d.kind] || '—');
      setField('[data-sheet-status]', STATUS[d.status] || d.status || '');
      /* «было → стало» */
      const changeRow = sheetPart('[data-sheet-change-row]');
      const route = sheetPart('[data-sheet-change]');
      const changed = d.status === 'moved' || d.status === 'cancelled' || d.status === 'substituted';
      if (changeRow) changeRow.hidden = !changed;
      if (changed && route) {
        route.replaceChildren();
        const sameTime = !d.prevStart || (d.prevStart === d.start && !d.prevDateLabel);
        let label = t('schedule.changeSub', 'Замена преподавателя');
        let note = (d.start || '') + '–' + (d.end || '');
        if (d.status === 'moved') {
          label = sameTime ? t('schedule.changeRoomMoved', 'Перенос аудитории') : t('schedule.changeClassMoved', 'Перенос занятия');
          if (d.prevRoom && d.prevRoom !== d.room) {
            const was = document.createElement('s');
            was.textContent = withBuilding(d.prevRoom, d.prevBuilding);
            const arrow = document.createElement('span');
            arrow.setAttribute('aria-hidden', 'true');
            arrow.textContent = '→';
            route.append(was, arrow);
          }
          const now = document.createElement('a');
          now.href = '/map?q=' + encodeURIComponent(d.room || '');
          now.textContent = withBuilding(d.room || '—', d.building);
          route.append(now);
          note = sameTime
            ? t('schedule.changeSameTime', 'Время прежнее') + ' · ' + note
            : t('schedule.changeWas', 'Было:') + ' ' + [d.prevDateLabel, d.prevStart].filter(Boolean).join(', ');
        } else if (d.status === 'cancelled') {
          label = t('schedule.changeCancelled', 'Занятие отменено');
          const was = document.createElement('s');
          was.textContent = withBuilding(d.room || '—', d.building);
          route.append(was);
        }
        route.hidden = route.childNodes.length === 0;
        setField('[data-sheet-change-label]', label);
        setField('[data-sheet-change-note]', note);
      }
      const link = sheetPart('[data-sheet-teacher]');
      const fallback = sheetPart('[data-sheet-teacher-empty]');
      const teacherId = parseInt(d.teacherId || '', 10);
      if (!sheet.open) sheet.showModal();
      if (link && Number.isFinite(teacherId)) {
        link.href = '/people/' + (teacherId + PEOPLE_ID_OFFSET);
        link.textContent = t('common.loading', 'Загрузка…');
        link.hidden = false;
        if (fallback) fallback.hidden = true;
        const person = await fetchJSON('/v1/people/' + (teacherId + PEOPLE_ID_OFFSET));
        if (seq !== sheetSeq) return; // открыт уже другой урок
        /* GET /v1/people/{id} отдаёт карточку плоско ({fio, …}); прежнее
           чтение person.person.fio всегда давало «Открыть профиль» */
        const fio = person && (person.fio || (person.person && person.person.fio));
        link.textContent = fio || t('common.openProfile', 'Открыть профиль');
      } else if (link && fallback) {
        link.hidden = true;
        fallback.hidden = false;
      }
    };
    /* делегирование (сегмент перерисовывается htmx — слушатель на документе
       переживает обмен): кнопка .lesson-open, «Подробнее» блока изменения
       ([data-open-event]) или клик по карточке мимо ссылок. Ссылки внутри
       карточки (аудитория → карта) не перехватываются. */
    document.addEventListener('click', (event) => {
      const target = event.target;
      if (!(target instanceof Element) || !sheet) return;
      const more = target.closest('[data-open-event]');
      if (more) {
        const box = segment();
        const card = box ? box.querySelector('[data-event][data-event-id="' + CSS.escape(more.dataset.openEvent) + '"]') : null;
        if (card) openSheet(card, more);
        return;
      }
      if (target.closest('a')) return;
      const card = target.closest('[data-event]');
      if (!card) return;
      const opener = card.querySelector('.lesson-open') || card;
      openSheet(card, opener);
    });
    if (sheet) {
      sheet.querySelectorAll('[data-sheet-close]').forEach((b) => b.addEventListener('click', () => sheet.close()));
      sheet.addEventListener('close', () => {
        if (sheetOpener && sheetOpener.isConnected) sheetOpener.focus();
        sheetOpener = null;
      });
      /* клик по подложке закрывает диалог */
      sheet.addEventListener('click', (e) => { if (e.target === sheet) sheet.close(); });
    }

    /* --- выбор дня: «Дата занятия» (input[type=date]) — htmx-обмен сегмента
       на выбранный день + адрес /schedule?date=… в истории; вне окна
       min/max — сообщение словаря. «Выбрать день» пустого дня — фокус в
       поле даты. --- */
    const loadDay = (ymd) => {
      if (!window.htmx) { window.location.href = '/schedule?date=' + ymd; return; }
      window.htmx.ajax('GET', '/schedule/segment?segment=today&date=' + encodeURIComponent(ymd), {
        target: '#schedule-segment', swap: 'outerHTML', indicator: '#schedule-indicator'
      }).then(() => {
        history.pushState({}, '', '/schedule?date=' + ymd);
        const input = document.getElementById('schedule-date');
        if (input) input.focus();
      });
    };
    document.addEventListener('change', (event) => {
      const input = event.target;
      if (!(input instanceof HTMLInputElement) || !input.matches('[data-schedule-date]')) return;
      input.setCustomValidity('');
      if (!input.value || !input.checkValidity()) {
        input.setCustomValidity(input.dataset.invalidMessage || t('schedule.dateInvalid', 'Выберите дату в пределах четырёх недель от текущей.'));
        input.reportValidity();
        return;
      }
      loadDay(input.value);
    });
    document.addEventListener('input', (event) => {
      if (event.target instanceof HTMLInputElement && event.target.matches('[data-schedule-date]')) event.target.setCustomValidity('');
    });
    document.addEventListener('click', (event) => {
      const btn = event.target instanceof Element ? event.target.closest('[data-focus-date]') : null;
      if (!btn) return;
      const input = document.getElementById('schedule-date');
      if (!input) return;
      event.preventDefault();
      input.focus();
      if (typeof input.showPicker === 'function') { try { input.showPicker(); } catch (e) { /* без жеста — просто фокус */ } }
    });

    /* --- «Показать: Все занятия / Лекции / Семинары» (Неделя): фильтр на
       клиенте по data-kind карточек (реальные виды DDL), счётчик «N занятий»
       — формы plural из data-one/few/many (словарь сервера) по
       Intl.PluralRules. Выбор переживает htmx-обмен (стрелки недель). --- */
    let lessonKind = '';
    const applyLessonFilter = () => {
      const box = segment();
      const select = box ? box.querySelector('[data-lesson-filter]') : null;
      if (!box || !select) return;
      if (select.value !== lessonKind) select.value = lessonKind;
      if (select.value !== lessonKind) { lessonKind = ''; select.value = ''; }
      let shown = 0;
      box.querySelectorAll('.lesson[data-kind]').forEach((card) => {
        const on = !lessonKind || card.dataset.kind === lessonKind;
        card.hidden = !on;
        if (on) shown += 1;
      });
      const count = box.querySelector('[data-lesson-count]');
      if (count) {
        const rule = new Intl.PluralRules(LOCALE).select(shown);
        const form = rule === 'one' ? count.dataset.one : rule === 'few' ? count.dataset.few : count.dataset.many;
        count.textContent = shown + ' ' + (form || '');
      }
    };
    document.addEventListener('change', (event) => {
      const select = event.target;
      if (!(select instanceof HTMLSelectElement) || !select.matches('[data-lesson-filter]')) return;
      lessonKind = select.value;
      applyLessonFilter();
    });

    /* --- заголовок страницы следует за сегментом (День/Неделя) --- */
    const syncHeading = () => {
      const box = segment();
      if (!box) return;
      const h1 = document.querySelector('[data-schedule-heading]');
      const lead = document.querySelector('[data-schedule-lead]');
      if (h1 && box.dataset.heading) h1.textContent = box.dataset.heading;
      if (lead && box.dataset.lead) lead.textContent = box.dataset.lead;
    };

    /* --- состояния сегмента (schedule-states.html) --------------------- */
    /* Ошибка (ответ не 2xx или сеть) — блок «Не получилось загрузить
       расписание» с «Повторить», перезапускающим запрос исходного
       элемента; офлайн — баннер каркаса; истёкшая сессия (фильтр отвечает
       303 → /login, XHR следует редиректу) — браузерный переход на вход. */
    let retryElt = null;
    document.body.addEventListener('htmx:afterRequest', (e) => {
      if (!errBox) return;
      if (e.detail && e.detail.successful) {
        errBox.hidden = true;
        return;
      }
      if (e.detail && e.detail.elt) retryElt = e.detail.elt;
      errBox.hidden = false;
    });
    document.body.addEventListener('htmx:sendError', (e) => {
      if (!navigator.onLine) applyOffline(true);
      if (errBox) {
        if (e.detail && e.detail.elt) retryElt = e.detail.elt;
        errBox.hidden = false;
      }
    });
    document.body.addEventListener('htmx:beforeSwap', (e) => {
      const xhr = e.detail && e.detail.xhr;
      if (xhr && /\/login(?:\?.*)?$/.test(xhr.responseURL || '')) {
        e.detail.shouldSwap = false;
        window.location.href = '/login';
      }
    });
    const scheduleRetry = document.querySelector('[data-schedule-retry]');
    if (scheduleRetry) {
      scheduleRetry.addEventListener('click', () => {
        if (retryElt && retryElt.isConnected && retryElt.matches('[hx-get]')) retryElt.click();
        else window.location.reload();
      });
    }

    /* --- сегмент «Неделя»: автоскролл к сегодняшнему дню (только когда
       дни идут стопкой, <1200px — на сетке вся неделя видна) и свайп
       влево/вправо = ‹/› неделя. Стрелки ищутся по data-week-prev/next
       (не по aria-label: он переводится, 5.18). Контейнер
       #schedule-segment свапается htmx целиком — ссылка на него
       перезапрашивается в каждом обработчике. --- */
    const reducedMotion = window.matchMedia('(prefers-reduced-motion: reduce)').matches;
    const gridWeek = window.matchMedia('(min-width: 1200px)');
    const scrollTodayIntoView = () => {
      if (gridWeek.matches) return;
      const box = segment();
      const todayGroup = box ? box.querySelector('.day-group.is-today') : null;
      if (!todayGroup) return;
      const r = todayGroup.getBoundingClientRect();
      if (r.top >= 0 && r.bottom <= window.innerHeight) return;
      todayGroup.scrollIntoView({ block: 'start', behavior: reducedMotion ? 'auto' : 'smooth' });
    };
    scrollTodayIntoView();
    document.body.addEventListener('htmx:afterSwap', () => {
      syncHeading();
      applyLessonFilter();
      setTimeout(scrollTodayIntoView, 0);
    });
    let touchX = 0, touchY = 0, swipeArmed = false;
    document.addEventListener('touchstart', (e) => {
      const box = segment();
      swipeArmed = Boolean(box) && box.contains(e.target) && !(sheet && sheet.open);
      if (!swipeArmed) return;
      const p = e.touches[0];
      touchX = p.clientX;
      touchY = p.clientY;
    }, { passive: true });
    document.addEventListener('touchend', (e) => {
      if (!swipeArmed) return;
      swipeArmed = false;
      const box = segment();
      if (!box) return;
      const p = e.changedTouches[0];
      const dX = p.clientX - touchX;
      const dY = p.clientY - touchY;
      if (Math.abs(dX) < 56 || Math.abs(dY) >= Math.abs(dX)) return;
      const arrow = box.querySelector(dX < 0 ? '[data-week-next]' : '[data-week-prev]');
      if (arrow && !arrow.disabled) arrow.click();
    }, { passive: true });
  }

  /* ---------- экран «Мои заявки» (views/applications.csp; 03 §4.5.1) ------ */
  /* Страница целиком SSR (тело ApplicationsService::list рендерит сервер;
     фильтр ?status= — чипы-ссылки). Здесь только «Повторить» состояния
     ошибки: реально перезапрашивает — перезагрузка страницы (03 §5.3);
     офлайн обслуживает баннер каркаса (topbar + applyOffline, 03 §5.4). */
  if (document.body.dataset.page === 'applications') {
    const applicationsRetry = document.querySelector('[data-applications-retry]');
    if (applicationsRetry) applicationsRetry.addEventListener('click', () => window.location.reload());
  }

  /* ---------- экран «Новая заявка» (views/application-form.csp; 03
     §4.5.2–4.5.3, С2/С7) --------------------------------------------------- */
  /* Каталог типов грузится fetch-ом GET /v1/application-types (значения
     НЕ захардкожены — постановка Э7), профиль — GET /v1/me (ФИО/группа,
     read-only). Выбор типа → форма 1–2 вопроса (наборы полей — по
     payload-примерам sql/fixtures.sql и меткам каталога 03 §4.5.2).
     «Сохранить черновик» → POST /v1/applications {submit:false} → тост
     «Черновик сохранён», далее автосохранение PUT /v1/applications/{id}
     раз в ~10 с и по visibilitychange (hidden) — ТОЛЬКО пока статус
     draft; ответ 409 (статус ушёл с черновика) — АВТОСОХРАНЕНИЕ МОЛЧА
     ПРЕКРАЩАЕТСЯ, без всплывающих ошибок (постановка Э7; try/catch).
     «Отправить заявку»: есть сохранённый черновик — PUT /v1/applications/{id}
     {payload_json, submit:true} (8.i, Р-29: подача существующего черновика
     одним вызовом, fe_rebuild шаг 5); черновика нет (или 409 — уже не
     черновик) — POST {submit:true}; затем редирект на карточку; ошибка —
     inline «Заявка не отправилась…» (03 §5.3).
     Вставка — только createElement/textContent (02 §6.4); innerHTML —
     НИКОГДА для данных сервера, только статические svg-константы ниже. */
  if (document.body.dataset.page === 'application-form') {
    const show = (sel, on) => { const n = document.querySelector(sel); if (n) n.hidden = !on; };
    const el = (tag, className, text) => {
      const node = document.createElement(tag);
      if (className) node.className = className;
      if (text != null) node.textContent = text;
      return node;
    };
    /* статические иконки строк каталога (по мотивам Lucide; НЕ данные) */
    const ICON = {
      file: '<svg class="icon" viewBox="0 0 24 24" aria-hidden="true"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8ZM14 2v6h6M8 13h8M8 17h5"/></svg>',
      pen: '<svg class="icon" viewBox="0 0 24 24" aria-hidden="true"><path d="M12 20h9M16.5 3.5a2.1 2.1 0 0 1 3 3L7 19l-4 1 1-4Z"/></svg>',
      arrow: '<svg class="icon" viewBox="0 0 24 24" aria-hidden="true"><path d="M5 12h14m-6-6 6 6-6 6"/></svg>'
    };
    /* 1–2 вопроса на тип: наборы полей — payload-примеры sql/fixtures.sql
       ({"copies":2,"delivery":"office"}, {"purpose":…,"copies":1,"delivery":"pdf"},
       {"topic":…,"supervisor":…}); подсказки строк каталога — 03 §4.5.2 */
    const TYPE_FIELDS = {
      study_certificate: ['copies', 'delivery'],
      period_certificate: ['copies', 'delivery'],
      /* academic_certificate — ровно 2 вопроса, как в вайрфрейме 03
         §4.5.3:597–625 («Количество экземпляров» + «Как получить»);
         «purpose» в форме НЕТ (правило «1–2 вопроса максимум» —
         03 §4.5.3:626), поле остаётся в примере payload фикстур
         ({"purpose":…}) и отображается в «Деталях заявки» карточки */
      academic_certificate: ['copies', 'delivery'],
      thesis_topic: ['topic', 'supervisor'],
      other: ['comment']
    };
    const TYPE_HINT = {
      study_certificate: 'Обычно до 3 рабочих дней · PDF или в деканате',
      period_certificate: 'Обычно до 3 рабочих дней · PDF или в деканате',
      academic_certificate: 'Обычно до 3 рабочих дней · PDF или в деканате',
      thesis_topic: 'Согласование темы и научного руководителя',
      other: 'Свободный текст заявления'
    };

    const state = { typeKey: null, typeTitle: '', draftId: null, draftStopped: false };
    let saveTimer = 0;
    const stopTimer = () => { if (saveTimer) { clearInterval(saveTimer); saveTimer = 0; } };

    /* --- сборка payload из полей активного типа (JSON-строка) ---------- */
    const field = (name) => document.querySelector('[data-field="' + name + '"]');
    const collectPayload = () => {
      const p = {};
      (TYPE_FIELDS[state.typeKey] || []).forEach((name) => {
        const f = field(name);
        if (!f) return;
        if (f.dataset.kind === 'radios') {
          const checked = f.querySelector('input:checked');
          p[name] = checked ? checked.value : '';
          if (name === 'copies' && p.copies !== '') p.copies = Number(p.copies);
        } else if (f.value) {
          p[name] = f.value;
        }
      });
      return p;
    };

    /* --- автосохранение черновика (С7): raw fetch — нужен статус 409;
       сетевой сбой/офлайн молча пропускается (баннер показывает каркас);
       409 «Править можно только черновик» — статус ушёл, автосохранение
       прекращается навсегда до перезагрузки (постановка Э7) ------------ */
    const autosaveNow = async () => {
      if (!state.draftId || state.draftStopped) return null;
      try {
        const resp = await fetch('/v1/applications/' + state.draftId, {
          method: 'PUT',
          headers: { Accept: 'application/json', 'Content-Type': 'application/json' },
          body: JSON.stringify({ payload_json: JSON.stringify(collectPayload()) })
        });
        if (resp.status === 409) {
          state.draftStopped = true;
          stopTimer();
          const statusLine = document.querySelector('[data-draft-status]');
          if (statusLine) statusLine.textContent = 'Черновик больше не черновик — правки не сохраняются';
          return false;
        }
        if (resp.status === 404) {
          /* заявка исчезла — автосохранению некуда писать; молча прекращаем */
          state.draftStopped = true;
          stopTimer();
          return false;
        }
        return resp.ok;
      } catch (e) { return null; } /* сети нет — молча (03 §5.4) */
    };
    const startAutosave = () => {
      if (!saveTimer) saveTimer = setInterval(autosaveNow, 10000);
    };
    document.addEventListener('visibilitychange', () => {
      if (document.hidden) autosaveNow();
    });

    /* --- поля формы: 1–2 вопроса, значения по умолчанию — как в
       черновике 03 §4.5.3 (copies 1, delivery pdf) --------------------- */
    const buildField = (name) => {
      if (name === 'copies') {
        const wrap = el('fieldset');
        wrap.dataset.field = 'copies';
        wrap.dataset.kind = 'radios';
        wrap.append(el('legend', null, 'Количество экземпляров'));
        [['1', '1 экземпляр', true], ['2', '2 экземпляра', false]].forEach(([v, label, on]) => {
          const opt = el('label', 'option');
          const input = document.createElement('input');
          input.type = 'radio';
          input.name = 'copies';
          input.value = v;
          input.checked = on;
          opt.append(input, document.createTextNode(label));
          wrap.append(opt);
        });
        return wrap;
      }
      if (name === 'delivery') {
        const wrap = el('fieldset');
        wrap.dataset.field = 'delivery';
        wrap.dataset.kind = 'radios';
        wrap.append(el('legend', null, 'Как получить'));
        [['pdf', 'PDF в кабинете', true], ['office', 'В деканате', false]].forEach(([v, label, on]) => {
          const opt = el('label', 'option');
          const input = document.createElement('input');
          input.type = 'radio';
          input.name = 'delivery';
          input.value = v;
          input.checked = on;
          opt.append(input, document.createTextNode(label));
          wrap.append(opt);
        });
        return wrap;
      }
      const wrap = el('div', 'field');
      wrap.dataset.field = name;
      const text = {
        purpose: ['Куда нужна справка', 'Например, в университет-партнёр'],
        topic: ['Тема ВКР', 'Тема выпускной квалификационной работы'],
        supervisor: ['Научный руководитель', 'ФИО руководителя'],
        comment: ['Комментарий · необязательно', 'Например, куда нужна справка']
      }[name] || [name, ''];
      wrap.append(el('label', null, text[0]));
      const input = name === 'comment' ? document.createElement('textarea') : document.createElement('input');
      if (name === 'comment') input.setAttribute('rows', '4');
      input.maxLength = 1000;
      input.placeholder = text[1];
      wrap.append(input);
      return wrap;
    };

    const setFormError = (text) => {
      const box = document.querySelector('[data-form-error-inline]');
      if (!box) return;
      box.textContent = text;
      box.hidden = !text;
    };

    /* --- выбор типа: каталог → форма; смена типа СБРАСЫВАЕТ привязку
       автосохранения (PUT чужого типа в старый черновик недопустим;
       сохранённый черновик остаётся в «Черновиках» списка) ------------- */
    const selectType = (key, title) => {
      if (state.typeKey !== key) stopTimer();
      state.typeKey = key;
      state.typeTitle = title;
      state.draftId = null;
      state.draftStopped = false;
      const fields = document.querySelector('[data-type-fields]');
      if (fields) {
        fields.replaceChildren();
        (TYPE_FIELDS[key] || []).forEach((name) => fields.append(buildField(name)));
      }
      const titleEl = document.querySelector('[data-type-title]');
      if (titleEl) titleEl.textContent = title;
      setFormError('');
      const statusLine = document.querySelector('[data-draft-status]');
      if (statusLine) statusLine.textContent = 'Обычно готова до 3 рабочих дней';
      show('[data-catalog]', false);
      show('[data-form]', true);
    };
    const backToCatalog = () => {
      stopTimer();
      state.draftId = null;
      state.draftStopped = false;
      show('[data-form]', false);
      show('[data-catalog]', true);
    };

    /* --- каталог и профиль (fetch; скелетон → данные | ошибка) --------- */
    const renderCatalog = (types) => {
      const box = document.querySelector('[data-catalog]');
      if (!box) return;
      box.replaceChildren();
      types.forEach((t) => {
        const row = el('button', 'document-type');
        row.type = 'button';
        row.append(document.createRange().createContextualFragment((t.key === 'thesis_topic' || t.key === 'other') ? ICON.pen : ICON.file));
        const mid = el('div');
        mid.append(el('h2', null, t.title || ''));
        mid.append(el('p', null, TYPE_HINT[t.key] || ''));
        row.append(mid);
        row.append(document.createRange().createContextualFragment(ICON.arrow));
        row.addEventListener('click', () => selectType(t.key, t.title || ''));
        box.append(row);
      });
    };
    const loadForm = async () => {
      show('[data-form-loading]', true);
      show('[data-catalog]', false);
      show('[data-form]', false);
      show('[data-form-error]', false);
      const [types, me] = await Promise.all([
        fetchJSON('/v1/application-types'),
        fetchJSON('/v1/me')
      ]);
      show('[data-form-loading]', false);
      if (!types || !Array.isArray(types.types)) {
        /* офлайн уже показал баннер (fetchJSON); ошибка API — экранная */
        show('[data-form-error]', true);
        return;
      }
      renderCatalog(types.types);
      show('[data-catalog]', true);
      if (me && me.profile) {
        const p = me.profile;
        const card = document.querySelector('[data-profile-card]');
        if (card) {
          const fio = document.querySelector('[data-profile-fio]');
          if (fio) fio.textContent = p.fio || '';
          const meta = document.querySelector('[data-profile-meta]');
          if (meta) {
            meta.textContent = [p.course ? p.course + ' курс' : '', p.faculty || '', p.group ? 'группа ' + p.group : '']
              .filter(Boolean).join(' · ');
          }
          card.hidden = false;
        }
      }
    };
    const formRetry = document.querySelector('[data-form-retry]');
    if (formRetry) formRetry.addEventListener('click', () => { loadForm(); });
    const typeBack = document.querySelector('[data-type-back]');
    if (typeBack) typeBack.addEventListener('click', backToCatalog);
    loadForm();

    /* --- кнопки формы --------------------------------------------------- */
    const busy = (btn, on, busyLabel) => {
      if (!btn) return;
      if (on) {
        btn.dataset.wasLabel = btn.textContent;
        btn.textContent = busyLabel || t('toast.sending', 'Отправляем…');
        btn.disabled = true;
      } else {
        btn.textContent = btn.dataset.wasLabel || btn.textContent;
        delete btn.dataset.wasLabel;
        btn.disabled = false;
      }
    };
    const saveDraftBtn = document.querySelector('[data-save-draft]');
    if (saveDraftBtn) {
      saveDraftBtn.addEventListener('click', async () => {
        if (!state.typeKey || saveDraftBtn.disabled) return;
        setFormError('');
        const statusLine = document.querySelector('[data-draft-status]');
        if (!navigator.onLine) { applyOffline(true); return; }
        if (state.draftStopped) {
          /* 409 ранее: черновик уже не draft — сохранение бессмысленно */
          setFormError('Черновик уже не черновик — изменение не сохраняется');
          return;
        }
        busy(saveDraftBtn, true, 'Сохраняем…');
        try {
          let ok = false;
          if (state.draftId) {
            ok = (await autosaveNow()) === true;
          } else {
            const resp = await fetch('/v1/applications', {
              method: 'POST',
              headers: { Accept: 'application/json', 'Content-Type': 'application/json' },
              body: JSON.stringify({
                type: state.typeKey,
                payload_json: JSON.stringify(collectPayload()),
                submit: false
              })
            });
            if (resp.ok) {
              const body = await resp.json();
              state.draftId = body && body.id;
              startAutosave();
              ok = true;
            } else if (resp.status !== 409) {
              ok = false;
            }
          }
          if (ok) {
            toast('Черновик сохранён');
            const t = new Date().toLocaleTimeString('ru-RU', { hour: '2-digit', minute: '2-digit' });
            if (statusLine) statusLine.textContent = 'Черновик сохранён · ' + t + ' · дальше сохраняем автоматически';
          } else if (!navigator.onLine) {
            applyOffline(true); /* каркас покажет баннер и helper на кнопке */
          } else {
            setFormError('Черновик не сохранился. Проверьте интернет — и попробуйте ещё раз');
          }
        } catch (e) {
          if (!navigator.onLine) applyOffline(true);
        } finally {
          busy(saveDraftBtn, false);
        }
      });
    }
    const form = document.querySelector('[data-form]');
    if (form) {
      form.addEventListener('submit', async (event) => {
        event.preventDefault();
        const submitBtn = form.querySelector('[data-submit-app]');
        if (!state.typeKey || (submitBtn && submitBtn.disabled)) return;
        setFormError('');
        if (!navigator.onLine) { applyOffline(true); return; }
        busy(submitBtn, true, t('toast.sending', 'Отправляем…'));
        try {
          /* 8.i (Р-29): есть живой черновик — подаём ЕГО одним PUT {payload_json,
             submit:true} (синк полей + переход draft→submitted, без дубля-черновика);
             409 «уже не черновик» — черновика больше нет, подаём новой POST-строкой. */
          let resp = null;
          if (state.draftId && !state.draftStopped) {
            resp = await fetch('/v1/applications/' + state.draftId, {
              method: 'PUT',
              headers: { Accept: 'application/json', 'Content-Type': 'application/json' },
              body: JSON.stringify({ payload_json: JSON.stringify(collectPayload()), submit: true })
            });
            if (resp.ok) {
              stopTimer();
              window.location.href = '/applications/' + state.draftId;
              return;
            }
            if (resp.status === 409 || resp.status === 404) { state.draftStopped = true; stopTimer(); resp = null; }
          }
          if (!resp) resp = await fetch('/v1/applications', {
            method: 'POST',
            headers: { Accept: 'application/json', 'Content-Type': 'application/json' },
            body: JSON.stringify({
              type: state.typeKey,
              payload_json: JSON.stringify(collectPayload()),
              submit: true
            })
          });
          if (resp.ok) {
            const body = await resp.json();
            window.location.href = '/applications/' + (body && body.id);
            return;
          }
          let message = 'Заявка не отправилась';
          try {
            const err = await resp.json();
            if (err && err.error && err.error.message) message = err.error.message;
          } catch (e) { /* тело не JSON */ }
          if (state.draftId && !state.draftStopped) {
            message += '. Черновик сохранён — попробуйте ещё раз, ничего не потеряется';
          } else {
            message += '. Проверьте интернет — и попробуем ещё раз';
          }
          setFormError(message);
        } catch (e) {
          if (!navigator.onLine) applyOffline(true);
          else setFormError('Заявка не отправилась. Проверьте интернет — и попробуем ещё раз');
        } finally {
          busy(submitBtn, false);
        }
      });
    }
  }

  /* ---------- карточка заявки (views/application.csp; 03 §4.5.5, С7) ----- */
  /* Страница SSR; здесь — ветка «Требуется уточнение» (С7.6): POST
     /{id}/messages → в ответе application_status=="in_progress" → бейдж
     «В работе» и событие в таймлайн; «Скачать PDF»: переход на
     /v1/applications/{id}/pdf через fetch — 200 blob → скачивание, 409
     not_ready → inline «Справка ещё не готова» у кнопки (03 §5); «Повторить»
     экрана ошибки — перезагрузка (03 §5.3). Вставка — только
     createElement/textContent (02 §6.4). */
  if (document.body.dataset.page === 'application') {
    const APP_STATUS = {
      draft: 'Черновик', submitted: 'Подана', accepted: 'Принята',
      in_progress: 'В работе', need_info: 'Требуется уточнение',
      ready: 'Готова', issued: 'Выдана', rejected: 'Отклонена', cancelled: 'Отменена'
    };
    const appRetry = document.querySelector('[data-app-retry]');
    if (appRetry) appRetry.addEventListener('click', () => window.location.reload());

    /* --- PDF: 200 → blob-скачивание; 409 not_ready → inline-сообщение
       «Справка ещё не готова» у кнопки (серверный message из 02 §5.2);
       прочие ошибки — inline c message тела (формат 02 §5.1) ----------- */
    const pdfLink = document.querySelector('[data-pdf-link]');
    const pdfError = document.querySelector('[data-pdf-error]');
    if (pdfLink) {
      pdfLink.addEventListener('click', async (event) => {
        event.preventDefault();
        if (!navigator.onLine) { applyOffline(true); return; }
        if (pdfError) pdfError.hidden = true;
        try {
          const resp = await fetch(pdfLink.href, { headers: { Accept: 'application/pdf' } });
          if (resp.ok) {
            const blob = await resp.blob();
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = 'para-spravka-' + (pdfLink.getAttribute('href').split('/').pop() || '') + '.pdf';
            document.body.append(a);
            a.click();
            a.remove();
            setTimeout(() => URL.revokeObjectURL(url), 5000);
            return;
          }
          let message = 'Не удалось скачать справку';
          try {
            const body = await resp.json();
            if (body && body.error && body.error.message) message = body.error.message;
          } catch (e) { /* тело не JSON */ }
          if (pdfError) {
            pdfError.textContent = message;
            pdfError.hidden = false;
          }
        } catch (e) {
          if (!navigator.onLine) applyOffline(true);
          if (pdfError) {
            pdfError.textContent = 'Не удалось скачать справку';
            pdfError.hidden = false;
          }
        }
      });
    }

    /* --- ветка С7.6: ответ на уточнение --------------------------------- */
    const replyBlock = document.querySelector('[data-reply-block]');
    const replyBtn = document.querySelector('[data-reply-send]');
    const replyText = document.querySelector('[data-reply-text]');
    const replyError = document.querySelector('[data-reply-error]');
    const replyResult = document.querySelector('[data-reply-result]');
    const badge = document.querySelector('[data-app-badge]');
    const timeline = document.querySelector('[data-app-timeline]');
    if (replyBtn && replyBlock) {
      replyBtn.addEventListener('click', async () => {
        if (replyBtn.disabled) return;
        if (!navigator.onLine) { applyOffline(true); return; }
        const appId = replyBlock.dataset.appId;
        const text = replyText ? replyText.value.trim() : '';
        if (replyError) replyError.hidden = true;
        if (!text) {
          if (replyText) replyText.setAttribute('aria-invalid', 'true');
          if (replyError) {
            replyError.textContent = 'Напишите ответ — деканат ждёт уточнение';
            replyError.hidden = false;
          }
          return;
        }
        if (replyText) replyText.removeAttribute('aria-invalid');
        const previousLabel = replyBtn.textContent;
        replyBtn.textContent = t('toast.sending', 'Отправляем…');
        replyBtn.disabled = true;
        try {
          /* fetchJSON форматует ошибки из {error:{code,message}} (02 §5.1);
             201 — Ok, тело = событие + application_status/status (С7.6) */
          const data = await fetchJSON('/v1/applications/' + appId + '/messages', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ text })
          });
          if (data) {
            const newStatus = data.application_status || data.status;
            if (newStatus === 'in_progress' && badge) {
              badge.className = 'badge info';
              badge.textContent = APP_STATUS.in_progress;
            }
            if (timeline) {
              const li = document.createElement('li');
              li.className = (data.to_status === 'in_progress') ? 'done' : '';
              const strong = document.createElement('strong');
              strong.textContent = APP_STATUS[data.to_status] || data.to_status || 'Сообщение';
              const p = document.createElement('p');
              const when = data.at ? new Date(data.at).toLocaleString('ru-RU', { day: 'numeric', month: 'long', hour: '2-digit', minute: '2-digit' }) : '';
              p.textContent = when + (text ? ' · ' + text : '');
              li.append(strong, p);
              timeline.append(li);
            }
            replyBlock.hidden = true;
            if (replyResult) {
              replyResult.textContent = newStatus === 'in_progress'
                ? 'Ответ отправлен — заявка снова в работе'
                : 'Ответ отправлен';
              replyResult.hidden = false;
            }
          } else if (!navigator.onLine) {
            applyOffline(true); /* каркас: баннер + helper на кнопке */
          } else if (replyError) {
            replyError.textContent = 'Ответ не отправился. Проверьте интернет — и попробуем ещё раз';
            replyError.hidden = false;
          }
        } catch (e) {
          if (!navigator.onLine) applyOffline(true);
        } finally {
          replyBtn.textContent = previousLabel;
          replyBtn.disabled = false;
        }
      });
    }
  }

  /* ---------- экран «Бронь аудитории» (views/rooms.csp; 03 §4.9.5) ------ */
  /* Данные — три fetch из ТЕХ ЖЕ JSON-роутов, что за API (02 §5.2):
     каталог GET /v1/rooms (query ?capacity= — фильтр «вместимость не
     меньше»), сетка дня GET /v1/rooms/{id}/slots?date= (дейт-пикер,
     дефолт — сегодня UTC проекта; слоты 08:00–22:00 UTC по 60 минут,
     состояния free/booked/mine — RoomsService), «Мои брони» GET
     /v1/room-bookings?upcoming=1.
     Поток брони — ТРИ ШАГА 03 §4.9.5 (правки по ревью — до этого тап
     слота POST-ил сразу, риск случайной брони): каталог → сетка →
     подтверждение. Тап свободного слота ТОЛЬКО выбирает его
     (aria-pressed, акцент .room-slot[aria-pressed=true]), карточка
     data-booking-card (.booking-summary черновика rooms.html) показывает
     «Ауд. …», корпус/вместимость/оборудование (features — из каталога,
     слотовый ответ features не отдаёт) и выбранный интервал, primary
     «Забронировать HH:MM–HH:MM» активируется — POST /v1/rooms/{id}/
     bookings {starts_at, ends_at} строками слота сетки «…Z» (границы не
     вычисляются на клиенте). Исходы (постановка Э7):
       201          → success-тост «Ауд. …, HH:MM–HH:MM — забронировано»
                      с действием «Мои брони» (якорь к секции; дословно
                      03 §4.9.5) + перерисовка сетки (слот стал mine) и
                      «Моих брони»;
       400 slot_taken → inline «Слот только что заняли — обновляем сетку»
                      + перерисовка сетки (данные могли устареть);
       409 conflict/blocked → inline с текстом {error:{message}}
                      (02 §5.1) + перерисовка сетки;
       404/прочие   → inline message тела; сеть/офлайн — баннер каркаса.
     Отмена — danger-диалог <dialog> как sheet «Детали пары»
     (03 §2.8/§6.4): подтверждение → DELETE /v1/room-bookings/{id};
     204 → строка уходит, тост «Бронь отменена», сетка тихо перечитывается
     (слот мог освободиться); 409 повторной отмены → inline у списка
     с message тела. Состояния 03 §5: скелетоны секций, пусто («На эту
     дату слотов нет» с primary «Каталог аудиторий» — якорь к каталогу —
     / «Броней нет»), ошибки секций с «Повторить» (повторный fetch),
     офлайн — баннер каркаса. Вставка — только createElement/textContent
     (02 §6.4). mine-слот не тапается: id брони в сетке слотов не
     отдаётся — отмена в «Моих брони». */
  if (document.body.dataset.page === 'rooms') {
    const show = (sel, on) => { const n = document.querySelector(sel); if (n) n.hidden = !on; };
    const el = (tag, className, text) => {
      const node = document.createElement(tag);
      if (className) node.className = className;
      if (text != null) node.textContent = text;
      return node;
    };
    const plural = (n, one, few, many) => {
      const m10 = n % 10, m100 = n % 100;
      if (m10 === 1 && m100 !== 11) return one;
      if (m10 >= 2 && m10 <= 4 && (m100 < 12 || m100 > 14)) return few;
      return many;
    };
    const timeOf = (iso) => (typeof iso === 'string' && iso.length >= 16) ? iso.slice(11, 16) : '--:--';
    const dateOf = (iso) => (typeof iso === 'string' && iso.length >= 10) ? iso.slice(0, 10) : '';
    /* формат дня по календарной дате UTC проекта (сдвига часового пояса нет) */
    const fmtDay = (ymd) => {
      const p = ymd.split('-').map(Number);
      if (p.length !== 3 || p.some(Number.isNaN)) return ymd;
      return new Date(Date.UTC(p[0], p[1] - 1, p[2])).toLocaleDateString('ru-RU', { weekday: 'short', day: 'numeric', month: 'long', timeZone: 'UTC' });
    };
    const todayUtc = () => new Date().toISOString().slice(0, 10);

    const capSelect = document.querySelector('[data-capacity-filter]');
    const dateInput = document.querySelector('[data-slots-date]');
    const state = { rooms: [], roomId: null, roomName: '', selected: null };
    let bookingBusy = false; // двойной submit подтверждения (быстрый тап)

    const showSlotWarn = (text) => {
      const box = document.querySelector('[data-slots-warn]');
      if (!box) return;
      box.textContent = text;
      box.hidden = false;
    };
    const showBookingsWarn = (text) => {
      const box = document.querySelector('[data-bookings-warn]');
      if (!box) return;
      box.textContent = text;
      box.hidden = false;
    };

    /* --- каталог аудиторий: чипы (номер · корпус · вместимость) -------- */
    const syncChips = () => {
      document.querySelectorAll('[data-room-chips] .chip').forEach((chip) => {
        chip.setAttribute('aria-pressed', String(Number(chip.dataset.roomId) === state.roomId));
      });
    };
    const selectRoom = (id) => {
      if (state.roomId === id) return;
      state.roomId = id;
      syncChips();
      loadSlots();
    };
    const loadCatalog = async () => {
      show('[data-rooms-loading]', true);
      show('[data-rooms-error]', false);
      show('[data-room-chips]', false);
      const cap = capSelect && capSelect.value ? capSelect.value : '';
      const data = await fetchJSON('/v1/rooms' + (cap ? '?capacity=' + encodeURIComponent(cap) : ''));
      show('[data-rooms-loading]', false);
      if (!data || !Array.isArray(data.rooms)) {
        show('[data-rooms-error]', true);
        return;
      }
      state.rooms = data.rooms;
      const chips = document.querySelector('[data-room-chips]');
      if (!chips) return;
      chips.replaceChildren();
      state.rooms.forEach((r) => {
        const chip = el('button', 'chip');
        chip.type = 'button';
        chip.dataset.roomId = String(r.id);
        chip.textContent = r.name + ' · ' + r.building + ' · ' + r.capacity + ' ' + plural(r.capacity, 'место', 'места', 'мест');
        chip.addEventListener('click', () => selectRoom(r.id));
        chips.append(chip);
      });
      if (!state.rooms.length) {
        chips.append(el('p', 'muted small', 'Аудиторий с такой вместимостью нет — смягчите фильтр'));
      }
      /* выбранная аудитория отсеялась фильтром → первая из оставшихся */
      if (!state.rooms.some((r) => r.id === state.roomId)) {
        state.roomId = state.rooms.length ? state.rooms[0].id : null;
      }
      chips.hidden = false;
      syncChips();
      const section = document.querySelector('[data-slots-section]');
      if (section) section.hidden = !state.roomId;
      if (state.roomId) loadSlots();
    };

    /* --- карточка подтверждения (03 §4.9.5, шаг 3; черновик rooms.html
       .booking-summary): до выбора — disabled «Выберите слот», с выбором —
       primary «Забронировать HH:MM–HH:MM». Оборудование (features) и
       вместимость — из строки каталога выбранной аудитории ------------- */
    const bookingCard = document.querySelector('[data-booking-card]');
    const bookingConfirm = bookingCard ? bookingCard.querySelector('[data-booking-confirm]') : null;
    const selectedRoom = () => state.rooms.find((r) => r.id === state.roomId) || null;
    const resetSelection = () => {
      state.selected = null;
      document.querySelectorAll('[data-slots-grid] .room-slot').forEach((b) => b.setAttribute('aria-pressed', 'false'));
      const choice = bookingCard ? bookingCard.querySelector('[data-booking-choice]') : null;
      if (choice) choice.textContent = 'Выберите свободное время';
      if (bookingConfirm) {
        bookingConfirm.disabled = true;
        bookingConfirm.textContent = 'Выберите слот';
      }
    };
    const selectSlot = (slot, btn) => {
      if (btn.disabled) return;
      state.selected = slot;
      document.querySelectorAll('[data-slots-grid] .room-slot').forEach((b) => b.setAttribute('aria-pressed', String(b === btn)));
      const room = selectedRoom();
      const set = (sel, text) => { const n = bookingCard ? bookingCard.querySelector(sel) : null; if (n) n.textContent = text; };
      set('[data-booking-room]', 'Ауд. ' + (state.roomName || '—'));
      set('[data-booking-meta]', room
        ? [room.building, room.capacity + ' ' + plural(room.capacity, 'место', 'места', 'мест'), room.features]
            .filter(Boolean).join(' · ')
        : '');
      const when = fmtDay(dateOf(slot.starts_at)) + ' · ' + timeOf(slot.starts_at) + '–' + timeOf(slot.ends_at);
      set('[data-booking-choice]', when);
      if (bookingConfirm) {
        bookingConfirm.disabled = false;
        bookingConfirm.textContent = 'Забронировать ' + timeOf(slot.starts_at) + '–' + timeOf(slot.ends_at);
      }
    };

    /* --- сетка слотов дня: free — выбор слота, booked — «Занято», mine —
       «Ваша бронь» (кнопки ≥44px — .room-grid/.room-slot app.css) ------- */
    const renderSlots = (slots) => {
      const grid = document.querySelector('[data-slots-grid]');
      if (!grid) return;
      resetSelection(); // сетка пересобрана — выбор слота не переживает её
      grid.replaceChildren();
      slots.forEach((s) => {
        const btn = el('button', 'room-slot');
        btn.type = 'button';
        btn.setAttribute('aria-pressed', 'false');
        const label = el('span');
        if (s.state === 'free') {
          label.textContent = 'Свободно';
          btn.addEventListener('click', () => selectSlot(s, btn));
        } else if (s.state === 'mine') {
          btn.classList.add('mine');
          btn.disabled = true;
          label.textContent = 'Ваша бронь';
        } else {
          btn.disabled = true;
          label.textContent = 'Занято';
        }
        btn.append(el('strong', null, timeOf(s.starts_at) + '–' + timeOf(s.ends_at)), label);
        grid.append(btn);
      });
    };
    /* silent — тихая перечитка после POST/DELETE: сетка не мигает
       скелетоном, заменяется по ответу; сбой — блок ошибки секции */
    const loadSlots = async (silent) => {
      if (!state.roomId) return;
      show('[data-slots-error]', false);
      show('[data-slots-warn]', false);
      show('[data-slots-empty]', false);
      if (!silent) {
        show('[data-slots-grid]', false);
        show('[data-slots-loading]', true);
      }
      const date = dateInput && dateInput.value ? dateInput.value : todayUtc();
      if (dateInput && !dateInput.value) dateInput.value = date;
      const data = await fetchJSON('/v1/rooms/' + state.roomId + '/slots?date=' + encodeURIComponent(date));
      show('[data-slots-loading]', false);
      if (!data || !Array.isArray(data.slots)) {
        show('[data-slots-grid]', false);
        show('[data-booking-card]', false); /* выбор слота скрыт вместе с сеткой */
        show('[data-slots-error]', true);
        return;
      }
      const line = document.querySelector('[data-slots-room-line]');
      if (line) line.textContent = 'Ауд. ' + data.room + ' · ' + data.building;
      state.roomName = data.room || '';
      if (!data.slots.length) {
        show('[data-slots-grid]', false);
        show('[data-booking-card]', false);
        show('[data-slots-empty]', true);
        return;
      }
      renderSlots(data.slots);
      show('[data-slots-grid]', true);
      show('[data-booking-card]', true);
    };

    /* --- бронирование (шаг «подтверждение» 03 §4.9.5): POST по кнопке
       карточки; исходы 201 / slot_taken / conflict / blocked / 404 ----- */
    const bookSlot = async () => {
      const slot = state.selected;
      if (!slot || bookingBusy || (bookingConfirm && bookingConfirm.disabled)) return;
      if (!navigator.onLine) { applyOffline(true); return; }
      bookingBusy = true;
      show('[data-slots-warn]', false);
      const was = bookingConfirm ? bookingConfirm.textContent : '';
      if (bookingConfirm) {
        bookingConfirm.textContent = 'Бронируем…';
        bookingConfirm.disabled = true;
      }
      try {
        const resp = await fetch('/v1/rooms/' + state.roomId + '/bookings', {
          method: 'POST',
          headers: { Accept: 'application/json', 'Content-Type': 'application/json' },
          body: JSON.stringify({ starts_at: slot.starts_at, ends_at: slot.ends_at })
        });
        if (resp.status === 201) {
          /* канон success-тоста 03 §4.9.5 («Ауд. 1207, 11:00–12:00 —
             забронировано» со ссылкой «Мои брони»): «Мои брони» — секция
             этой же страницы, действие — якорь к ней (действие тоста —
             --bg на инверсном тосте: --primary на нём не проходит WCAG,
             замер в шапке toast-хелпера) */
          toast('Ауд. ' + (state.roomName || '') + ', ' + timeOf(slot.starts_at) + '–' + timeOf(slot.ends_at) + ' — забронировано',
            { label: 'Мои брони', href: '#bookings-title' });
        } else {
          let code = '';
          let message = t('toast.failBooking', 'Не получилось забронировать');
          try {
            const body = await resp.json();
            if (body && body.error) {
              code = body.error.code || '';
              if (body.error.message) message = body.error.message;
            }
          } catch (e) { /* тело не JSON — остаётся общий текст */ }
          if (code === 'slot_taken') {
            showSlotWarn('Слот только что заняли — обновляем сетку');
          } else if (code === 'not_found') {
            showSlotWarn('Аудитория не найдена — обновляем каталог');
            loadCatalog();
          } else {
            showSlotWarn(message);
          }
        }
      } catch (e) {
        if (!navigator.onLine) applyOffline(true);
        else showSlotWarn(t('toast.failBookingNet', 'Не получилось забронировать. Проверьте интернет — и попробуйте ещё раз'));
      } finally {
        bookingBusy = false;
        /* выбор и подпись кнопки восстановит renderSlots/resetSelection
           после тихой перечитки (или inline-восстановление ниже при сбое) */
        if (bookingConfirm && state.selected) {
          bookingConfirm.textContent = was;
          bookingConfirm.disabled = false;
        }
      }
      /* после любого исхода данные могли устареть — сетка и «Мои брони»
         перечитываются (после 201 слот стал mine и выбор сбрасывается —
         renderSlots вызывает resetSelection; после конфликтов — чужая
         бронь/блокировка); 404 уже перечитал каталог */
      loadSlots(true);
      loadBookings(true);
    };

    /* --- «Мои брони»: строки + отмена через danger-диалог --------------- */
    const renderBookings = (bookings) => {
      const list = document.querySelector('[data-bookings-list]');
      if (!list) return;
      list.replaceChildren();
      bookings.forEach((b) => {
        const row = el('div', 'booking-row');
        const main = el('div', 'booking-main');
        main.append(el('strong', null, 'Ауд. ' + (b.room || '—')));
        main.append(el('span', 'muted small', b.building || ''));
        const when = el('div', 'booking-when', fmtDay(dateOf(b.starts_at)) + ' · ' + timeOf(b.starts_at) + '–' + timeOf(b.ends_at));
        const cancel = el('button', 'button', 'Отменить');
        cancel.type = 'button';
        cancel.addEventListener('click', () => openCancelDialog(b));
        row.append(main, when, cancel);
        list.append(row);
      });
      list.hidden = false;
    };
    const loadBookings = async (silent) => {
      show('[data-bookings-error]', false);
      show('[data-bookings-warn]', false);
      show('[data-bookings-empty]', false);
      if (!silent) {
        show('[data-bookings-list]', false);
        show('[data-bookings-loading]', true);
      }
      const data = await fetchJSON('/v1/room-bookings?upcoming=1');
      show('[data-bookings-loading]', false);
      if (!data || !Array.isArray(data.bookings)) {
        show('[data-bookings-list]', false);
        show('[data-bookings-error]', true);
        return;
      }
      const count = document.querySelector('[data-bookings-count]');
      if (count) {
        count.textContent = data.bookings.length
          ? data.bookings.length + ' ' + plural(data.bookings.length, 'предстоящая', 'предстоящие', 'предстоящих')
          : '';
      }
      if (!data.bookings.length) {
        show('[data-bookings-list]', false);
        show('[data-bookings-empty]', true);
        return;
      }
      renderBookings(data.bookings);
    };

    /* --- danger-диалог отмены (03 §2.8: danger только в диалогах) ------- */
    const dlg = document.getElementById('cancel-booking-dialog');
    const dlgDesc = dlg ? dlg.querySelector('[data-cancel-booking-desc]') : null;
    const dlgConfirm = dlg ? dlg.querySelector('[data-cancel-booking-confirm]') : null;
    const dlgClose = dlg ? dlg.querySelector('[data-cancel-booking-close]') : null;
    let pendingCancel = null;
    const openCancelDialog = (b) => {
      if (!dlg) return;
      pendingCancel = b;
      if (dlgDesc) {
        dlgDesc.textContent = 'Ауд. ' + (b.room || '—') + ' (' + (b.building || '') + ') · '
          + fmtDay(dateOf(b.starts_at)) + ', ' + timeOf(b.starts_at) + '–' + timeOf(b.ends_at);
      }
      dlg.showModal();
    };
    if (dlgClose) dlgClose.addEventListener('click', () => dlg.close());
    if (dlgConfirm) {
      dlgConfirm.addEventListener('click', async () => {
        if (!pendingCancel || dlgConfirm.disabled) return;
        if (!navigator.onLine) { applyOffline(true); return; }
        dlgConfirm.disabled = true;
        dlgConfirm.textContent = 'Отменяем…';
        try {
          const resp = await fetch('/v1/room-bookings/' + pendingCancel.id, {
            method: 'DELETE',
            headers: { Accept: 'application/json' }
          });
          if (resp.status === 204) {
            dlg.close();
            toast('Бронь отменена');
          } else {
            /* 409 повторной отмены/блокировка → message тела (02 §5.1);
               403/404/прочие — тот же inline, без деталей наружу */
            let message = t('toast.failBookingCancel', 'Не получилось отменить бронь');
            try {
              const body = await resp.json();
              if (body && body.error && body.error.message) message = body.error.message;
            } catch (e) { /* тело не JSON */ }
            dlg.close();
            showBookingsWarn(message);
          }
        } catch (e) {
          dlg.close();
          if (!navigator.onLine) applyOffline(true);
          else showBookingsWarn(t('toast.failBookingCancelNet', 'Не получилось отменить бронь. Проверьте интернет — и попробуйте ещё раз'));
        } finally {
          dlgConfirm.disabled = false;
          dlgConfirm.textContent = 'Отменить бронь';
          pendingCancel = null;
        }
        /* строка уходит из списка (после 204) или перечитывается (409 —
           брони могла уже не быть); сетка тихо перечитывается — слот мог
           освободиться/уйти */
        loadBookings(true);
        loadSlots(true);
      });
    }

    /* --- ретраи и переключатели ----------------------------------------- */
    if (bookingConfirm) bookingConfirm.addEventListener('click', () => bookSlot());
    const roomsRetry = document.querySelector('[data-rooms-retry]');
    if (roomsRetry) roomsRetry.addEventListener('click', () => loadCatalog());
    const slotsRetry = document.querySelector('[data-slots-retry]');
    if (slotsRetry) slotsRetry.addEventListener('click', () => loadSlots());
    const bookingsRetry = document.querySelector('[data-bookings-retry]');
    if (bookingsRetry) bookingsRetry.addEventListener('click', () => loadBookings());
    if (capSelect) capSelect.addEventListener('change', () => loadCatalog());
    if (dateInput) {
      if (!dateInput.value) dateInput.value = todayUtc(); /* дефолт — сегодня UTC проекта */
      dateInput.addEventListener('change', () => loadSlots());
    }

    loadCatalog();
    loadBookings();
  }

  /* ---------- экран «Библиотека» (views/library.csp; 03 §4.9.6/5.12) ----- */
  /* Наличие литературы: два fetch из ТЕХ ЖЕ JSON-роутов, что за API
     (02 §5.2): филиалы GET /v1/library/branches (чипы «Все филиалы» + по
     одному, aria-pressed) и каталог GET /v1/library/items — query q=
     (debounce ~300 мс, как на «Людях»), branch_id=, available=1 (чип-
     тумблер «Только доступные»); гонку устаревших ответов гасит
     монотонный seq. Строка книги — .book-row черновика library.html:
     обложка-иконка, название, «автор · тип издания» (kind — словарь DDL
     book|journal|textbook), бейдж «свободно N из M» (available>0 —
     .badge.info, 0 — нейтральный: «свободно 0 из 4» честно виден) и
     подпись «филиал · зал» (hall nullable — пустой зал не рисуется;
     author nullable — строка автора без имени не рисуется).
     Пустой результат — «Ничего не нашлось» (дословно 03 §5.2/черновик).
     Состояния 03 §5: скелетон только при ответе > 300 мс, ошибка с
     «Повторить», офлайн — баннер каркаса. Вставка — только
     createElement/textContent (02 §6.4). */
  if (document.body.dataset.page === 'library') {
    const show = (sel, on) => { const n = document.querySelector(sel); if (n) n.hidden = !on; };
    const el = (tag, className, text) => {
      const node = document.createElement(tag);
      if (className) node.className = className;
      if (text != null) node.textContent = text;
      return node;
    };
    const BOOK_ICON = '<svg class="icon" viewBox="0 0 24 24" aria-hidden="true"><path d="M4 3h7v18H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2Zm7 0h9a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2h-9"/></svg>';
    const KIND_LABEL = { book: 'книга', journal: 'журнал', textbook: 'учебное пособие' };
    const input = document.querySelector('[data-library-q]');
    const availChip = document.querySelector('[data-library-available]');
    const branchesBox = document.querySelector('[data-library-branches]');
    const state = { q: '', branchId: '', available: false, seq: 0 };
    let skTimer = 0; /* таймер показа скелетона (03 §5.1: ответ > 300 мс) */
    let debounce = 0;

    const rowFor = (it) => {
      const row = el('article', 'book-row');
      const cover = el('div', 'book-cover');
      cover.innerHTML = BOOK_ICON; /* статическая svg-константа — 02 §6.4 */
      row.append(cover);
      const mid = el('div');
      mid.append(el('h2', null, it.title || ''));
      const by = [it.author || '', KIND_LABEL[it.kind] || it.kind || ''].filter(Boolean).join(' · ');
      if (by) mid.append(el('p', null, by));
      const have = Number(it.available) || 0;
      const total = Number(it.total) || 0;
      const badge = el('span', 'badge' + (have > 0 ? ' info' : ''), 'Свободно ' + have + ' из ' + total);
      mid.append(badge);
      mid.append(el('p', null, [it.branch || '', it.hall || ''].filter(Boolean).join(' · ')));
      row.append(mid);
      return row;
    };

    const loadItems = async () => {
      const seq = ++state.seq;
      clearTimeout(skTimer);
      show('[data-library-loading]', false);
      show('[data-library-empty]', false);
      show('[data-library-error]', false);
      show('[data-library-results]', false);
      const params = new URLSearchParams();
      const q = state.q.trim();
      if (q) params.set('q', q);
      if (state.branchId) params.set('branch_id', state.branchId);
      if (state.available) params.set('available', '1');
      skTimer = setTimeout(() => show('[data-library-loading]', true), 300);
      const qs = params.toString();
      const data = await fetchJSON('/v1/library/items' + (qs ? '?' + qs : ''));
      if (seq !== state.seq) return;
      clearTimeout(skTimer);
      show('[data-library-loading]', false);
      if (!data || !Array.isArray(data.items)) {
        show('[data-library-error]', true);
        return;
      }
      if (!data.items.length) {
        show('[data-library-empty]', true);
        return;
      }
      const list = document.querySelector('[data-library-list]');
      if (!list) return;
      list.replaceChildren();
      data.items.forEach((it) => list.append(rowFor(it)));
      show('[data-library-results]', true);
    };

    /* филиалы: «Все филиалы» + по одному (GET /v1/library/branches) */
    const loadBranches = async () => {
      const data = await fetchJSON('/v1/library/branches');
      if (!data || !Array.isArray(data.branches) || !branchesBox) return;
      data.branches.forEach((b) => {
        const chip = el('button', 'chip');
        chip.type = 'button';
        chip.dataset.branchId = String(b.id);
        chip.textContent = b.name || '';
        chip.setAttribute('aria-pressed', String(String(b.id) === state.branchId));
        chip.addEventListener('click', () => {
          state.branchId = String(b.id);
          syncBranchChips();
          clearTimeout(debounce);
          loadItems();
        });
        branchesBox.append(chip);
      });
    };
    const syncBranchChips = () => {
      if (!branchesBox) return;
      branchesBox.querySelectorAll('.chip').forEach((chip) => {
        chip.setAttribute('aria-pressed', String((chip.dataset.branchId || '') === state.branchId));
      });
    };
    if (branchesBox) {
      branchesBox.querySelector('.chip').addEventListener('click', () => {
        state.branchId = '';
        syncBranchChips();
        clearTimeout(debounce);
        loadItems();
      });
    }

    if (input) {
      input.addEventListener('input', () => {
        state.q = input.value;
        clearTimeout(debounce);
        debounce = setTimeout(loadItems, 300);
      });
    }
    if (availChip) {
      availChip.addEventListener('click', () => {
        state.available = availChip.getAttribute('aria-pressed') !== 'true';
        availChip.setAttribute('aria-pressed', String(state.available));
        clearTimeout(debounce);
        loadItems();
      });
    }
    const libraryRetry = document.querySelector('[data-library-retry]');
    if (libraryRetry) libraryRetry.addEventListener('click', () => loadItems());

    loadBranches();
    loadItems();
  }

  /* ---------- экран «Столовые» (views/canteens.csp; 03 §4.9.6/5.13) ------ */
  /* Меню столовых: два fetch из ТЕХ ЖЕ JSON-роутов, что за API (02 §5.2):
     список GET /v1/canteens (чипы «название · корпус · часы») и меню
     выбранной GET /v1/canteens/{id}/menu (гонку ответов гасит seq).
     Комплексные (is_combo) — блоки .combo черновика cafeteria.html:
     цена справа, состав и КБЖУ раскрываются details/summary (03 §4.9.6
     «у комбо — раскрытие состава и строка КБЖУ caption»). Остальные
     позиции сгруппированы по category (словарь DDL) строками .list-row:
     название, строка КБЖУ caption, цена справа (.price; tnum — общая
     настройка body). КБЖУ nullable: null — строка НЕ рисуется (вода без
     КБЖУ, фикстуры Э6б; «не рисовать пустое» — постановка экрана).
     Пустое меню — «Меню пока не опубликовано» (в 03 §5.2 отдельной строки
     для столовых нет). Состояния 03 §5: скелетон только при ответе
     > 300 мс, ошибка с «Повторить», офлайн — баннер каркаса. Вставка —
     только createElement/textContent (02 §6.4). */
  if (document.body.dataset.page === 'canteens') {
    const show = (sel, on) => { const n = document.querySelector(sel); if (n) n.hidden = !on; };
    const el = (tag, className, text) => {
      const node = document.createElement(tag);
      if (className) node.className = className;
      if (text != null) node.textContent = text;
      return node;
    };
    const CATEGORY = [
      ['soup', 'Первое'],
      ['main', 'Горячее'],
      ['drink', 'Напитки'],
      ['bakery', 'Выпечка'],
    ];
    const chipsBox = document.querySelector('[data-canteen-chips]');
    const state = { canteens: [], id: null, seq: 0 };
    let skTimer = 0; /* таймер показа скелетона (03 §5.1: ответ > 300 мс) */

    /* «620 ккал · Б 28 · Ж 22 · У 65» (формат 03 §4.9.6); null КБЖУ — null */
    const kbjuOf = (item) => {
      if (item.kcal == null) return null;
      const n = (v) => (v == null ? '' : String(Number(v)));
      return item.kcal + ' ккал · Б ' + n(item.protein) + ' · Ж ' + n(item.fat) + ' · У ' + n(item.carbs);
    };

    const comboBlock = (item) => {
      const box = el('div', 'combo section');
      box.append(el('p', 'overline', 'Комбо'));
      const head = el('div', 'row section');
      head.append(el('h2', null, item.name || ''));
      head.append(el('span', 'price', (item.price != null ? item.price : '—') + ' ₽'));
      box.append(head);
      const kbju = kbjuOf(item);
      if (kbju) {
        const det = document.createElement('details');
        const sum = document.createElement('summary');
        sum.textContent = 'Состав и КБЖУ';
        const body = el('div');
        body.append(el('p', 'small', kbju));
        det.append(sum, body);
        box.append(det);
      }
      return box;
    };

    const itemRow = (item) => {
      const row = el('div', 'list-row');
      const mid = el('div');
      mid.append(el('strong', null, item.name || ''));
      const kbju = kbjuOf(item);
      if (kbju) mid.append(el('p', 'caption', kbju));
      row.append(mid);
      row.append(el('span', 'price', (item.price != null ? item.price : '—') + ' ₽'));
      return row;
    };

    const loadMenu = async () => {
      if (!state.id) return;
      const seq = ++state.seq;
      clearTimeout(skTimer);
      show('[data-menu-loading]', false);
      show('[data-menu-empty]', false);
      show('[data-menu-error]', false);
      show('[data-menu-body]', false);
      skTimer = setTimeout(() => show('[data-menu-loading]', true), 300);
      const data = await fetchJSON('/v1/canteens/' + state.id + '/menu');
      if (seq !== state.seq) return;
      clearTimeout(skTimer);
      show('[data-menu-loading]', false);
      if (!data || !Array.isArray(data.menu)) {
        show('[data-menu-error]', true);
        return;
      }
      const hours = document.querySelector('[data-canteen-hours]');
      if (hours) {
        hours.textContent = 'Часы работы: ' + (data.hours || '—');
        hours.hidden = false;
      }
      if (!data.menu.length) {
        show('[data-menu-empty]', true);
        return;
      }
      const body = document.querySelector('[data-menu-body]');
      if (!body) return;
      body.replaceChildren();
      data.menu.filter((m) => m.is_combo).forEach((m) => body.append(comboBlock(m)));
      CATEGORY.forEach(([key, label]) => {
        const items = data.menu.filter((m) => !m.is_combo && m.category === key);
        if (!items.length) return;
        const section = el('section', 'section');
        const title = el('div', 'section-title');
        title.append(el('h2', null, label));
        section.append(title);
        items.forEach((m) => section.append(itemRow(m)));
        body.append(section);
      });
      body.hidden = false;
    };

    const loadCanteens = async () => {
      const data = await fetchJSON('/v1/canteens');
      if (!data || !Array.isArray(data.canteens) || !chipsBox) return;
      state.canteens = data.canteens;
      chipsBox.replaceChildren();
      state.canteens.forEach((c) => {
        const chip = el('button', 'chip');
        chip.type = 'button';
        chip.dataset.canteenId = String(c.id);
        chip.textContent = [c.name, c.building, c.hours].filter(Boolean).join(' · ');
        chip.addEventListener('click', () => {
          if (state.id === c.id) return;
          state.id = c.id;
          syncChips();
          loadMenu();
        });
        chipsBox.append(chip);
      });
      if (!state.canteens.length) return;
      state.id = state.canteens[0].id;
      syncChips();
      loadMenu();
    };
    const syncChips = () => {
      chipsBox.querySelectorAll('.chip').forEach((chip) => {
        chip.setAttribute('aria-pressed', String(Number(chip.dataset.canteenId) === state.id));
      });
    };

    const menuRetry = document.querySelector('[data-menu-retry]');
    if (menuRetry) menuRetry.addEventListener('click', () => loadMenu());

    loadCanteens();
  }

  /* ---------- экран «Карта кампуса» (views/map.csp; дизайн v3, Р-32) ---------- */
  /* Интерактивная SVG-схема кампуса из прод-данных вместо 3D Three.js/OSM эталона:
     GET /v1/map/buildings {id,name,address} + GET /v1/map/rooms {id,name,building_id,
     building,floor,kind} (без query — весь каталог одним запросом; поиск — тот же
     роут с q=). Параметра floor у API нет: этажи — distinct floor кабинетов корпуса.
     Корпус — изометрический блок (крыша + две стены), высота ∝ max(floor) его
     кабинетов (корпус без кабинетов — 1 этаж, «без аудиторий»); раскладка
     детерминированная: сортировка по id в сетку ceil(√n). Выбранный корпус
     раскрывается стопкой этажей, активный этаж — плита --primary. Зум/пан — viewBox
     (кнопки, клавиатура на сцене, перетаскивание pointer events). Нижний блок —
     корпус → этаж → план этажа (авто-раскладка по двум сторонам коридора в порядке
     name) → карточка помещения. Deep link ?q= / ?room= / ?building=. Вставка — только
     createElement(NS)/textContent (02 §6.4). Строки — словарь map.* (5.18). */
  if (document.body.dataset.page === 'map') {
    const SVGNS = 'http://www.w3.org/2000/svg';
    const $m = (sel) => document.querySelector(sel);
    const show = (sel, on) => { const n = $m(sel); if (n) n.hidden = !on; };
    const el = (tag, className, text) => {
      const node = document.createElement(tag);
      if (className) node.className = className;
      if (text != null) node.textContent = text;
      return node;
    };
    const svgEl = (tag, attrs) => {
      const node = document.createElementNS(SVGNS, tag);
      Object.keys(attrs || {}).forEach((k) => node.setAttribute(k, attrs[k]));
      return node;
    };
    const fmt = (key, fallback, n) => t(key, fallback).replace('{n}', String(n));
    const KIND_KEYS = { 'аудитория': ['map.kindRoom', 'аудитория'], 'лаборатория': ['map.kindLab', 'лаборатория'], 'деканат': ['map.kindDean', 'деканат'] };
    const kindLabel = (kind) => (kind && KIND_KEYS[kind] ? t(KIND_KEYS[kind][0], KIND_KEYS[kind][1]) : (kind || ''));
    const BOOKABLE = { 'аудитория': true, 'лаборатория': true };
    const floorLabel = (f) => (f == null ? t('map.noFloor', 'Без этажа') : fmt('map.floorN', 'этаж {n}', f));
    const byName = (a, b) => String(a.name || '').localeCompare(String(b.name || ''), 'ru', { numeric: true });
    const reduceMotion = () => document.documentElement.classList.contains('para-reduce-motion') ||
      (window.matchMedia && window.matchMedia('(prefers-reduced-motion: reduce)').matches);

    const scene = $m('[data-campus-scene]');
    const statusEl = $m('[data-campus-status]');
    const meta = $m('[data-campus-selection]');
    const picker = $m('[data-campus-picker]');
    const bSelect = $m('[data-map-building]');
    const fSelect = $m('[data-map-floor]');
    const input = $m('[data-map-q]');
    const topBtn = $m('[data-campus-action="top"]');
    const say = (text) => { if (statusEl) statusEl.textContent = text; };

    const state = {
      buildings: [], rooms: [], info: new Map(), loaded: false,
      buildingId: null, floor: undefined, roomId: null, foundId: null,
      mode: 'iso', k: 1, cx: 0, cy: 0, base: null, searchSeq: 0, loadSeq: 0,
    };

    /* --- геометрия схемы --------------------------------------------------- */
    const FOOT_W = 60, FOOT_D = 42, STEP = 120, FLOOR_H = 12, GAP = 6;
    const C30 = Math.cos(Math.PI / 6), S30 = 0.5;
    const proj = (x, y, z) => (state.mode === 'top' ? [x, y] : [(x - y) * C30, (x + y) * S30 - z]);
    const pts = (list) => list.map((p) => { const q = proj(p[0], p[1], p[2]); return q[0].toFixed(1) + ',' + q[1].toFixed(1); }).join(' ');
    const layout = () => {
      const cols = Math.max(1, Math.ceil(Math.sqrt(state.buildings.length)));
      state.buildings.forEach((b, i) => {
        const inf = state.info.get(b.id);
        inf.x = (i % cols) * STEP;
        inf.y = Math.floor(i / cols) * STEP;
      });
    };
    const stackHeight = (inf) => inf.levels * FLOOR_H + (inf.levels - 1) * GAP;
    /* блок: крыша + две видимые стены (грани x+w и y+d при взгляде с +x+y) */
    const block = (g, x, y, z0, h, cls) => {
      const z1 = z0 + h, w = FOOT_W, d = FOOT_D;
      if (state.mode !== 'top') {
        g.append(svgEl('polygon', { class: 'campus-wall-l ' + cls, points: pts([[x, y + d, z0], [x + w, y + d, z0], [x + w, y + d, z1], [x, y + d, z1]]) }));
        g.append(svgEl('polygon', { class: 'campus-wall-r ' + cls, points: pts([[x + w, y, z0], [x + w, y + d, z0], [x + w, y + d, z1], [x + w, y, z1]]) }));
      }
      g.append(svgEl('polygon', { class: 'campus-roof ' + cls, points: pts([[x, y, z1], [x + w, y, z1], [x + w, y + d, z1], [x, y + d, z1]]) }));
    };
    const computeBase = () => {
      let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity;
      state.buildings.forEach((b) => {
        const inf = state.info.get(b.id);
        const top = stackHeight(inf);
        [[0, 0], [FOOT_W, 0], [0, FOOT_D], [FOOT_W, FOOT_D]].forEach((c) => {
          [0, top].forEach((z) => {
            const q = proj(inf.x + c[0], inf.y + c[1], z);
            minX = Math.min(minX, q[0]); maxX = Math.max(maxX, q[0]);
            minY = Math.min(minY, q[1]); maxY = Math.max(maxY, q[1]);
          });
        });
      });
      const pad = 36;
      state.base = { x: minX - pad, y: minY - pad - 30, w: maxX - minX + pad * 2, h: maxY - minY + pad * 2 + 30 };
    };
    const resetView = () => {
      computeBase();
      state.k = 1;
      state.cx = state.base.x + state.base.w / 2;
      state.cy = state.base.y + state.base.h / 2;
    };
    const clampView = () => {
      state.k = Math.min(3, Math.max(0.6, state.k));
      const b = state.base;
      state.cx = Math.min(b.x + b.w, Math.max(b.x, state.cx));
      state.cy = Math.min(b.y + b.h, Math.max(b.y, state.cy));
    };
    const viewBox = () => {
      const w = state.base.w / state.k, h = state.base.h / state.k;
      return { x: state.cx - w / 2, y: state.cy - h / 2, w, h };
    };
    /* масштаб «единица схемы → px» при preserveAspectRatio xMidYMid meet */
    const screenMap = () => {
      const vb = viewBox();
      const cw = scene.clientWidth || 1, ch = scene.clientHeight || 1;
      const s = Math.min(cw / vb.w, ch / vb.h);
      return { vb, s, ox: (cw - vb.w * s) / 2, oy: (ch - vb.h * s) / 2, cw, ch };
    };

    /* --- отрисовка сцены ---------------------------------------------------- */
    let svg = null, labels = null;
    const applyViewBox = () => {
      if (!svg) return;
      const vb = viewBox();
      svg.setAttribute('viewBox', [vb.x, vb.y, vb.w, vb.h].map((v) => v.toFixed(1)).join(' '));
      placeLabels();
    };
    const placeLabels = () => {
      if (!labels) return;
      const m = screenMap();
      const placed = [];
      const nodes = Array.from(labels.children);
      /* приоритет — выбранный корпус, затем по id; перекрывающиеся подписи скрываются */
      nodes.sort((a, b) => (Number(b.dataset.selected) - Number(a.dataset.selected)) || (Number(a.dataset.buildingId) - Number(b.dataset.buildingId)));
      nodes.forEach((node) => {
        const px = m.ox + (Number(node.dataset.ax) - m.vb.x) * m.s;
        const py = m.oy + (Number(node.dataset.ay) - m.vb.y) * m.s;
        node.style.left = px.toFixed(1) + 'px';
        node.style.top = py.toFixed(1) + 'px';
        node.hidden = false;
        const w = node.offsetWidth, h = node.offsetHeight;
        const r = { l: px - w / 2, r: px + w / 2, t: py - h, b: py };
        const outside = r.r < 0 || r.l > m.cw || r.b < 0 || r.t > m.ch;
        const hit = placed.some((p) => r.l < p.r + 4 && r.r > p.l - 4 && r.t < p.b + 2 && r.b > p.t - 2);
        if (outside || hit) { node.hidden = true; return; }
        placed.push(r);
      });
    };
    const renderScene = () => {
      if (!scene || !state.buildings.length) return;
      const focusedId = document.activeElement && document.activeElement.dataset ? document.activeElement.dataset.buildingId : null;
      if (!svg) {
        svg = svgEl('svg', { class: 'campus-svg', preserveAspectRatio: 'xMidYMid meet', focusable: 'false' });
        labels = el('div', 'campus-labels');
        scene.append(svg, labels);
      }
      svg.replaceChildren();
      labels.replaceChildren();
      svg.setAttribute('data-mode', state.mode);
      /* участок кампуса — подложка под сеткой корпусов */
      const cols = Math.max(1, Math.ceil(Math.sqrt(state.buildings.length)));
      const rows = Math.ceil(state.buildings.length / cols);
      const m = 24;
      svg.append(svgEl('polygon', {
        class: 'campus-ground', 'aria-hidden': 'true',
        points: pts([[-m, -m, 0], [(cols - 1) * STEP + FOOT_W + m, -m, 0], [(cols - 1) * STEP + FOOT_W + m, (rows - 1) * STEP + FOOT_D + m, 0], [-m, (rows - 1) * STEP + FOOT_D + m, 0]]),
      }));
      /* художник: сначала дальние корпуса (меньше x+y) */
      const order = state.buildings.slice().sort((a, b) => {
        const ia = state.info.get(a.id), ib = state.info.get(b.id);
        return (ia.x + ia.y) - (ib.x + ib.y) || a.id - b.id;
      });
      order.forEach((b) => {
        const inf = state.info.get(b.id);
        const selected = b.id === state.buildingId;
        const g = svgEl('g', {
          class: 'campus-block' + (selected ? ' is-selected' : '') + (inf.noRooms ? ' is-empty' : ''),
          role: 'button', tabindex: '0', 'data-building-id': String(b.id),
          'aria-pressed': String(selected),
          'aria-label': (b.name || '') + '. ' + (inf.noRooms ? t('map.noRooms', 'без аудиторий') : fmt('map.floorsCount', 'этажей с аудиториями: {n}', inf.levels)),
        });
        let topZ;
        if (selected && state.mode !== 'top' && !inf.noRooms) {
          /* стопка этажей выбранного корпуса; активный этаж — плита --primary */
          for (let f = 1; f <= inf.levels; f += 1) {
            const z0 = (f - 1) * (FLOOR_H + GAP);
            const slab = svgEl('g', { class: 'campus-slab' + (f === state.floor ? ' is-active' : ''), 'data-floor': String(f) });
            block(slab, inf.x, inf.y, z0, FLOOR_H, '');
            g.append(slab);
          }
          topZ = stackHeight(inf);
        } else {
          topZ = inf.levels * FLOOR_H;
          block(g, inf.x, inf.y, 0, topZ, '');
        }
        svg.append(g);
        const anchor = proj(inf.x + FOOT_W / 2, inf.y + FOOT_D / 2, topZ);
        const label = el('span', 'campus-building-label' + (selected ? ' is-selected' : ''), b.name || '');
        label.setAttribute('aria-hidden', 'true');
        label.dataset.buildingId = String(b.id);
        label.dataset.selected = selected ? '1' : '0';
        label.dataset.ax = String(anchor[0]);
        label.dataset.ay = String(anchor[1] - 6);
        labels.append(label);
      });
      applyViewBox();
      if (focusedId) {
        const again = svg.querySelector('[data-building-id="' + focusedId + '"]');
        if (again) again.focus({ preventScroll: true });
      }
    };

    /* --- выбор корпуса / этажа / кабинета ------------------------------------ */
    const roomsOf = (id) => state.rooms.filter((r) => r.building_id === id);
    const floorsOf = (id) => {
      const out = [];
      roomsOf(id).forEach((r) => { if (!out.includes(r.floor)) out.push(r.floor); });
      return out.sort((a, b) => (a == null ? 1 : b == null ? -1 : a - b));
    };
    const building = (id) => state.buildings.find((b) => b.id === id) || null;
    const renderMeta = () => {
      if (!meta) return;
      meta.replaceChildren();
      const b = building(state.buildingId);
      if (!b) {
        meta.append(el('strong', null, t('map.sceneLabel', 'Схема кампуса')), el('span', null, t('map.metaHint', 'Нажмите на здание, чтобы рассмотреть его ближе.')));
        return;
      }
      const inf = state.info.get(b.id);
      meta.append(el('strong', null, b.name || ''));
      if (b.address) meta.append(el('span', null, b.address));
      meta.append(el('span', null, inf.noRooms ? t('map.noRooms', 'без аудиторий') : fmt('map.floorsCount', 'этажей с аудиториями: {n}', inf.levels)));
    };
    const renderFloorSelect = () => {
      if (!fSelect) return;
      fSelect.replaceChildren();
      const floors = state.buildingId == null ? [] : floorsOf(state.buildingId);
      floors.forEach((f) => {
        const opt = el('option', null, f == null ? t('map.noFloor', 'Без этажа') : String(f));
        opt.value = f == null ? '' : String(f);
        fSelect.append(opt);
      });
      fSelect.disabled = !floors.length;
      if (floors.length) fSelect.value = state.floor == null ? '' : String(state.floor);
    };
    const showCard = (room) => {
      const title = $m('[data-room-title]'), body = $m('[data-room-body]'), actions = $m('[data-room-actions]');
      if (!room) {
        if (title) title.textContent = t('map.roomPick', 'Выберите кабинет на плане');
        if (body) body.textContent = t('map.roomPickHint', 'Здесь появятся корпус, этаж и тип помещения.');
        if (actions) actions.hidden = true;
        return;
      }
      if (title) title.textContent = room.name || '';
      if (body) body.textContent = [room.building || '', room.floor != null ? floorLabel(room.floor) : '', kindLabel(room.kind)].filter(Boolean).join(' · ');
      if (actions) actions.hidden = !BOOKABLE[room.kind];
    };
    const renderPlan = () => {
      const grid = $m('[data-rooms-grid]');
      const loc = $m('[data-map-location]');
      const empty = $m('[data-rooms-empty]');
      if (!grid) return;
      grid.replaceChildren();
      const b = building(state.buildingId);
      show('[data-rooms-hint]', !b);
      if (!b) {
        if (loc) loc.textContent = t('map.chooseBuilding', 'Выберите корпус');
        grid.hidden = true;
        if (empty) empty.hidden = true;
        return;
      }
      const hasFloors = floorsOf(b.id).length > 0;
      if (loc) loc.textContent = (b.name || '') + (hasFloors ? ' · ' + floorLabel(state.floor) : '');
      const onFloor = roomsOf(b.id).filter((r) => r.floor === state.floor).sort(byName);
      if (!onFloor.length) {
        grid.hidden = true;
        if (empty) {
          empty.textContent = hasFloors ? t('map.floorEmpty', 'На этом этаже кабинеты ещё не добавлены. Выберите другой этаж.') : t('map.buildingEmpty', 'В каталоге нет аудиторий этого корпуса.');
          empty.hidden = false;
        }
        return;
      }
      if (empty) empty.hidden = true;
      /* авто-раскладка: первая половина по name — одна сторона коридора, вторая — другая */
      const half = Math.ceil(onFloor.length / 2);
      const sideA = el('div', 'map-side'), sideB = el('div', 'map-side');
      onFloor.forEach((r, i) => {
        const tile = el('button', 'map-room' + (r.id === state.foundId ? ' is-found' : ''));
        tile.type = 'button';
        tile.dataset.roomId = String(r.id);
        tile.setAttribute('aria-pressed', String(r.id === state.roomId));
        tile.append(el('strong', null, r.name || ''));
        if (r.kind) tile.append(el('span', 'caption', kindLabel(r.kind)));
        (i < half ? sideA : sideB).append(tile);
      });
      grid.append(sideA, el('p', 'corridor', t('map.corridor', 'Коридор · схема этажа')));
      if (sideB.children.length) grid.append(sideB);
      grid.hidden = false;
    };
    const syncAll = () => {
      if (bSelect) bSelect.value = state.buildingId == null ? '' : String(state.buildingId);
      if (picker) picker.value = state.buildingId == null ? '' : String(state.buildingId);
      renderFloorSelect();
      renderMeta();
      renderPlan();
      renderScene();
    };
    const selectBuilding = (id, opts = {}) => {
      const b = building(id);
      state.buildingId = b ? b.id : null;
      const floors = b ? floorsOf(b.id) : [];
      state.floor = opts.floor !== undefined && floors.includes(opts.floor) ? opts.floor : (floors.length ? floors[0] : undefined);
      if (!opts.keepRoom) { state.roomId = null; state.foundId = null; showCard(null); }
      if (opts.focus && b) {
        const inf = state.info.get(b.id);
        const c = proj(inf.x + FOOT_W / 2, inf.y + FOOT_D / 2, stackHeight(inf) / 2);
        state.cx = c[0]; state.cy = c[1]; state.k = 1.8;
        clampView();
      }
      syncAll();
      if (b) say(t('map.statusSelected', 'Выбрано:') + ' ' + (b.name || ''));
    };
    const selectRoom = (room, found, scroll = true) => {
      state.roomId = room.id;
      state.foundId = found ? room.id : null;
      selectBuilding(room.building_id, { floor: room.floor, keepRoom: true });
      showCard(room);
      if (found && scroll) {
        const tile = $m('.map-room[data-room-id="' + room.id + '"]');
        if (tile) tile.scrollIntoView({ block: 'nearest', behavior: reduceMotion() ? 'auto' : 'smooth' });
      }
    };

    /* --- загрузка ----------------------------------------------------------- */
    let skTimer = 0;
    const fillSelect = (sel, keepFirst) => {
      if (!sel) return;
      while (sel.options.length > keepFirst) sel.remove(keepFirst);
      state.buildings.forEach((b) => {
        const opt = el('option', null, b.name || '');
        opt.value = String(b.id);
        sel.append(opt);
      });
    };
    const load = async () => {
      const seq = ++state.loadSeq;
      show('[data-campus-error]', false);
      show('[data-campus-empty]', false);
      clearTimeout(skTimer);
      skTimer = setTimeout(() => show('[data-campus-loading]', true), 300);
      say(t('map.loading', 'Загружаем схему кампуса…'));
      const [bData, rData] = await Promise.all([fetchJSON('/v1/map/buildings'), fetchJSON('/v1/map/rooms')]);
      if (seq !== state.loadSeq) return false;
      clearTimeout(skTimer);
      show('[data-campus-loading]', false);
      if (!bData || !Array.isArray(bData.buildings) || !rData || !Array.isArray(rData.rooms)) {
        show('[data-campus-error]', true);
        say(t('map.errorLoad', 'Не получилось загрузить схему кампуса'));
        return false;
      }
      state.buildings = bData.buildings.slice().sort((a, b) => a.id - b.id);
      state.rooms = rData.rooms;
      state.info = new Map();
      state.buildings.forEach((b) => {
        const max = roomsOf(b.id).reduce((acc, r) => Math.max(acc, Number(r.floor) || 0), 0);
        state.info.set(b.id, { levels: Math.max(1, max), noRooms: !roomsOf(b.id).length });
      });
      state.loaded = true;
      if (!state.buildings.length) {
        show('[data-campus-empty]', true);
        say(t('map.emptyTitle', 'Корпуса ещё не добавлены'));
        return true;
      }
      fillSelect(picker, 1);
      fillSelect(bSelect, 1);
      layout();
      resetView();
      syncAll();
      say(t('map.statusReady', 'Схема готова. Выберите здание или найдите аудиторию.'));
      return true;
    };

    /* --- поиск: GET /v1/map/rooms?q= ----------------------------------------- */
    const searchStatus = $m('[data-search-status]');
    const searchList = $m('[data-search-results]');
    const runSearch = async (rawQ, scroll = true) => {
      const q = String(rawQ || '').trim();
      if (searchList) { searchList.replaceChildren(); searchList.hidden = true; }
      show('[data-search-retry]', false);
      if (!q) { if (searchStatus) searchStatus.textContent = ''; return; }
      const seq = ++state.searchSeq;
      const data = await fetchJSON('/v1/map/rooms?q=' + encodeURIComponent(q));
      if (seq !== state.searchSeq) return;
      if (!data || !Array.isArray(data.rooms)) {
        if (searchStatus) searchStatus.textContent = t('map.searchError', 'Не получилось выполнить поиск.');
        show('[data-search-retry]', true);
        return;
      }
      const rooms = data.rooms.slice().sort(byName);
      const exact = rooms.filter((r) => String(r.name).toLowerCase() === q.toLowerCase());
      const hits = exact.length === 1 ? exact : rooms;
      if (!hits.length) {
        if (searchStatus) searchStatus.textContent = t('map.searchNotFound', 'Не нашли кабинет. Проверьте номер — например, 1207 или Б-204.');
        return;
      }
      const describe = (r) => [r.name, r.building, r.floor != null ? floorLabel(r.floor) : ''].filter(Boolean).join(' · ');
      if (hits.length === 1) {
        if (searchStatus) searchStatus.textContent = t('map.searchFound', 'Нашли:') + ' ' + describe(hits[0]);
        selectRoom(hits[0], true, scroll);
        return;
      }
      if (searchStatus) searchStatus.textContent = t('map.searchMany', 'Нашли несколько кабинетов — выберите нужный:');
      if (searchList) {
        hits.forEach((r) => {
          const chip = el('button', 'chip', describe(r));
          chip.type = 'button';
          chip.dataset.roomId = String(r.id);
          searchList.append(chip);
        });
        searchList.hidden = false;
      }
    };
    const form = $m('[data-map-form]');
    if (form) {
      form.addEventListener('submit', (event) => {
        event.preventDefault();
        const q = input ? input.value.trim() : '';
        window.history.replaceState(null, '', q ? '/map?q=' + encodeURIComponent(q) : '/map');
        runSearch(q);
      });
    }
    const searchRetry = $m('[data-search-retry]');
    if (searchRetry) searchRetry.addEventListener('click', () => runSearch(input ? input.value : ''));
    if (searchList) {
      searchList.addEventListener('click', (event) => {
        const chip = event.target.closest('[data-room-id]');
        if (!chip) return;
        const room = state.rooms.find((r) => String(r.id) === chip.dataset.roomId);
        if (room) {
          searchList.querySelectorAll('.chip').forEach((c) => c.setAttribute('aria-pressed', String(c === chip)));
          selectRoom(room, true);
        }
      });
    }

    /* --- нижний блок: selects и план ------------------------------------------ */
    if (bSelect) bSelect.addEventListener('change', () => selectBuilding(bSelect.value ? Number(bSelect.value) : null));
    if (picker) picker.addEventListener('change', () => {
      if (picker.value) selectBuilding(Number(picker.value), { focus: true });
      else { selectBuilding(null); resetView(); applyViewBox(); say(t('map.statusReset', 'Исходный вид восстановлен.')); }
    });
    if (fSelect) fSelect.addEventListener('change', () => {
      state.floor = fSelect.value === '' ? null : Number(fSelect.value);
      state.roomId = null; state.foundId = null; showCard(null);
      renderPlan();
      renderScene();
    });
    const grid = $m('[data-rooms-grid]');
    if (grid) grid.addEventListener('click', (event) => {
      const tile = event.target.closest('.map-room');
      if (!tile) return;
      const room = state.rooms.find((r) => String(r.id) === tile.dataset.roomId);
      if (!room) return;
      state.roomId = room.id;
      grid.querySelectorAll('.map-room').forEach((n) => n.setAttribute('aria-pressed', String(n === tile)));
      showCard(room);
    });

    /* --- сцена: клик, клавиатура, зум/пан, перетаскивание ---------------------- */
    let drag = null, dragMoved = false;
    const zoom = (f) => { state.k *= f; clampView(); applyViewBox(); say(t('map.statusZoom', 'Масштаб изменён.')); };
    const pan = (dx, dy) => {
      const vb = viewBox();
      state.cx += dx * vb.w * 0.15; state.cy += dy * vb.h * 0.15;
      clampView(); applyViewBox(); say(t('map.statusPan', 'Карта перемещена.'));
    };
    const fullReset = () => {
      state.mode = 'iso';
      if (topBtn) topBtn.setAttribute('aria-pressed', 'false');
      resetView(); renderScene();
      say(t('map.statusReset', 'Исходный вид восстановлен.'));
    };
    const ACTIONS = {
      'zoom-in': () => zoom(1.25), 'zoom-out': () => zoom(0.8),
      'pan-up': () => pan(0, -1), 'pan-down': () => pan(0, 1), 'pan-left': () => pan(-1, 0), 'pan-right': () => pan(1, 0),
      reset: fullReset,
      top: () => {
        state.mode = state.mode === 'top' ? 'iso' : 'top';
        if (topBtn) topBtn.setAttribute('aria-pressed', String(state.mode === 'top'));
        resetView(); renderScene();
        say(state.mode === 'top' ? t('map.statusTop', 'Вид сверху.') : t('map.statusIso', 'Объёмный вид.'));
      },
    };
    document.querySelectorAll('[data-campus-action]').forEach((btn) => {
      btn.addEventListener('click', () => { if (state.base && ACTIONS[btn.dataset.campusAction]) ACTIONS[btn.dataset.campusAction](); });
    });
    const retry = $m('[data-campus-retry]');
    if (retry) retry.addEventListener('click', () => load());
    if (scene) {
      const pickFrom = (target) => {
        const label = target.closest('.campus-building-label');
        const g = target.closest('.campus-block');
        const node = label || g;
        return node ? Number(node.dataset.buildingId) : null;
      };
      scene.addEventListener('click', (event) => {
        if (dragMoved) { dragMoved = false; return; }
        const id = pickFrom(event.target);
        if (id == null) return;
        const slab = event.target.closest('.campus-slab');
        if (slab && id === state.buildingId) {
          state.floor = Number(slab.dataset.floor);
          state.roomId = null; state.foundId = null; showCard(null);
          syncAll();
          return;
        }
        if (id !== state.buildingId) selectBuilding(id);
      });
      scene.addEventListener('keydown', (event) => {
        if (!state.base) return;
        const g = event.target.closest && event.target.closest('.campus-block');
        if (g && (event.key === 'Enter' || event.key === ' ')) {
          event.preventDefault();
          selectBuilding(Number(g.dataset.buildingId));
          return;
        }
        const step = event.shiftKey ? 2 : 1;
        const keys = {
          ArrowUp: () => pan(0, -step), ArrowDown: () => pan(0, step), ArrowLeft: () => pan(-step, 0), ArrowRight: () => pan(step, 0),
          '+': () => zoom(1.25), '=': () => zoom(1.25), '-': () => zoom(0.8), '_': () => zoom(0.8),
        };
        if (keys[event.key]) { event.preventDefault(); keys[event.key](); return; }
        if (event.code === 'KeyR' && !event.ctrlKey && !event.metaKey && !event.altKey) { event.preventDefault(); fullReset(); }
      });
      scene.addEventListener('pointerdown', (event) => {
        if (!state.base || (event.pointerType === 'mouse' && event.button !== 0)) return;
        drag = { id: event.pointerId, x: event.clientX, y: event.clientY, cx: state.cx, cy: state.cy, captured: false };
        dragMoved = false;
      });
      scene.addEventListener('pointermove', (event) => {
        if (!drag || event.pointerId !== drag.id) return;
        const dx = event.clientX - drag.x, dy = event.clientY - drag.y;
        if (!drag.captured) {
          if (Math.hypot(dx, dy) < 6) return;
          drag.captured = true; dragMoved = true;
          scene.setPointerCapture(event.pointerId);
          scene.classList.add('is-dragging');
        }
        const m = screenMap();
        state.cx = drag.cx - dx / m.s; state.cy = drag.cy - dy / m.s;
        clampView(); applyViewBox();
      });
      const endDrag = (event) => {
        if (!drag || event.pointerId !== drag.id) return;
        if (drag.captured) say(t('map.statusPan', 'Карта перемещена.'));
        drag = null;
        scene.classList.remove('is-dragging');
      };
      scene.addEventListener('pointerup', endDrag);
      scene.addEventListener('pointercancel', endDrag);
      if (window.ResizeObserver) new ResizeObserver(() => placeLabels()).observe(scene);
    }

    /* --- старт и deep link ?q= / ?room= / ?building= -------------------------- */
    const params = new URLSearchParams(window.location.search);
    const initialQ = params.get('q') || params.get('room');
    const initialB = Number(params.get('building'));
    if (initialQ && input) input.value = initialQ;
    load().then((ok) => {
      if (!ok) return;
      /* deep link: без прокрутки — корпус и плита этажа видны на схеме сразу */
      if (initialQ) runSearch(initialQ, false);
      else if (initialB && building(initialB)) selectBuilding(initialB, { focus: true });
    });
  }

  /* ---------- экран «Уведомления» (views/notifications.csp; 03 §4.6, §5) -- */
  /* Плоская лента GET /v1/notifications (02 §5.2): query type= (словарь
     CHECK 0004 sql/migrations/0004:10 — schedule/deadline/application/
     announcement; чипы «Все | Расписание | Дедлайны | Заявки | Объявления»
     дословно вайрфрейма 03 §4.6:657 — «Система» из первого раунда снята
     по замечанию ревью: «system» в словаре нет, сервис отвечает
     400 bad_request), unread=1 (чип-тумблер «Непрочитанные»), cursor= —
     составная пагинация: «Ещё» догружает следующую страницу по
     next_cursor и исчезает при null (постановка экрана). Тап по строке
     с read_at IS NULL → POST /{id}/read В ФОНЕ: переход по deep_link
     его НЕ ждёт (постановка); fetch с keepalive — запрос переживает
     уход со страницы. Не-2xx (403 чужое — guard ДО пометки 02 §5.2,
     404, 5xx) — просто не помечаем: точка остаётся, бейдж не трогаем.
     deep_link — только относительный путь (первый символ «/»;
     javascript:-URI и внешние схемы отсекаются при рендере, 02 §6.4),
     ПЕРЕД рендером нормализуется deepLinkTarget(): пути страниц,
     которых в приложении нет, маппятся на живые экраны — таблица
     маппинга в комментарии рядом с DEEP_LINK_MAP ниже;
     строки без deep_link — <button>, только пометка на месте.
     «Прочитать всё» → POST read-all → точки сняты, тост (дословно
     03 §4.6) и бейдж обнулён ПЕРЕИСПОЛЬЗОВАННОЙ каркасной
     refreshUnreadBadge() (та же функция, что у каркаса/шапки, «если её
     нет — добавь одну общую» — она есть, дубль не заводим); при
     активном чипе «Непрочитанные» список перечитывается с сервера
     (unread=1 теперь пуст → «По этой теме пока ничего не было»,
     03 §5.2 — замечание ревью раунда 1: локальные прочитанные строки
     не должны оставаться видимыми под фильтром «только непрочитанные»).
     POST-запросы (204 без тела) — сырым fetch: fetchJSON разбирает
     resp.json() и на пустом 204 падает в ветку catch → ложный офлайн
     каркаса (паттерн POST/DELETE брони rooms-секции). Вставка —
     createElement/textContent; данные сервера в innerHTML НИКОГДА —
     createContextualFragment только для статических svg-констант иконок
     типов ниже (02 §6.4, как ICON application-form-секции). */
  if (document.body.dataset.page === 'notifications') {
    const show = (sel, on) => { const n = document.querySelector(sel); if (n) n.hidden = !on; };
    const el = (tag, className, text) => {
      const node = document.createElement(tag);
      if (className) node.className = className;
      if (text != null) node.textContent = text;
      return node;
    };
    /* иконки типов — статические константы по мотивам Lucide (03 §4.6:
       перенос — arrow-right-left, дедлайн — clock, заявка — file-text,
       объявление — megaphone; fallback — колокольчик для типа вне
       словаря, которых словарь CHECK 0004 не выпускает); НЕ данные
       сервера */
    const FEED_ICON = {
      schedule: '<svg class="icon" viewBox="0 0 24 24" aria-hidden="true"><path d="m16 3 4 4-4 4M20 7H4m8 14-4-4 4-4M4 17h16"/></svg>',
      deadline: '<svg class="icon" viewBox="0 0 24 24" aria-hidden="true"><circle cx="12" cy="12" r="9"/><path d="M12 7v5l3 2"/></svg>',
      application: '<svg class="icon" viewBox="0 0 24 24" aria-hidden="true"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8ZM14 2v6h6M8 13h8M8 17h5"/></svg>',
      announcement: '<svg class="icon" viewBox="0 0 24 24" aria-hidden="true"><path d="m3 11 18-5v12L3 14v-3zM11.6 16.8a3 3 0 1 1-5.8-1.6"/></svg>',
      fallback: '<svg class="icon" viewBox="0 0 24 24" aria-hidden="true"><path d="M18 8a6 6 0 0 0-12 0c0 7-3 7-3 9h18c0-2-3-2-3-9M10 21h4"/></svg>'
    };
    /* время — UTC проекта (created_at strftime «YYYY-MM-DD HH:MM:SS»):
       сегодня/вчера — словами (03 §4.6), дальше — «D месяца, HH:MM» */
    const stampOf = (iso) => {
      if (typeof iso !== 'string' || iso.length < 16) return '';
      const date = iso.slice(0, 10);
      const time = iso.slice(11, 16);
      const today = new Date().toISOString().slice(0, 10);
      if (date === today) return time;
      if (date === new Date(Date.now() - 86400000).toISOString().slice(0, 10)) return 'Вчера, ' + time;
      const p = date.split('-').map(Number);
      if (p.length !== 3 || p.some(Number.isNaN)) return date;
      const day = new Date(Date.UTC(p[0], p[1] - 1, p[2])).toLocaleDateString('ru-RU', { day: 'numeric', month: 'long', timeZone: 'UTC' });
      return day + ', ' + time;
    };
    /* ISO-метка ВНУТРИ текста с сервера (тело напоминания ReminderService:
       «Срок: 2026-11-08T21:00:00Z» — QA-проход) не должна попадать в UI
       сырой: каждый ISO-фрагмент строки заменяется на «8 ноября, 21:00» —
       тот же формат «D месяца, HH:MM», что у stampOf выше и во всех
       форматированных местах (дашборд shortStamp/dayLabel, staff stampOf,
       таймлайн заявки toLocaleString — grep по app.js: сырых подстановок
       больше нет). created_at/title/body карточек уведомлений идут через
       это при рендере ниже */
    const humanizeDates = (text) => {
      if (typeof text !== 'string') return '';
      return text.replace(/\d{4}-\d{2}-\d{2}(?:[T ]\d{2}:\d{2}(?::\d{2})?Z?)?/g, (m) => {
        const p = m.slice(0, 10).split('-').map(Number);
        if (p.length !== 3 || p.some(Number.isNaN)) return m;
        const day = new Date(Date.UTC(p[0], p[1] - 1, p[2]));
        if (Number.isNaN(day.getTime())) return m;
        const time = m.length >= 16 ? m.slice(11, 16) : '';
        const label = day.toLocaleDateString('ru-RU', { day: 'numeric', month: 'long', timeZone: 'UTC' });
        return time ? label + ', ' + time : label;
      });
    };

    const state = { type: '', unread: false, cursor: null, items: [] };
    const feedUrl = (cursor) => {
      const p = new URLSearchParams();
      if (state.type) p.set('type', state.type);
      if (state.unread) p.set('unread', '1');
      if (cursor) p.set('cursor', String(cursor));
      const q = p.toString();
      return '/v1/notifications' + (q ? '?' + q : '');
    };

    /* ===== deep_link → живая страница (багфикс QA: тап по уведомлению
       «Отчёт по эконометрике» вёл на /deadlines — страницы нет, голая
       системная 404 drogon вне каркаса). Сервер и sql/ клиент не правит
       (дорожки 05 §4.5), поэтому deep_link нормализуется ЗДЕСЬ.
       Все источники deep_link (проверено grep по sql/fixtures.sql:144
       и src/services):
         фикстуры (ids 1–6) и ScheduleChangeService.cpp:187
                                       → /schedule?date=ГГГГ-ММ-ДД
         ApplicationsService.cpp:306   → /applications/{id}
         ReminderService.cpp:57        → /deadlines          (СТРАНИЦЫ НЕТ)
         RoomsService.h:209 (снятие брони блокировкой)
                                       → /room-bookings      (НЕТ)
       Живые страницы (роуты ADD_METHOD_TO в src/controllers/*.h, views/):
         / /login /schedule /grades /applications /profile
         /notifications /rooms /library /canteens /map /people
         /gradebook /rating /staff — с валидными вложенными
         /applications/{id}, /applications/new, /grades/discipline/{id},
         /people/{id}, /schedule/segment (htmx), /staff/app-card (htmx).
       МАППИНГ несуществующих путей на родительский экран объекта:
         /deadlines       → /grades   — дедлайны: виджет «Дедлайны»
                                       дашборда ведёт «Все» на /grades
                                       (views/dashboard.csp:126)
         /exams           → /grades   — сессия/экзамены живут на /grades
         /room-bookings   → /rooms    — брони: каталог и сетка аудиторий
         /news            → /notifications?segment=news — лента новостей
                                       5.17: сегмент «Новости» экрана
                                       уведомлений (страница живая,
                                       ?segment= читает NotificationsFeedCtrl::page)
         /ics, /ics/…     → /profile  — подписка ICS — карточка профиля
                                       (/ics/{token} — отдача календаря,
                                       не экран)
         прочее, чей первый сегмент пути не из живых
                          → /notifications — тап случился на ленте:
                                       возврат на неё безопаснее 404 */
    const DEEP_LINK_MAP = {
      '/deadlines': '/grades',
      '/exams': '/grades',
      '/room-bookings': '/rooms',
      '/news': '/notifications?segment=news'
    };
    const LIVE_FIRST_SEG = new Set([
      '', 'login', 'schedule', 'grades', 'applications', 'profile',
      'notifications', 'rooms', 'library', 'canteens', 'map', 'people',
      'gradebook', 'rating', 'staff'
    ]);
    const deepLinkTarget = (href) => {
      if (typeof href !== 'string' || href.charAt(0) !== '/') return '';
      const path = href.split(/[?#]/)[0] || '/';
      if (path === '/ics' || path.indexOf('/ics/') === 0) return '/profile';
      if (Object.prototype.hasOwnProperty.call(DEEP_LINK_MAP, path)) return DEEP_LINK_MAP[path];
      const seg = path.slice(1).split('/')[0];
      return LIVE_FIRST_SEG.has(seg) ? href : '/notifications';
    };

    /* строка ленты: <a> с deep_link / <button> без него (в <a> вложенный
       <a> невалиден — заголовок текстом + точка .unread-mark каркаса) */
    const renderRow = (n) => {
      const href = deepLinkTarget(n.deep_link);
      const row = href ? el('a', 'notification') : el('button', 'notification');
      if (href) row.href = href;
      else row.type = 'button';
      row.dataset.notifId = String(n.id);
      row.classList.add(n.read_at ? 'is-read' : 'is-unread');
      row.append(document.createRange().createContextualFragment(FEED_ICON[n.type] || FEED_ICON.fallback));
      const mid = el('div');
      const title = el('div', 'ntitle', humanizeDates(n.title) || t('notif.itemOne', 'Уведомление'));
      if (!n.read_at) {
        const dot = el('span', 'unread-mark');
        dot.setAttribute('aria-hidden', 'true');
        title.append(dot);
      }
      mid.append(title);
      if (n.body) mid.append(el('p', 'nbody', humanizeDates(n.body)));
      const when = stampOf(n.created_at);
      if (when) mid.append(el('p', 'nwhen', when));
      row.append(mid);
      return row;
    };
    const renderList = () => {
      const list = document.querySelector('[data-feed-list]');
      if (!list) return;
      list.replaceChildren();
      state.items.forEach((n) => list.append(renderRow(n)));
      list.hidden = !state.items.length;
    };

    /* загрузка страницы ленты: без cursor — первая (скелетон/сброс),
       с cursor — догрузка «Ещё» (append, скелетон не мигает) */
    const load = async (cursor) => {
      show('[data-feed-error]', false);
      show('[data-feed-warn]', false);
      if (!cursor) {
        show('[data-feed-list]', false);
        show('[data-feed-empty]', false);
        show('[data-feed-empty-filter]', false);
        show('[data-feed-more]', false);
        show('[data-feed-loading]', true);
      }
      const data = await fetchJSON(feedUrl(cursor));
      show('[data-feed-loading]', false);
      if (!data || !Array.isArray(data.notifications)) {
        /* офлайн: баннер уже показал каркас (fetchJSON); ошибка API —
           03 §5.3: первая страница — экранная ошибка с «Повторить»,
           догрузка — inline у «Ещё» (повтор — та же кнопка) */
        if (cursor) {
          const warn = document.querySelector('[data-feed-warn]');
          if (warn) warn.textContent = t('toast.failLoadMore', 'Не получилось загрузить ещё — попробуйте ещё раз');
          show('[data-feed-warn]', true);
        } else {
          show('[data-feed-list]', false);
          show('[data-feed-error]', true);
        }
        return;
      }
      state.cursor = data.next_cursor || null;
      state.items = cursor ? state.items.concat(data.notifications) : data.notifications;
      renderList();
      /* «Ещё» видна, пока есть следующая страница; next_cursor=null —
         кнопка исчезает (постановка экрана) */
      show('[data-feed-more]', Boolean(state.cursor));
      if (!state.items.length) {
        /* пусто: без фильтров — полное пустое состояние (03 §5.2),
           с фильтром type/unread — текст пустого фильтра (тоже 03 §5.2) */
        show(state.type || state.unread ? '[data-feed-empty-filter]' : '[data-feed-empty]', true);
      }
    };

    /* ===== 5.17: сегменты «Уведомления | Новости | События» ============
       Экран переключается htmx-фрагментами GET /notifications/segment
       (outerHTML #notifications-segment — паттерн /schedule/segment,
       Э7/Р-18): после обмена initCurrentSegment() перечитывает
       data-segment контейнера и запускает логику активной вкладки.
       Данные — клиент: лента — прежний GET /v1/notifications (Э6, без
       изменений), новости/события — GET /v1/news (NewsCtrl, 5.17).
       Контролы — делегирование на document: контейнер сегмента
       перерисовывается htmx ЦЕЛИКОМ, слушатели на его элементах не
       переживают обмен (тот же приём, что у sheet «Детали пары» экрана
       «Расписание»). */
    const NEWS_CATEGORY = {
      official: 'Официальная', conference: 'Конференция',
      org: 'Организация', event: 'Событие'
    };
    /* «D месяца, HH:MM» из ISO-метки новости (published_at/event_at —
       строки «…Z»); тот же формат, что у stampOf/humanizeDates выше */
    const newsDayLabel = (iso) => {
      if (typeof iso !== 'string' || iso.length < 10) return '';
      const p = iso.slice(0, 10).split('-').map(Number);
      if (p.length !== 3 || p.some(Number.isNaN)) return '';
      const day = new Date(Date.UTC(p[0], p[1] - 1, p[2]));
      if (Number.isNaN(day.getTime())) return '';
      const label = day.toLocaleDateString('ru-RU', { day: 'numeric', month: 'long', timeZone: 'UTC' });
      return iso.length >= 16 ? label + ', ' + iso.slice(11, 16) : label;
    };
    /* строка новости (5.17, 01 §5.17 «На экране»): бейдж категории
       (словарь CHECK 0007), заголовок, дата и место события (когда есть),
       тело; дата публикации — справа от бейджа */
    const renderNewsRow = (n) => {
      const row = el('article', 'news-item');
      const head = el('div', 'news-head');
      head.append(el('span', 'badge', NEWS_CATEGORY[n.category] || 'Новость'));
      const pub = stampOf(n.published_at);
      if (pub) head.append(el('span', 'news-date', pub));
      row.append(head);
      row.append(el('h2', null, humanizeDates(n.title) || 'Новость'));
      const meta = [];
      if (n.event_at) meta.push(newsDayLabel(n.event_at));
      if (n.location) meta.push(n.location);
      if (meta.length) row.append(el('p', 'news-meta', meta.join(' · ')));
      if (n.body) row.append(el('p', 'news-body', humanizeDates(n.body)));
      return row;
    };
    const newsState = { mode: null, cursor: null, items: [] };
    const renderNewsList = () => {
      const list = document.querySelector('[data-news-list]');
      if (!list) return;
      list.replaceChildren();
      newsState.items.forEach((n) => list.append(renderNewsRow(n)));
      list.hidden = !newsState.items.length;
    };
    /* Страница новостей/событий: «События» — те же строки /v1/news с
       event_at (BE-фильтра «только события» в API нет, 02 §5.2 — фильтр
       на клиенте). Страница могла не дать ни одного события: пока
       строк-событий нет и есть next_cursor, догружаем следующую страницу
       сами (кап 10 — лента конечна, детектор next_cursor её закрывает). */
    const loadNews = async (cursor, depth) => {
      show('[data-news-error]', false);
      show('[data-news-warn]', false);
      if (!cursor) {
        show('[data-news-list]', false);
        show('[data-news-empty]', false);
        show('[data-news-more]', false);
        show('[data-news-loading]', true);
      }
      const data = await fetchJSON('/v1/news' + (cursor ? '?cursor=' + encodeURIComponent(cursor) : ''));
      show('[data-news-loading]', false);
      if (!data || !Array.isArray(data.news)) {
        /* офлайн: баннер уже показал каркас (fetchJSON); ошибка API —
           03 §5.3: первая страница — экранная ошибка с «Повторить»,
           догрузка — inline у «Ещё» (повтор — та же кнопка) */
        if (cursor) {
          const warn = document.querySelector('[data-news-warn]');
          if (warn) warn.textContent = t('toast.failLoadMore', 'Не получилось загрузить ещё — попробуйте ещё раз');
          show('[data-news-warn]', true);
        } else {
          show('[data-news-list]', false);
          show('[data-news-error]', true);
        }
        return;
      }
      newsState.cursor = data.next_cursor || null;
      const page = newsState.mode === 'events'
        ? data.news.filter((n) => typeof n.event_at === 'string' && n.event_at)
        : data.news;
      newsState.items = cursor ? newsState.items.concat(page) : page;
      renderNewsList();
      show('[data-news-more]', Boolean(newsState.cursor));
      if (!newsState.items.length) {
        if (newsState.mode === 'events' && newsState.cursor && (depth || 0) < 10) {
          /* страница без событий — искать дальше (лента свежее сверху,
             анонсы могут быть глубже первой страницы) */
          loadNews(newsState.cursor, (depth || 0) + 1);
          return;
        }
        show('[data-news-empty]', true);
      }
    };
    const initCurrentSegment = () => {
      const segBox = document.getElementById('notifications-segment');
      const seg = segBox && segBox.dataset.segment ? segBox.dataset.segment : 'notifications';
      if (seg === 'news' || seg === 'events') {
        newsState.mode = seg;
        newsState.cursor = null;
        newsState.items = [];
        loadNews(null, 0);
        return;
      }
      /* лента Э6: фрагмент приехал с чипами по умолчанию («Все», без
         unread) — state приводится к разметке, первая страница */
      state.type = '';
      state.unread = false;
      state.cursor = null;
      state.items = [];
      load(null);
    };
    initCurrentSegment();
    document.body.addEventListener('htmx:afterSwap', (e) => {
      const target = e.detail && e.detail.target;
      if (target && target.id === 'notifications-segment') initCurrentSegment();
    });

    /* --- тап по строке ленты: пометка В ФОНЕ, deep_link-переход не ждёт её */
    const markReadInBackground = (row) => {
      if (!row.classList.contains('is-unread')) return;
      const id = Number(row.dataset.notifId);
      const item = state.items.find((x) => x.id === id);
      if (!item || item.read_at) return; // повторный тап до ответа/уже прочитано
      if (!navigator.onLine) { applyOffline(true); return; }
      fetch('/v1/notifications/' + id + '/read', {
        method: 'POST',
        headers: { Accept: 'application/json' },
        keepalive: true /* фон: запрос переживает уход по deep_link */
      }).then((resp) => {
        if (!resp.ok) return; /* 403 чужое (guard ДО пометки)/404/5xx — не помечаем */
        item.read_at = new Date().toISOString();
        row.classList.remove('is-unread');
        row.classList.add('is-read');
        const dot = row.querySelector('.unread-mark');
        if (dot) dot.remove();
        /* синхронизация бейджа колокольчика — та же каркасная функция */
        refreshUnreadBadge();
      }).catch(() => { /* сеть ушла — точка остаётся, баннер покажет каркас */ });
      /* <a>-строка: браузер сам уходит по deep_link (preventDefault нет);
         строки без deep_link (<button>) остаются на месте — точка гаснет */
    };

    /* «Прочитать всё»: 204 идемпотентно → точки сняты, бейдж обнулён */
    const readAll = async (readAllBtn) => {
      if (readAllBtn.disabled) return;
      if (!navigator.onLine) { applyOffline(true); return; }
      readAllBtn.disabled = true;
      try {
        const resp = await fetch('/v1/notifications/read-all', {
          method: 'POST',
          headers: { Accept: 'application/json' }
        });
        if (resp.status === 204) {
          if (state.unread) {
            /* замечание ревью раунда 1: под фильтром «только
               непрочитанные» после read-all локально показанные
               прочитанные строки НЕ оставляем — сервер на этот фильтр
               теперь отвечает пусто («По этой теме пока ничего не
               было», 03 §5.2); перечитываем первую страницу */
            load(null);
          } else {
            const now = new Date().toISOString();
            state.items.forEach((n) => { if (!n.read_at) n.read_at = now; });
            renderList();
          }
          refreshUnreadBadge();
          toast('Все уведомления прочитаны'); /* дословно 03 §4.6 */
        } else {
          let message = t('toast.failReadAll', 'Не получилось пометить всё прочитанным');
          try {
            const body = await resp.json();
            if (body && body.error && body.error.message) message = body.error.message;
          } catch (e) { /* тело не JSON */ }
          toast(message);
        }
      } catch (e) {
        if (!navigator.onLine) applyOffline(true);
        else toast(t('toast.failReadAllNet', 'Не получилось пометить всё прочитанным. Проверьте интернет — и попробуем ещё раз'));
      } finally {
        readAllBtn.disabled = false;
      }
    };

    /* клики контролов — делегирование (переживают htmx-обмен сегмента):
       строки ленты, «Прочитать всё», чипы-фильтры (смена — первая
       страница нового фильтра, cursor сброс), «Ещё»/«Повторить» обеих
       вкладок */
    document.addEventListener('click', (event) => {
      const row = event.target.closest('[data-notif-id]');
      if (row) { markReadInBackground(row); return; }
      const readAllBtn = event.target.closest('[data-read-all]');
      if (readAllBtn) { readAll(readAllBtn); return; }
      const chip = event.target.closest('[data-feed-filter]');
      if (chip) {
        if (chip.getAttribute('aria-pressed') === 'true') return;
        state.type = chip.dataset.feedFilter || '';
        document.querySelectorAll('[data-feed-filter]').forEach((c) => {
          c.setAttribute('aria-pressed', String(c === chip));
        });
        load(null);
        return;
      }
      const unreadChip = event.target.closest('[data-feed-unread]');
      if (unreadChip) {
        state.unread = unreadChip.getAttribute('aria-pressed') !== 'true';
        unreadChip.setAttribute('aria-pressed', String(state.unread));
        load(null);
        return;
      }
      const moreBtn = event.target.closest('[data-feed-more]');
      if (moreBtn) {
        if (state.cursor) load(state.cursor);
        return;
      }
      const feedRetry = event.target.closest('[data-feed-retry]');
      if (feedRetry) { load(null); return; }
      const newsMore = event.target.closest('[data-news-more]');
      if (newsMore) {
        if (newsState.cursor) loadNews(newsState.cursor, 0);
        return;
      }
      const newsRetry = event.target.closest('[data-news-retry]');
      if (newsRetry) { loadNews(null, 0); return; }
    });

    /* --- состояния ПЕРЕКЛЮЧЕНИЯ сегмента (03 §5.3/§5.4; паттерн экрана
       «Расписание»): ошибка сетевого GET /notifications/segment — блок
       [data-segment-error], прежний сегмент остаётся на месте;
       «Повторить» перезапускает запрос элемента, на котором он оборвался
       (ретриггер клика; 03 §5.3 — «Повторить» всегда реально
       перезапрашивает); офлайн — баннер каркаса; истёкшая сессия (фильтр
       отвечает 303 → /login, XHR следует редиректу) — браузерный переход
       на вход. */
    const segmentErrorBox = document.querySelector('[data-segment-error]');
    let segmentRetryElt = null;
    document.body.addEventListener('htmx:afterRequest', (e) => {
      if (!segmentErrorBox) return;
      if (e.detail && e.detail.successful) {
        segmentErrorBox.hidden = true;
        return;
      }
      if (e.detail && e.detail.elt) segmentRetryElt = e.detail.elt;
      segmentErrorBox.hidden = false;
    });
    document.body.addEventListener('htmx:sendError', (e) => {
      if (!navigator.onLine) applyOffline(true);
      if (segmentErrorBox) {
        if (e.detail && e.detail.elt) segmentRetryElt = e.detail.elt;
        segmentErrorBox.hidden = false;
      }
    });
    document.body.addEventListener('htmx:beforeSwap', (e) => {
      const xhr = e.detail && e.detail.xhr;
      if (xhr && /\/login(?:\?.*)?$/.test(xhr.responseURL || '')) {
        e.detail.shouldSwap = false;
        window.location.href = '/login';
      }
    });
    const segmentRetryBtn = document.querySelector('[data-segment-retry]');
    if (segmentRetryBtn) {
      segmentRetryBtn.addEventListener('click', () => {
        if (segmentRetryElt) segmentRetryElt.click();
        else window.location.reload();
      });
    }
  }

  /* ---------- экран «Профиль» (views/profile.csp; 03 §4.7, §5) ---------- */
  /* Данные — клиентские fetch (паттерн views/rooms.csp): /v1/me,
     /v1/library/my-card, /v1/notifications/settings (GET при загрузке),
     POST/DELETE /v1/ics/token, POST /v1/auth/logout. Вставка — только
     createElement/textContent (02 §6.4). QR — вендоренный
     qrcode-generator 1.5.0 (Р-21, docs/decisions.md): canvas по
     getModuleCount()/isDark(); данные — вымышленные номера карточек
     «para-demo:{student_card_no}» и card_no (константа приложения — не
     секрет, 01 §5.9/§5.12); stringToBytes переключается на UTF-8 —
     дефолтная усекает charCodeAt до 8 бит и портит кириллицу номеров
     («ФМО-101-26/001»). Пиксели — светлые значения существующей пары
     токенов --surface/--text: код сканируется камерой и не тематизируется
     (комментарий .qr-canvas в app.css). Пустых состояний нет — профиль
     есть у каждого студента (постановка); сотрудник без профиля
     (02 §4.1) — студенческие строки скрыты, имя = login. */
  if (document.body.dataset.page === 'profile') {
    const show = (sel, on) => { const n = document.querySelector(sel); if (n) n.hidden = !on; };

    /* --- профиль (GET /v1/me): скелетон → карточка | ошибка с «Повторить» --- */
    let meData = null; /* для QR-карточки студента (данные последнего ответа) */
    const fillProfile = (me) => {
      meData = me;
      const p = me && me.profile ? me.profile : null;
      const name = p ? p.fio : ((me && me.login) || '');
      const initials = name.split(/\s+/).filter(Boolean).slice(0, 2)
        .map((w) => w.charAt(0)).join('').toUpperCase();
      const av = $('[data-avatar-initials]');
      if (av) av.textContent = initials;
      const nameEl = $('[data-account-name]');
      if (nameEl) nameEl.textContent = name;
      const meta = $('[data-account-meta]');
      if (meta) meta.textContent = p ? p.course + ' курс · ' + p.faculty : '';
      const prog = $('[data-account-program]');
      if (prog) prog.textContent = p ? p.program : '';
      const group = $('[data-account-group]');
      if (group) group.textContent = p ? p.group : '';
      const card = $('[data-account-card]');
      if (card) card.textContent = p ? p.student_card_no : '';
      /* сотрудник без профиля (02 §4.1): студенческие строки и QR скрыты */
      document.querySelectorAll('[data-student-line]').forEach((n) => { n.hidden = !p; });
      const lib = $('[data-library-block]');
      if (lib && !p) lib.hidden = true;
      show('[data-profile-body]', true);
    };
    const loadProfile = async () => {
      show('[data-profile-loading]', true);
      show('[data-profile-error]', false);
      show('[data-profile-body]', false);
      const me = await fetchJSON('/v1/me');
      show('[data-profile-loading]', false);
      if (!me) { show('[data-profile-error]', true); return; }
      fillProfile(me);
      /* читательский билет (5.12): 200 — номер + QR; 404 (нет билета —
         office.demo, Э6а) и любой другой ответ — блок остаётся скрытым */
      const mine = await fetchJSON('/v1/library/my-card');
      if (me.profile && mine && mine.card_no) {
        const no = $('[data-library-card-no]');
        if (no) no.textContent = mine.card_no;
        show('[data-library-block]', true);
      }
    };
    const profileRetry = $('[data-profile-retry]');
    if (profileRetry) profileRetry.addEventListener('click', () => { loadProfile(); });
    loadProfile();

    /* --- настройки (GET при загрузке): тумблеры, тихие часы, тема,
       онбординг С5. Состояния секции — как у профиля (03 §5). --- */
    const markTheme = (t) => {
      document.querySelectorAll('[data-theme-choice]').forEach((b) => {
        b.setAttribute('aria-pressed', String(b.dataset.themeChoice === t));
      });
    };
    const applySettings = (s) => {
      document.querySelectorAll('[data-setting]').forEach((input) => {
        input.checked = Boolean(s[input.dataset.setting]);
      });
      const q = (s && s.quiet_hours) || {};
      const from = $('[data-quiet-from]');
      const to = $('[data-quiet-to]');
      if (from && typeof q.from === 'string') from.value = q.from;
      if (to && typeof q.to === 'string') to.value = q.to;
      if (s.theme) { applyTheme(s.theme); markTheme(s.theme); }
      markLang(s.lang);
      show('[data-settings-body]', true);
      show('[data-theme-group]', true);
      show('[data-lang-group]', true);
    };
    const loadSettingsUI = async () => {
      show('[data-settings-loading]', true);
      show('[data-settings-error]', false);
      show('[data-settings-body]', false);
      show('[data-theme-group]', false);
      show('[data-lang-group]', false);
      const s = await loadSettings();
      show('[data-settings-loading]', false);
      if (!s) { show('[data-settings-error]', true); return; }
      applySettings(s);
      /* онбординг С5: выполненные отмечены, шаг отмечается кнопкой
         (renderOnboarding — общий с оверлеем дашборда) */
      const obRoot = $('[data-ob-card]');
      if (obRoot) {
        const onChange = (steps) => {
          const done = $('[data-ob-done]');
          if (done) done.hidden = steps.length < ONBOARD_KEYS.length;
        };
        onChange(renderOnboarding(obRoot, obNormalize(s.onboarding), onChange));
      }
    };
    const settingsRetry = $('[data-settings-retry]');
    if (settingsRetry) settingsRetry.addEventListener('click', () => { loadSettingsUI(); });
    loadSettingsUI();

    /* тумблеры: change → PUT {notify_*} (частичное обновление 02 §5.2);
       ошибка — значение откатывается, текст — toast */
    document.querySelectorAll('[data-setting]').forEach((input) => {
      input.addEventListener('change', async () => {
        if (!navigator.onLine) { applyOffline(true); input.checked = !input.checked; return; }
        input.disabled = true;
        const patch = {};
        patch[input.dataset.setting] = input.checked;
        const saved = await putSettings(patch);
        input.disabled = false;
        if (!saved) {
          input.checked = !input.checked;
          toast(t('toast.failSetting', 'Не получилось сохранить настройку'));
        }
      });
    });
    /* тихие часы: два time-инпута → PUT quiet_hours_json (строка с JSON —
       формат сервера {"from":"HH:MM","to":"HH:MM"}, дефолт 23:00–07:00;
       интервал через полночь допустим — проверка на не-пустоту) */
    const saveQuiet = async () => {
      const from = $('[data-quiet-from]');
      const to = $('[data-quiet-to]');
      if (!from || !to || !from.value || !to.value) return;
      if (!navigator.onLine) { applyOffline(true); return; }
      const saved = await putSettings({
        quiet_hours_json: JSON.stringify({ from: from.value, to: to.value })
      });
      if (saved) toast('Тихие часы сохранены');
      else toast(t('toast.failQuiet', 'Не получилось сохранить тихие часы'));
    };
    const quietFrom = $('[data-quiet-from]');
    const quietTo = $('[data-quiet-to]');
    if (quietFrom) quietFrom.addEventListener('change', saveQuiet);
    if (quietTo) quietTo.addEventListener('change', saveQuiet);
    /* тема: смена применяется СРАЗУ (applyTheme — data-theme на <html>),
       сохранение — PUT {theme}; сбой сети — вид остаётся локальным,
       офлайн-баннер покажет каркас */
    document.querySelectorAll('[data-theme-choice]').forEach((btn) => {
      btn.addEventListener('click', async () => {
        const t = btn.dataset.themeChoice;
        applyTheme(t);
        markTheme(t);
        if (!navigator.onLine) { applyOffline(true); return; }
        const saved = await putSettings({ theme: t });
        if (saved) toast('Оформление сохранено');
        else toast(t('toast.failTheme', 'Не получилось сохранить тему'));
      });
    });

    /* --- подписка на календарь (5.4): POST /v1/ics/token → 201 {token}
       (Р-11: 256 бит, 64 hex); ПОВТОРНОЕ создание ротирует токен — прежняя
       ссылка перестаёт обновляться (подсказка в блоке); DELETE → 204.
       GET «есть ли подписка» в API нет (02 §5.2) — экран стартует в
       состоянии «нет подписки» (постановка). --- */
    const showIcs = (token) => {
      if (!/^[0-9a-f]{64}$/.test(token)) return;
      const path = '/ics/' + token;
      const url = $('[data-ics-url]');
      const webcal = $('[data-ics-webcal]');
      if (url) url.textContent = location.origin + path;
      if (webcal) webcal.href = 'webcal://' + location.host + path;
      show('[data-ics-none]', false);
      show('[data-ics-active]', true);
    };
    const icsCreate = $('[data-ics-create]');
    if (icsCreate) {
      icsCreate.addEventListener('click', async () => {
        if (icsCreate.disabled) return;
        if (!navigator.onLine) { applyOffline(true); return; }
        icsCreate.disabled = true;
        icsCreate.textContent = 'Создаём…';
        const data = await fetchJSON('/v1/ics/token', { method: 'POST' });
        icsCreate.disabled = false;
        icsCreate.textContent = 'Создать подписку';
        if (data && data.token) {
          showIcs(data.token);
          toast('Подписка создана — прежняя ссылка отозвана');
        } else {
          toast(t('toast.failIcsCreate', 'Не получилось создать подписку'));
        }
      });
    }
    const icsRevoke = $('[data-ics-revoke]');
    if (icsRevoke) {
      icsRevoke.addEventListener('click', async () => {
        if (icsRevoke.disabled) return;
        if (!navigator.onLine) { applyOffline(true); return; }
        icsRevoke.disabled = true;
        let revoked = false;
        try {
          const resp = await fetch('/v1/ics/token', {
            method: 'DELETE',
            headers: { Accept: 'application/json' }
          });
          revoked = resp.status === 204;
        } catch (e) { /* сеть ушла — баннер покажет каркас */ }
        icsRevoke.disabled = false;
        if (revoked) {
          show('[data-ics-active]', false);
          show('[data-ics-none]', true);
          toast('Подписка отозвана — календарь больше не обновляется');
        } else {
          toast(t('toast.failIcsRevoke', 'Не получилось отозвать подписку'));
        }
      });
    }

    /* --- QR-карточки (5.9 студбилет, 5.12 библиотека): dialog-sheet
       (как «Детали пары»/отмена брони), QR ~240px, имя и номер под ним,
       закрытие тапом по коду или кнопкой (01 §5.9). --- */
    const qrDialog = $('#qr-dialog');
    const drawQr = (text) => {
      if (!qrDialog || typeof window.qrcode !== 'function') {
        toast('QR-модуль не загрузился — обновите страницу');
        return false;
      }
      window.qrcode.stringToBytes = window.qrcode.stringToBytesFuncs['UTF-8'];
      const qr = window.qrcode(0, 'M'); /* автоподбор версии, коррекция M */
      qr.addData(text);
      qr.make();
      const count = qr.getModuleCount();
      const quiet = 2; /* тихая зона в модулях */
      const cell = Math.max(2, Math.floor(240 / (count + quiet * 2)));
      const size = cell * (count + quiet * 2);
      const canvas = qrDialog.querySelector('[data-qr-canvas]');
      canvas.width = size;
      canvas.height = size;
      const ctx = canvas.getContext('2d');
      ctx.fillStyle = '#FFFFFF'; /* --surface светлой темы */
      ctx.fillRect(0, 0, size, size);
      ctx.fillStyle = '#273F3A'; /* --text светлой темы */
      for (let row = 0; row < count; row++) {
        for (let col = 0; col < count; col++) {
          if (qr.isDark(row, col)) {
            ctx.fillRect((col + quiet) * cell, (row + quiet) * cell, cell, cell);
          }
        }
      }
      return true;
    };
    const openQr = (title, text, value) => {
      if (!drawQr(text)) return;
      const t = $('[data-qr-title]');
      const v = $('[data-qr-value]');
      if (t) t.textContent = title;
      if (v) v.textContent = value;
      qrDialog.showModal();
    };
    const studentQr = $('[data-qr-student]');
    if (studentQr) {
      studentQr.addEventListener('click', () => {
        const p = meData && meData.profile;
        if (!p) return;
        /* «para-demo:{student_card_no}» — константа приложения, не секрет */
        openQr(t('profile.qrStudent', 'QR-карточка студента'), 'para-demo:' + p.student_card_no,
          p.fio + ' · ' + t('profile.cardNoLabel', 'студ. билет №') + ' ' + p.student_card_no);
      });
    }
    const libraryQr = $('[data-qr-library]');
    if (libraryQr) {
      libraryQr.addEventListener('click', () => {
        const no = $('[data-library-card-no]');
        openQr(t('profile.libraryCard', 'Читательский билет'), no ? no.textContent : '', t('profile.libraryNoLabel', 'Библиотека · билет №') + ' ' + (no ? no.textContent : ''));
      });
    }
    if (qrDialog) {
      qrDialog.querySelector('[data-qr-canvas]').addEventListener('click', () => qrDialog.close());
      const qrClose = qrDialog.querySelector('[data-qr-close]');
      if (qrClose) qrClose.addEventListener('click', () => qrDialog.close());
    }

    /* --- выход (03 §4.7: диалог с danger-подтверждением; POST
       /v1/auth/logout → 200 {ok} — сессия инвалидирована на сервере —
       редирект /login). --- */
    const logoutDialog = $('#logout-dialog');
    const logoutOpen = $('[data-logout-open]');
    if (logoutDialog && logoutOpen) {
      logoutOpen.addEventListener('click', () => logoutDialog.showModal());
      const logoutClose = logoutDialog.querySelector('[data-logout-close]');
      if (logoutClose) logoutClose.addEventListener('click', () => logoutDialog.close());
      const logoutConfirm = logoutDialog.querySelector('[data-logout-confirm]');
      if (logoutConfirm) {
        logoutConfirm.addEventListener('click', async () => {
          if (logoutConfirm.disabled) return;
          if (!navigator.onLine) { applyOffline(true); return; }
          logoutConfirm.disabled = true;
          logoutConfirm.textContent = 'Выходим…';
          let done = false;
          try {
            const resp = await fetch('/v1/auth/logout', {
              method: 'POST',
              headers: { Accept: 'application/json' }
            });
            done = resp.ok; /* 200 {ok:true} — сессия удалена (02 §5.2) */
          } catch (e) { /* сеть ушла — баннер покажет каркас */ }
          if (done) {
            window.location.href = '/login';
            return;
          }
          logoutConfirm.disabled = false;
          logoutConfirm.textContent = 'Выйти';
          toast(t('toast.failLogout', 'Не получилось выйти — проверьте интернет'));
        });
      }
    }
  }

  /* Страничный алиас /map/rooms → 303 /map?q=… приземлился (коммит
     экрана «Карта») — временный guard ссылок a[href^="/map/"] (правки
     по ревью раунд 1: честный тост вместо 404, пока страницы не было)
     снят по его собственному предписанию «удаляется, когда страница
     карты приземлится». Ссылки кабинета в person.csp переходят обычным
     образом: браузер следует 303 на /map?q=…, страница читает q. */

  /* ---------- экран «Люди» (views/people.csp; 03 §4.9.6, Э7) ------------ */
  /* Каталог-поиск: fetch GET /v1/people/search (query q/role — ТЕ ЖЕ
     JSON-роуты, что стоят за API; паттерн notifications-секции). Debounce
     ввода ~300 мс (постановка экрана); гонку устаревших ответов гасит
     монотонный номер запроса (seq) — последний ввод всегда побеждает.
     Скелетон показывается ТОЛЬКО если ответ дольше 300 мс (03 §5.1 —
     таймер на показ, снимается по ответу). Пустой запрос — подсказка
     «Начните вводить…» + «Преподаватели»: первая страница search БЕЗ q
     (role=teacher) — пустой q сервис не фильтрует (паттерн LIKE '%%',
     поведение PeopleService::search). Строки: имя КАК ПРИШЛО (чужие
     студенты уже маскированы сервером, Р-15 — рисуем как пришло), роль,
     группа/кафедра; тап по строке → /people/{id}. Чипы ролей — словарь
     02 §5.2 teacher|student|staff (пусто — «все»). Состояния 03 §5:
     пусто «Никого не нашли», ошибка с «Повторить» (повторный fetch),
     офлайн — баннер каркаса (fetchJSON сам переводит в офлайн-режим).
     Вставка — только createElement/textContent (02 §6.4);
     createContextualFragment — только статическая svg-константа стрелки,
     данные сервера в innerHTML НИКОГДА. */
  if (document.body.dataset.page === 'people') {
    const show = (sel, on) => { const n = document.querySelector(sel); if (n) n.hidden = !on; };
    const el = (tag, className, text) => {
      const node = document.createElement(tag);
      if (className) node.className = className;
      if (text != null) node.textContent = text;
      return node;
    };
    const ARROW = '<svg class="icon" viewBox="0 0 24 24" aria-hidden="true"><path d="M5 12h14m-6-6 6 6-6 6"/></svg>';
    const ROLE_LABEL = { teacher: 'Преподаватель', student: 'Студент', staff: 'Сотрудник' };
    const input = document.querySelector('[data-people-q]');
    const state = { q: '', role: '', seq: 0 };
    let skTimer = 0; /* таймер показа скелетона (03 §5.1: ответ > 300 мс) */
    let debounce = 0;

    const subOf = (p) => {
      const parts = [ROLE_LABEL[p.role] || ''];
      if (p.role === 'student') {
        if (p.group) parts.push('группа ' + p.group);
      } else {
        if (p.title) parts.push(p.title);
        if (p.department) parts.push(p.department);
      }
      return parts.filter(Boolean).join(' · ');
    };
    const rowFor = (p) => {
      const a = el('a', 'list-row');
      a.href = '/people/' + p.id;
      const mid = el('div');
      mid.append(el('strong', null, p.fio || ''));
      mid.append(el('p', null, subOf(p)));
      a.append(mid);
      a.append(document.createRange().createContextualFragment(ARROW));
      return a;
    };
    const renderInto = (box, people) => {
      if (!box) return;
      box.replaceChildren();
      people.forEach((p) => box.append(rowFor(p)));
    };

    const load = async () => {
      const seq = ++state.seq;
      clearTimeout(skTimer);
      show('[data-people-loading]', false);
      show('[data-people-hint]', false);
      show('[data-people-empty]', false);
      show('[data-people-error]', false);
      show('[data-people-results]', false);
      const q = state.q.trim();
      /* Пустой запрос — «каталог». БЕЗ чипа роли — подсказка + преподаватели
         (первая страница search без q, role=teacher — пустой q сервис не
         фильтрует). С АКТИВНЫМ чипом роли фильтр уважается и без текста
         запроса: подсказка не показывается, список — выбранной роли
         (правки по ревью раунд 1: role=teacher не перебивает чип «Студенты»). */
      const browse = !q && !state.role;
      if (browse) show('[data-people-hint]', true);
      const params = new URLSearchParams();
      if (q) params.set('q', q);
      if (state.role) {
        params.set('role', state.role);
      } else if (browse) {
        params.set('role', 'teacher');
      }
      if (!browse) skTimer = setTimeout(() => show('[data-people-loading]', true), 300);
      const qs = params.toString();
      const data = await fetchJSON('/v1/people/search' + (qs ? '?' + qs : ''));
      if (seq !== state.seq) return;
      clearTimeout(skTimer);
      show('[data-people-loading]', false);
      if (!data || !Array.isArray(data.people)) {
        if (browse) show('[data-people-hint]', false);
        show('[data-people-error]', true);
        return;
      }
      if (!data.people.length) {
        /* для каталога выбранной роли (пустой запрос + чип) «Никого не
           нашли» честно: выход — сбросить фильтр (текст пустого состояния
           это и предлагает); подсказка при этом скрыта */
        if (browse) show('[data-people-hint]', false);
        show('[data-people-empty]', true);
        return;
      }
      if (browse) {
        renderInto(document.querySelector('[data-people-hint-list]'), data.people);
        return;
      }
      renderInto(document.querySelector('[data-people-list]'), data.people);
      show('[data-people-results]', true);
    };

    if (input) {
      input.addEventListener('input', () => {
        state.q = input.value;
        clearTimeout(debounce);
        debounce = setTimeout(load, 300);
      });
    }
    document.querySelectorAll('[data-people-role]').forEach((chip) => {
      chip.addEventListener('click', () => {
        if (chip.getAttribute('aria-pressed') === 'true') return;
        state.role = chip.dataset.peopleRole || '';
        document.querySelectorAll('[data-people-role]').forEach((c) => {
          c.setAttribute('aria-pressed', String(c === chip));
        });
        clearTimeout(debounce);
        load();
      });
    });
    const peopleRetry = document.querySelector('[data-people-retry]');
    if (peopleRetry) peopleRetry.addEventListener('click', () => load());

    load();
  }

  /* ---------- профиль человека (views/person.csp; 03 §4.9.6, Э7) -------- */
  /* Шапка профиля — SSR (тело PeopleService::profile, ветвление по полям
     ответа в шаблоне: преподаватель/сотрудник — mailto и кабинет ссылкой
     /map/rooms?q=<номер>; чужой студент — только публичные поля, БЕЗ
     блоков оценок/заявок — их нет и в данных, 02 §5.2/Р-15). Здесь — блок
     «Занятия» / «Расписание группы»: fetch GET /v1/people/{id}/schedule
     (id — уже провалидированный контроллером data-person-id на body;
     кроме data-режима блока нет — fetch не запускается). Пары — компоненты
     каркаса .schedule-list/.lesson; статусы — как на «Расписании»:
     перенос — левый акцент + бейдж «Перенос» + было → стало в кабинете,
     отмена — зачёркнуто (классы .lesson.moved/.cancelled — app.css).
     Дни группируются по дате UTC проекта (сервер отдаёт ORDER BY
     starts_at — порядок стабилен, новая дата = новая группа). Состояния
     03 §5 блока: скелетон, пусто (текст по ветке — отрисовал сервер),
     ошибка с «Повторить» (повторный fetch), офлайн — баннер каркаса.
     «Повторить» SSR-экрана ошибки (dberror) — перезагрузка страницы
     (03 §5.3, прецедент application.csp). Вставка — только
     createElement/textContent (02 §6.4). */
  if (document.body.dataset.page === 'person') {
    const show = (sel, on) => { const n = document.querySelector(sel); if (n) n.hidden = !on; };
    const el = (tag, className, text) => {
      const node = document.createElement(tag);
      if (className) node.className = className;
      if (text != null) node.textContent = text;
      return node;
    };
    const KIND = { lecture: 'Лекция', seminar: 'Семинар', practice: 'Практика' };
    const timeOf = (iso) => (typeof iso === 'string' && iso.length >= 16) ? iso.slice(11, 16) : '';
    const dateOf = (iso) => (typeof iso === 'string' && iso.length >= 10) ? iso.slice(0, 10) : '';
    /* день по календарной дате UTC проекта (без сдвига часового пояса) */
    const fmtDay = (ymd) => {
      const p = ymd.split('-').map(Number);
      if (p.length !== 3 || p.some(Number.isNaN)) return ymd;
      return new Date(Date.UTC(p[0], p[1] - 1, p[2])).toLocaleDateString('ru-RU', { weekday: 'long', day: 'numeric', month: 'long', timeZone: 'UTC' });
    };
    /* «2 ноября» / для диапазона «Занятия 2–8 ноября» (черновик
       design-drafts/person.html) */
    const dayMonth = (ymd) => {
      const p = ymd.split('-').map(Number);
      if (p.length !== 3 || p.some(Number.isNaN)) return ymd;
      return new Date(Date.UTC(p[0], p[1] - 1, p[2])).toLocaleDateString('ru-RU', { day: 'numeric', month: 'long', timeZone: 'UTC' });
    };

    const pageRetry = document.querySelector('[data-person-retry]');
    if (pageRetry) pageRetry.addEventListener('click', () => window.location.reload());

    const scheduleRoot = document.querySelector('[data-person-schedule]');
    const personId = document.body.dataset.personId || '';
    const lessonRow = (e) => {
      const row = el('div', 'lesson');
      if (e.status === 'moved') row.classList.add('moved');
      if (e.status === 'cancelled') row.classList.add('cancelled');
      const time = el('div', 'lesson-time');
      time.append(el('strong', null, timeOf(e.starts_at)));
      if (timeOf(e.ends_at)) time.append(el('span', null, timeOf(e.ends_at)));
      const mid = el('div');
      mid.append(el('div', 'lesson-subject', e.discipline || ''));
      const detail = el('p', 'lesson-detail', KIND[e.kind] || '');
      if (e.status === 'moved') detail.append(' ', el('span', 'badge warn', 'Перенос'));
      if (e.status === 'cancelled') detail.append(' ', el('span', 'badge danger', 'Отменена'));
      mid.append(detail);
      const room = el('div', 'lesson-room');
      if (e.status === 'cancelled') {
        room.append(el('s', null, e.room || '—'));
        room.append(el('small', null, 'отменена'));
      } else if (e.status === 'moved' && e.prev_room) {
        room.append(el('s', null, e.prev_room));
        room.append(document.createTextNode(' → ' + (e.room || '—')));
      } else {
        room.append(el('span', null, e.room || '—'));
      }
      row.append(time, mid, room);
      return row;
    };
    const loadSchedule = async () => {
      if (!scheduleRoot || !/^\d+$/.test(personId)) return;
      show('[data-ps-loading]', true);
      show('[data-ps-error]', false);
      show('[data-ps-empty]', false);
      const list = document.querySelector('[data-ps-list]');
      if (list) { list.hidden = true; list.replaceChildren(); }
      const data = await fetchJSON('/v1/people/' + personId + '/schedule');
      show('[data-ps-loading]', false);
      if (!data || !Array.isArray(data.events)) {
        show('[data-ps-error]', true);
        return;
      }
      if (!data.events.length) {
        show('[data-ps-empty]', true);
        return;
      }
      if (!list) return;
      /* Заголовок блока с РЕАЛЬНЫМ диапазоном отрисованных занятий
         («Занятия 2–8 ноября», один день — «Занятия 4 ноября») — формулировка
         черновика person.html/вайрфрейма 03 §4.9.6; у студента —
         «Расписание группы» не меняется. Сервер отдаёт ORDER BY starts_at —
         первый/последний элементы и есть границы диапазона. */
      const title = document.querySelector('[data-ps-title]');
      const role = data.person && data.person.role;
      if (title && (role === 'teacher' || role === 'staff')) {
        const first = dateOf(data.events[0].starts_at);
        const last = dateOf(data.events[data.events.length - 1].starts_at);
        if (first && last) {
          title.textContent = 'Занятия ' + (first === last
            ? dayMonth(first)
            : first.slice(0, 7) === last.slice(0, 7)
              ? first.slice(8, 10).replace(/^0/, '') + '–' + dayMonth(last)
              : dayMonth(first) + ' – ' + dayMonth(last));
        }
      }
      let current = '';
      let dayBox = null;
      data.events.forEach((e) => {
        const day = dateOf(e.starts_at);
        if (day && day !== current) {
          current = day;
          list.append(el('p', 'caption', fmtDay(day)));
          dayBox = el('div', 'schedule-list');
          list.append(dayBox);
        }
        if (dayBox) dayBox.append(lessonRow(e));
      });
      list.hidden = false;
    };
    const psRetry = document.querySelector('[data-ps-retry]');
    if (psRetry) psRetry.addEventListener('click', () => loadSchedule());

    loadSchedule();
  }

  /* ---------- стол сотрудника (views/staff.csp; 03 §4.8, Э7) ------------ */
  /* Последний экран Э7. Сводка (GET /v1/staff/dashboard: счётчики очереди
     5 активных статусов + свежие изменения), очередь (GET /v1/staff/
     applications?status=&type=&overdue=1 — чипы-фильтры, карточки с
     владельцем и «Срок горит»), перенос пары (POST /v1/staff/schedule/
     changes; 201 {recipients} → «Уведомлено N студентов») и объявления
     (POST /v1/staff/announcements — без push, подпись в форме, 01 §4.5;
     лента GET /v1/announcements?cursor=).
     Смена статуса — кнопки допустимых переходов матрицы (TRANSITIONS —
     зеркало constexpr-матрицы src/models/Applications.h, как APP_STATUS —
     зеркало словаря статусов): POST /v1/staff/applications/{id}/status
     (raw fetch — нужны коды 409/400/404); успех → htmx-перерисовка
     карточки серверным фрагментом GET /staff/app-card?id= (htmx.ajax,
     outerHTML — паттерн /schedule/segment, Р-19; разметка карточки —
     views/staff/app_card.csp, клиентская сборка повторяет её классы);
     409 «Недопустимый переход статуса заявки» (и 400/404) — inline-
     предупреждение в слоте data-app-warn карточки, БЕЗ перерисовки.
     Состояния 03 §5: скелетоны в разметке, пустые тексты («Очередь
     разобрана» — staff-states.html), ошибки с «Повторить» (повторный
     fetch), офлайн — баннер каркаса (fetchJSON/postJSON переводят сами).
     Вставка — только createElement/textContent (02 §6.4). */
  if (document.body.dataset.page === 'staff') {
    const show = (sel, on) => { const n = document.querySelector(sel); if (n) n.hidden = !on; };
    const el = (tag, className, text) => {
      const node = document.createElement(tag);
      if (className) node.className = className;
      if (text != null) node.textContent = text;
      return node;
    };
    /* метки статусов — слова таймлайна 01 §4.4 (словарь CHECK DDL 0003) */
    const APP_STATUS = {
      draft: 'Черновик', submitted: 'Подана', accepted: 'Принята',
      in_progress: 'В работе', need_info: 'Требуется уточнение',
      ready: 'Готова', issued: 'Выдана', rejected: 'Отклонена', cancelled: 'Отменена'
    };
    /* класс бейджа — словарь views/applications.csp (03 §4.5.1) */
    const BADGE = {
      draft: 'outline', submitted: 'info', accepted: 'info', in_progress: 'info',
      need_info: 'warn', ready: 'success', issued: 'success',
      rejected: 'danger', cancelled: 'danger'
    };
    /* матрица переходов — зеркало kApplicationStatusTransitions (модели);
       кнопки карточки строятся только из неё */
    const TRANSITIONS = {
      draft: ['submitted', 'cancelled'],
      submitted: ['accepted', 'rejected', 'cancelled'],
      accepted: ['in_progress', 'rejected', 'cancelled'],
      in_progress: ['ready', 'need_info', 'rejected', 'cancelled'],
      need_info: ['in_progress', 'rejected', 'cancelled'],
      ready: ['issued'],
      issued: [], rejected: [], cancelled: []
    };
    const CHANGE_LABEL = { moved: 'Перенесена', cancelled: 'Отменена', substituted: 'Замена' };
    const timeOf = (iso) => (typeof iso === 'string' && iso.length >= 16) ? iso.slice(11, 16) : '';
    const dateOf = (iso) => (typeof iso === 'string' && iso.length >= 10) ? iso.slice(0, 10) : '';
    const dayMonth = (ymd) => {
      const p = (ymd || '').split('-').map(Number);
      if (p.length !== 3 || p.some(Number.isNaN)) return ymd || '';
      return new Date(Date.UTC(p[0], p[1] - 1, p[2])).toLocaleDateString('ru-RU', { day: 'numeric', month: 'long', timeZone: 'UTC' });
    };
    const stampOf = (iso) => (typeof iso === 'string' && iso.length >= 16)
      ? dayMonth(iso.slice(0, 10)) + ', ' + iso.slice(11, 16) : '';
    const plural = (n, one, few, many) => {
      const m10 = n % 10, m100 = n % 100;
      if (m10 === 1 && m100 !== 11) return one;
      if (m10 >= 2 && m10 <= 4 && (m100 < 12 || m100 > 14)) return few;
      return many;
    };
    const busy = (btn, on, busyLabel) => {
      if (!btn) return;
      if (on) {
        btn.dataset.wasLabel = btn.textContent;
        btn.textContent = busyLabel || 'Сохраняем…';
        btn.disabled = true;
      } else {
        btn.textContent = btn.dataset.wasLabel || btn.textContent;
        delete btn.dataset.wasLabel;
        btn.disabled = false;
      }
    };
    /* POST JSON с кодами ответа (raw fetch — inline-ошибки из тела
       {error:{code,message}}, 02 §5.1; сетевой сброс — офлайн-баннер) */
    const postJSON = async (url, payload) => {
      if (!navigator.onLine) { applyOffline(true); return null; }
      try {
        const resp = await fetch(url, {
          method: 'POST',
          headers: { Accept: 'application/json', 'Content-Type': 'application/json' },
          body: JSON.stringify(payload)
        });
        let body = null;
        try { body = await resp.json(); } catch (e) { /* тело не JSON */ }
        return { ok: resp.ok, status: resp.status, body };
      } catch (e) {
        if (!navigator.onLine) applyOffline(true);
        return null;
      }
    };
    const errorMessage = (r) => (r && r.body && r.body.error && r.body.error.message)
      ? String(r.body.error.message) : '';

    /* истёкшая сессия на htmx-запросе: фильтр отвечает 303 → /login, XHR
       следует редиректу — прерываем обмен и уводим браузер на вход
       (паттерн экрана «Расписание») */
    document.body.addEventListener('htmx:beforeSwap', (e) => {
      const xhr = e.detail && e.detail.xhr;
      if (xhr && /\/login(?:\?.*)?$/.test(xhr.responseURL || '')) {
        e.detail.shouldSwap = false;
        window.location.href = '/login';
      }
    });

    /* --- сводка: счётчики + свежие изменения (03 §4.8) ------------------ */
    const QUEUE_KEYS = ['submitted', 'accepted', 'in_progress', 'need_info', 'ready'];
    const changeRow = (e) => {
      const li = el('li');
      const link = el('a', 'staff-change');
      const day = dateOf(e.starts_at);
      link.href = day ? '/schedule?date=' + day : '/schedule';
      link.append(el('strong', null, e.discipline || 'Занятие'));
      if (CHANGE_LABEL[e.status]) link.append(' ', el('span', 'badge ' + (e.status === 'cancelled' ? 'danger' : e.status === 'substituted' ? 'info' : 'warn'), CHANGE_LABEL[e.status]));
      const pieces = [];
      if (e.status === 'moved') {
        pieces.push('ауд. ' + (e.prev_room || '—') + ' → ' + (e.room || '—'));
        if (timeOf(e.prev_starts_at) && timeOf(e.prev_starts_at) !== timeOf(e.starts_at)) {
          pieces.push(timeOf(e.prev_starts_at) + ' → ' + timeOf(e.starts_at));
        }
      } else if (e.status === 'cancelled') {
        pieces.push('ауд. ' + (e.room || '—') + ' — отменена');
      } else if (e.room) {
        pieces.push('ауд. ' + e.room);
      }
      if (pieces.length) link.append(el('p', 'caption', pieces.join(' · ')));
      if (e.changed_at) link.append(el('p', 'caption muted tnum', 'сообщено ' + stampOf(e.changed_at)));
      li.append(link);
      return li;
    };
    /* select формы переноса — из «свежих изменений» (постановка экрана) */
    const fillEventSelect = (changes) => {
      const sel = document.querySelector('[data-change-event]');
      if (!sel) return;
      const previous = sel.value;
      sel.replaceChildren();
      const ph = el('option', null, changes.length ? 'Выберите событие…' : 'Свежих изменений нет — укажите id события вручную');
      ph.value = '';
      sel.append(ph);
      changes.forEach((e) => {
        const label = '#' + e.id + ' · ' + (e.discipline || 'Занятие')
          + ' · ' + dayMonth(dateOf(e.starts_at)) + ' ' + timeOf(e.starts_at);
        const o = el('option', null, label);
        o.value = String(e.id);
        o.dataset.starts = e.starts_at || '';
        o.dataset.room = e.room || '';
        sel.append(o);
      });
      if (previous && Array.from(sel.options).some((o) => o.value === previous)) sel.value = previous;
    };
    const loadSummary = async (silent) => {
      if (!silent) {
        show('[data-staff-error]', false);
        show('[data-metrics-loading]', true);
        show('[data-changes-loading]', true);
        show('[data-metrics-body]', false);
        show('[data-changes-body]', false);
        show('[data-changes-empty]', false);
      }
      const data = await fetchJSON('/v1/staff/dashboard');
      if (!silent) {
        show('[data-metrics-loading]', false);
        show('[data-changes-loading]', false);
      }
      if (!data || !data.queue) {
        if (!silent) show('[data-staff-error]', true);
        return;
      }
      /* тихое обновление удалось — ошибка (если показывалась) снята */
      show('[data-staff-error]', false);
      QUEUE_KEYS.forEach((k) => {
        const n = document.querySelector('[data-queue-count="' + k + '"]');
        if (n) n.textContent = String(data.queue[k] || 0);
      });
      if (!silent) show('[data-metrics-body]', true);
      const changes = Array.isArray(data.recent_changes) ? data.recent_changes : [];
      const list = document.querySelector('[data-changes-body]');
      show('[data-changes-empty]', false);
      if (list) {
        list.replaceChildren();
        if (!changes.length) {
          list.hidden = true;
          show('[data-changes-empty]', true);
        } else {
          changes.forEach((e) => list.append(changeRow(e)));
          list.hidden = false;
        }
      }
      fillEventSelect(changes);
    };
    const summaryRetry = document.querySelector('[data-staff-retry]');
    if (summaryRetry) summaryRetry.addEventListener('click', () => loadSummary(false));

    /* --- очередь: чипы-фильтры + карточки (GET /v1/staff/applications) -- */
    const filters = { status: '', type: '', overdue: false };
    document.querySelectorAll('[data-queue-chip]').forEach((chip) => {
      chip.addEventListener('click', () => {
        const f = chip.dataset.filter;
        if (f === 'overdue') {
          filters.overdue = !filters.overdue;
          chip.setAttribute('aria-pressed', String(filters.overdue));
        } else {
          filters[f] = chip.dataset.value || '';
          document.querySelectorAll('[data-queue-chip][data-filter="' + f + '"]').forEach((c) => {
            c.setAttribute('aria-pressed', String(c === chip));
          });
        }
        loadQueue();
      });
    });
    const hasFilter = () => Boolean(filters.status || filters.type || filters.overdue);
    /* клиентская сборка карточки — ТЕ ЖЕ классы и data-атрибуты, что в
       серверном фрагменте views/staff/app_card.csp (после смены статуса
       карточку заменит фрагмент: контракт — классы) */
    const appCard = (a) => {
      const card = el('article', 'app-card');
      card.dataset.appCard = '';
      card.dataset.appId = String(a.id);
      card.dataset.status = a.status || '';
      const head = el('div', 'row wrap');
      head.append(el('h3', null, a.type_title || a.type || 'Заявка'));
      head.append(el('span', 'badge ' + (BADGE[a.status] || ''), APP_STATUS[a.status] || a.status || ''));
      if (a.overdue) head.append(el('span', 'badge danger', 'Срок горит'));
      card.append(head);
      const fio = a.owner && a.owner.fio ? a.owner.fio : '—';
      const group = a.owner && a.owner.group ? a.owner.group : '';
      card.append(el('p', 'caption', fio + (group ? ' · группа ' + group : '')));
      const upd = el('p', 'caption muted');
      upd.append('обновлено ');
      upd.append(el('span', 'tnum', stampOf(a.updated_at)));
      card.append(upd);
      const warn = el('p', 'field-error');
      warn.hidden = true;
      warn.setAttribute('aria-live', 'polite');
      warn.setAttribute('data-app-warn', '');
      card.append(warn);
      const tos = TRANSITIONS[a.status] || [];
      if (!tos.length) {
        card.append(el('p', 'caption muted staff-actions-done', 'Дальнейших шагов нет — заявка завершена.'));
      } else {
        const actions = el('div', 'actions staff-actions');
        tos.forEach((to) => {
          const b = el('button', 'button', APP_STATUS[to] || to);
          b.type = 'button';
          b.setAttribute('data-app-status-btn', '');
          b.setAttribute('data-to-status', to);
          actions.append(b);
        });
        card.append(actions);
      }
      return card;
    };
    const setWarn = (card, text) => {
      const warn = card ? card.querySelector('[data-app-warn]') : null;
      if (!warn) return;
      warn.textContent = text;
      warn.hidden = !text;
    };
    const loadQueue = async () => {
      show('[data-queue-loading]', true);
      show('[data-queue-error]', false);
      show('[data-queue-empty]', false);
      const body = document.querySelector('[data-queue-body]');
      if (body) { body.hidden = true; body.replaceChildren(); }
      const qs = new URLSearchParams();
      if (filters.status) qs.set('status', filters.status);
      if (filters.type) qs.set('type', filters.type);
      if (filters.overdue) qs.set('overdue', '1');
      const url = '/v1/staff/applications' + (qs.toString() ? '?' + qs.toString() : '');
      const data = await fetchJSON(url);
      show('[data-queue-loading]', false);
      if (!data || !Array.isArray(data.applications)) {
        show('[data-queue-error]', true);
        return;
      }
      if (!data.applications.length) {
        const t = document.querySelector('[data-queue-empty-text]');
        if (t) t.textContent = hasFilter() ? 'По этому фильтру пока ничего не было.' : 'Новые заявки появятся здесь автоматически.';
        show('[data-queue-empty]', true);
        return;
      }
      if (!body) return;
      data.applications.forEach((a) => body.append(appCard(a)));
      body.hidden = false;
    };
    const queueRetry = document.querySelector('[data-queue-retry]');
    if (queueRetry) queueRetry.addEventListener('click', () => loadQueue());

    /* --- смена статуса: POST API → htmx-перерисовка фрагментом ---------- */
    const redrawCard = (id, card) => {
      /* данные уже изменены (200 от API); карточку заменяет серверный
         фрагмент GET /staff/app-card (HX-Request автоматически) — свежие
         статус/владелец/«Срок горит» и кнопки новой матрицы. Сбой перери-
         совки (сеть) — перестраиваем очередь целиком. */
      if (window.htmx && card && card.isConnected) {
        htmx.ajax('GET', '/staff/app-card?id=' + encodeURIComponent(id), { target: card, swap: 'outerHTML' })
          .catch(() => loadQueue());
        return;
      }
      loadQueue();
    };
    document.addEventListener('click', async (event) => {
      const btn = event.target.closest('[data-app-status-btn]');
      if (!btn || btn.disabled) return;
      const card = btn.closest('[data-app-card]');
      if (!card) return;
      if (!navigator.onLine) { applyOffline(true); return; }
      const id = card.dataset.appId;
      const to = btn.dataset.toStatus;
      if (!id || !to) return;
      setWarn(card, '');
      busy(btn, true, 'Меняем…');
      const r = await postJSON('/v1/staff/applications/' + encodeURIComponent(id) + '/status', { to_status: to });
      busy(btn, false);
      if (r && r.ok) {
        redrawCard(id, card);
        loadSummary(true); /* счётчики и «свежие изменения» — молча */
        return;
      }
      if (r === null) {
        setWarn(card, 'Не получилось сменить статус — проверьте интернет');
        return;
      }
      const message = errorMessage(r) || 'Ошибка ' + r.status;
      if (r.status >= 500) toast(message);
      /* 409 «Недопустимый переход статуса заявки» (и 400/404) — inline в
          карточке, без перерисовки (постановка экрана) */
      setWarn(card, message);
    });

    /* --- перенос пары (POST /v1/staff/schedule/changes) ----------------- */
    const changeForm = document.querySelector('[data-change-form]');
    if (changeForm) {
      const sel = changeForm.querySelector('[data-change-event]');
      const manualId = changeForm.querySelector('[data-change-event-id]');
      const actionSel = changeForm.querySelector('[data-change-action]');
      const moveFields = changeForm.querySelector('[data-change-move-fields]');
      const startsInput = changeForm.querySelector('[data-change-starts]');
      const roomInput = changeForm.querySelector('[data-change-room]');
      const reasonInput = changeForm.querySelector('[data-change-reason]');
      const errBox = changeForm.querySelector('[data-change-error]');
      const resultBox = changeForm.querySelector('[data-change-result]');
      const submitBtn = changeForm.querySelector('button[type="submit"]');
      const showErr = (text) => { if (errBox) { errBox.textContent = text; errBox.hidden = !text; } };
      /* выбор события — подставляем его ТЕКУЩИЕ время/аудиторию как
         заготовку «стало» (черновик staff.html: правка конкретной пары) */
      sel.addEventListener('change', () => {
        const o = sel.selectedOptions[0];
        if (!o || !o.value) return;
        if (startsInput && o.dataset.starts && o.dataset.starts.length >= 16) {
          startsInput.value = o.dataset.starts.slice(0, 16);
        }
        if (roomInput && o.dataset.room) roomInput.value = o.dataset.room;
      });
      actionSel.addEventListener('change', () => {
        if (moveFields) moveFields.hidden = actionSel.value !== 'move';
      });
      changeForm.addEventListener('submit', async (event) => {
        event.preventDefault();
        if (submitBtn && submitBtn.disabled) return;
        if (!navigator.onLine) { applyOffline(true); return; }
        showErr('');
        if (resultBox) resultBox.hidden = true;
        const rawId = manualId && manualId.value.trim() ? manualId.value.trim() : (sel ? sel.value : '');
        if (!/^[1-9][0-9]*$/.test(rawId)) {
          showErr('Выберите событие из списка или укажите его id');
          return;
        }
        const action = actionSel ? actionSel.value : 'move';
        const payload = { event_id: Number(rawId), action, reason: reasonInput ? reasonInput.value.trim() : '' };
        if (action === 'move') {
          const dt = startsInput ? startsInput.value : '';
          const room = roomInput ? roomInput.value.trim() : '';
          if (!dt) { showErr('Укажите новые дату и время'); return; }
          if (!room) { showErr('Укажите новую аудиторию'); return; }
          /* datetime-local (минуты) → ISO «…Z» 02 §5.2 */
          payload.starts_at = dt.length === 16 ? dt + ':00Z' : dt + 'Z';
          payload.room = room;
        }
        busy(submitBtn, true, 'Сохраняем…');
        const r = await postJSON('/v1/staff/schedule/changes', payload);
        busy(submitBtn, false);
        if (r && r.ok) {
          const n = r.body && typeof r.body.recipients !== 'undefined' ? Number(r.body.recipients) || 0 : null;
          if (resultBox) {
            resultBox.textContent = n === null
              ? 'Перенос сохранён.'
              : 'Уведомлено ' + n + ' ' + plural(n, 'студент', 'студента', 'студентов') + ' о переносе.';
            resultBox.hidden = false;
          }
          toast('Изменение расписания сохранено');
          changeForm.reset();
          if (moveFields) moveFields.hidden = false;
          loadSummary(true); /* счётчики и «свежие изменения» — молча */
          return;
        }
        if (r === null) {
          showErr('Не получилось сохранить перенос — проверьте интернет');
          return;
        }
        const message = errorMessage(r) || 'Ошибка ' + r.status;
        if (r.status >= 500) toast(message);
        showErr(message);
      });
    }

    /* --- объявления и новости (POST /v1/staff/announcements — kind
       'announcement' по умолчанию / 'news' 5.17; лента GET
       /v1/announcements?cursor= — лента объявлений, новости живут в
       сегменте «Новости» экрана уведомлений) ------------------------------ */
    const announceForm = document.querySelector('[data-announce-form]');
    const announceTitle = document.querySelector('[data-announce-title]');
    const announceBody = document.querySelector('[data-announce-body]');
    const announceGroup = document.querySelector('[data-announce-group]');
    const announceErr = document.querySelector('[data-announce-error]');
    const announceNewsFields = document.querySelector('[data-announce-news-fields]');
    const announceCategory = document.querySelector('[data-announce-category]');
    const announceEvent = document.querySelector('[data-announce-event]');
    const announceLocation = document.querySelector('[data-announce-location]');
    const showAnnounceErr = (text) => { if (announceErr) { announceErr.textContent = text; announceErr.hidden = !text; } };
    if (announceForm) {
      /* «Тип записи» (5.17): kind=news показывает категорию/дату/место
         события и меняет подпись кнопки; сброс формы (reset/успех)
         возвращает тип «Объявление» — syncKindUI приводит UI к радиокнопке */
      const announceKind = () => {
        const checked = announceForm.querySelector('input[name="announce-kind"]:checked');
        return checked ? checked.value : 'announcement';
      };
      const syncKindUI = () => {
        const isNews = announceKind() === 'news';
        if (announceNewsFields) announceNewsFields.hidden = !isNews;
        const btn = announceForm.querySelector('button[type="submit"]');
        if (btn) btn.textContent = isNews ? 'Опубликовать новость' : 'Опубликовать объявление';
      };
      announceForm.querySelectorAll('input[name="announce-kind"]').forEach((radio) => {
        radio.addEventListener('change', syncKindUI);
      });
      /* радио «Группе» — включаем поле номера группы */
      announceForm.querySelectorAll('input[name="announce-audience"]').forEach((radio) => {
        radio.addEventListener('change', () => {
          const checked = announceForm.querySelector('input[name="announce-audience"]:checked');
          if (announceGroup) announceGroup.disabled = !(checked && checked.value === 'group');
        });
      });
      announceForm.addEventListener('submit', async (event) => {
        event.preventDefault();
        const btn = announceForm.querySelector('button[type="submit"]');
        if (btn && btn.disabled) return;
        if (!navigator.onLine) { applyOffline(true); return; }
        showAnnounceErr('');
        const title = announceTitle ? announceTitle.value.trim() : '';
        const body = announceBody ? announceBody.value.trim() : '';
        if (!title) { showAnnounceErr('Напишите заголовок'); return; }
        if (!body) { showAnnounceErr('Напишите текст объявления'); return; }
        const payload = { title, body };
        const mode = announceForm.querySelector('input[name="announce-audience"]:checked');
        if (mode && mode.value === 'group') {
          const g = announceGroup ? announceGroup.value.trim() : '';
          if (!/^[1-9][0-9]*$/.test(g)) { showAnnounceErr('Укажите номер группы — число от 1'); return; }
          payload.audience = ['group:' + Number(g)];
        }
        if (announceKind() === 'news') {
          /* kind=news (5.17): категория — словарь CHECK 0007, дата
             события — datetime-local «ГГГГ-ММ-ДДTЧЧ:ММ» → строгий
             «…TЧЧ:ММ:SSZ» контракта NewsService (проект — UTC), место —
             необязательная строка; публикация попадёт в таблицу news */
          const category = announceCategory ? announceCategory.value : '';
          if (!category) { showAnnounceErr('Выберите категорию новости'); return; }
          payload.kind = 'news';
          payload.category = category;
          const ev = announceEvent ? announceEvent.value : '';
          if (ev) payload.event_at = ev.length === 16 ? ev + ':00Z' : ev;
          const location = announceLocation ? announceLocation.value.trim() : '';
          if (location) payload.location = location;
        }
        busy(btn, true, 'Публикуем…');
        const r = await postJSON('/v1/staff/announcements', payload);
        busy(btn, false);
        if (r && r.ok) {
          /* объявления и новости БЕЗ push и БЕЗ уведомлений — только
             лента (01 §4.5; подпись об этом стоит в форме); новость — в
             сегмент «Новости» экрана уведомлений, НЕ в ленту объявлений */
          toast(announceKind() === 'news'
            ? 'Новость опубликована — она появится в ленте «Новости»'
            : 'Объявление опубликовано — оно появится только в ленте');
          announceForm.reset();
          if (announceGroup) announceGroup.disabled = true;
          syncKindUI();
          loadFeed(true);
          return;
        }
        if (r === null) { showAnnounceErr('Не получилось опубликовать — проверьте интернет'); return; }
        const message = errorMessage(r) || 'Ошибка ' + r.status;
        if (r.status >= 500) toast(message);
        showAnnounceErr(message);
      });
      syncKindUI();
    }
    let feedCursor = null;
    const audienceLabel = (a) => {
      if (a === 'all') return 'для всех';
      if (Array.isArray(a) && a.length) {
        const items = a.map((x) => typeof x === 'string' && x.indexOf('group:') === 0
          ? 'группа ' + x.slice(6) : String(x));
        return 'для ' + items.join(', ');
      }
      return '';
    };
    const renderFeed = (items, append) => {
      const body = document.querySelector('[data-feed-body]');
      if (!body) return;
      if (!append) body.replaceChildren();
      items.forEach((n) => {
        const li = el('li', 'staff-feed-item');
        li.append(el('strong', null, n.title || ''));
        li.append(el('p', 'small', n.body || ''));
        const cap = [n.author, stampOf(n.published_at), audienceLabel(n.audience)].filter(Boolean).join(' · ');
        if (cap) li.append(el('p', 'caption muted', cap));
        body.append(li);
      });
      body.hidden = items.length === 0 && !append;
    };
    const loadFeed = async (reset) => {
      if (reset) feedCursor = null;
      show('[data-feed-loading]', true);
      show('[data-feed-error]', false);
      if (reset) show('[data-feed-empty]', false);
      const url = '/v1/announcements' + (feedCursor ? '?cursor=' + encodeURIComponent(feedCursor) : '');
      const data = await fetchJSON(url);
      show('[data-feed-loading]', false);
      if (!data || !Array.isArray(data.announcements)) {
        show('[data-feed-error]', true);
        return;
      }
      renderFeed(data.announcements, !reset);
      if (reset && !data.announcements.length) show('[data-feed-empty]', true);
      feedCursor = data.next_cursor || null;
      const more = document.querySelector('[data-feed-more]');
      if (more) more.hidden = !feedCursor;
    };
    const feedRetry = document.querySelector('[data-feed-retry]');
    if (feedRetry) feedRetry.addEventListener('click', () => loadFeed(true));
    const feedMore = document.querySelector('[data-feed-more]');
    if (feedMore) feedMore.addEventListener('click', () => loadFeed(false));

    loadSummary(false);
    loadQueue();
    loadFeed(true);
  }
})();
