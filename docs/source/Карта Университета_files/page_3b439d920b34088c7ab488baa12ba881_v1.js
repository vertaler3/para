
; /* Start:"a:4:{s:4:"full";s:66:"/local/templates/.default/markup/new/js/map_new.js?159958167019674";s:6:"source";s:50:"/local/templates/.default/markup/new/js/map_new.js";s:3:"min";s:0:"";s:3:"map";s:0:"";}"*/
$.browser = {
    msie: false,
    safari: false,
    mozilla: false,
    opera: false,
    version: 0
};

var Rooms = {};
			var MiniMapHgl={a:false,b:false,v:false,g:false,n:false,l:false,d:false,s:false,r:false};
			$(window).load(
				function () {
					var zoom_detection_par = 0;
					intMap.Init("", "");
				}
			);
	
		function supportsSvg() {
			//
			if (
				($.browser.msie && (parseInt($.browser.version) < 7)) || //а шестой? 9)) || //не нужно, старые ослы нормально отсекаются проверкой фич
//				($.browser.safari && (parseInt($.browser.version) < а хрен его знает, у нас макинтоша нет)) ||
				($.browser.mozilla && (parseInt($.browser.version) < 10)) ||
				($.browser.opera && (parseInt($.browser.version) < 11))
			) return false;
			
			if (!document.implementation) return false;			
			return document.implementation.hasFeature("http://www.w3.org/TR/SVG11/feature#BasicStructure", "1.1");
		}

		
		// (цельнотянутое) Определим свои функции добавления/удаления класса, так как те, что в jQuery не работают для SVG
		jQuery.fn.myAddClass = function (classTitle) {
		  return this.each(function() {
			var oldClass = jQuery(this).attr("class");
			oldClass = oldClass ? oldClass : '';
			jQuery(this).attr("class", (oldClass+" "+classTitle).trim());
		  });
		}
		
		jQuery.fn.myRemoveClass = function (classTitle) {
		  return this.each(function() {
			  var oldClass = jQuery(this).attr("class");
			  var startpos = oldClass.indexOf(classTitle);
			  var endpos = startpos + classTitle.length;
			  var newClass = oldClass.substring(0, startpos).trim() + " " + oldClass.substring(endpos).trim();
			  if (!newClass.trim())
				jQuery(this).removeAttr("class");
			  else
				jQuery(this).attr("class", newClass.trim());
		  });
		}

		function ObjectToStr(o, sep) {
			var result = '';
			for (property in o) {
				result += sep + property + ': ' + o[property];
			}
			return result;
		}
		
		//Цельнотянуто из повидла
		function FindObjectXY(obj) {
			var curleft = curtop = 0;
			if (obj.offsetParent) {
				do {
					curleft += obj.offsetLeft;
					curtop += obj.offsetTop;
				} while (obj = obj.offsetParent);
			}
			return [curleft, curtop];
		}	

		function GetPageScroll() {
				// This works for all browsers except IE versions 8 and before
			if (window.pageYOffset != null) return [window.pageXOffset, window.pageYOffset];
			// For IE (or any browser) in Standards mode
			var d = window.document;
			if (d.compatMode == "CSS1Compat") return [d.documentElement.scrollLeft, d.documentElement.scrollTop];
			// For browsers in Quirks mode
			return [d.body.scrollLeft, d.body.scrollTop];
		}
		
		var intMap = {
			popup_state: 0,
			_floor_id: '',
			_room_id: '',
			flash_on: 0,
			flash_off: 1,
			flash_on_interval: 2,
			flash_off_interval: 12,
			floor_id: false,
			popupX: 0,
			popupY: 0,
			old_popup_height: 0,
			GetRoomPosition: function(room_id) {

				var rect = document.getElementById(room_id);
				if (!rect) return false;
				var box = rect.getBoundingClientRect();
				var offs = [0,0];//FindObjectXY(document.getElementById(this.floor_id)); // + "_c"));
//alert(ObjectToStr(box, "\n"));						
				return{
					"left": parseInt(box.left + offs[0]),
					"top": parseInt(box.top + offs[1]),
					"right": parseInt(box.right + offs[0]),
					"bottom": parseInt(box.bottom + offs[1]),
                    "width": parseInt(box.width),
					"height": parseInt(box.height)
				};				
			},
			GetPopupHeight: function() {
				//if (!this.popup_state) return 0;

				if (window.Rooms[this.popup_state] && (window.Rooms[this.popup_state]["type"] == 9)) return -100;	
				
				var ruler = document.getElementById('popapcheg_underline');
				if (!ruler) {
					//abort the current operation
					return 0;
				}
				var result = ruler.offsetTop;	
				if (result) {
					return result;
				} else {
					return this.old_popup_height;
				}
			},
			ShowPopupFallback: function (room_id) {
				if (!room_id) return;
				if (!window.Rooms[room_id]) return;
				var	rname = window.Rooms[room_id]["name"];
				var	tekst = window.Rooms[room_id]["text"];
				var dvanbsp = '&nbsp;&nbsp;'; if (rname == '&nbsp;') dvanbsp = '';
				document.getElementById("pseudobaloon").innerHTML = 'выбранный элемент: <b>' + rname.split('<br>').join('') + '</b>' + dvanbsp + tekst;
				$("#pseudobaloon").show();
			},
			ShowPopup: function (room_id, tekst) {
				if (this.popup_state) $('#rn_' + this.popup_state).show();
				if ((this.popup_state == room_id) || (this.popup_state && !room_id)) {
					$("#popapcheg").hide();//fadeOut();
					this.popup_state = 0;
					this.old_popup_height = -1;
					return true;
				}
				var pp = document.getElementById("popapcheg");
				if (!pp) return;
				var rname = room_id;
				if (window.Rooms[room_id]) {
					rname = window.Rooms[room_id]["name"];
					tekst = window.Rooms[room_id]["text"];
				} else {
					$("#popapcheg").hide();
					this.popup_state = 0;
					return false;
				}				
				if (!tekst) tekst ="<br><i>(нет описания)</i>";
				$('#rn_' + room_id).hide();
				var dvanbsp = '&nbsp;&nbsp;'; if (rname == '&nbsp;') dvanbsp = '';
				document.getElementById("popapcheg_inner").innerHTML = '<b>' + rname.split('<br>').join('') + '</b>' + dvanbsp + tekst;
			//	document.getElementById("balo_linkleg_list").innerHTML = 'Ссылка на комнату:<br>http://mgimo.ru/map?room=' + room_id;
				this.popup_state = room_id;
				this.RepositionPopup();
				return true;
				
			},
			RepositionPopup: function () {
				if (!this.popup_state) return;
				var rp = this.GetRoomPosition(this.popup_state);
				var pageScroll = GetPageScroll();
				if (!rp) return;
				this.popupX = parseInt((rp.right + rp.left)/2) + pageScroll[0] - 7; 
				$("#popapcheg").show();
				var tph = this.GetPopupHeight();
				$("#popapcheg").hide();				
				this.popupY = parseInt((rp.top+rp.bottom)/2) + pageScroll[1] - tph + 7;
				$("#popapcheg").css({
					"left": (this.popupX) + "px",
					"top": (this.popupY) + "px"
				});
				this.ScrollToPopup();
				//$("#popapcheg").hide();
	
				if (window.Rooms[this.popup_state] && (window.Rooms[this.popup_state]["type"] == 9)) {
					$("#" + this.popup_state).myAddClass("imared");
					$("#rn_" + this.popup_state).show();					
				} else {
					$("#popapcheg").show();//fadeIn();
				}					
				this.old_popup_height = this.GetPopupHeight();
			},
			CheckPopupState: function() {
				if (this.old_popup_height != this.GetPopupHeight()) this.RepositionPopup;
			},
			ScrollToPopup: function() {
				if (!this.popup_state) return;
				if (!window["innerWidth"]) return;
				//прокрутка до нужного места
				var scroll = GetPageScroll();
				var sX = Math.max(Math.max(0, Math.min(this.popupX - 50, scroll[0])), this.popupX + 500 + 50 - parseInt(window.innerWidth));
				var sY = Math.max(Math.max(0, Math.min(this.popupY - 50, scroll[1])), this.popupY + this.GetPopupHeight() + 50 - parseInt(window.innerHeight));
				$('html, body').animate({scrollTop: sY, scrollLeft: sX}, 200);


				//setTimeout('window.scrollTo(' + sX + ',' + sY + ');', 1000);
			},
			GetNameLen: function(s) {
				var L = s.split('<br>');
				var mw = 0;
				for (var i = 0; i < L.length; i++) {
					mw = Math.max(mw, L[i].length);
				}
				return mw;
			},
			createText: function(id, text, x, y, angle, zoom) {

				//суфикс для класса, чтобы не кликались тип 44 (серый без деталей) и тип 9 (надписи к стрелкам)
				var nocl ='';
				if ((window.Rooms[id]["type"] == 44) || (window.Rooms[id]["type"] == 9) || (window.Rooms[id]["type"] == 11)) nocl = '_nocl'; 
				
				//подгоняем координаты с учётом поворота текста
				x-= (15 * 0.35 *zoom) * Math.sin(angle * Math.PI /180);
				y+= (15 * 0.35 *zoom) * Math.cos(angle * Math.PI /180);
				
				
				var rn = document.getElementById(id);
				var svgNS = "http://www.w3.org/2000/svg";
				var newText = document.createElementNS(svgNS, "text");
				newText.setAttributeNS(null,"x", x/zoom);		
				newText.setAttributeNS(null,"y", y/zoom);
				newText.setAttributeNS(null,"id", 'rn_' + id);
				newText.setAttributeNS(null,"class", "room_number" + nocl);
				newText.setAttributeNS(null,"font-size","13px");
				newText.setAttributeNS(null,"text-anchor","middle");
				newText.setAttributeNS(null,"transform", 
					'rotate(' + angle + ',' + x + ',' + y + ') scale(' + zoom + ',' + zoom + ')');
				var text_lines = text.split('<br>');
				if (text_lines.length > 1) {
					//несколько строк
					var spans = [];
					spans.length = text_lines.length;
					var linez = [];
					linez.length = text_lines.length;
					for (var i = 0; i < text_lines.length; i++ ) {
						linez[i] = document.createTextNode(text_lines[i]);
						spans[i] = document.createElementNS(svgNS, "tspan");
						spans[i].setAttributeNS(null, "y", (y / zoom) + (i + 0.5 - ((text_lines.length) / 2)) * 15);
						spans[i].setAttributeNS(null, "x", x / zoom);
						spans[i].appendChild(linez[i]);
						newText.appendChild(spans[i]);
					}
				} else {
					//одна строчка
					var textNode = document.createTextNode(text);
					newText.appendChild(textNode);
				}	
				var par = this.svgobject;
				par.appendChild(newText);//insertBefore(newText, rn);
			},			
			
			TransformRoomRect: function (rm, gm, bb, shift) {
				var zoom_hack = 1;// / Math.sqrt(rm.a*rm.a + rm.b*rm.b);
				var rx = bb.x;
				var ry = bb.y;
				var tx1 = rx * zoom_hack * rm.a + ry * zoom_hack * rm.c + rm.e / zoom_hack;
				var ty1 = rx * zoom_hack * rm.b + ry * zoom_hack * rm.d + rm.f / zoom_hack;				
				var rx = bb.x + bb.width;
				var ry = bb.y + bb.height;
				
				var tx2 = rx * zoom_hack * rm.a + ry * zoom_hack * rm.c + rm.e / zoom_hack;
				var ty2 = rx * zoom_hack * rm.b + ry * zoom_hack * rm.d + rm.f / zoom_hack;				
				
				var tx = ((tx1 + tx2) / 2) * gm.a + ((ty1 + ty2) / 2) * gm.c + gm.e;
				var ty = ((tx1 + tx2) / 2) * gm.b + ((ty1 + ty2) / 2) * gm.d + gm.f;
				
				return [tx + shift[0], ty + shift[1]];
			
			},
			TransformBox: function(rm, bb) {
				return {
					width: bb.width * rm.a + bb.height * rm.c,
					height: bb.width * rm.b + bb.height * rm.d
				};
			},
			PlaceRoomNumber: function(room_id, room_name) {
				var rn = document.getElementById(room_id);
				try {
					var matr = rn.getCTM();
				}
				catch(err) {
					this.Fib('Не найдена getCTM() комнаты id="' + room_id + '"'); 
					return;
				}
				
				/* это хак. Матрица комнаты оказывается отмасштабирована согласно тому, 
					как браузер отображает SVG в данный момент (то есть, как бог на душу положит), 
					в результате все координаты оказываются умноженными на некий с-потолка-коэффициент.
					Приходится выполнять обратное преобразование. */
				var gmat = this.svgobject.getCTM();
				if (!gmat) {
					//Взбрыки файрфокса
					var gmat = this.svgobject.getScreenCTM().inverse();
					gmat.e = 0;
					gmat.f = 0;
				} else {
					gmat = gmat.inverse();					
				}
				var gmat_zoom = Math.sqrt(gmat.a * gmat.a + gmat.b * gmat.b);
//var gmat= {a:1,b:0,c:0,d:1,e:0,f:0};				

				//сдвиг, если задан
				var shift = [0,0,0,1];
				if ((window.Rooms[room_id]) && window.Rooms[room_id]["shift"]) {
					var shift = window.Rooms[room_id]["shift"];
				}
				
				var room_bbox = rn.getBBox();
				var name = room_name.replace('&nbsp;', ' ');
				
				//автоматический поворот набок для узких комнат
				var real_bbox = this.TransformBox(matr, room_bbox); //apply room transformations
				real_bbox.width = Math.abs(real_bbox.width);
				real_bbox.height = Math.abs(real_bbox.height);
				if ((real_bbox.width < 15 * this.GetNameLen(name) * 0.7 * shift[3] / gmat_zoom) && ((real_bbox.height > real_bbox.width)) && (shift[2] == 0)) {
					shift[2] = -90;
				}
				
				//окончательное вычисление координат
				var centerpoint = this.TransformRoomRect(matr, gmat, room_bbox, shift);
				this.createText(room_id, name
					, centerpoint[0], centerpoint[1], shift[2], shift[3]
				);//, document.getElementById(room_id));
			},
			FlashRoom: function (room_id) {
				return; //По глазам им, вишь ты, бьёт :( А я так старался её прикручивая.
				if (this.flash_off > 0) {
					this.flash_off--;
					if (0 == this.flash_off) {
						this.flash_on = this.flash_on_interval;
						$("#" + room_id).myAddClass("imaselected");
						return;
					}
				}
				if (this.flash_on > 0) {
					this.flash_on--;
					if (0 == this.flash_on) {
						this.flash_off = this.flash_off_interval;
						$("#" + room_id).myRemoveClass("imaselected");
					}
				}
			},
			Init: function(floor_id, room_id) {
				if (!supportsSvg()) {
					this.Fallback(room_id); //this.OhShi__(); 
					return;
				}
				this._floor_id = floor_id;
				this._room_id = room_id;

				if (!floor_id) {

					this.ResizeMiniMap();
					this.Unobscure();
					return;
				}
				var svgobject = document.getElementById('main_map' /*floor_id*/);
				if (!svgobject) {
					setTimeout("intMap.Init();", 100);

					return;
				}

				$(svgobject).ready(
					function () {
						intMap.Process();					
					}
				);
			},
			Retry: function() {
				window.SetTimeout("intMap.Process();", 100);	
			},
			Process: function() { //SelectFloor: function (floor_id, room_id) {
//			  try {
				var floor_id = this._floor_id;
				var room_id = this._room_id;
		
				if (!floor_id) return;
				this.floor_id = floor_id;
				
				var svgobject = document.getElementById('main_map' /*floor_id*/);
				if (!svgobject) {
					this.Fib('SVG объект не найден!');		
					return;
				} else {
					this.svgobject = svgobject;
					var svgdiv = document.getElementById('svgcontainer');
					var viewBox = svgobject.getAttribute("viewBox");
					if (!viewBox) {
						$(svgobject).hide();
						this.Fib('Ошибка в SVG файле: не найден параметр viewBox!');
						return;
					}	
					var viewBox = viewBox.split(" ");
					var aspectRatio = viewBox[2] / viewBox[3];
					var windowWidth = $(window).width(); 
					if (aspectRatio > 1) {
						if (windowWidth < 1300 ){ 
							svgdiv.style.width = parseInt(800 + 1) + "px";
						svgdiv.style.height = parseInt((800 / aspectRatio) + 1) + "px";
						svgobject.setAttribute("height", parseInt(800 / aspectRatio));
						} else {
							svgdiv.style.width = parseInt(1000 + 1) + "px";
							svgdiv.style.height = parseInt((1000 / aspectRatio) + 1) + "px";
							svgobject.setAttribute("height", parseInt(1000 / aspectRatio));
						}
						
						//svgobject.setAttribute("width", 1000);
						
					} else {
						if (windowWidth < 1300 ){
							svgdiv.style.width = parseInt((800 * aspectRatio) + 1)+ "px";
						svgdiv.style.height = parseInt(800 + 1) + "px";
						svgobject.setAttribute("height", 800);

						 } else {
							svgdiv.style.width = parseInt((1000 * aspectRatio) + 1)+ "px";
						svgdiv.style.height = parseInt(1000 + 1) + "px";
						svgobject.setAttribute("height", 1000);
						}
						
						
					}
				}
				this.ResizeMiniMap();
				
				for (rid in window.Rooms) {
					var st = "imatype" + window.Rooms[rid]["type"];
					$("#" + rid).myAddClass(st);
					if ((window.Rooms[rid]["type"] != 44) && (window.Rooms[rid]["type"] != 5) && (window.Rooms[rid]["type"] != 11)) {
						this.PlaceRoomNumber(rid, window.Rooms[rid]["name"]);//id, name
					}	
				}
				if (room_id) {
					var rect = document.getElementById(room_id);
					this.ShowPopup(room_id);
				}
				$("#room_numbers").show();
				
				$('.imatype1,.imatype2,.imatype3,.imatype4,.imatype5,.imatype6,.imatype7,.imatype8,.imatype10,.room_number').click(function(){
					var id = $(this).attr("id");
					id = id.replace('rn_', '');
					intMap.ShowPopup(id);//if (intMap.ShowPopup(id)) return false;
				});
				$('.imatype9').click(function(){
					var id = $(this).attr("id");
					id = id.replace('rn_', '');
					intMap.GoLinkGo(id);
				});
				$('.imatype11').click(function(){
					var id = $(this).attr("id");
					id = id.replace('rn_', '');
					intMap.OpenExternalLink(id);
				});
				$('.inners').click(function(){
					var id = $(this.parentNode).attr("id");
					id = id.replace('rn_', '');
					intMap.ShowPopup(id);//if (intMap.ShowPopup(id)) return false;
				});				
				
				this.ScrollToPopup();
				
				setInterval(
					function(){
						if(jQuery(window).width() != window.zoom_detection_par){
							window.zoom_detection_par = jQuery(window).width();
							intMap.RepositionPopup();
						}
						intMap.CheckPopupState();
					}, 50);	
				
				this.Unobscure();
			 
			},
			Unobscure: function() {
				$('#loading_white_sheet').hide();
			},
			Fib: function(s) {
				var ded = document.getElementById("ded");
				ded.innerHTML+= '<br />' + s;
				this.Unobscure();
			},
			ResizeMiniMap: function() {
				var legsvg = document.getElementById('legend_map');

				if (legsvg) {
					legsvg.setAttribute("height", 300);
				
					for(var bid in MiniMapHgl) {
						if (MiniMapHgl[bid]) {
							$('#legend_' + bid).myAddClass('mmap_' + bid + '_tut');
						} else {
							$('#legend_' + bid).myAddClass('mmap_' + bid);
							$('#legend_' + bid).hover(
								function() {
									var b = $(this).attr("id").substring(7, 8);
									$(this).myAddClass('mmap_' + b + '_tut');
								},
								function() {
									var b = $(this).attr("id").substring(7, 8);
									$(this).myRemoveClass('mmap_' + b + '_tut');
								}
							);						
						}
						$('#legend_' + bid).click(
							function() {
								var b = $(this).attr("id").substring(7, 8);
								intMap.GoToBuilding(b);
							}
						);
					}
				} else {
					this.Fib('Не найдена мини-карта.');
				}				
			},
			GoTo: function(bid, fid) {
				window.location.href = '?building=' + bid + '&floor=' + fid; 
			},
			GoToBuilding: function(bid) {
				this.GoTo(bid, this.floor_id);
			},
			GoToFloor: function(fid) {
				this.GoTo("", fid);
			},
			OhShi__: function() {
				$('#mysvgfallback').show();
				$('#svgcontainer').remove();			
				$('#mm_container').remove();
				$('#search_controls').hide();
				this.Fib('Произошёл сбой скрипта.');
				this.Unobscure();
			},
			GoLinkGo: function(id) {
				if ((window.Rooms[id]) && (window.Rooms[id]["link"])) {
					window.location.href = '?' + window.Rooms[id]["link"];
				} else {
					this.Fib("Under Construction: не найдена связь для стрелочки id=" + id);
				}
			},
			OpenExternalLink: function (id) {
				if ((window.Rooms[id]) && (window.Rooms[id]["link"])) {
					window.open(window.Rooms[id]["link"]);
				} else {
					this.Fib("Under Construction: не найдена связь для внешней ссылки id=" + id);
				}
			},
			Fallback: function(room_id) {
				$('#svgcontainer').remove();			
				$('#mm_container').remove();
				$('#mysvgfallback').show();
				$('.oopstext').show();
				$('.legend_hidden_block').show();
				$('.hidden_png').show();				
				this.ShowPopupFallback(room_id);
				this.Unobscure();
			}
		};
/* End */
;
; /* Start:"a:4:{s:4:"full";s:103:"/local/templates/mgimo_new/components/project/territory.map.filter/territory_new/script.js?166253474579";s:6:"source";s:90:"/local/templates/mgimo_new/components/project/territory.map.filter/territory_new/script.js";s:3:"min";s:0:"";s:3:"map";s:0:"";}"*/
$(document).ready(function() {
	$('.bs-select-map-corp').trigger('change');
});
/* End */
;; /* /local/templates/.default/markup/new/js/map_new.js?159958167019674*/
; /* /local/templates/mgimo_new/components/project/territory.map.filter/territory_new/script.js?166253474579*/
