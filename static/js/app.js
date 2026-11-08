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
          ? 'Нет сети. Показываем данные на момент ' + lastSyncAt
          : 'Нет сети. Проверьте подключение — и попробуйте ещё раз.';
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
        el.title = 'Нужна сеть для этого действия';
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
        toast('Сеть вернулась — данные обновлены');
      }
    } catch (e) { /* сети нет — баннер остаётся, пользователь может повторить */ }
    probing = false;
  };
  if (banner) banner.addEventListener('click', retryNetwork);
  window.addEventListener('offline', () => applyOffline(true));
  window.addEventListener('online', () => {
    applyOffline(false);
    refreshUnreadBadge();
    toast('Сеть вернулась — данные обновлены');
  });
  applyOffline(!navigator.onLine);

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
      if (bellLink) bellLink.setAttribute('aria-label', 'Уведомления, непрочитанных: ' + label);
    } else {
      badge.hidden = true;
      if (bellLink) bellLink.setAttribute('aria-label', 'Уведомления');
    }
  };
  const refreshUnreadBadge = async () => {
    if (!badge) return;
    const data = await fetchJSON('/v1/notifications?unread=1');
    if (data && Array.isArray(data.notifications)) renderBadge(data.notifications.length);
  };
  refreshUnreadBadge();

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
        submitButton.textContent = 'Входим…';
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
     grades-discipline.csp; 03 §4.4/§4.9.3) ---------- */
  /* Контент — SSR (тела GradesService::summary/list/exams рендерит
     сервер); догрузки на клиенте нет — скелетон не нужен (03 §5.1),
     ошибку данных показывает сам шаблон (failed=1, состояние 03 §5.3).
     Здесь только «Повторить»: реально перезапрашивает — перезагрузка
     страницы; офлайн обслуживает баннер каркаса (topbar + applyOffline,
     03 §5.4). */
  if (document.body.dataset.page === 'grades' || document.body.dataset.page === 'discipline') {
    const gradesRetry = document.querySelector('[data-grades-retry]');
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
      greetingEl.textContent = hour < 12 ? 'Доброе утро' : hour < 18 ? 'Добрый день' : 'Добрый вечер';
    }
    const dateEl = $$('[data-today-date]');
    if (dateEl) {
      dateEl.textContent = new Date().toLocaleDateString('ru-RU', { weekday: 'long', day: 'numeric', month: 'long' });
    }

    /* словари-константы словарей CHECK DDL (0002/0003/0004) — метки FE */
    const LESSON_KIND = { lecture: 'Лекция', seminar: 'Семинар' };
    const EXAM_KIND = { consultation: 'Консультация', exam: 'Экзамен', credit: 'Зачёт', retake: 'Пересдача' };
    const APP_STATUS = {
      draft: 'Черновик', submitted: 'Подана', accepted: 'Принята',
      in_progress: 'В работе', need_info: 'Требуется уточнение',
      ready: 'Готова', issued: 'Выдана', rejected: 'Отклонена', cancelled: 'Отменена'
    };
    const NEWS_TYPE = { schedule: 'Расписание', deadline: 'Дедлайн', application: 'Заявка', announcement: 'Объявление' };

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
      if (diff === 0) return 'сегодня';
      if (diff === 1) return 'завтра';
      return new Date(iso).toLocaleDateString('ru-RU', { day: 'numeric', month: 'long' });
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
        meta.textContent = 'идёт сейчас · до ' + timeOf(next.ends_at);
        return;
      }
      const mins = minutesUntil(next.starts_at);
      meta.textContent = mins >= 1
        ? 'через ' + mins + ' ' + plural(mins, 'минуту', 'минуты', 'минут')
        : 'меньше минуты';
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
      if (failed) {
        body.hidden = true;
        if (title) title.textContent = 'Сегодня';
        return;
      }
      if (title) {
        title.textContent = events.length
          ? 'Сегодня · ' + events.length + ' ' + plural(events.length, 'пара', 'пары', 'пар')
          : 'Сегодня';
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
        const link = el('a', null, a.type_title || a.type || 'Заявка');
        link.href = '/applications/' + a.id;
        const status = APP_STATUS[a.status] || a.status || '';
        const stamp = shortStamp(a.updated_at);
        row.append(link, el('p', 'caption', status + (stamp ? ' · ' + stamp : '')));
        body.append(row);
      });
    };

    /* --- виджет «Что нового» → /notifications; счётчик «· N»; ночью
       заголовок — «За ночь: N событий» (03 §4.2) ------------------------- */
    const renderNews = (items, failed, unreadCount) => {
      show('[data-news-loading]', false);
      show('[data-news-error]', Boolean(failed));
      const body = $$('[data-news-body]');
      const title = $$('[data-news-title]');
      if (!body) return;
      const nightMode = (() => { const h = new Date().getHours(); return h >= 23 || h < 6; })();
      if (title) {
        title.textContent = nightMode
          ? 'За ночь: ' + (unreadCount || 0) + ' ' + plural(unreadCount || 0, 'событие', 'события', 'событий')
          : (items.length ? 'Что нового · ' + items.length : 'Что нового');
      }
      if (failed) { body.hidden = true; return; }
      if (!items.length) { body.hidden = true; show('[data-news-empty]', true); return; }
      show('[data-news-empty]', false);
      body.hidden = false;
      body.replaceChildren();
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
        if (r.status === 'moved') detail.append(' ', el('span', 'badge', 'Перенос'));
        if (r.status === 'cancelled') detail.append(' ', el('span', 'badge danger', 'Отменена'));
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
      if (newsTitle) newsTitle.textContent = 'Что нового';
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
      renderNews(Array.isArray(data.news) ? data.news : [], hit('news'), data.unread_count);
      await applyDayMode();
    };
    const retry = $$('[data-dashboard-retry]');
    if (retry) retry.addEventListener('click', () => { loadDashboard(); });
    /* компактные «Повторить» виджетов — перезапрашивают весь агрегат
       (03 §5.3: «кнопка „Повторить“ всегда реально перезапрашивает») */
    document.querySelectorAll('[data-widget-retry]').forEach((b) => b.addEventListener('click', () => { loadDashboard(); }));
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
    const KIND = { lecture: 'Лекция', seminar: 'Семинар', practice: 'Практика' };
    const STATUS = { normal: 'По расписанию', moved: 'Перенесена', cancelled: 'Отменена', substituted: 'Замена' };

    /* --- sheet «Детали пары» (03 §4.3: тап по паре → bottom-sheet;
       <dialog> в views/schedule.csp, закрытие — кнопка/Esc). Имя
       преподавателя — GET /v1/people/{teacher_id+1000} (Э6а, Р-15);
       страница /people/{id} — следующий экран Э7, ссылка уже ведёт
       туда; без teacher_id (или если профиль не отвечает) — ссылка
       «Открыть профиль» скрыта/нейтральна, «—» в строке. --- */
    const setField = (sel, text) => { const n = sheet ? sheet.querySelector(sel) : null; if (n) n.textContent = text; };
    const showRow = (sel, on) => { const n = sheet ? sheet.querySelector(sel) : null; if (n) n.hidden = !on; };
    const openSheet = async (card) => {
      if (!sheet) return;
      const subject = card.querySelector('.lesson-subject');
      setField('[data-sheet-title]', subject ? subject.textContent : 'Пара');
      setField('[data-sheet-kind]', KIND[card.dataset.kind] || '');
      setField('[data-sheet-when]', [card.dataset.date, card.dataset.start, card.dataset.end].filter(Boolean).join(' · '));
      setField('[data-sheet-room]', 'аудитория ' + (card.dataset.room || '—'));
      const changed = [];
      if (card.dataset.prevRoom) changed.push('аудитория ' + card.dataset.prevRoom + ' → ' + (card.dataset.room || '—'));
      if (card.dataset.prevStart && card.dataset.prevStart !== card.dataset.start) changed.push('время ' + card.dataset.prevStart + ' → ' + card.dataset.start);
      showRow('[data-sheet-change-row]', changed.length > 0);
      if (changed.length) setField('[data-sheet-change]', 'было → стало: ' + changed.join(' · '));
      setField('[data-sheet-status]', STATUS[card.dataset.status] || card.dataset.status || '');
      const link = sheet.querySelector('[data-sheet-teacher]');
      const fallback = sheet.querySelector('[data-sheet-teacher-empty]');
      const teacherId = parseInt(card.dataset.teacherId || '', 10);
      if (link && Number.isFinite(teacherId)) {
        const href = '/people/' + (teacherId + PEOPLE_ID_OFFSET);
        link.href = href;
        link.textContent = 'Загрузка…';
        link.hidden = false;
        if (fallback) fallback.hidden = true;
        const person = await fetchJSON('/v1/people/' + (teacherId + PEOPLE_ID_OFFSET));
        link.textContent = person && person.person && person.person.fio ? person.person.fio : 'Открыть профиль';
      } else if (link && fallback) {
        link.hidden = true;
        fallback.hidden = false;
      }
      sheet.showModal();
    };
    /* делегирование: контейнер сегмента перерисовывается htmx —
       слушатель на документе переживает обмен */
    document.addEventListener('click', (event) => {
      const card = event.target.closest('[data-event]');
      if (card && sheet) {
        event.preventDefault();
        openSheet(card);
      }
    });
    const sheetClose = sheet ? sheet.querySelector('[data-sheet-close]') : null;
    if (sheetClose) sheetClose.addEventListener('click', () => sheet.close());

    /* --- состояния сегмента (03 §5.3/§5.4) ---------------------------- */
    /* Ошибка (ответ не 2xx или сеть) — блок «Не получилось загрузить
       расписание» с «Повторить», перезапускающим запрос исходного
       элемента (03 §5.3: «Повторить» всегда реально перезапрашивает);
       офлайн — баннер каркаса; истёкшая сессия (фильтр отвечает 303 →
       /login, XHR следует редиректу) — браузерный переход на вход. */
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
        if (retryElt) retryElt.click();
        else window.location.reload();
      });
    }

    /* --- сегмент «Неделя» (03 §4.3): автоскролл к сегодняшнему дню и
       свайп влево/вправо = ‹/› неделя. Скролл — к .day-group.is-today,
       только если он вне вьюпорта; при reduced-motion — без анимации.
       Свайп — тап-жест по контейнеру сегмента: |dX| ≥ 56px и горизонтальнее
       вертикали, иначе это скролл страницы; кнопки-стрелки сами disabled
       на границе окна [-4..4].
       ВАЖНО: контейнер #schedule-segment свапается htmx ЦЕЛИКОМ
       (hx-swap="outerHTML" в партиалах), поэтому ссылку на него нельзя
       кэшировать в замыкании — после первого же обмена она указывала бы
       на отсоединённый узел (свайп умер бы, автоскролл искал бы в старом
       дереве). getElementById перезапрашивается в каждом обработчике. --- */
    const reducedMotion = window.matchMedia('(prefers-reduced-motion: reduce)').matches;
    const scrollTodayIntoView = () => {
      const segBox = document.getElementById('schedule-segment');
      const todayGroup = segBox ? segBox.querySelector('.day-group.is-today') : null;
      if (!todayGroup) return;
      const r = todayGroup.getBoundingClientRect();
      if (r.top >= 0 && r.bottom <= window.innerHeight) return;
      todayGroup.scrollIntoView({ block: 'start', behavior: reducedMotion ? 'auto' : 'smooth' });
    };
    scrollTodayIntoView();
    document.body.addEventListener('htmx:afterSwap', () => {
      setTimeout(scrollTodayIntoView, 0);
    });
    let touchX = 0, touchY = 0, swipeArmed = false;
    document.addEventListener('touchstart', (e) => {
      const segBox = document.getElementById('schedule-segment');
      swipeArmed = Boolean(segBox) && segBox.contains(e.target) && !(sheet && sheet.open);
      if (!swipeArmed) return;
      const t = e.touches[0];
      touchX = t.clientX;
      touchY = t.clientY;
    }, { passive: true });
    document.addEventListener('touchend', (e) => {
      if (!swipeArmed) return;
      swipeArmed = false;
      const segBox = document.getElementById('schedule-segment');
      if (!segBox) return;
      const t = e.changedTouches[0];
      const dX = t.clientX - touchX;
      const dY = t.clientY - touchY;
      if (Math.abs(dX) < 56 || Math.abs(dY) >= Math.abs(dX)) return;
      const arrow = segBox.querySelector(dX < 0
        ? '[aria-label="Следующая неделя"]'
        : '[aria-label="Предыдущая неделя"]');
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
     «Отправить заявку» → POST {submit:true} → редирект на карточку;
     ошибка — inline «Заявка не отправилась…» (03 §5.3). Автосохранённый
     черновик при подаче остаётся в списке черновиков — у студента нет
     эндпоинта submit существующей строки (02 §5.2: submit — только при
     POST-создании) — решение задокументировано в журнале сессии.
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
        btn.textContent = busyLabel || 'Отправляем…';
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
        busy(submitBtn, true, 'Отправляем…');
        try {
          if (state.draftId && !state.draftStopped) await autosaveNow(); /* синк лучшего усилия */
          const resp = await fetch('/v1/applications', {
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
        replyBtn.textContent = 'Отправляем…';
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
          let message = 'Не получилось забронировать';
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
        else showSlotWarn('Не получилось забронировать. Проверьте интернет — и попробуйте ещё раз');
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
            let message = 'Не получилось отменить бронь';
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
          else showBookingsWarn('Не получилось отменить бронь. Проверьте интернет — и попробуйте ещё раз');
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
})();
