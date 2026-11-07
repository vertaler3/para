/* Только локальный демонстрационный интерактив. Сетевых запросов нет. */
(() => {
 const $ = s => document.querySelector(s);
 const $$ = s => [...document.querySelectorAll(s)];
 const read = (k, fallback = '') => {try{return localStorage.getItem('para-draft-'+k)||fallback}catch{return fallback}};
 const save = (k,v) => {try{localStorage.setItem('para-draft-'+k,v)}catch{}};
 const theme = read('theme','system');
 if(theme !== 'system') document.documentElement.dataset.theme=theme;
 const toast = message => {const t=$('#toast');if(!t)return;t.textContent=message;t.hidden=false;setTimeout(()=>t.hidden=true,5000)};
 $$('[data-theme-choice]').forEach(b=>{b.setAttribute('aria-pressed',String(b.dataset.themeChoice===theme));b.addEventListener('click',()=>{save('theme',b.dataset.themeChoice);if(b.dataset.themeChoice==='system')delete document.documentElement.dataset.theme;else document.documentElement.dataset.theme=b.dataset.themeChoice;$$('[data-theme-choice]').forEach(x=>x.setAttribute('aria-pressed',String(x===b)));toast('Тема сохранена на этом устройстве')})});
 $$('[data-demo-login]').forEach(b=>b.addEventListener('click',()=>{$('#login').value=b.dataset.demoLogin;$('#password').focus();toast('Логин подставлен. В прототипе подойдёт любой непустой пароль.')}));
 $('[data-password-toggle]')?.addEventListener('click',e=>{const p=$('#password');p.type=p.type==='password'?'text':'password';e.currentTarget.setAttribute('aria-label',p.type==='password'?'Показать пароль':'Скрыть пароль');e.currentTarget.setAttribute('aria-pressed',String(p.type==='text'))});
 $('#login-form')?.addEventListener('submit',e=>{e.preventDefault();const login=$('#login').value.trim();if(!['anya.demo','mark.demo','dean.demo'].includes(login)){$('#login-error').hidden=false;$('#password').focus();return}save('account',login);location.href=login==='dean.demo'?'staff.html':'dashboard.html'});
 const account=read('account','anya.demo');
 $$('[data-account-name]').forEach(x=>x.textContent=account==='mark.demo'?'Марк Ливанов':account==='dean.demo'?'dean.demo':'Аня Соколова');
 $$('[data-account-short]').forEach(x=>x.textContent=account==='mark.demo'?'Марк':account==='dean.demo'?'dean.demo':'Аня');
 $$('[data-account-meta]').forEach(x=>x.textContent=account==='mark.demo'?'4 курс · МЭО · группа 4-МЭО':'1 курс · ФМО · группа 1-ФМО');
 $$('[data-profile-avatar]').forEach(x=>x.textContent=account==='mark.demo'?'МЛ':'АС');
 if($('#anya-grades')&&$('#mark-grades')){const filled=new URLSearchParams(location.search).get('demo')==='mark'||account==='mark.demo';$('#anya-grades').hidden=filled;$('#mark-grades').hidden=!filled}
 $$('[data-select-account]').forEach(b=>b.addEventListener('click',()=>{save('account',b.dataset.selectAccount);location.reload()}));
 $$('[data-open-dialog]').forEach(b=>b.addEventListener('click',()=>$('#'+b.dataset.openDialog).showModal()));
 $$('[data-close-dialog]').forEach(b=>b.addEventListener('click',()=>b.closest('dialog').close()));
 $('#logout-confirm')?.addEventListener('click',()=>{save('account','anya.demo');location.href='login.html'});
 $$('[data-setting]').forEach(x=>{const v=read(x.dataset.setting);if(v)x.checked=v==='true';x.addEventListener('change',()=>{save(x.dataset.setting,String(x.checked));toast('Настройка сохранена в прототипе')})});
 const query=new URLSearchParams(location.search);
 const type=query.get('type')||'Академическая справка';
 $$('[data-application-type]').forEach(x=>x.textContent=type);
 const form=$('#application-form');
 if(form){const key='draft-'+type;try{const v=JSON.parse(read(key,'{}'));if(v.copies)form.elements.copies.value=v.copies;if(v.delivery)form.elements.delivery.value=v.delivery;if(v.comment)form.elements.comment.value=v.comment;if(v.copies)$('#draft-restored').hidden=false}catch{}const persist=()=>{save(key,JSON.stringify(Object.fromEntries(new FormData(form))));$('#draft-status').textContent='Черновик сохранён на этом устройстве'};form.addEventListener('input',persist);setInterval(persist,10000);form.addEventListener('submit',e=>{e.preventDefault();persist();save('submitted',JSON.stringify({type,...Object.fromEntries(new FormData(form))}));location.href='application-success.html?type='+encodeURIComponent(type)})}
 const stages=['Подана','Принята','В работе','Готова','Выдана'];
 $('#staff-status-form')?.addEventListener('submit',e=>{e.preventDefault();const status=$('#staff-status').value;save('application-status',status);$('#staff-status-result').textContent='Демо-статус заявки З-2026-0142: '+status;toast('Статус изменён в локальном прототипе')});
 const savedStatus=read('application-status');
 if(savedStatus&&$('#application-current-status')){$('#application-current-status').textContent=savedStatus;const index=stages.indexOf(savedStatus);$$('[data-stage]').forEach((x,i)=>x.className=i<index?'done':i===index?'current':'future')}
 $('#reply-form')?.addEventListener('submit',e=>{e.preventDefault();$('#reply-result').textContent='Ответ: '+$('#reply').value+'. Заявка возвращена в работу.';save('application-status','В работе');toast('Ответ сохранён. Заявка в работе.');$('#reply-form').hidden=true});
 $$('[data-notification]').forEach(x=>{if(read('read-'+x.dataset.notification))x.querySelector('.unread')?.setAttribute('hidden','');x.addEventListener('click',()=>save('read-'+x.dataset.notification,'true'))});
 $('#read-all')?.addEventListener('click',()=>{$$('[data-notification]').forEach(x=>{save('read-'+x.dataset.notification,'true');x.querySelector('.unread')?.setAttribute('hidden','')});toast('Все уведомления прочитаны')});
 $$('[data-notification-filter]').forEach(b=>b.addEventListener('click',()=>{$$('[data-notification-filter]').forEach(x=>x.setAttribute('aria-pressed',String(x===b)));let count=0;$$('[data-notification]').forEach(x=>{x.hidden=b.dataset.notificationFilter!=='all'&&x.dataset.kind!==b.dataset.notificationFilter;if(!x.hidden)count++});$('#notifications-empty').hidden=count>0}));
 let selectedSlot='';let room=$('#room')?.value||'1207';
 const bookings=()=>{try{return JSON.parse(read('bookings','[]'))}catch{return []}};
 const paintRooms=()=>{$$('[data-slot]').forEach(b=>{const hour=Number(b.dataset.slot);const mine=(room==='3105'&&hour===14)||bookings().some(x=>x.room===room&&x.hour===hour);const busy=room==='1207'&&hour===11;b.disabled=busy;b.classList.toggle('mine',mine);b.dataset.mine=String(mine);b.setAttribute('aria-pressed',String(selectedSlot===String(hour)));b.innerHTML=`<strong>${hour}:00–${hour+1}:00</strong><span>${busy?'Занято':mine?'Ваша бронь':'Свободно'}</span>`});if($('#book-slot')){$('#book-slot').disabled=!selectedSlot;$('#book-slot').textContent=selectedSlot?`Забронировать ${selectedSlot}:00–${Number(selectedSlot)+1}:00`:'Выберите свободный слот'}const list=$('#my-bookings');if(list){const b=[{room:'3105',hour:14},...bookings()];list.innerHTML=b.map(x=>`<div class="list-row"><div><strong>Ауд. ${x.room}</strong><p>6 ноября · ${x.hour}:00–${x.hour+1}:00</p></div><button class="button" data-cancel-booking="${x.room}|${x.hour}">Отменить</button></div>`).join('');list.querySelectorAll('[data-cancel-booking]').forEach(b=>b.addEventListener('click',()=>{const [r,h]=b.dataset.cancelBooking.split('|');$('#cancel-booking').dataset.room=r;$('#cancel-booking').dataset.hour=h;$('#cancel-dialog').showModal()}))}};
 $$('[data-slot]').forEach(b=>b.addEventListener('click',()=>{if(b.dataset.mine==='true'){$('#cancel-booking').dataset.room=room;$('#cancel-booking').dataset.hour=b.dataset.slot;$('#cancel-dialog').showModal();return}selectedSlot=b.dataset.slot;paintRooms()}));
 $('#room')?.addEventListener('change',()=>{room=$('#room').value;selectedSlot='';paintRooms()});
 $('#book-slot')?.addEventListener('click',()=>{if(!selectedSlot)return;const data=bookings();data.push({room,hour:Number(selectedSlot)});save('bookings',JSON.stringify(data));toast(`Ауд. ${room}, ${selectedSlot}:00–${Number(selectedSlot)+1}:00 — забронировано`);selectedSlot='';paintRooms()});
 $('#cancel-booking')?.addEventListener('click',e=>{const b=e.currentTarget;const r=b.dataset.room,h=Number(b.dataset.hour);if(r==='3105'&&h===14)save('fixture-cancelled','true');save('bookings',JSON.stringify(bookings().filter(x=>!(x.room===r&&x.hour===h))));b.closest('dialog').close();toast('Бронь отменена в прототипе');paintRooms()});
 if($('#room'))paintRooms();
 let branch='all';const search=$('#book-search');const filterBooks=()=>{let count=0;$$('[data-book]').forEach(x=>{x.hidden=!(branch==='all'||x.dataset.branch===branch)||!x.textContent.toLowerCase().includes((search?.value||'').toLowerCase());if(!x.hidden)count++});if($('#books-empty'))$('#books-empty').hidden=count>0};search?.addEventListener('input',filterBooks);$$('[data-branch]').forEach(b=>b.addEventListener('click',()=>{branch=b.dataset.branch;$$('[data-branch]').forEach(x=>x.setAttribute('aria-pressed',String(x===b)));filterBooks()}));
 $$('[data-cafe]').forEach(b=>b.addEventListener('click',()=>{$$('[data-cafe]').forEach(x=>x.setAttribute('aria-pressed',String(x===b)));$$('[data-menu]').forEach(x=>x.hidden=x.dataset.menu!==b.dataset.cafe)}));
 const map=$('#map-building');const floor=$('#map-floor');const renderMap=()=>{const building=map?.value||'А',level=floor?.value||'2';$$('[data-map-room]').forEach(x=>x.hidden=x.dataset.building!==building||x.dataset.floor!==level);$('#map-location').textContent=`Корпус ${building} · этаж ${level}`;$('#map-empty').hidden=$$('[data-map-room]').some(x=>!x.hidden)};
 map?.addEventListener('change',renderMap);floor?.addEventListener('change',renderMap);
 $('#room-search-form')?.addEventListener('submit',e=>{e.preventDefault();const value=$('#room-search').value.trim().toUpperCase();const target=$$('[data-map-room]').find(x=>x.dataset.mapRoom===value);if(target){map.value=target.dataset.building;floor.value=target.dataset.floor;renderMap();target.scrollIntoView({block:'center'});target.focus();$('#map-search-result').textContent=`Ауд. ${value}: корпус ${target.dataset.building}, этаж ${target.dataset.floor}`}else $('#map-search-result').textContent='Ничего не нашлось — попробуйте другое название'});
 if(map){const target=$$('[data-map-room]').find(x=>x.dataset.mapRoom===query.get('room'));if(target){map.value=target.dataset.building;floor.value=target.dataset.floor}renderMap()}
 $$('[data-map-room]').forEach(b=>b.addEventListener('click',()=>{$('#room-detail-title').textContent='Аудитория '+b.dataset.mapRoom;$('#room-detail-body').textContent=`Корпус ${b.dataset.building} · этаж ${b.dataset.floor}. ${b.dataset.route}`;$('#room-detail').showModal()}));
 $('#staff-move-form')?.addEventListener('submit',e=>{e.preventDefault();$('#staff-move-result').textContent='Демо-перенос: 3105 → '+$('#new-room').value+'. Уведомление показано в ленте прототипа.';toast('Изменение сохранено в локальном прототипе')});
 $$('[data-copy]').forEach(b=>b.addEventListener('click',async()=>{try{await navigator.clipboard.writeText(b.dataset.copy);toast('Адрес скопирован')}catch{toast('Адрес: '+b.dataset.copy)}}));
 $('#print-document')?.addEventListener('click',()=>window.print());
 $$('[data-retry]').forEach(b=>b.addEventListener('click',()=>{location.href=b.dataset.retry}));
})();
