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
  const toastEl = $('#toast');
  let toastTimer = 0;
  const toast = (text) => {
    if (!toastEl) return;
    toastEl.textContent = text;
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
})();
