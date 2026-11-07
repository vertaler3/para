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
})();
