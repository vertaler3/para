
; /* Start:"a:4:{s:4:"full";s:75:"/local/templates/.default/markup/new/js/jquery-3.3.1.min.js?164206818586927";s:6:"source";s:59:"/local/templates/.default/markup/new/js/jquery-3.3.1.min.js";s:3:"min";s:0:"";s:3:"map";s:0:"";}"*/
/*! jQuery v3.3.1 | (c) JS Foundation and other contributors | jquery.org/license */
!function(e,t){"use strict";"object"==typeof module&&"object"==typeof module.exports?module.exports=e.document?t(e,!0):function(e){if(!e.document)throw new Error("jQuery requires a window with a document");return t(e)}:t(e)}("undefined"!=typeof window?window:this,function(e,t){"use strict";var n=[],r=e.document,i=Object.getPrototypeOf,o=n.slice,a=n.concat,s=n.push,u=n.indexOf,l={},c=l.toString,f=l.hasOwnProperty,p=f.toString,d=p.call(Object),h={},g=function e(t){return"function"==typeof t&&"number"!=typeof t.nodeType},y=function e(t){return null!=t&&t===t.window},v={type:!0,src:!0,noModule:!0};function m(e,t,n){var i,o=(t=t||r).createElement("script");if(o.text=e,n)for(i in v)n[i]&&(o[i]=n[i]);t.head.appendChild(o).parentNode.removeChild(o)}function x(e){return null==e?e+"":"object"==typeof e||"function"==typeof e?l[c.call(e)]||"object":typeof e}var b="3.3.1",w=function(e,t){return new w.fn.init(e,t)},T=/^[\s\uFEFF\xA0]+|[\s\uFEFF\xA0]+$/g;w.fn=w.prototype={jquery:"3.3.1",constructor:w,length:0,toArray:function(){return o.call(this)},get:function(e){return null==e?o.call(this):e<0?this[e+this.length]:this[e]},pushStack:function(e){var t=w.merge(this.constructor(),e);return t.prevObject=this,t},each:function(e){return w.each(this,e)},map:function(e){return this.pushStack(w.map(this,function(t,n){return e.call(t,n,t)}))},slice:function(){return this.pushStack(o.apply(this,arguments))},first:function(){return this.eq(0)},last:function(){return this.eq(-1)},eq:function(e){var t=this.length,n=+e+(e<0?t:0);return this.pushStack(n>=0&&n<t?[this[n]]:[])},end:function(){return this.prevObject||this.constructor()},push:s,sort:n.sort,splice:n.splice},w.extend=w.fn.extend=function(){var e,t,n,r,i,o,a=arguments[0]||{},s=1,u=arguments.length,l=!1;for("boolean"==typeof a&&(l=a,a=arguments[s]||{},s++),"object"==typeof a||g(a)||(a={}),s===u&&(a=this,s--);s<u;s++)if(null!=(e=arguments[s]))for(t in e)n=a[t],a!==(r=e[t])&&(l&&r&&(w.isPlainObject(r)||(i=Array.isArray(r)))?(i?(i=!1,o=n&&Array.isArray(n)?n:[]):o=n&&w.isPlainObject(n)?n:{},a[t]=w.extend(l,o,r)):void 0!==r&&(a[t]=r));return a},w.extend({expando:"jQuery"+("3.3.1"+Math.random()).replace(/\D/g,""),isReady:!0,error:function(e){throw new Error(e)},noop:function(){},isPlainObject:function(e){var t,n;return!(!e||"[object Object]"!==c.call(e))&&(!(t=i(e))||"function"==typeof(n=f.call(t,"constructor")&&t.constructor)&&p.call(n)===d)},isEmptyObject:function(e){var t;for(t in e)return!1;return!0},globalEval:function(e){m(e)},each:function(e,t){var n,r=0;if(C(e)){for(n=e.length;r<n;r++)if(!1===t.call(e[r],r,e[r]))break}else for(r in e)if(!1===t.call(e[r],r,e[r]))break;return e},trim:function(e){return null==e?"":(e+"").replace(T,"")},makeArray:function(e,t){var n=t||[];return null!=e&&(C(Object(e))?w.merge(n,"string"==typeof e?[e]:e):s.call(n,e)),n},inArray:function(e,t,n){return null==t?-1:u.call(t,e,n)},merge:function(e,t){for(var n=+t.length,r=0,i=e.length;r<n;r++)e[i++]=t[r];return e.length=i,e},grep:function(e,t,n){for(var r,i=[],o=0,a=e.length,s=!n;o<a;o++)(r=!t(e[o],o))!==s&&i.push(e[o]);return i},map:function(e,t,n){var r,i,o=0,s=[];if(C(e))for(r=e.length;o<r;o++)null!=(i=t(e[o],o,n))&&s.push(i);else for(o in e)null!=(i=t(e[o],o,n))&&s.push(i);return a.apply([],s)},guid:1,support:h}),"function"==typeof Symbol&&(w.fn[Symbol.iterator]=n[Symbol.iterator]),w.each("Boolean Number String Function Array Date RegExp Object Error Symbol".split(" "),function(e,t){l["[object "+t+"]"]=t.toLowerCase()});function C(e){var t=!!e&&"length"in e&&e.length,n=x(e);return!g(e)&&!y(e)&&("array"===n||0===t||"number"==typeof t&&t>0&&t-1 in e)}var E=function(e){var t,n,r,i,o,a,s,u,l,c,f,p,d,h,g,y,v,m,x,b="sizzle"+1*new Date,w=e.document,T=0,C=0,E=ae(),k=ae(),S=ae(),D=function(e,t){return e===t&&(f=!0),0},N={}.hasOwnProperty,A=[],j=A.pop,q=A.push,L=A.push,H=A.slice,O=function(e,t){for(var n=0,r=e.length;n<r;n++)if(e[n]===t)return n;return-1},P="checked|selected|async|autofocus|autoplay|controls|defer|disabled|hidden|ismap|loop|multiple|open|readonly|required|scoped",M="[\\x20\\t\\r\\n\\f]",R="(?:\\\\.|[\\w-]|[^\0-\\xa0])+",I="\\["+M+"*("+R+")(?:"+M+"*([*^$|!~]?=)"+M+"*(?:'((?:\\\\.|[^\\\\'])*)'|\"((?:\\\\.|[^\\\\\"])*)\"|("+R+"))|)"+M+"*\\]",W=":("+R+")(?:\\((('((?:\\\\.|[^\\\\'])*)'|\"((?:\\\\.|[^\\\\\"])*)\")|((?:\\\\.|[^\\\\()[\\]]|"+I+")*)|.*)\\)|)",$=new RegExp(M+"+","g"),B=new RegExp("^"+M+"+|((?:^|[^\\\\])(?:\\\\.)*)"+M+"+$","g"),F=new RegExp("^"+M+"*,"+M+"*"),_=new RegExp("^"+M+"*([>+~]|"+M+")"+M+"*"),z=new RegExp("="+M+"*([^\\]'\"]*?)"+M+"*\\]","g"),X=new RegExp(W),U=new RegExp("^"+R+"$"),V={ID:new RegExp("^#("+R+")"),CLASS:new RegExp("^\\.("+R+")"),TAG:new RegExp("^("+R+"|[*])"),ATTR:new RegExp("^"+I),PSEUDO:new RegExp("^"+W),CHILD:new RegExp("^:(only|first|last|nth|nth-last)-(child|of-type)(?:\\("+M+"*(even|odd|(([+-]|)(\\d*)n|)"+M+"*(?:([+-]|)"+M+"*(\\d+)|))"+M+"*\\)|)","i"),bool:new RegExp("^(?:"+P+")$","i"),needsContext:new RegExp("^"+M+"*[>+~]|:(even|odd|eq|gt|lt|nth|first|last)(?:\\("+M+"*((?:-\\d)?\\d*)"+M+"*\\)|)(?=[^-]|$)","i")},G=/^(?:input|select|textarea|button)$/i,Y=/^h\d$/i,Q=/^[^{]+\{\s*\[native \w/,J=/^(?:#([\w-]+)|(\w+)|\.([\w-]+))$/,K=/[+~]/,Z=new RegExp("\\\\([\\da-f]{1,6}"+M+"?|("+M+")|.)","ig"),ee=function(e,t,n){var r="0x"+t-65536;return r!==r||n?t:r<0?String.fromCharCode(r+65536):String.fromCharCode(r>>10|55296,1023&r|56320)},te=/([\0-\x1f\x7f]|^-?\d)|^-$|[^\0-\x1f\x7f-\uFFFF\w-]/g,ne=function(e,t){return t?"\0"===e?"\ufffd":e.slice(0,-1)+"\\"+e.charCodeAt(e.length-1).toString(16)+" ":"\\"+e},re=function(){p()},ie=me(function(e){return!0===e.disabled&&("form"in e||"label"in e)},{dir:"parentNode",next:"legend"});try{L.apply(A=H.call(w.childNodes),w.childNodes),A[w.childNodes.length].nodeType}catch(e){L={apply:A.length?function(e,t){q.apply(e,H.call(t))}:function(e,t){var n=e.length,r=0;while(e[n++]=t[r++]);e.length=n-1}}}function oe(e,t,r,i){var o,s,l,c,f,h,v,m=t&&t.ownerDocument,T=t?t.nodeType:9;if(r=r||[],"string"!=typeof e||!e||1!==T&&9!==T&&11!==T)return r;if(!i&&((t?t.ownerDocument||t:w)!==d&&p(t),t=t||d,g)){if(11!==T&&(f=J.exec(e)))if(o=f[1]){if(9===T){if(!(l=t.getElementById(o)))return r;if(l.id===o)return r.push(l),r}else if(m&&(l=m.getElementById(o))&&x(t,l)&&l.id===o)return r.push(l),r}else{if(f[2])return L.apply(r,t.getElementsByTagName(e)),r;if((o=f[3])&&n.getElementsByClassName&&t.getElementsByClassName)return L.apply(r,t.getElementsByClassName(o)),r}if(n.qsa&&!S[e+" "]&&(!y||!y.test(e))){if(1!==T)m=t,v=e;else if("object"!==t.nodeName.toLowerCase()){(c=t.getAttribute("id"))?c=c.replace(te,ne):t.setAttribute("id",c=b),s=(h=a(e)).length;while(s--)h[s]="#"+c+" "+ve(h[s]);v=h.join(","),m=K.test(e)&&ge(t.parentNode)||t}if(v)try{return L.apply(r,m.querySelectorAll(v)),r}catch(e){}finally{c===b&&t.removeAttribute("id")}}}return u(e.replace(B,"$1"),t,r,i)}function ae(){var e=[];function t(n,i){return e.push(n+" ")>r.cacheLength&&delete t[e.shift()],t[n+" "]=i}return t}function se(e){return e[b]=!0,e}function ue(e){var t=d.createElement("fieldset");try{return!!e(t)}catch(e){return!1}finally{t.parentNode&&t.parentNode.removeChild(t),t=null}}function le(e,t){var n=e.split("|"),i=n.length;while(i--)r.attrHandle[n[i]]=t}function ce(e,t){var n=t&&e,r=n&&1===e.nodeType&&1===t.nodeType&&e.sourceIndex-t.sourceIndex;if(r)return r;if(n)while(n=n.nextSibling)if(n===t)return-1;return e?1:-1}function fe(e){return function(t){return"input"===t.nodeName.toLowerCase()&&t.type===e}}function pe(e){return function(t){var n=t.nodeName.toLowerCase();return("input"===n||"button"===n)&&t.type===e}}function de(e){return function(t){return"form"in t?t.parentNode&&!1===t.disabled?"label"in t?"label"in t.parentNode?t.parentNode.disabled===e:t.disabled===e:t.isDisabled===e||t.isDisabled!==!e&&ie(t)===e:t.disabled===e:"label"in t&&t.disabled===e}}function he(e){return se(function(t){return t=+t,se(function(n,r){var i,o=e([],n.length,t),a=o.length;while(a--)n[i=o[a]]&&(n[i]=!(r[i]=n[i]))})})}function ge(e){return e&&"undefined"!=typeof e.getElementsByTagName&&e}n=oe.support={},o=oe.isXML=function(e){var t=e&&(e.ownerDocument||e).documentElement;return!!t&&"HTML"!==t.nodeName},p=oe.setDocument=function(e){var t,i,a=e?e.ownerDocument||e:w;return a!==d&&9===a.nodeType&&a.documentElement?(d=a,h=d.documentElement,g=!o(d),w!==d&&(i=d.defaultView)&&i.top!==i&&(i.addEventListener?i.addEventListener("unload",re,!1):i.attachEvent&&i.attachEvent("onunload",re)),n.attributes=ue(function(e){return e.className="i",!e.getAttribute("className")}),n.getElementsByTagName=ue(function(e){return e.appendChild(d.createComment("")),!e.getElementsByTagName("*").length}),n.getElementsByClassName=Q.test(d.getElementsByClassName),n.getById=ue(function(e){return h.appendChild(e).id=b,!d.getElementsByName||!d.getElementsByName(b).length}),n.getById?(r.filter.ID=function(e){var t=e.replace(Z,ee);return function(e){return e.getAttribute("id")===t}},r.find.ID=function(e,t){if("undefined"!=typeof t.getElementById&&g){var n=t.getElementById(e);return n?[n]:[]}}):(r.filter.ID=function(e){var t=e.replace(Z,ee);return function(e){var n="undefined"!=typeof e.getAttributeNode&&e.getAttributeNode("id");return n&&n.value===t}},r.find.ID=function(e,t){if("undefined"!=typeof t.getElementById&&g){var n,r,i,o=t.getElementById(e);if(o){if((n=o.getAttributeNode("id"))&&n.value===e)return[o];i=t.getElementsByName(e),r=0;while(o=i[r++])if((n=o.getAttributeNode("id"))&&n.value===e)return[o]}return[]}}),r.find.TAG=n.getElementsByTagName?function(e,t){return"undefined"!=typeof t.getElementsByTagName?t.getElementsByTagName(e):n.qsa?t.querySelectorAll(e):void 0}:function(e,t){var n,r=[],i=0,o=t.getElementsByTagName(e);if("*"===e){while(n=o[i++])1===n.nodeType&&r.push(n);return r}return o},r.find.CLASS=n.getElementsByClassName&&function(e,t){if("undefined"!=typeof t.getElementsByClassName&&g)return t.getElementsByClassName(e)},v=[],y=[],(n.qsa=Q.test(d.querySelectorAll))&&(ue(function(e){h.appendChild(e).innerHTML="<a id='"+b+"'></a><select id='"+b+"-\r\\' msallowcapture=''><option selected=''></option></select>",e.querySelectorAll("[msallowcapture^='']").length&&y.push("[*^$]="+M+"*(?:''|\"\")"),e.querySelectorAll("[selected]").length||y.push("\\["+M+"*(?:value|"+P+")"),e.querySelectorAll("[id~="+b+"-]").length||y.push("~="),e.querySelectorAll(":checked").length||y.push(":checked"),e.querySelectorAll("a#"+b+"+*").length||y.push(".#.+[+~]")}),ue(function(e){e.innerHTML="<a href='' disabled='disabled'></a><select disabled='disabled'><option/></select>";var t=d.createElement("input");t.setAttribute("type","hidden"),e.appendChild(t).setAttribute("name","D"),e.querySelectorAll("[name=d]").length&&y.push("name"+M+"*[*^$|!~]?="),2!==e.querySelectorAll(":enabled").length&&y.push(":enabled",":disabled"),h.appendChild(e).disabled=!0,2!==e.querySelectorAll(":disabled").length&&y.push(":enabled",":disabled"),e.querySelectorAll("*,:x"),y.push(",.*:")})),(n.matchesSelector=Q.test(m=h.matches||h.webkitMatchesSelector||h.mozMatchesSelector||h.oMatchesSelector||h.msMatchesSelector))&&ue(function(e){n.disconnectedMatch=m.call(e,"*"),m.call(e,"[s!='']:x"),v.push("!=",W)}),y=y.length&&new RegExp(y.join("|")),v=v.length&&new RegExp(v.join("|")),t=Q.test(h.compareDocumentPosition),x=t||Q.test(h.contains)?function(e,t){var n=9===e.nodeType?e.documentElement:e,r=t&&t.parentNode;return e===r||!(!r||1!==r.nodeType||!(n.contains?n.contains(r):e.compareDocumentPosition&&16&e.compareDocumentPosition(r)))}:function(e,t){if(t)while(t=t.parentNode)if(t===e)return!0;return!1},D=t?function(e,t){if(e===t)return f=!0,0;var r=!e.compareDocumentPosition-!t.compareDocumentPosition;return r||(1&(r=(e.ownerDocument||e)===(t.ownerDocument||t)?e.compareDocumentPosition(t):1)||!n.sortDetached&&t.compareDocumentPosition(e)===r?e===d||e.ownerDocument===w&&x(w,e)?-1:t===d||t.ownerDocument===w&&x(w,t)?1:c?O(c,e)-O(c,t):0:4&r?-1:1)}:function(e,t){if(e===t)return f=!0,0;var n,r=0,i=e.parentNode,o=t.parentNode,a=[e],s=[t];if(!i||!o)return e===d?-1:t===d?1:i?-1:o?1:c?O(c,e)-O(c,t):0;if(i===o)return ce(e,t);n=e;while(n=n.parentNode)a.unshift(n);n=t;while(n=n.parentNode)s.unshift(n);while(a[r]===s[r])r++;return r?ce(a[r],s[r]):a[r]===w?-1:s[r]===w?1:0},d):d},oe.matches=function(e,t){return oe(e,null,null,t)},oe.matchesSelector=function(e,t){if((e.ownerDocument||e)!==d&&p(e),t=t.replace(z,"='$1']"),n.matchesSelector&&g&&!S[t+" "]&&(!v||!v.test(t))&&(!y||!y.test(t)))try{var r=m.call(e,t);if(r||n.disconnectedMatch||e.document&&11!==e.document.nodeType)return r}catch(e){}return oe(t,d,null,[e]).length>0},oe.contains=function(e,t){return(e.ownerDocument||e)!==d&&p(e),x(e,t)},oe.attr=function(e,t){(e.ownerDocument||e)!==d&&p(e);var i=r.attrHandle[t.toLowerCase()],o=i&&N.call(r.attrHandle,t.toLowerCase())?i(e,t,!g):void 0;return void 0!==o?o:n.attributes||!g?e.getAttribute(t):(o=e.getAttributeNode(t))&&o.specified?o.value:null},oe.escape=function(e){return(e+"").replace(te,ne)},oe.error=function(e){throw new Error("Syntax error, unrecognized expression: "+e)},oe.uniqueSort=function(e){var t,r=[],i=0,o=0;if(f=!n.detectDuplicates,c=!n.sortStable&&e.slice(0),e.sort(D),f){while(t=e[o++])t===e[o]&&(i=r.push(o));while(i--)e.splice(r[i],1)}return c=null,e},i=oe.getText=function(e){var t,n="",r=0,o=e.nodeType;if(o){if(1===o||9===o||11===o){if("string"==typeof e.textContent)return e.textContent;for(e=e.firstChild;e;e=e.nextSibling)n+=i(e)}else if(3===o||4===o)return e.nodeValue}else while(t=e[r++])n+=i(t);return n},(r=oe.selectors={cacheLength:50,createPseudo:se,match:V,attrHandle:{},find:{},relative:{">":{dir:"parentNode",first:!0}," ":{dir:"parentNode"},"+":{dir:"previousSibling",first:!0},"~":{dir:"previousSibling"}},preFilter:{ATTR:function(e){return e[1]=e[1].replace(Z,ee),e[3]=(e[3]||e[4]||e[5]||"").replace(Z,ee),"~="===e[2]&&(e[3]=" "+e[3]+" "),e.slice(0,4)},CHILD:function(e){return e[1]=e[1].toLowerCase(),"nth"===e[1].slice(0,3)?(e[3]||oe.error(e[0]),e[4]=+(e[4]?e[5]+(e[6]||1):2*("even"===e[3]||"odd"===e[3])),e[5]=+(e[7]+e[8]||"odd"===e[3])):e[3]&&oe.error(e[0]),e},PSEUDO:function(e){var t,n=!e[6]&&e[2];return V.CHILD.test(e[0])?null:(e[3]?e[2]=e[4]||e[5]||"":n&&X.test(n)&&(t=a(n,!0))&&(t=n.indexOf(")",n.length-t)-n.length)&&(e[0]=e[0].slice(0,t),e[2]=n.slice(0,t)),e.slice(0,3))}},filter:{TAG:function(e){var t=e.replace(Z,ee).toLowerCase();return"*"===e?function(){return!0}:function(e){return e.nodeName&&e.nodeName.toLowerCase()===t}},CLASS:function(e){var t=E[e+" "];return t||(t=new RegExp("(^|"+M+")"+e+"("+M+"|$)"))&&E(e,function(e){return t.test("string"==typeof e.className&&e.className||"undefined"!=typeof e.getAttribute&&e.getAttribute("class")||"")})},ATTR:function(e,t,n){return function(r){var i=oe.attr(r,e);return null==i?"!="===t:!t||(i+="","="===t?i===n:"!="===t?i!==n:"^="===t?n&&0===i.indexOf(n):"*="===t?n&&i.indexOf(n)>-1:"$="===t?n&&i.slice(-n.length)===n:"~="===t?(" "+i.replace($," ")+" ").indexOf(n)>-1:"|="===t&&(i===n||i.slice(0,n.length+1)===n+"-"))}},CHILD:function(e,t,n,r,i){var o="nth"!==e.slice(0,3),a="last"!==e.slice(-4),s="of-type"===t;return 1===r&&0===i?function(e){return!!e.parentNode}:function(t,n,u){var l,c,f,p,d,h,g=o!==a?"nextSibling":"previousSibling",y=t.parentNode,v=s&&t.nodeName.toLowerCase(),m=!u&&!s,x=!1;if(y){if(o){while(g){p=t;while(p=p[g])if(s?p.nodeName.toLowerCase()===v:1===p.nodeType)return!1;h=g="only"===e&&!h&&"nextSibling"}return!0}if(h=[a?y.firstChild:y.lastChild],a&&m){x=(d=(l=(c=(f=(p=y)[b]||(p[b]={}))[p.uniqueID]||(f[p.uniqueID]={}))[e]||[])[0]===T&&l[1])&&l[2],p=d&&y.childNodes[d];while(p=++d&&p&&p[g]||(x=d=0)||h.pop())if(1===p.nodeType&&++x&&p===t){c[e]=[T,d,x];break}}else if(m&&(x=d=(l=(c=(f=(p=t)[b]||(p[b]={}))[p.uniqueID]||(f[p.uniqueID]={}))[e]||[])[0]===T&&l[1]),!1===x)while(p=++d&&p&&p[g]||(x=d=0)||h.pop())if((s?p.nodeName.toLowerCase()===v:1===p.nodeType)&&++x&&(m&&((c=(f=p[b]||(p[b]={}))[p.uniqueID]||(f[p.uniqueID]={}))[e]=[T,x]),p===t))break;return(x-=i)===r||x%r==0&&x/r>=0}}},PSEUDO:function(e,t){var n,i=r.pseudos[e]||r.setFilters[e.toLowerCase()]||oe.error("unsupported pseudo: "+e);return i[b]?i(t):i.length>1?(n=[e,e,"",t],r.setFilters.hasOwnProperty(e.toLowerCase())?se(function(e,n){var r,o=i(e,t),a=o.length;while(a--)e[r=O(e,o[a])]=!(n[r]=o[a])}):function(e){return i(e,0,n)}):i}},pseudos:{not:se(function(e){var t=[],n=[],r=s(e.replace(B,"$1"));return r[b]?se(function(e,t,n,i){var o,a=r(e,null,i,[]),s=e.length;while(s--)(o=a[s])&&(e[s]=!(t[s]=o))}):function(e,i,o){return t[0]=e,r(t,null,o,n),t[0]=null,!n.pop()}}),has:se(function(e){return function(t){return oe(e,t).length>0}}),contains:se(function(e){return e=e.replace(Z,ee),function(t){return(t.textContent||t.innerText||i(t)).indexOf(e)>-1}}),lang:se(function(e){return U.test(e||"")||oe.error("unsupported lang: "+e),e=e.replace(Z,ee).toLowerCase(),function(t){var n;do{if(n=g?t.lang:t.getAttribute("xml:lang")||t.getAttribute("lang"))return(n=n.toLowerCase())===e||0===n.indexOf(e+"-")}while((t=t.parentNode)&&1===t.nodeType);return!1}}),target:function(t){var n=e.location&&e.location.hash;return n&&n.slice(1)===t.id},root:function(e){return e===h},focus:function(e){return e===d.activeElement&&(!d.hasFocus||d.hasFocus())&&!!(e.type||e.href||~e.tabIndex)},enabled:de(!1),disabled:de(!0),checked:function(e){var t=e.nodeName.toLowerCase();return"input"===t&&!!e.checked||"option"===t&&!!e.selected},selected:function(e){return e.parentNode&&e.parentNode.selectedIndex,!0===e.selected},empty:function(e){for(e=e.firstChild;e;e=e.nextSibling)if(e.nodeType<6)return!1;return!0},parent:function(e){return!r.pseudos.empty(e)},header:function(e){return Y.test(e.nodeName)},input:function(e){return G.test(e.nodeName)},button:function(e){var t=e.nodeName.toLowerCase();return"input"===t&&"button"===e.type||"button"===t},text:function(e){var t;return"input"===e.nodeName.toLowerCase()&&"text"===e.type&&(null==(t=e.getAttribute("type"))||"text"===t.toLowerCase())},first:he(function(){return[0]}),last:he(function(e,t){return[t-1]}),eq:he(function(e,t,n){return[n<0?n+t:n]}),even:he(function(e,t){for(var n=0;n<t;n+=2)e.push(n);return e}),odd:he(function(e,t){for(var n=1;n<t;n+=2)e.push(n);return e}),lt:he(function(e,t,n){for(var r=n<0?n+t:n;--r>=0;)e.push(r);return e}),gt:he(function(e,t,n){for(var r=n<0?n+t:n;++r<t;)e.push(r);return e})}}).pseudos.nth=r.pseudos.eq;for(t in{radio:!0,checkbox:!0,file:!0,password:!0,image:!0})r.pseudos[t]=fe(t);for(t in{submit:!0,reset:!0})r.pseudos[t]=pe(t);function ye(){}ye.prototype=r.filters=r.pseudos,r.setFilters=new ye,a=oe.tokenize=function(e,t){var n,i,o,a,s,u,l,c=k[e+" "];if(c)return t?0:c.slice(0);s=e,u=[],l=r.preFilter;while(s){n&&!(i=F.exec(s))||(i&&(s=s.slice(i[0].length)||s),u.push(o=[])),n=!1,(i=_.exec(s))&&(n=i.shift(),o.push({value:n,type:i[0].replace(B," ")}),s=s.slice(n.length));for(a in r.filter)!(i=V[a].exec(s))||l[a]&&!(i=l[a](i))||(n=i.shift(),o.push({value:n,type:a,matches:i}),s=s.slice(n.length));if(!n)break}return t?s.length:s?oe.error(e):k(e,u).slice(0)};function ve(e){for(var t=0,n=e.length,r="";t<n;t++)r+=e[t].value;return r}function me(e,t,n){var r=t.dir,i=t.next,o=i||r,a=n&&"parentNode"===o,s=C++;return t.first?function(t,n,i){while(t=t[r])if(1===t.nodeType||a)return e(t,n,i);return!1}:function(t,n,u){var l,c,f,p=[T,s];if(u){while(t=t[r])if((1===t.nodeType||a)&&e(t,n,u))return!0}else while(t=t[r])if(1===t.nodeType||a)if(f=t[b]||(t[b]={}),c=f[t.uniqueID]||(f[t.uniqueID]={}),i&&i===t.nodeName.toLowerCase())t=t[r]||t;else{if((l=c[o])&&l[0]===T&&l[1]===s)return p[2]=l[2];if(c[o]=p,p[2]=e(t,n,u))return!0}return!1}}function xe(e){return e.length>1?function(t,n,r){var i=e.length;while(i--)if(!e[i](t,n,r))return!1;return!0}:e[0]}function be(e,t,n){for(var r=0,i=t.length;r<i;r++)oe(e,t[r],n);return n}function we(e,t,n,r,i){for(var o,a=[],s=0,u=e.length,l=null!=t;s<u;s++)(o=e[s])&&(n&&!n(o,r,i)||(a.push(o),l&&t.push(s)));return a}function Te(e,t,n,r,i,o){return r&&!r[b]&&(r=Te(r)),i&&!i[b]&&(i=Te(i,o)),se(function(o,a,s,u){var l,c,f,p=[],d=[],h=a.length,g=o||be(t||"*",s.nodeType?[s]:s,[]),y=!e||!o&&t?g:we(g,p,e,s,u),v=n?i||(o?e:h||r)?[]:a:y;if(n&&n(y,v,s,u),r){l=we(v,d),r(l,[],s,u),c=l.length;while(c--)(f=l[c])&&(v[d[c]]=!(y[d[c]]=f))}if(o){if(i||e){if(i){l=[],c=v.length;while(c--)(f=v[c])&&l.push(y[c]=f);i(null,v=[],l,u)}c=v.length;while(c--)(f=v[c])&&(l=i?O(o,f):p[c])>-1&&(o[l]=!(a[l]=f))}}else v=we(v===a?v.splice(h,v.length):v),i?i(null,a,v,u):L.apply(a,v)})}function Ce(e){for(var t,n,i,o=e.length,a=r.relative[e[0].type],s=a||r.relative[" "],u=a?1:0,c=me(function(e){return e===t},s,!0),f=me(function(e){return O(t,e)>-1},s,!0),p=[function(e,n,r){var i=!a&&(r||n!==l)||((t=n).nodeType?c(e,n,r):f(e,n,r));return t=null,i}];u<o;u++)if(n=r.relative[e[u].type])p=[me(xe(p),n)];else{if((n=r.filter[e[u].type].apply(null,e[u].matches))[b]){for(i=++u;i<o;i++)if(r.relative[e[i].type])break;return Te(u>1&&xe(p),u>1&&ve(e.slice(0,u-1).concat({value:" "===e[u-2].type?"*":""})).replace(B,"$1"),n,u<i&&Ce(e.slice(u,i)),i<o&&Ce(e=e.slice(i)),i<o&&ve(e))}p.push(n)}return xe(p)}function Ee(e,t){var n=t.length>0,i=e.length>0,o=function(o,a,s,u,c){var f,h,y,v=0,m="0",x=o&&[],b=[],w=l,C=o||i&&r.find.TAG("*",c),E=T+=null==w?1:Math.random()||.1,k=C.length;for(c&&(l=a===d||a||c);m!==k&&null!=(f=C[m]);m++){if(i&&f){h=0,a||f.ownerDocument===d||(p(f),s=!g);while(y=e[h++])if(y(f,a||d,s)){u.push(f);break}c&&(T=E)}n&&((f=!y&&f)&&v--,o&&x.push(f))}if(v+=m,n&&m!==v){h=0;while(y=t[h++])y(x,b,a,s);if(o){if(v>0)while(m--)x[m]||b[m]||(b[m]=j.call(u));b=we(b)}L.apply(u,b),c&&!o&&b.length>0&&v+t.length>1&&oe.uniqueSort(u)}return c&&(T=E,l=w),x};return n?se(o):o}return s=oe.compile=function(e,t){var n,r=[],i=[],o=S[e+" "];if(!o){t||(t=a(e)),n=t.length;while(n--)(o=Ce(t[n]))[b]?r.push(o):i.push(o);(o=S(e,Ee(i,r))).selector=e}return o},u=oe.select=function(e,t,n,i){var o,u,l,c,f,p="function"==typeof e&&e,d=!i&&a(e=p.selector||e);if(n=n||[],1===d.length){if((u=d[0]=d[0].slice(0)).length>2&&"ID"===(l=u[0]).type&&9===t.nodeType&&g&&r.relative[u[1].type]){if(!(t=(r.find.ID(l.matches[0].replace(Z,ee),t)||[])[0]))return n;p&&(t=t.parentNode),e=e.slice(u.shift().value.length)}o=V.needsContext.test(e)?0:u.length;while(o--){if(l=u[o],r.relative[c=l.type])break;if((f=r.find[c])&&(i=f(l.matches[0].replace(Z,ee),K.test(u[0].type)&&ge(t.parentNode)||t))){if(u.splice(o,1),!(e=i.length&&ve(u)))return L.apply(n,i),n;break}}}return(p||s(e,d))(i,t,!g,n,!t||K.test(e)&&ge(t.parentNode)||t),n},n.sortStable=b.split("").sort(D).join("")===b,n.detectDuplicates=!!f,p(),n.sortDetached=ue(function(e){return 1&e.compareDocumentPosition(d.createElement("fieldset"))}),ue(function(e){return e.innerHTML="<a href='#'></a>","#"===e.firstChild.getAttribute("href")})||le("type|href|height|width",function(e,t,n){if(!n)return e.getAttribute(t,"type"===t.toLowerCase()?1:2)}),n.attributes&&ue(function(e){return e.innerHTML="<input/>",e.firstChild.setAttribute("value",""),""===e.firstChild.getAttribute("value")})||le("value",function(e,t,n){if(!n&&"input"===e.nodeName.toLowerCase())return e.defaultValue}),ue(function(e){return null==e.getAttribute("disabled")})||le(P,function(e,t,n){var r;if(!n)return!0===e[t]?t.toLowerCase():(r=e.getAttributeNode(t))&&r.specified?r.value:null}),oe}(e);w.find=E,w.expr=E.selectors,w.expr[":"]=w.expr.pseudos,w.uniqueSort=w.unique=E.uniqueSort,w.text=E.getText,w.isXMLDoc=E.isXML,w.contains=E.contains,w.escapeSelector=E.escape;var k=function(e,t,n){var r=[],i=void 0!==n;while((e=e[t])&&9!==e.nodeType)if(1===e.nodeType){if(i&&w(e).is(n))break;r.push(e)}return r},S=function(e,t){for(var n=[];e;e=e.nextSibling)1===e.nodeType&&e!==t&&n.push(e);return n},D=w.expr.match.needsContext;function N(e,t){return e.nodeName&&e.nodeName.toLowerCase()===t.toLowerCase()}var A=/^<([a-z][^\/\0>:\x20\t\r\n\f]*)[\x20\t\r\n\f]*\/?>(?:<\/\1>|)$/i;function j(e,t,n){return g(t)?w.grep(e,function(e,r){return!!t.call(e,r,e)!==n}):t.nodeType?w.grep(e,function(e){return e===t!==n}):"string"!=typeof t?w.grep(e,function(e){return u.call(t,e)>-1!==n}):w.filter(t,e,n)}w.filter=function(e,t,n){var r=t[0];return n&&(e=":not("+e+")"),1===t.length&&1===r.nodeType?w.find.matchesSelector(r,e)?[r]:[]:w.find.matches(e,w.grep(t,function(e){return 1===e.nodeType}))},w.fn.extend({find:function(e){var t,n,r=this.length,i=this;if("string"!=typeof e)return this.pushStack(w(e).filter(function(){for(t=0;t<r;t++)if(w.contains(i[t],this))return!0}));for(n=this.pushStack([]),t=0;t<r;t++)w.find(e,i[t],n);return r>1?w.uniqueSort(n):n},filter:function(e){return this.pushStack(j(this,e||[],!1))},not:function(e){return this.pushStack(j(this,e||[],!0))},is:function(e){return!!j(this,"string"==typeof e&&D.test(e)?w(e):e||[],!1).length}});var q,L=/^(?:\s*(<[\w\W]+>)[^>]*|#([\w-]+))$/;(w.fn.init=function(e,t,n){var i,o;if(!e)return this;if(n=n||q,"string"==typeof e){if(!(i="<"===e[0]&&">"===e[e.length-1]&&e.length>=3?[null,e,null]:L.exec(e))||!i[1]&&t)return!t||t.jquery?(t||n).find(e):this.constructor(t).find(e);if(i[1]){if(t=t instanceof w?t[0]:t,w.merge(this,w.parseHTML(i[1],t&&t.nodeType?t.ownerDocument||t:r,!0)),A.test(i[1])&&w.isPlainObject(t))for(i in t)g(this[i])?this[i](t[i]):this.attr(i,t[i]);return this}return(o=r.getElementById(i[2]))&&(this[0]=o,this.length=1),this}return e.nodeType?(this[0]=e,this.length=1,this):g(e)?void 0!==n.ready?n.ready(e):e(w):w.makeArray(e,this)}).prototype=w.fn,q=w(r);var H=/^(?:parents|prev(?:Until|All))/,O={children:!0,contents:!0,next:!0,prev:!0};w.fn.extend({has:function(e){var t=w(e,this),n=t.length;return this.filter(function(){for(var e=0;e<n;e++)if(w.contains(this,t[e]))return!0})},closest:function(e,t){var n,r=0,i=this.length,o=[],a="string"!=typeof e&&w(e);if(!D.test(e))for(;r<i;r++)for(n=this[r];n&&n!==t;n=n.parentNode)if(n.nodeType<11&&(a?a.index(n)>-1:1===n.nodeType&&w.find.matchesSelector(n,e))){o.push(n);break}return this.pushStack(o.length>1?w.uniqueSort(o):o)},index:function(e){return e?"string"==typeof e?u.call(w(e),this[0]):u.call(this,e.jquery?e[0]:e):this[0]&&this[0].parentNode?this.first().prevAll().length:-1},add:function(e,t){return this.pushStack(w.uniqueSort(w.merge(this.get(),w(e,t))))},addBack:function(e){return this.add(null==e?this.prevObject:this.prevObject.filter(e))}});function P(e,t){while((e=e[t])&&1!==e.nodeType);return e}w.each({parent:function(e){var t=e.parentNode;return t&&11!==t.nodeType?t:null},parents:function(e){return k(e,"parentNode")},parentsUntil:function(e,t,n){return k(e,"parentNode",n)},next:function(e){return P(e,"nextSibling")},prev:function(e){return P(e,"previousSibling")},nextAll:function(e){return k(e,"nextSibling")},prevAll:function(e){return k(e,"previousSibling")},nextUntil:function(e,t,n){return k(e,"nextSibling",n)},prevUntil:function(e,t,n){return k(e,"previousSibling",n)},siblings:function(e){return S((e.parentNode||{}).firstChild,e)},children:function(e){return S(e.firstChild)},contents:function(e){return N(e,"iframe")?e.contentDocument:(N(e,"template")&&(e=e.content||e),w.merge([],e.childNodes))}},function(e,t){w.fn[e]=function(n,r){var i=w.map(this,t,n);return"Until"!==e.slice(-5)&&(r=n),r&&"string"==typeof r&&(i=w.filter(r,i)),this.length>1&&(O[e]||w.uniqueSort(i),H.test(e)&&i.reverse()),this.pushStack(i)}});var M=/[^\x20\t\r\n\f]+/g;function R(e){var t={};return w.each(e.match(M)||[],function(e,n){t[n]=!0}),t}w.Callbacks=function(e){e="string"==typeof e?R(e):w.extend({},e);var t,n,r,i,o=[],a=[],s=-1,u=function(){for(i=i||e.once,r=t=!0;a.length;s=-1){n=a.shift();while(++s<o.length)!1===o[s].apply(n[0],n[1])&&e.stopOnFalse&&(s=o.length,n=!1)}e.memory||(n=!1),t=!1,i&&(o=n?[]:"")},l={add:function(){return o&&(n&&!t&&(s=o.length-1,a.push(n)),function t(n){w.each(n,function(n,r){g(r)?e.unique&&l.has(r)||o.push(r):r&&r.length&&"string"!==x(r)&&t(r)})}(arguments),n&&!t&&u()),this},remove:function(){return w.each(arguments,function(e,t){var n;while((n=w.inArray(t,o,n))>-1)o.splice(n,1),n<=s&&s--}),this},has:function(e){return e?w.inArray(e,o)>-1:o.length>0},empty:function(){return o&&(o=[]),this},disable:function(){return i=a=[],o=n="",this},disabled:function(){return!o},lock:function(){return i=a=[],n||t||(o=n=""),this},locked:function(){return!!i},fireWith:function(e,n){return i||(n=[e,(n=n||[]).slice?n.slice():n],a.push(n),t||u()),this},fire:function(){return l.fireWith(this,arguments),this},fired:function(){return!!r}};return l};function I(e){return e}function W(e){throw e}function $(e,t,n,r){var i;try{e&&g(i=e.promise)?i.call(e).done(t).fail(n):e&&g(i=e.then)?i.call(e,t,n):t.apply(void 0,[e].slice(r))}catch(e){n.apply(void 0,[e])}}w.extend({Deferred:function(t){var n=[["notify","progress",w.Callbacks("memory"),w.Callbacks("memory"),2],["resolve","done",w.Callbacks("once memory"),w.Callbacks("once memory"),0,"resolved"],["reject","fail",w.Callbacks("once memory"),w.Callbacks("once memory"),1,"rejected"]],r="pending",i={state:function(){return r},always:function(){return o.done(arguments).fail(arguments),this},"catch":function(e){return i.then(null,e)},pipe:function(){var e=arguments;return w.Deferred(function(t){w.each(n,function(n,r){var i=g(e[r[4]])&&e[r[4]];o[r[1]](function(){var e=i&&i.apply(this,arguments);e&&g(e.promise)?e.promise().progress(t.notify).done(t.resolve).fail(t.reject):t[r[0]+"With"](this,i?[e]:arguments)})}),e=null}).promise()},then:function(t,r,i){var o=0;function a(t,n,r,i){return function(){var s=this,u=arguments,l=function(){var e,l;if(!(t<o)){if((e=r.apply(s,u))===n.promise())throw new TypeError("Thenable self-resolution");l=e&&("object"==typeof e||"function"==typeof e)&&e.then,g(l)?i?l.call(e,a(o,n,I,i),a(o,n,W,i)):(o++,l.call(e,a(o,n,I,i),a(o,n,W,i),a(o,n,I,n.notifyWith))):(r!==I&&(s=void 0,u=[e]),(i||n.resolveWith)(s,u))}},c=i?l:function(){try{l()}catch(e){w.Deferred.exceptionHook&&w.Deferred.exceptionHook(e,c.stackTrace),t+1>=o&&(r!==W&&(s=void 0,u=[e]),n.rejectWith(s,u))}};t?c():(w.Deferred.getStackHook&&(c.stackTrace=w.Deferred.getStackHook()),e.setTimeout(c))}}return w.Deferred(function(e){n[0][3].add(a(0,e,g(i)?i:I,e.notifyWith)),n[1][3].add(a(0,e,g(t)?t:I)),n[2][3].add(a(0,e,g(r)?r:W))}).promise()},promise:function(e){return null!=e?w.extend(e,i):i}},o={};return w.each(n,function(e,t){var a=t[2],s=t[5];i[t[1]]=a.add,s&&a.add(function(){r=s},n[3-e][2].disable,n[3-e][3].disable,n[0][2].lock,n[0][3].lock),a.add(t[3].fire),o[t[0]]=function(){return o[t[0]+"With"](this===o?void 0:this,arguments),this},o[t[0]+"With"]=a.fireWith}),i.promise(o),t&&t.call(o,o),o},when:function(e){var t=arguments.length,n=t,r=Array(n),i=o.call(arguments),a=w.Deferred(),s=function(e){return function(n){r[e]=this,i[e]=arguments.length>1?o.call(arguments):n,--t||a.resolveWith(r,i)}};if(t<=1&&($(e,a.done(s(n)).resolve,a.reject,!t),"pending"===a.state()||g(i[n]&&i[n].then)))return a.then();while(n--)$(i[n],s(n),a.reject);return a.promise()}});var B=/^(Eval|Internal|Range|Reference|Syntax|Type|URI)Error$/;w.Deferred.exceptionHook=function(t,n){e.console&&e.console.warn&&t&&B.test(t.name)&&e.console.warn("jQuery.Deferred exception: "+t.message,t.stack,n)},w.readyException=function(t){e.setTimeout(function(){throw t})};var F=w.Deferred();w.fn.ready=function(e){return F.then(e)["catch"](function(e){w.readyException(e)}),this},w.extend({isReady:!1,readyWait:1,ready:function(e){(!0===e?--w.readyWait:w.isReady)||(w.isReady=!0,!0!==e&&--w.readyWait>0||F.resolveWith(r,[w]))}}),w.ready.then=F.then;function _(){r.removeEventListener("DOMContentLoaded",_),e.removeEventListener("load",_),w.ready()}"complete"===r.readyState||"loading"!==r.readyState&&!r.documentElement.doScroll?e.setTimeout(w.ready):(r.addEventListener("DOMContentLoaded",_),e.addEventListener("load",_));var z=function(e,t,n,r,i,o,a){var s=0,u=e.length,l=null==n;if("object"===x(n)){i=!0;for(s in n)z(e,t,s,n[s],!0,o,a)}else if(void 0!==r&&(i=!0,g(r)||(a=!0),l&&(a?(t.call(e,r),t=null):(l=t,t=function(e,t,n){return l.call(w(e),n)})),t))for(;s<u;s++)t(e[s],n,a?r:r.call(e[s],s,t(e[s],n)));return i?e:l?t.call(e):u?t(e[0],n):o},X=/^-ms-/,U=/-([a-z])/g;function V(e,t){return t.toUpperCase()}function G(e){return e.replace(X,"ms-").replace(U,V)}var Y=function(e){return 1===e.nodeType||9===e.nodeType||!+e.nodeType};function Q(){this.expando=w.expando+Q.uid++}Q.uid=1,Q.prototype={cache:function(e){var t=e[this.expando];return t||(t={},Y(e)&&(e.nodeType?e[this.expando]=t:Object.defineProperty(e,this.expando,{value:t,configurable:!0}))),t},set:function(e,t,n){var r,i=this.cache(e);if("string"==typeof t)i[G(t)]=n;else for(r in t)i[G(r)]=t[r];return i},get:function(e,t){return void 0===t?this.cache(e):e[this.expando]&&e[this.expando][G(t)]},access:function(e,t,n){return void 0===t||t&&"string"==typeof t&&void 0===n?this.get(e,t):(this.set(e,t,n),void 0!==n?n:t)},remove:function(e,t){var n,r=e[this.expando];if(void 0!==r){if(void 0!==t){n=(t=Array.isArray(t)?t.map(G):(t=G(t))in r?[t]:t.match(M)||[]).length;while(n--)delete r[t[n]]}(void 0===t||w.isEmptyObject(r))&&(e.nodeType?e[this.expando]=void 0:delete e[this.expando])}},hasData:function(e){var t=e[this.expando];return void 0!==t&&!w.isEmptyObject(t)}};var J=new Q,K=new Q,Z=/^(?:\{[\w\W]*\}|\[[\w\W]*\])$/,ee=/[A-Z]/g;function te(e){return"true"===e||"false"!==e&&("null"===e?null:e===+e+""?+e:Z.test(e)?JSON.parse(e):e)}function ne(e,t,n){var r;if(void 0===n&&1===e.nodeType)if(r="data-"+t.replace(ee,"-$&").toLowerCase(),"string"==typeof(n=e.getAttribute(r))){try{n=te(n)}catch(e){}K.set(e,t,n)}else n=void 0;return n}w.extend({hasData:function(e){return K.hasData(e)||J.hasData(e)},data:function(e,t,n){return K.access(e,t,n)},removeData:function(e,t){K.remove(e,t)},_data:function(e,t,n){return J.access(e,t,n)},_removeData:function(e,t){J.remove(e,t)}}),w.fn.extend({data:function(e,t){var n,r,i,o=this[0],a=o&&o.attributes;if(void 0===e){if(this.length&&(i=K.get(o),1===o.nodeType&&!J.get(o,"hasDataAttrs"))){n=a.length;while(n--)a[n]&&0===(r=a[n].name).indexOf("data-")&&(r=G(r.slice(5)),ne(o,r,i[r]));J.set(o,"hasDataAttrs",!0)}return i}return"object"==typeof e?this.each(function(){K.set(this,e)}):z(this,function(t){var n;if(o&&void 0===t){if(void 0!==(n=K.get(o,e)))return n;if(void 0!==(n=ne(o,e)))return n}else this.each(function(){K.set(this,e,t)})},null,t,arguments.length>1,null,!0)},removeData:function(e){return this.each(function(){K.remove(this,e)})}}),w.extend({queue:function(e,t,n){var r;if(e)return t=(t||"fx")+"queue",r=J.get(e,t),n&&(!r||Array.isArray(n)?r=J.access(e,t,w.makeArray(n)):r.push(n)),r||[]},dequeue:function(e,t){t=t||"fx";var n=w.queue(e,t),r=n.length,i=n.shift(),o=w._queueHooks(e,t),a=function(){w.dequeue(e,t)};"inprogress"===i&&(i=n.shift(),r--),i&&("fx"===t&&n.unshift("inprogress"),delete o.stop,i.call(e,a,o)),!r&&o&&o.empty.fire()},_queueHooks:function(e,t){var n=t+"queueHooks";return J.get(e,n)||J.access(e,n,{empty:w.Callbacks("once memory").add(function(){J.remove(e,[t+"queue",n])})})}}),w.fn.extend({queue:function(e,t){var n=2;return"string"!=typeof e&&(t=e,e="fx",n--),arguments.length<n?w.queue(this[0],e):void 0===t?this:this.each(function(){var n=w.queue(this,e,t);w._queueHooks(this,e),"fx"===e&&"inprogress"!==n[0]&&w.dequeue(this,e)})},dequeue:function(e){return this.each(function(){w.dequeue(this,e)})},clearQueue:function(e){return this.queue(e||"fx",[])},promise:function(e,t){var n,r=1,i=w.Deferred(),o=this,a=this.length,s=function(){--r||i.resolveWith(o,[o])};"string"!=typeof e&&(t=e,e=void 0),e=e||"fx";while(a--)(n=J.get(o[a],e+"queueHooks"))&&n.empty&&(r++,n.empty.add(s));return s(),i.promise(t)}});var re=/[+-]?(?:\d*\.|)\d+(?:[eE][+-]?\d+|)/.source,ie=new RegExp("^(?:([+-])=|)("+re+")([a-z%]*)$","i"),oe=["Top","Right","Bottom","Left"],ae=function(e,t){return"none"===(e=t||e).style.display||""===e.style.display&&w.contains(e.ownerDocument,e)&&"none"===w.css(e,"display")},se=function(e,t,n,r){var i,o,a={};for(o in t)a[o]=e.style[o],e.style[o]=t[o];i=n.apply(e,r||[]);for(o in t)e.style[o]=a[o];return i};function ue(e,t,n,r){var i,o,a=20,s=r?function(){return r.cur()}:function(){return w.css(e,t,"")},u=s(),l=n&&n[3]||(w.cssNumber[t]?"":"px"),c=(w.cssNumber[t]||"px"!==l&&+u)&&ie.exec(w.css(e,t));if(c&&c[3]!==l){u/=2,l=l||c[3],c=+u||1;while(a--)w.style(e,t,c+l),(1-o)*(1-(o=s()/u||.5))<=0&&(a=0),c/=o;c*=2,w.style(e,t,c+l),n=n||[]}return n&&(c=+c||+u||0,i=n[1]?c+(n[1]+1)*n[2]:+n[2],r&&(r.unit=l,r.start=c,r.end=i)),i}var le={};function ce(e){var t,n=e.ownerDocument,r=e.nodeName,i=le[r];return i||(t=n.body.appendChild(n.createElement(r)),i=w.css(t,"display"),t.parentNode.removeChild(t),"none"===i&&(i="block"),le[r]=i,i)}function fe(e,t){for(var n,r,i=[],o=0,a=e.length;o<a;o++)(r=e[o]).style&&(n=r.style.display,t?("none"===n&&(i[o]=J.get(r,"display")||null,i[o]||(r.style.display="")),""===r.style.display&&ae(r)&&(i[o]=ce(r))):"none"!==n&&(i[o]="none",J.set(r,"display",n)));for(o=0;o<a;o++)null!=i[o]&&(e[o].style.display=i[o]);return e}w.fn.extend({show:function(){return fe(this,!0)},hide:function(){return fe(this)},toggle:function(e){return"boolean"==typeof e?e?this.show():this.hide():this.each(function(){ae(this)?w(this).show():w(this).hide()})}});var pe=/^(?:checkbox|radio)$/i,de=/<([a-z][^\/\0>\x20\t\r\n\f]+)/i,he=/^$|^module$|\/(?:java|ecma)script/i,ge={option:[1,"<select multiple='multiple'>","</select>"],thead:[1,"<table>","</table>"],col:[2,"<table><colgroup>","</colgroup></table>"],tr:[2,"<table><tbody>","</tbody></table>"],td:[3,"<table><tbody><tr>","</tr></tbody></table>"],_default:[0,"",""]};ge.optgroup=ge.option,ge.tbody=ge.tfoot=ge.colgroup=ge.caption=ge.thead,ge.th=ge.td;function ye(e,t){var n;return n="undefined"!=typeof e.getElementsByTagName?e.getElementsByTagName(t||"*"):"undefined"!=typeof e.querySelectorAll?e.querySelectorAll(t||"*"):[],void 0===t||t&&N(e,t)?w.merge([e],n):n}function ve(e,t){for(var n=0,r=e.length;n<r;n++)J.set(e[n],"globalEval",!t||J.get(t[n],"globalEval"))}var me=/<|&#?\w+;/;function xe(e,t,n,r,i){for(var o,a,s,u,l,c,f=t.createDocumentFragment(),p=[],d=0,h=e.length;d<h;d++)if((o=e[d])||0===o)if("object"===x(o))w.merge(p,o.nodeType?[o]:o);else if(me.test(o)){a=a||f.appendChild(t.createElement("div")),s=(de.exec(o)||["",""])[1].toLowerCase(),u=ge[s]||ge._default,a.innerHTML=u[1]+w.htmlPrefilter(o)+u[2],c=u[0];while(c--)a=a.lastChild;w.merge(p,a.childNodes),(a=f.firstChild).textContent=""}else p.push(t.createTextNode(o));f.textContent="",d=0;while(o=p[d++])if(r&&w.inArray(o,r)>-1)i&&i.push(o);else if(l=w.contains(o.ownerDocument,o),a=ye(f.appendChild(o),"script"),l&&ve(a),n){c=0;while(o=a[c++])he.test(o.type||"")&&n.push(o)}return f}!function(){var e=r.createDocumentFragment().appendChild(r.createElement("div")),t=r.createElement("input");t.setAttribute("type","radio"),t.setAttribute("checked","checked"),t.setAttribute("name","t"),e.appendChild(t),h.checkClone=e.cloneNode(!0).cloneNode(!0).lastChild.checked,e.innerHTML="<textarea>x</textarea>",h.noCloneChecked=!!e.cloneNode(!0).lastChild.defaultValue}();var be=r.documentElement,we=/^key/,Te=/^(?:mouse|pointer|contextmenu|drag|drop)|click/,Ce=/^([^.]*)(?:\.(.+)|)/;function Ee(){return!0}function ke(){return!1}function Se(){try{return r.activeElement}catch(e){}}function De(e,t,n,r,i,o){var a,s;if("object"==typeof t){"string"!=typeof n&&(r=r||n,n=void 0);for(s in t)De(e,s,n,r,t[s],o);return e}if(null==r&&null==i?(i=n,r=n=void 0):null==i&&("string"==typeof n?(i=r,r=void 0):(i=r,r=n,n=void 0)),!1===i)i=ke;else if(!i)return e;return 1===o&&(a=i,(i=function(e){return w().off(e),a.apply(this,arguments)}).guid=a.guid||(a.guid=w.guid++)),e.each(function(){w.event.add(this,t,i,r,n)})}w.event={global:{},add:function(e,t,n,r,i){var o,a,s,u,l,c,f,p,d,h,g,y=J.get(e);if(y){n.handler&&(n=(o=n).handler,i=o.selector),i&&w.find.matchesSelector(be,i),n.guid||(n.guid=w.guid++),(u=y.events)||(u=y.events={}),(a=y.handle)||(a=y.handle=function(t){return"undefined"!=typeof w&&w.event.triggered!==t.type?w.event.dispatch.apply(e,arguments):void 0}),l=(t=(t||"").match(M)||[""]).length;while(l--)d=g=(s=Ce.exec(t[l])||[])[1],h=(s[2]||"").split(".").sort(),d&&(f=w.event.special[d]||{},d=(i?f.delegateType:f.bindType)||d,f=w.event.special[d]||{},c=w.extend({type:d,origType:g,data:r,handler:n,guid:n.guid,selector:i,needsContext:i&&w.expr.match.needsContext.test(i),namespace:h.join(".")},o),(p=u[d])||((p=u[d]=[]).delegateCount=0,f.setup&&!1!==f.setup.call(e,r,h,a)||e.addEventListener&&e.addEventListener(d,a)),f.add&&(f.add.call(e,c),c.handler.guid||(c.handler.guid=n.guid)),i?p.splice(p.delegateCount++,0,c):p.push(c),w.event.global[d]=!0)}},remove:function(e,t,n,r,i){var o,a,s,u,l,c,f,p,d,h,g,y=J.hasData(e)&&J.get(e);if(y&&(u=y.events)){l=(t=(t||"").match(M)||[""]).length;while(l--)if(s=Ce.exec(t[l])||[],d=g=s[1],h=(s[2]||"").split(".").sort(),d){f=w.event.special[d]||{},p=u[d=(r?f.delegateType:f.bindType)||d]||[],s=s[2]&&new RegExp("(^|\\.)"+h.join("\\.(?:.*\\.|)")+"(\\.|$)"),a=o=p.length;while(o--)c=p[o],!i&&g!==c.origType||n&&n.guid!==c.guid||s&&!s.test(c.namespace)||r&&r!==c.selector&&("**"!==r||!c.selector)||(p.splice(o,1),c.selector&&p.delegateCount--,f.remove&&f.remove.call(e,c));a&&!p.length&&(f.teardown&&!1!==f.teardown.call(e,h,y.handle)||w.removeEvent(e,d,y.handle),delete u[d])}else for(d in u)w.event.remove(e,d+t[l],n,r,!0);w.isEmptyObject(u)&&J.remove(e,"handle events")}},dispatch:function(e){var t=w.event.fix(e),n,r,i,o,a,s,u=new Array(arguments.length),l=(J.get(this,"events")||{})[t.type]||[],c=w.event.special[t.type]||{};for(u[0]=t,n=1;n<arguments.length;n++)u[n]=arguments[n];if(t.delegateTarget=this,!c.preDispatch||!1!==c.preDispatch.call(this,t)){s=w.event.handlers.call(this,t,l),n=0;while((o=s[n++])&&!t.isPropagationStopped()){t.currentTarget=o.elem,r=0;while((a=o.handlers[r++])&&!t.isImmediatePropagationStopped())t.rnamespace&&!t.rnamespace.test(a.namespace)||(t.handleObj=a,t.data=a.data,void 0!==(i=((w.event.special[a.origType]||{}).handle||a.handler).apply(o.elem,u))&&!1===(t.result=i)&&(t.preventDefault(),t.stopPropagation()))}return c.postDispatch&&c.postDispatch.call(this,t),t.result}},handlers:function(e,t){var n,r,i,o,a,s=[],u=t.delegateCount,l=e.target;if(u&&l.nodeType&&!("click"===e.type&&e.button>=1))for(;l!==this;l=l.parentNode||this)if(1===l.nodeType&&("click"!==e.type||!0!==l.disabled)){for(o=[],a={},n=0;n<u;n++)void 0===a[i=(r=t[n]).selector+" "]&&(a[i]=r.needsContext?w(i,this).index(l)>-1:w.find(i,this,null,[l]).length),a[i]&&o.push(r);o.length&&s.push({elem:l,handlers:o})}return l=this,u<t.length&&s.push({elem:l,handlers:t.slice(u)}),s},addProp:function(e,t){Object.defineProperty(w.Event.prototype,e,{enumerable:!0,configurable:!0,get:g(t)?function(){if(this.originalEvent)return t(this.originalEvent)}:function(){if(this.originalEvent)return this.originalEvent[e]},set:function(t){Object.defineProperty(this,e,{enumerable:!0,configurable:!0,writable:!0,value:t})}})},fix:function(e){return e[w.expando]?e:new w.Event(e)},special:{load:{noBubble:!0},focus:{trigger:function(){if(this!==Se()&&this.focus)return this.focus(),!1},delegateType:"focusin"},blur:{trigger:function(){if(this===Se()&&this.blur)return this.blur(),!1},delegateType:"focusout"},click:{trigger:function(){if("checkbox"===this.type&&this.click&&N(this,"input"))return this.click(),!1},_default:function(e){return N(e.target,"a")}},beforeunload:{postDispatch:function(e){void 0!==e.result&&e.originalEvent&&(e.originalEvent.returnValue=e.result)}}}},w.removeEvent=function(e,t,n){e.removeEventListener&&e.removeEventListener(t,n)},w.Event=function(e,t){if(!(this instanceof w.Event))return new w.Event(e,t);e&&e.type?(this.originalEvent=e,this.type=e.type,this.isDefaultPrevented=e.defaultPrevented||void 0===e.defaultPrevented&&!1===e.returnValue?Ee:ke,this.target=e.target&&3===e.target.nodeType?e.target.parentNode:e.target,this.currentTarget=e.currentTarget,this.relatedTarget=e.relatedTarget):this.type=e,t&&w.extend(this,t),this.timeStamp=e&&e.timeStamp||Date.now(),this[w.expando]=!0},w.Event.prototype={constructor:w.Event,isDefaultPrevented:ke,isPropagationStopped:ke,isImmediatePropagationStopped:ke,isSimulated:!1,preventDefault:function(){var e=this.originalEvent;this.isDefaultPrevented=Ee,e&&!this.isSimulated&&e.preventDefault()},stopPropagation:function(){var e=this.originalEvent;this.isPropagationStopped=Ee,e&&!this.isSimulated&&e.stopPropagation()},stopImmediatePropagation:function(){var e=this.originalEvent;this.isImmediatePropagationStopped=Ee,e&&!this.isSimulated&&e.stopImmediatePropagation(),this.stopPropagation()}},w.each({altKey:!0,bubbles:!0,cancelable:!0,changedTouches:!0,ctrlKey:!0,detail:!0,eventPhase:!0,metaKey:!0,pageX:!0,pageY:!0,shiftKey:!0,view:!0,"char":!0,charCode:!0,key:!0,keyCode:!0,button:!0,buttons:!0,clientX:!0,clientY:!0,offsetX:!0,offsetY:!0,pointerId:!0,pointerType:!0,screenX:!0,screenY:!0,targetTouches:!0,toElement:!0,touches:!0,which:function(e){var t=e.button;return null==e.which&&we.test(e.type)?null!=e.charCode?e.charCode:e.keyCode:!e.which&&void 0!==t&&Te.test(e.type)?1&t?1:2&t?3:4&t?2:0:e.which}},w.event.addProp),w.each({mouseenter:"mouseover",mouseleave:"mouseout",pointerenter:"pointerover",pointerleave:"pointerout"},function(e,t){w.event.special[e]={delegateType:t,bindType:t,handle:function(e){var n,r=this,i=e.relatedTarget,o=e.handleObj;return i&&(i===r||w.contains(r,i))||(e.type=o.origType,n=o.handler.apply(this,arguments),e.type=t),n}}}),w.fn.extend({on:function(e,t,n,r){return De(this,e,t,n,r)},one:function(e,t,n,r){return De(this,e,t,n,r,1)},off:function(e,t,n){var r,i;if(e&&e.preventDefault&&e.handleObj)return r=e.handleObj,w(e.delegateTarget).off(r.namespace?r.origType+"."+r.namespace:r.origType,r.selector,r.handler),this;if("object"==typeof e){for(i in e)this.off(i,t,e[i]);return this}return!1!==t&&"function"!=typeof t||(n=t,t=void 0),!1===n&&(n=ke),this.each(function(){w.event.remove(this,e,n,t)})}});var Ne=/<(?!area|br|col|embed|hr|img|input|link|meta|param)(([a-z][^\/\0>\x20\t\r\n\f]*)[^>]*)\/>/gi,Ae=/<script|<style|<link/i,je=/checked\s*(?:[^=]|=\s*.checked.)/i,qe=/^\s*<!(?:\[CDATA\[|--)|(?:\]\]|--)>\s*$/g;function Le(e,t){return N(e,"table")&&N(11!==t.nodeType?t:t.firstChild,"tr")?w(e).children("tbody")[0]||e:e}function He(e){return e.type=(null!==e.getAttribute("type"))+"/"+e.type,e}function Oe(e){return"true/"===(e.type||"").slice(0,5)?e.type=e.type.slice(5):e.removeAttribute("type"),e}function Pe(e,t){var n,r,i,o,a,s,u,l;if(1===t.nodeType){if(J.hasData(e)&&(o=J.access(e),a=J.set(t,o),l=o.events)){delete a.handle,a.events={};for(i in l)for(n=0,r=l[i].length;n<r;n++)w.event.add(t,i,l[i][n])}K.hasData(e)&&(s=K.access(e),u=w.extend({},s),K.set(t,u))}}function Me(e,t){var n=t.nodeName.toLowerCase();"input"===n&&pe.test(e.type)?t.checked=e.checked:"input"!==n&&"textarea"!==n||(t.defaultValue=e.defaultValue)}function Re(e,t,n,r){t=a.apply([],t);var i,o,s,u,l,c,f=0,p=e.length,d=p-1,y=t[0],v=g(y);if(v||p>1&&"string"==typeof y&&!h.checkClone&&je.test(y))return e.each(function(i){var o=e.eq(i);v&&(t[0]=y.call(this,i,o.html())),Re(o,t,n,r)});if(p&&(i=xe(t,e[0].ownerDocument,!1,e,r),o=i.firstChild,1===i.childNodes.length&&(i=o),o||r)){for(u=(s=w.map(ye(i,"script"),He)).length;f<p;f++)l=i,f!==d&&(l=w.clone(l,!0,!0),u&&w.merge(s,ye(l,"script"))),n.call(e[f],l,f);if(u)for(c=s[s.length-1].ownerDocument,w.map(s,Oe),f=0;f<u;f++)l=s[f],he.test(l.type||"")&&!J.access(l,"globalEval")&&w.contains(c,l)&&(l.src&&"module"!==(l.type||"").toLowerCase()?w._evalUrl&&w._evalUrl(l.src):m(l.textContent.replace(qe,""),c,l))}return e}function Ie(e,t,n){for(var r,i=t?w.filter(t,e):e,o=0;null!=(r=i[o]);o++)n||1!==r.nodeType||w.cleanData(ye(r)),r.parentNode&&(n&&w.contains(r.ownerDocument,r)&&ve(ye(r,"script")),r.parentNode.removeChild(r));return e}w.extend({htmlPrefilter:function(e){return e.replace(Ne,"<$1></$2>")},clone:function(e,t,n){var r,i,o,a,s=e.cloneNode(!0),u=w.contains(e.ownerDocument,e);if(!(h.noCloneChecked||1!==e.nodeType&&11!==e.nodeType||w.isXMLDoc(e)))for(a=ye(s),r=0,i=(o=ye(e)).length;r<i;r++)Me(o[r],a[r]);if(t)if(n)for(o=o||ye(e),a=a||ye(s),r=0,i=o.length;r<i;r++)Pe(o[r],a[r]);else Pe(e,s);return(a=ye(s,"script")).length>0&&ve(a,!u&&ye(e,"script")),s},cleanData:function(e){for(var t,n,r,i=w.event.special,o=0;void 0!==(n=e[o]);o++)if(Y(n)){if(t=n[J.expando]){if(t.events)for(r in t.events)i[r]?w.event.remove(n,r):w.removeEvent(n,r,t.handle);n[J.expando]=void 0}n[K.expando]&&(n[K.expando]=void 0)}}}),w.fn.extend({detach:function(e){return Ie(this,e,!0)},remove:function(e){return Ie(this,e)},text:function(e){return z(this,function(e){return void 0===e?w.text(this):this.empty().each(function(){1!==this.nodeType&&11!==this.nodeType&&9!==this.nodeType||(this.textContent=e)})},null,e,arguments.length)},append:function(){return Re(this,arguments,function(e){1!==this.nodeType&&11!==this.nodeType&&9!==this.nodeType||Le(this,e).appendChild(e)})},prepend:function(){return Re(this,arguments,function(e){if(1===this.nodeType||11===this.nodeType||9===this.nodeType){var t=Le(this,e);t.insertBefore(e,t.firstChild)}})},before:function(){return Re(this,arguments,function(e){this.parentNode&&this.parentNode.insertBefore(e,this)})},after:function(){return Re(this,arguments,function(e){this.parentNode&&this.parentNode.insertBefore(e,this.nextSibling)})},empty:function(){for(var e,t=0;null!=(e=this[t]);t++)1===e.nodeType&&(w.cleanData(ye(e,!1)),e.textContent="");return this},clone:function(e,t){return e=null!=e&&e,t=null==t?e:t,this.map(function(){return w.clone(this,e,t)})},html:function(e){return z(this,function(e){var t=this[0]||{},n=0,r=this.length;if(void 0===e&&1===t.nodeType)return t.innerHTML;if("string"==typeof e&&!Ae.test(e)&&!ge[(de.exec(e)||["",""])[1].toLowerCase()]){e=w.htmlPrefilter(e);try{for(;n<r;n++)1===(t=this[n]||{}).nodeType&&(w.cleanData(ye(t,!1)),t.innerHTML=e);t=0}catch(e){}}t&&this.empty().append(e)},null,e,arguments.length)},replaceWith:function(){var e=[];return Re(this,arguments,function(t){var n=this.parentNode;w.inArray(this,e)<0&&(w.cleanData(ye(this)),n&&n.replaceChild(t,this))},e)}}),w.each({appendTo:"append",prependTo:"prepend",insertBefore:"before",insertAfter:"after",replaceAll:"replaceWith"},function(e,t){w.fn[e]=function(e){for(var n,r=[],i=w(e),o=i.length-1,a=0;a<=o;a++)n=a===o?this:this.clone(!0),w(i[a])[t](n),s.apply(r,n.get());return this.pushStack(r)}});var We=new RegExp("^("+re+")(?!px)[a-z%]+$","i"),$e=function(t){var n=t.ownerDocument.defaultView;return n&&n.opener||(n=e),n.getComputedStyle(t)},Be=new RegExp(oe.join("|"),"i");!function(){function t(){if(c){l.style.cssText="position:absolute;left:-11111px;width:60px;margin-top:1px;padding:0;border:0",c.style.cssText="position:relative;display:block;box-sizing:border-box;overflow:scroll;margin:auto;border:1px;padding:1px;width:60%;top:1%",be.appendChild(l).appendChild(c);var t=e.getComputedStyle(c);i="1%"!==t.top,u=12===n(t.marginLeft),c.style.right="60%",s=36===n(t.right),o=36===n(t.width),c.style.position="absolute",a=36===c.offsetWidth||"absolute",be.removeChild(l),c=null}}function n(e){return Math.round(parseFloat(e))}var i,o,a,s,u,l=r.createElement("div"),c=r.createElement("div");c.style&&(c.style.backgroundClip="content-box",c.cloneNode(!0).style.backgroundClip="",h.clearCloneStyle="content-box"===c.style.backgroundClip,w.extend(h,{boxSizingReliable:function(){return t(),o},pixelBoxStyles:function(){return t(),s},pixelPosition:function(){return t(),i},reliableMarginLeft:function(){return t(),u},scrollboxSize:function(){return t(),a}}))}();function Fe(e,t,n){var r,i,o,a,s=e.style;return(n=n||$e(e))&&(""!==(a=n.getPropertyValue(t)||n[t])||w.contains(e.ownerDocument,e)||(a=w.style(e,t)),!h.pixelBoxStyles()&&We.test(a)&&Be.test(t)&&(r=s.width,i=s.minWidth,o=s.maxWidth,s.minWidth=s.maxWidth=s.width=a,a=n.width,s.width=r,s.minWidth=i,s.maxWidth=o)),void 0!==a?a+"":a}function _e(e,t){return{get:function(){if(!e())return(this.get=t).apply(this,arguments);delete this.get}}}var ze=/^(none|table(?!-c[ea]).+)/,Xe=/^--/,Ue={position:"absolute",visibility:"hidden",display:"block"},Ve={letterSpacing:"0",fontWeight:"400"},Ge=["Webkit","Moz","ms"],Ye=r.createElement("div").style;function Qe(e){if(e in Ye)return e;var t=e[0].toUpperCase()+e.slice(1),n=Ge.length;while(n--)if((e=Ge[n]+t)in Ye)return e}function Je(e){var t=w.cssProps[e];return t||(t=w.cssProps[e]=Qe(e)||e),t}function Ke(e,t,n){var r=ie.exec(t);return r?Math.max(0,r[2]-(n||0))+(r[3]||"px"):t}function Ze(e,t,n,r,i,o){var a="width"===t?1:0,s=0,u=0;if(n===(r?"border":"content"))return 0;for(;a<4;a+=2)"margin"===n&&(u+=w.css(e,n+oe[a],!0,i)),r?("content"===n&&(u-=w.css(e,"padding"+oe[a],!0,i)),"margin"!==n&&(u-=w.css(e,"border"+oe[a]+"Width",!0,i))):(u+=w.css(e,"padding"+oe[a],!0,i),"padding"!==n?u+=w.css(e,"border"+oe[a]+"Width",!0,i):s+=w.css(e,"border"+oe[a]+"Width",!0,i));return!r&&o>=0&&(u+=Math.max(0,Math.ceil(e["offset"+t[0].toUpperCase()+t.slice(1)]-o-u-s-.5))),u}function et(e,t,n){var r=$e(e),i=Fe(e,t,r),o="border-box"===w.css(e,"boxSizing",!1,r),a=o;if(We.test(i)){if(!n)return i;i="auto"}return a=a&&(h.boxSizingReliable()||i===e.style[t]),("auto"===i||!parseFloat(i)&&"inline"===w.css(e,"display",!1,r))&&(i=e["offset"+t[0].toUpperCase()+t.slice(1)],a=!0),(i=parseFloat(i)||0)+Ze(e,t,n||(o?"border":"content"),a,r,i)+"px"}w.extend({cssHooks:{opacity:{get:function(e,t){if(t){var n=Fe(e,"opacity");return""===n?"1":n}}}},cssNumber:{animationIterationCount:!0,columnCount:!0,fillOpacity:!0,flexGrow:!0,flexShrink:!0,fontWeight:!0,lineHeight:!0,opacity:!0,order:!0,orphans:!0,widows:!0,zIndex:!0,zoom:!0},cssProps:{},style:function(e,t,n,r){if(e&&3!==e.nodeType&&8!==e.nodeType&&e.style){var i,o,a,s=G(t),u=Xe.test(t),l=e.style;if(u||(t=Je(s)),a=w.cssHooks[t]||w.cssHooks[s],void 0===n)return a&&"get"in a&&void 0!==(i=a.get(e,!1,r))?i:l[t];"string"==(o=typeof n)&&(i=ie.exec(n))&&i[1]&&(n=ue(e,t,i),o="number"),null!=n&&n===n&&("number"===o&&(n+=i&&i[3]||(w.cssNumber[s]?"":"px")),h.clearCloneStyle||""!==n||0!==t.indexOf("background")||(l[t]="inherit"),a&&"set"in a&&void 0===(n=a.set(e,n,r))||(u?l.setProperty(t,n):l[t]=n))}},css:function(e,t,n,r){var i,o,a,s=G(t);return Xe.test(t)||(t=Je(s)),(a=w.cssHooks[t]||w.cssHooks[s])&&"get"in a&&(i=a.get(e,!0,n)),void 0===i&&(i=Fe(e,t,r)),"normal"===i&&t in Ve&&(i=Ve[t]),""===n||n?(o=parseFloat(i),!0===n||isFinite(o)?o||0:i):i}}),w.each(["height","width"],function(e,t){w.cssHooks[t]={get:function(e,n,r){if(n)return!ze.test(w.css(e,"display"))||e.getClientRects().length&&e.getBoundingClientRect().width?et(e,t,r):se(e,Ue,function(){return et(e,t,r)})},set:function(e,n,r){var i,o=$e(e),a="border-box"===w.css(e,"boxSizing",!1,o),s=r&&Ze(e,t,r,a,o);return a&&h.scrollboxSize()===o.position&&(s-=Math.ceil(e["offset"+t[0].toUpperCase()+t.slice(1)]-parseFloat(o[t])-Ze(e,t,"border",!1,o)-.5)),s&&(i=ie.exec(n))&&"px"!==(i[3]||"px")&&(e.style[t]=n,n=w.css(e,t)),Ke(e,n,s)}}}),w.cssHooks.marginLeft=_e(h.reliableMarginLeft,function(e,t){if(t)return(parseFloat(Fe(e,"marginLeft"))||e.getBoundingClientRect().left-se(e,{marginLeft:0},function(){return e.getBoundingClientRect().left}))+"px"}),w.each({margin:"",padding:"",border:"Width"},function(e,t){w.cssHooks[e+t]={expand:function(n){for(var r=0,i={},o="string"==typeof n?n.split(" "):[n];r<4;r++)i[e+oe[r]+t]=o[r]||o[r-2]||o[0];return i}},"margin"!==e&&(w.cssHooks[e+t].set=Ke)}),w.fn.extend({css:function(e,t){return z(this,function(e,t,n){var r,i,o={},a=0;if(Array.isArray(t)){for(r=$e(e),i=t.length;a<i;a++)o[t[a]]=w.css(e,t[a],!1,r);return o}return void 0!==n?w.style(e,t,n):w.css(e,t)},e,t,arguments.length>1)}});function tt(e,t,n,r,i){return new tt.prototype.init(e,t,n,r,i)}w.Tween=tt,tt.prototype={constructor:tt,init:function(e,t,n,r,i,o){this.elem=e,this.prop=n,this.easing=i||w.easing._default,this.options=t,this.start=this.now=this.cur(),this.end=r,this.unit=o||(w.cssNumber[n]?"":"px")},cur:function(){var e=tt.propHooks[this.prop];return e&&e.get?e.get(this):tt.propHooks._default.get(this)},run:function(e){var t,n=tt.propHooks[this.prop];return this.options.duration?this.pos=t=w.easing[this.easing](e,this.options.duration*e,0,1,this.options.duration):this.pos=t=e,this.now=(this.end-this.start)*t+this.start,this.options.step&&this.options.step.call(this.elem,this.now,this),n&&n.set?n.set(this):tt.propHooks._default.set(this),this}},tt.prototype.init.prototype=tt.prototype,tt.propHooks={_default:{get:function(e){var t;return 1!==e.elem.nodeType||null!=e.elem[e.prop]&&null==e.elem.style[e.prop]?e.elem[e.prop]:(t=w.css(e.elem,e.prop,""))&&"auto"!==t?t:0},set:function(e){w.fx.step[e.prop]?w.fx.step[e.prop](e):1!==e.elem.nodeType||null==e.elem.style[w.cssProps[e.prop]]&&!w.cssHooks[e.prop]?e.elem[e.prop]=e.now:w.style(e.elem,e.prop,e.now+e.unit)}}},tt.propHooks.scrollTop=tt.propHooks.scrollLeft={set:function(e){e.elem.nodeType&&e.elem.parentNode&&(e.elem[e.prop]=e.now)}},w.easing={linear:function(e){return e},swing:function(e){return.5-Math.cos(e*Math.PI)/2},_default:"swing"},w.fx=tt.prototype.init,w.fx.step={};var nt,rt,it=/^(?:toggle|show|hide)$/,ot=/queueHooks$/;function at(){rt&&(!1===r.hidden&&e.requestAnimationFrame?e.requestAnimationFrame(at):e.setTimeout(at,w.fx.interval),w.fx.tick())}function st(){return e.setTimeout(function(){nt=void 0}),nt=Date.now()}function ut(e,t){var n,r=0,i={height:e};for(t=t?1:0;r<4;r+=2-t)i["margin"+(n=oe[r])]=i["padding"+n]=e;return t&&(i.opacity=i.width=e),i}function lt(e,t,n){for(var r,i=(pt.tweeners[t]||[]).concat(pt.tweeners["*"]),o=0,a=i.length;o<a;o++)if(r=i[o].call(n,t,e))return r}function ct(e,t,n){var r,i,o,a,s,u,l,c,f="width"in t||"height"in t,p=this,d={},h=e.style,g=e.nodeType&&ae(e),y=J.get(e,"fxshow");n.queue||(null==(a=w._queueHooks(e,"fx")).unqueued&&(a.unqueued=0,s=a.empty.fire,a.empty.fire=function(){a.unqueued||s()}),a.unqueued++,p.always(function(){p.always(function(){a.unqueued--,w.queue(e,"fx").length||a.empty.fire()})}));for(r in t)if(i=t[r],it.test(i)){if(delete t[r],o=o||"toggle"===i,i===(g?"hide":"show")){if("show"!==i||!y||void 0===y[r])continue;g=!0}d[r]=y&&y[r]||w.style(e,r)}if((u=!w.isEmptyObject(t))||!w.isEmptyObject(d)){f&&1===e.nodeType&&(n.overflow=[h.overflow,h.overflowX,h.overflowY],null==(l=y&&y.display)&&(l=J.get(e,"display")),"none"===(c=w.css(e,"display"))&&(l?c=l:(fe([e],!0),l=e.style.display||l,c=w.css(e,"display"),fe([e]))),("inline"===c||"inline-block"===c&&null!=l)&&"none"===w.css(e,"float")&&(u||(p.done(function(){h.display=l}),null==l&&(c=h.display,l="none"===c?"":c)),h.display="inline-block")),n.overflow&&(h.overflow="hidden",p.always(function(){h.overflow=n.overflow[0],h.overflowX=n.overflow[1],h.overflowY=n.overflow[2]})),u=!1;for(r in d)u||(y?"hidden"in y&&(g=y.hidden):y=J.access(e,"fxshow",{display:l}),o&&(y.hidden=!g),g&&fe([e],!0),p.done(function(){g||fe([e]),J.remove(e,"fxshow");for(r in d)w.style(e,r,d[r])})),u=lt(g?y[r]:0,r,p),r in y||(y[r]=u.start,g&&(u.end=u.start,u.start=0))}}function ft(e,t){var n,r,i,o,a;for(n in e)if(r=G(n),i=t[r],o=e[n],Array.isArray(o)&&(i=o[1],o=e[n]=o[0]),n!==r&&(e[r]=o,delete e[n]),(a=w.cssHooks[r])&&"expand"in a){o=a.expand(o),delete e[r];for(n in o)n in e||(e[n]=o[n],t[n]=i)}else t[r]=i}function pt(e,t,n){var r,i,o=0,a=pt.prefilters.length,s=w.Deferred().always(function(){delete u.elem}),u=function(){if(i)return!1;for(var t=nt||st(),n=Math.max(0,l.startTime+l.duration-t),r=1-(n/l.duration||0),o=0,a=l.tweens.length;o<a;o++)l.tweens[o].run(r);return s.notifyWith(e,[l,r,n]),r<1&&a?n:(a||s.notifyWith(e,[l,1,0]),s.resolveWith(e,[l]),!1)},l=s.promise({elem:e,props:w.extend({},t),opts:w.extend(!0,{specialEasing:{},easing:w.easing._default},n),originalProperties:t,originalOptions:n,startTime:nt||st(),duration:n.duration,tweens:[],createTween:function(t,n){var r=w.Tween(e,l.opts,t,n,l.opts.specialEasing[t]||l.opts.easing);return l.tweens.push(r),r},stop:function(t){var n=0,r=t?l.tweens.length:0;if(i)return this;for(i=!0;n<r;n++)l.tweens[n].run(1);return t?(s.notifyWith(e,[l,1,0]),s.resolveWith(e,[l,t])):s.rejectWith(e,[l,t]),this}}),c=l.props;for(ft(c,l.opts.specialEasing);o<a;o++)if(r=pt.prefilters[o].call(l,e,c,l.opts))return g(r.stop)&&(w._queueHooks(l.elem,l.opts.queue).stop=r.stop.bind(r)),r;return w.map(c,lt,l),g(l.opts.start)&&l.opts.start.call(e,l),l.progress(l.opts.progress).done(l.opts.done,l.opts.complete).fail(l.opts.fail).always(l.opts.always),w.fx.timer(w.extend(u,{elem:e,anim:l,queue:l.opts.queue})),l}w.Animation=w.extend(pt,{tweeners:{"*":[function(e,t){var n=this.createTween(e,t);return ue(n.elem,e,ie.exec(t),n),n}]},tweener:function(e,t){g(e)?(t=e,e=["*"]):e=e.match(M);for(var n,r=0,i=e.length;r<i;r++)n=e[r],pt.tweeners[n]=pt.tweeners[n]||[],pt.tweeners[n].unshift(t)},prefilters:[ct],prefilter:function(e,t){t?pt.prefilters.unshift(e):pt.prefilters.push(e)}}),w.speed=function(e,t,n){var r=e&&"object"==typeof e?w.extend({},e):{complete:n||!n&&t||g(e)&&e,duration:e,easing:n&&t||t&&!g(t)&&t};return w.fx.off?r.duration=0:"number"!=typeof r.duration&&(r.duration in w.fx.speeds?r.duration=w.fx.speeds[r.duration]:r.duration=w.fx.speeds._default),null!=r.queue&&!0!==r.queue||(r.queue="fx"),r.old=r.complete,r.complete=function(){g(r.old)&&r.old.call(this),r.queue&&w.dequeue(this,r.queue)},r},w.fn.extend({fadeTo:function(e,t,n,r){return this.filter(ae).css("opacity",0).show().end().animate({opacity:t},e,n,r)},animate:function(e,t,n,r){var i=w.isEmptyObject(e),o=w.speed(t,n,r),a=function(){var t=pt(this,w.extend({},e),o);(i||J.get(this,"finish"))&&t.stop(!0)};return a.finish=a,i||!1===o.queue?this.each(a):this.queue(o.queue,a)},stop:function(e,t,n){var r=function(e){var t=e.stop;delete e.stop,t(n)};return"string"!=typeof e&&(n=t,t=e,e=void 0),t&&!1!==e&&this.queue(e||"fx",[]),this.each(function(){var t=!0,i=null!=e&&e+"queueHooks",o=w.timers,a=J.get(this);if(i)a[i]&&a[i].stop&&r(a[i]);else for(i in a)a[i]&&a[i].stop&&ot.test(i)&&r(a[i]);for(i=o.length;i--;)o[i].elem!==this||null!=e&&o[i].queue!==e||(o[i].anim.stop(n),t=!1,o.splice(i,1));!t&&n||w.dequeue(this,e)})},finish:function(e){return!1!==e&&(e=e||"fx"),this.each(function(){var t,n=J.get(this),r=n[e+"queue"],i=n[e+"queueHooks"],o=w.timers,a=r?r.length:0;for(n.finish=!0,w.queue(this,e,[]),i&&i.stop&&i.stop.call(this,!0),t=o.length;t--;)o[t].elem===this&&o[t].queue===e&&(o[t].anim.stop(!0),o.splice(t,1));for(t=0;t<a;t++)r[t]&&r[t].finish&&r[t].finish.call(this);delete n.finish})}}),w.each(["toggle","show","hide"],function(e,t){var n=w.fn[t];w.fn[t]=function(e,r,i){return null==e||"boolean"==typeof e?n.apply(this,arguments):this.animate(ut(t,!0),e,r,i)}}),w.each({slideDown:ut("show"),slideUp:ut("hide"),slideToggle:ut("toggle"),fadeIn:{opacity:"show"},fadeOut:{opacity:"hide"},fadeToggle:{opacity:"toggle"}},function(e,t){w.fn[e]=function(e,n,r){return this.animate(t,e,n,r)}}),w.timers=[],w.fx.tick=function(){var e,t=0,n=w.timers;for(nt=Date.now();t<n.length;t++)(e=n[t])()||n[t]!==e||n.splice(t--,1);n.length||w.fx.stop(),nt=void 0},w.fx.timer=function(e){w.timers.push(e),w.fx.start()},w.fx.interval=13,w.fx.start=function(){rt||(rt=!0,at())},w.fx.stop=function(){rt=null},w.fx.speeds={slow:600,fast:200,_default:400},w.fn.delay=function(t,n){return t=w.fx?w.fx.speeds[t]||t:t,n=n||"fx",this.queue(n,function(n,r){var i=e.setTimeout(n,t);r.stop=function(){e.clearTimeout(i)}})},function(){var e=r.createElement("input"),t=r.createElement("select").appendChild(r.createElement("option"));e.type="checkbox",h.checkOn=""!==e.value,h.optSelected=t.selected,(e=r.createElement("input")).value="t",e.type="radio",h.radioValue="t"===e.value}();var dt,ht=w.expr.attrHandle;w.fn.extend({attr:function(e,t){return z(this,w.attr,e,t,arguments.length>1)},removeAttr:function(e){return this.each(function(){w.removeAttr(this,e)})}}),w.extend({attr:function(e,t,n){var r,i,o=e.nodeType;if(3!==o&&8!==o&&2!==o)return"undefined"==typeof e.getAttribute?w.prop(e,t,n):(1===o&&w.isXMLDoc(e)||(i=w.attrHooks[t.toLowerCase()]||(w.expr.match.bool.test(t)?dt:void 0)),void 0!==n?null===n?void w.removeAttr(e,t):i&&"set"in i&&void 0!==(r=i.set(e,n,t))?r:(e.setAttribute(t,n+""),n):i&&"get"in i&&null!==(r=i.get(e,t))?r:null==(r=w.find.attr(e,t))?void 0:r)},attrHooks:{type:{set:function(e,t){if(!h.radioValue&&"radio"===t&&N(e,"input")){var n=e.value;return e.setAttribute("type",t),n&&(e.value=n),t}}}},removeAttr:function(e,t){var n,r=0,i=t&&t.match(M);if(i&&1===e.nodeType)while(n=i[r++])e.removeAttribute(n)}}),dt={set:function(e,t,n){return!1===t?w.removeAttr(e,n):e.setAttribute(n,n),n}},w.each(w.expr.match.bool.source.match(/\w+/g),function(e,t){var n=ht[t]||w.find.attr;ht[t]=function(e,t,r){var i,o,a=t.toLowerCase();return r||(o=ht[a],ht[a]=i,i=null!=n(e,t,r)?a:null,ht[a]=o),i}});var gt=/^(?:input|select|textarea|button)$/i,yt=/^(?:a|area)$/i;w.fn.extend({prop:function(e,t){return z(this,w.prop,e,t,arguments.length>1)},removeProp:function(e){return this.each(function(){delete this[w.propFix[e]||e]})}}),w.extend({prop:function(e,t,n){var r,i,o=e.nodeType;if(3!==o&&8!==o&&2!==o)return 1===o&&w.isXMLDoc(e)||(t=w.propFix[t]||t,i=w.propHooks[t]),void 0!==n?i&&"set"in i&&void 0!==(r=i.set(e,n,t))?r:e[t]=n:i&&"get"in i&&null!==(r=i.get(e,t))?r:e[t]},propHooks:{tabIndex:{get:function(e){var t=w.find.attr(e,"tabindex");return t?parseInt(t,10):gt.test(e.nodeName)||yt.test(e.nodeName)&&e.href?0:-1}}},propFix:{"for":"htmlFor","class":"className"}}),h.optSelected||(w.propHooks.selected={get:function(e){var t=e.parentNode;return t&&t.parentNode&&t.parentNode.selectedIndex,null},set:function(e){var t=e.parentNode;t&&(t.selectedIndex,t.parentNode&&t.parentNode.selectedIndex)}}),w.each(["tabIndex","readOnly","maxLength","cellSpacing","cellPadding","rowSpan","colSpan","useMap","frameBorder","contentEditable"],function(){w.propFix[this.toLowerCase()]=this});function vt(e){return(e.match(M)||[]).join(" ")}function mt(e){return e.getAttribute&&e.getAttribute("class")||""}function xt(e){return Array.isArray(e)?e:"string"==typeof e?e.match(M)||[]:[]}w.fn.extend({addClass:function(e){var t,n,r,i,o,a,s,u=0;if(g(e))return this.each(function(t){w(this).addClass(e.call(this,t,mt(this)))});if((t=xt(e)).length)while(n=this[u++])if(i=mt(n),r=1===n.nodeType&&" "+vt(i)+" "){a=0;while(o=t[a++])r.indexOf(" "+o+" ")<0&&(r+=o+" ");i!==(s=vt(r))&&n.setAttribute("class",s)}return this},removeClass:function(e){var t,n,r,i,o,a,s,u=0;if(g(e))return this.each(function(t){w(this).removeClass(e.call(this,t,mt(this)))});if(!arguments.length)return this.attr("class","");if((t=xt(e)).length)while(n=this[u++])if(i=mt(n),r=1===n.nodeType&&" "+vt(i)+" "){a=0;while(o=t[a++])while(r.indexOf(" "+o+" ")>-1)r=r.replace(" "+o+" "," ");i!==(s=vt(r))&&n.setAttribute("class",s)}return this},toggleClass:function(e,t){var n=typeof e,r="string"===n||Array.isArray(e);return"boolean"==typeof t&&r?t?this.addClass(e):this.removeClass(e):g(e)?this.each(function(n){w(this).toggleClass(e.call(this,n,mt(this),t),t)}):this.each(function(){var t,i,o,a;if(r){i=0,o=w(this),a=xt(e);while(t=a[i++])o.hasClass(t)?o.removeClass(t):o.addClass(t)}else void 0!==e&&"boolean"!==n||((t=mt(this))&&J.set(this,"__className__",t),this.setAttribute&&this.setAttribute("class",t||!1===e?"":J.get(this,"__className__")||""))})},hasClass:function(e){var t,n,r=0;t=" "+e+" ";while(n=this[r++])if(1===n.nodeType&&(" "+vt(mt(n))+" ").indexOf(t)>-1)return!0;return!1}});var bt=/\r/g;w.fn.extend({val:function(e){var t,n,r,i=this[0];{if(arguments.length)return r=g(e),this.each(function(n){var i;1===this.nodeType&&(null==(i=r?e.call(this,n,w(this).val()):e)?i="":"number"==typeof i?i+="":Array.isArray(i)&&(i=w.map(i,function(e){return null==e?"":e+""})),(t=w.valHooks[this.type]||w.valHooks[this.nodeName.toLowerCase()])&&"set"in t&&void 0!==t.set(this,i,"value")||(this.value=i))});if(i)return(t=w.valHooks[i.type]||w.valHooks[i.nodeName.toLowerCase()])&&"get"in t&&void 0!==(n=t.get(i,"value"))?n:"string"==typeof(n=i.value)?n.replace(bt,""):null==n?"":n}}}),w.extend({valHooks:{option:{get:function(e){var t=w.find.attr(e,"value");return null!=t?t:vt(w.text(e))}},select:{get:function(e){var t,n,r,i=e.options,o=e.selectedIndex,a="select-one"===e.type,s=a?null:[],u=a?o+1:i.length;for(r=o<0?u:a?o:0;r<u;r++)if(((n=i[r]).selected||r===o)&&!n.disabled&&(!n.parentNode.disabled||!N(n.parentNode,"optgroup"))){if(t=w(n).val(),a)return t;s.push(t)}return s},set:function(e,t){var n,r,i=e.options,o=w.makeArray(t),a=i.length;while(a--)((r=i[a]).selected=w.inArray(w.valHooks.option.get(r),o)>-1)&&(n=!0);return n||(e.selectedIndex=-1),o}}}}),w.each(["radio","checkbox"],function(){w.valHooks[this]={set:function(e,t){if(Array.isArray(t))return e.checked=w.inArray(w(e).val(),t)>-1}},h.checkOn||(w.valHooks[this].get=function(e){return null===e.getAttribute("value")?"on":e.value})}),h.focusin="onfocusin"in e;var wt=/^(?:focusinfocus|focusoutblur)$/,Tt=function(e){e.stopPropagation()};w.extend(w.event,{trigger:function(t,n,i,o){var a,s,u,l,c,p,d,h,v=[i||r],m=f.call(t,"type")?t.type:t,x=f.call(t,"namespace")?t.namespace.split("."):[];if(s=h=u=i=i||r,3!==i.nodeType&&8!==i.nodeType&&!wt.test(m+w.event.triggered)&&(m.indexOf(".")>-1&&(m=(x=m.split(".")).shift(),x.sort()),c=m.indexOf(":")<0&&"on"+m,t=t[w.expando]?t:new w.Event(m,"object"==typeof t&&t),t.isTrigger=o?2:3,t.namespace=x.join("."),t.rnamespace=t.namespace?new RegExp("(^|\\.)"+x.join("\\.(?:.*\\.|)")+"(\\.|$)"):null,t.result=void 0,t.target||(t.target=i),n=null==n?[t]:w.makeArray(n,[t]),d=w.event.special[m]||{},o||!d.trigger||!1!==d.trigger.apply(i,n))){if(!o&&!d.noBubble&&!y(i)){for(l=d.delegateType||m,wt.test(l+m)||(s=s.parentNode);s;s=s.parentNode)v.push(s),u=s;u===(i.ownerDocument||r)&&v.push(u.defaultView||u.parentWindow||e)}a=0;while((s=v[a++])&&!t.isPropagationStopped())h=s,t.type=a>1?l:d.bindType||m,(p=(J.get(s,"events")||{})[t.type]&&J.get(s,"handle"))&&p.apply(s,n),(p=c&&s[c])&&p.apply&&Y(s)&&(t.result=p.apply(s,n),!1===t.result&&t.preventDefault());return t.type=m,o||t.isDefaultPrevented()||d._default&&!1!==d._default.apply(v.pop(),n)||!Y(i)||c&&g(i[m])&&!y(i)&&((u=i[c])&&(i[c]=null),w.event.triggered=m,t.isPropagationStopped()&&h.addEventListener(m,Tt),i[m](),t.isPropagationStopped()&&h.removeEventListener(m,Tt),w.event.triggered=void 0,u&&(i[c]=u)),t.result}},simulate:function(e,t,n){var r=w.extend(new w.Event,n,{type:e,isSimulated:!0});w.event.trigger(r,null,t)}}),w.fn.extend({trigger:function(e,t){return this.each(function(){w.event.trigger(e,t,this)})},triggerHandler:function(e,t){var n=this[0];if(n)return w.event.trigger(e,t,n,!0)}}),h.focusin||w.each({focus:"focusin",blur:"focusout"},function(e,t){var n=function(e){w.event.simulate(t,e.target,w.event.fix(e))};w.event.special[t]={setup:function(){var r=this.ownerDocument||this,i=J.access(r,t);i||r.addEventListener(e,n,!0),J.access(r,t,(i||0)+1)},teardown:function(){var r=this.ownerDocument||this,i=J.access(r,t)-1;i?J.access(r,t,i):(r.removeEventListener(e,n,!0),J.remove(r,t))}}});var Ct=e.location,Et=Date.now(),kt=/\?/;w.parseXML=function(t){var n;if(!t||"string"!=typeof t)return null;try{n=(new e.DOMParser).parseFromString(t,"text/xml")}catch(e){n=void 0}return n&&!n.getElementsByTagName("parsererror").length||w.error("Invalid XML: "+t),n};var St=/\[\]$/,Dt=/\r?\n/g,Nt=/^(?:submit|button|image|reset|file)$/i,At=/^(?:input|select|textarea|keygen)/i;function jt(e,t,n,r){var i;if(Array.isArray(t))w.each(t,function(t,i){n||St.test(e)?r(e,i):jt(e+"["+("object"==typeof i&&null!=i?t:"")+"]",i,n,r)});else if(n||"object"!==x(t))r(e,t);else for(i in t)jt(e+"["+i+"]",t[i],n,r)}w.param=function(e,t){var n,r=[],i=function(e,t){var n=g(t)?t():t;r[r.length]=encodeURIComponent(e)+"="+encodeURIComponent(null==n?"":n)};if(Array.isArray(e)||e.jquery&&!w.isPlainObject(e))w.each(e,function(){i(this.name,this.value)});else for(n in e)jt(n,e[n],t,i);return r.join("&")},w.fn.extend({serialize:function(){return w.param(this.serializeArray())},serializeArray:function(){return this.map(function(){var e=w.prop(this,"elements");return e?w.makeArray(e):this}).filter(function(){var e=this.type;return this.name&&!w(this).is(":disabled")&&At.test(this.nodeName)&&!Nt.test(e)&&(this.checked||!pe.test(e))}).map(function(e,t){var n=w(this).val();return null==n?null:Array.isArray(n)?w.map(n,function(e){return{name:t.name,value:e.replace(Dt,"\r\n")}}):{name:t.name,value:n.replace(Dt,"\r\n")}}).get()}});var qt=/%20/g,Lt=/#.*$/,Ht=/([?&])_=[^&]*/,Ot=/^(.*?):[ \t]*([^\r\n]*)$/gm,Pt=/^(?:about|app|app-storage|.+-extension|file|res|widget):$/,Mt=/^(?:GET|HEAD)$/,Rt=/^\/\//,It={},Wt={},$t="*/".concat("*"),Bt=r.createElement("a");Bt.href=Ct.href;function Ft(e){return function(t,n){"string"!=typeof t&&(n=t,t="*");var r,i=0,o=t.toLowerCase().match(M)||[];if(g(n))while(r=o[i++])"+"===r[0]?(r=r.slice(1)||"*",(e[r]=e[r]||[]).unshift(n)):(e[r]=e[r]||[]).push(n)}}function _t(e,t,n,r){var i={},o=e===Wt;function a(s){var u;return i[s]=!0,w.each(e[s]||[],function(e,s){var l=s(t,n,r);return"string"!=typeof l||o||i[l]?o?!(u=l):void 0:(t.dataTypes.unshift(l),a(l),!1)}),u}return a(t.dataTypes[0])||!i["*"]&&a("*")}function zt(e,t){var n,r,i=w.ajaxSettings.flatOptions||{};for(n in t)void 0!==t[n]&&((i[n]?e:r||(r={}))[n]=t[n]);return r&&w.extend(!0,e,r),e}function Xt(e,t,n){var r,i,o,a,s=e.contents,u=e.dataTypes;while("*"===u[0])u.shift(),void 0===r&&(r=e.mimeType||t.getResponseHeader("Content-Type"));if(r)for(i in s)if(s[i]&&s[i].test(r)){u.unshift(i);break}if(u[0]in n)o=u[0];else{for(i in n){if(!u[0]||e.converters[i+" "+u[0]]){o=i;break}a||(a=i)}o=o||a}if(o)return o!==u[0]&&u.unshift(o),n[o]}function Ut(e,t,n,r){var i,o,a,s,u,l={},c=e.dataTypes.slice();if(c[1])for(a in e.converters)l[a.toLowerCase()]=e.converters[a];o=c.shift();while(o)if(e.responseFields[o]&&(n[e.responseFields[o]]=t),!u&&r&&e.dataFilter&&(t=e.dataFilter(t,e.dataType)),u=o,o=c.shift())if("*"===o)o=u;else if("*"!==u&&u!==o){if(!(a=l[u+" "+o]||l["* "+o]))for(i in l)if((s=i.split(" "))[1]===o&&(a=l[u+" "+s[0]]||l["* "+s[0]])){!0===a?a=l[i]:!0!==l[i]&&(o=s[0],c.unshift(s[1]));break}if(!0!==a)if(a&&e["throws"])t=a(t);else try{t=a(t)}catch(e){return{state:"parsererror",error:a?e:"No conversion from "+u+" to "+o}}}return{state:"success",data:t}}w.extend({active:0,lastModified:{},etag:{},ajaxSettings:{url:Ct.href,type:"GET",isLocal:Pt.test(Ct.protocol),global:!0,processData:!0,async:!0,contentType:"application/x-www-form-urlencoded; charset=UTF-8",accepts:{"*":$t,text:"text/plain",html:"text/html",xml:"application/xml, text/xml",json:"application/json, text/javascript"},contents:{xml:/\bxml\b/,html:/\bhtml/,json:/\bjson\b/},responseFields:{xml:"responseXML",text:"responseText",json:"responseJSON"},converters:{"* text":String,"text html":!0,"text json":JSON.parse,"text xml":w.parseXML},flatOptions:{url:!0,context:!0}},ajaxSetup:function(e,t){return t?zt(zt(e,w.ajaxSettings),t):zt(w.ajaxSettings,e)},ajaxPrefilter:Ft(It),ajaxTransport:Ft(Wt),ajax:function(t,n){"object"==typeof t&&(n=t,t=void 0),n=n||{};var i,o,a,s,u,l,c,f,p,d,h=w.ajaxSetup({},n),g=h.context||h,y=h.context&&(g.nodeType||g.jquery)?w(g):w.event,v=w.Deferred(),m=w.Callbacks("once memory"),x=h.statusCode||{},b={},T={},C="canceled",E={readyState:0,getResponseHeader:function(e){var t;if(c){if(!s){s={};while(t=Ot.exec(a))s[t[1].toLowerCase()]=t[2]}t=s[e.toLowerCase()]}return null==t?null:t},getAllResponseHeaders:function(){return c?a:null},setRequestHeader:function(e,t){return null==c&&(e=T[e.toLowerCase()]=T[e.toLowerCase()]||e,b[e]=t),this},overrideMimeType:function(e){return null==c&&(h.mimeType=e),this},statusCode:function(e){var t;if(e)if(c)E.always(e[E.status]);else for(t in e)x[t]=[x[t],e[t]];return this},abort:function(e){var t=e||C;return i&&i.abort(t),k(0,t),this}};if(v.promise(E),h.url=((t||h.url||Ct.href)+"").replace(Rt,Ct.protocol+"//"),h.type=n.method||n.type||h.method||h.type,h.dataTypes=(h.dataType||"*").toLowerCase().match(M)||[""],null==h.crossDomain){l=r.createElement("a");try{l.href=h.url,l.href=l.href,h.crossDomain=Bt.protocol+"//"+Bt.host!=l.protocol+"//"+l.host}catch(e){h.crossDomain=!0}}if(h.data&&h.processData&&"string"!=typeof h.data&&(h.data=w.param(h.data,h.traditional)),_t(It,h,n,E),c)return E;(f=w.event&&h.global)&&0==w.active++&&w.event.trigger("ajaxStart"),h.type=h.type.toUpperCase(),h.hasContent=!Mt.test(h.type),o=h.url.replace(Lt,""),h.hasContent?h.data&&h.processData&&0===(h.contentType||"").indexOf("application/x-www-form-urlencoded")&&(h.data=h.data.replace(qt,"+")):(d=h.url.slice(o.length),h.data&&(h.processData||"string"==typeof h.data)&&(o+=(kt.test(o)?"&":"?")+h.data,delete h.data),!1===h.cache&&(o=o.replace(Ht,"$1"),d=(kt.test(o)?"&":"?")+"_="+Et+++d),h.url=o+d),h.ifModified&&(w.lastModified[o]&&E.setRequestHeader("If-Modified-Since",w.lastModified[o]),w.etag[o]&&E.setRequestHeader("If-None-Match",w.etag[o])),(h.data&&h.hasContent&&!1!==h.contentType||n.contentType)&&E.setRequestHeader("Content-Type",h.contentType),E.setRequestHeader("Accept",h.dataTypes[0]&&h.accepts[h.dataTypes[0]]?h.accepts[h.dataTypes[0]]+("*"!==h.dataTypes[0]?", "+$t+"; q=0.01":""):h.accepts["*"]);for(p in h.headers)E.setRequestHeader(p,h.headers[p]);if(h.beforeSend&&(!1===h.beforeSend.call(g,E,h)||c))return E.abort();if(C="abort",m.add(h.complete),E.done(h.success),E.fail(h.error),i=_t(Wt,h,n,E)){if(E.readyState=1,f&&y.trigger("ajaxSend",[E,h]),c)return E;h.async&&h.timeout>0&&(u=e.setTimeout(function(){E.abort("timeout")},h.timeout));try{c=!1,i.send(b,k)}catch(e){if(c)throw e;k(-1,e)}}else k(-1,"No Transport");function k(t,n,r,s){var l,p,d,b,T,C=n;c||(c=!0,u&&e.clearTimeout(u),i=void 0,a=s||"",E.readyState=t>0?4:0,l=t>=200&&t<300||304===t,r&&(b=Xt(h,E,r)),b=Ut(h,b,E,l),l?(h.ifModified&&((T=E.getResponseHeader("Last-Modified"))&&(w.lastModified[o]=T),(T=E.getResponseHeader("etag"))&&(w.etag[o]=T)),204===t||"HEAD"===h.type?C="nocontent":304===t?C="notmodified":(C=b.state,p=b.data,l=!(d=b.error))):(d=C,!t&&C||(C="error",t<0&&(t=0))),E.status=t,E.statusText=(n||C)+"",l?v.resolveWith(g,[p,C,E]):v.rejectWith(g,[E,C,d]),E.statusCode(x),x=void 0,f&&y.trigger(l?"ajaxSuccess":"ajaxError",[E,h,l?p:d]),m.fireWith(g,[E,C]),f&&(y.trigger("ajaxComplete",[E,h]),--w.active||w.event.trigger("ajaxStop")))}return E},getJSON:function(e,t,n){return w.get(e,t,n,"json")},getScript:function(e,t){return w.get(e,void 0,t,"script")}}),w.each(["get","post"],function(e,t){w[t]=function(e,n,r,i){return g(n)&&(i=i||r,r=n,n=void 0),w.ajax(w.extend({url:e,type:t,dataType:i,data:n,success:r},w.isPlainObject(e)&&e))}}),w._evalUrl=function(e){return w.ajax({url:e,type:"GET",dataType:"script",cache:!0,async:!1,global:!1,"throws":!0})},w.fn.extend({wrapAll:function(e){var t;return this[0]&&(g(e)&&(e=e.call(this[0])),t=w(e,this[0].ownerDocument).eq(0).clone(!0),this[0].parentNode&&t.insertBefore(this[0]),t.map(function(){var e=this;while(e.firstElementChild)e=e.firstElementChild;return e}).append(this)),this},wrapInner:function(e){return g(e)?this.each(function(t){w(this).wrapInner(e.call(this,t))}):this.each(function(){var t=w(this),n=t.contents();n.length?n.wrapAll(e):t.append(e)})},wrap:function(e){var t=g(e);return this.each(function(n){w(this).wrapAll(t?e.call(this,n):e)})},unwrap:function(e){return this.parent(e).not("body").each(function(){w(this).replaceWith(this.childNodes)}),this}}),w.expr.pseudos.hidden=function(e){return!w.expr.pseudos.visible(e)},w.expr.pseudos.visible=function(e){return!!(e.offsetWidth||e.offsetHeight||e.getClientRects().length)},w.ajaxSettings.xhr=function(){try{return new e.XMLHttpRequest}catch(e){}};var Vt={0:200,1223:204},Gt=w.ajaxSettings.xhr();h.cors=!!Gt&&"withCredentials"in Gt,h.ajax=Gt=!!Gt,w.ajaxTransport(function(t){var n,r;if(h.cors||Gt&&!t.crossDomain)return{send:function(i,o){var a,s=t.xhr();if(s.open(t.type,t.url,t.async,t.username,t.password),t.xhrFields)for(a in t.xhrFields)s[a]=t.xhrFields[a];t.mimeType&&s.overrideMimeType&&s.overrideMimeType(t.mimeType),t.crossDomain||i["X-Requested-With"]||(i["X-Requested-With"]="XMLHttpRequest");for(a in i)s.setRequestHeader(a,i[a]);n=function(e){return function(){n&&(n=r=s.onload=s.onerror=s.onabort=s.ontimeout=s.onreadystatechange=null,"abort"===e?s.abort():"error"===e?"number"!=typeof s.status?o(0,"error"):o(s.status,s.statusText):o(Vt[s.status]||s.status,s.statusText,"text"!==(s.responseType||"text")||"string"!=typeof s.responseText?{binary:s.response}:{text:s.responseText},s.getAllResponseHeaders()))}},s.onload=n(),r=s.onerror=s.ontimeout=n("error"),void 0!==s.onabort?s.onabort=r:s.onreadystatechange=function(){4===s.readyState&&e.setTimeout(function(){n&&r()})},n=n("abort");try{s.send(t.hasContent&&t.data||null)}catch(e){if(n)throw e}},abort:function(){n&&n()}}}),w.ajaxPrefilter(function(e){e.crossDomain&&(e.contents.script=!1)}),w.ajaxSetup({accepts:{script:"text/javascript, application/javascript, application/ecmascript, application/x-ecmascript"},contents:{script:/\b(?:java|ecma)script\b/},converters:{"text script":function(e){return w.globalEval(e),e}}}),w.ajaxPrefilter("script",function(e){void 0===e.cache&&(e.cache=!1),e.crossDomain&&(e.type="GET")}),w.ajaxTransport("script",function(e){if(e.crossDomain){var t,n;return{send:function(i,o){t=w("<script>").prop({charset:e.scriptCharset,src:e.url}).on("load error",n=function(e){t.remove(),n=null,e&&o("error"===e.type?404:200,e.type)}),r.head.appendChild(t[0])},abort:function(){n&&n()}}}});var Yt=[],Qt=/(=)\?(?=&|$)|\?\?/;w.ajaxSetup({jsonp:"callback",jsonpCallback:function(){var e=Yt.pop()||w.expando+"_"+Et++;return this[e]=!0,e}}),w.ajaxPrefilter("json jsonp",function(t,n,r){var i,o,a,s=!1!==t.jsonp&&(Qt.test(t.url)?"url":"string"==typeof t.data&&0===(t.contentType||"").indexOf("application/x-www-form-urlencoded")&&Qt.test(t.data)&&"data");if(s||"jsonp"===t.dataTypes[0])return i=t.jsonpCallback=g(t.jsonpCallback)?t.jsonpCallback():t.jsonpCallback,s?t[s]=t[s].replace(Qt,"$1"+i):!1!==t.jsonp&&(t.url+=(kt.test(t.url)?"&":"?")+t.jsonp+"="+i),t.converters["script json"]=function(){return a||w.error(i+" was not called"),a[0]},t.dataTypes[0]="json",o=e[i],e[i]=function(){a=arguments},r.always(function(){void 0===o?w(e).removeProp(i):e[i]=o,t[i]&&(t.jsonpCallback=n.jsonpCallback,Yt.push(i)),a&&g(o)&&o(a[0]),a=o=void 0}),"script"}),h.createHTMLDocument=function(){var e=r.implementation.createHTMLDocument("").body;return e.innerHTML="<form></form><form></form>",2===e.childNodes.length}(),w.parseHTML=function(e,t,n){if("string"!=typeof e)return[];"boolean"==typeof t&&(n=t,t=!1);var i,o,a;return t||(h.createHTMLDocument?((i=(t=r.implementation.createHTMLDocument("")).createElement("base")).href=r.location.href,t.head.appendChild(i)):t=r),o=A.exec(e),a=!n&&[],o?[t.createElement(o[1])]:(o=xe([e],t,a),a&&a.length&&w(a).remove(),w.merge([],o.childNodes))},w.fn.load=function(e,t,n){var r,i,o,a=this,s=e.indexOf(" ");return s>-1&&(r=vt(e.slice(s)),e=e.slice(0,s)),g(t)?(n=t,t=void 0):t&&"object"==typeof t&&(i="POST"),a.length>0&&w.ajax({url:e,type:i||"GET",dataType:"html",data:t}).done(function(e){o=arguments,a.html(r?w("<div>").append(w.parseHTML(e)).find(r):e)}).always(n&&function(e,t){a.each(function(){n.apply(this,o||[e.responseText,t,e])})}),this},w.each(["ajaxStart","ajaxStop","ajaxComplete","ajaxError","ajaxSuccess","ajaxSend"],function(e,t){w.fn[t]=function(e){return this.on(t,e)}}),w.expr.pseudos.animated=function(e){return w.grep(w.timers,function(t){return e===t.elem}).length},w.offset={setOffset:function(e,t,n){var r,i,o,a,s,u,l,c=w.css(e,"position"),f=w(e),p={};"static"===c&&(e.style.position="relative"),s=f.offset(),o=w.css(e,"top"),u=w.css(e,"left"),(l=("absolute"===c||"fixed"===c)&&(o+u).indexOf("auto")>-1)?(a=(r=f.position()).top,i=r.left):(a=parseFloat(o)||0,i=parseFloat(u)||0),g(t)&&(t=t.call(e,n,w.extend({},s))),null!=t.top&&(p.top=t.top-s.top+a),null!=t.left&&(p.left=t.left-s.left+i),"using"in t?t.using.call(e,p):f.css(p)}},w.fn.extend({offset:function(e){if(arguments.length)return void 0===e?this:this.each(function(t){w.offset.setOffset(this,e,t)});var t,n,r=this[0];if(r)return r.getClientRects().length?(t=r.getBoundingClientRect(),n=r.ownerDocument.defaultView,{top:t.top+n.pageYOffset,left:t.left+n.pageXOffset}):{top:0,left:0}},position:function(){if(this[0]){var e,t,n,r=this[0],i={top:0,left:0};if("fixed"===w.css(r,"position"))t=r.getBoundingClientRect();else{t=this.offset(),n=r.ownerDocument,e=r.offsetParent||n.documentElement;while(e&&(e===n.body||e===n.documentElement)&&"static"===w.css(e,"position"))e=e.parentNode;e&&e!==r&&1===e.nodeType&&((i=w(e).offset()).top+=w.css(e,"borderTopWidth",!0),i.left+=w.css(e,"borderLeftWidth",!0))}return{top:t.top-i.top-w.css(r,"marginTop",!0),left:t.left-i.left-w.css(r,"marginLeft",!0)}}},offsetParent:function(){return this.map(function(){var e=this.offsetParent;while(e&&"static"===w.css(e,"position"))e=e.offsetParent;return e||be})}}),w.each({scrollLeft:"pageXOffset",scrollTop:"pageYOffset"},function(e,t){var n="pageYOffset"===t;w.fn[e]=function(r){return z(this,function(e,r,i){var o;if(y(e)?o=e:9===e.nodeType&&(o=e.defaultView),void 0===i)return o?o[t]:e[r];o?o.scrollTo(n?o.pageXOffset:i,n?i:o.pageYOffset):e[r]=i},e,r,arguments.length)}}),w.each(["top","left"],function(e,t){w.cssHooks[t]=_e(h.pixelPosition,function(e,n){if(n)return n=Fe(e,t),We.test(n)?w(e).position()[t]+"px":n})}),w.each({Height:"height",Width:"width"},function(e,t){w.each({padding:"inner"+e,content:t,"":"outer"+e},function(n,r){w.fn[r]=function(i,o){var a=arguments.length&&(n||"boolean"!=typeof i),s=n||(!0===i||!0===o?"margin":"border");return z(this,function(t,n,i){var o;return y(t)?0===r.indexOf("outer")?t["inner"+e]:t.document.documentElement["client"+e]:9===t.nodeType?(o=t.documentElement,Math.max(t.body["scroll"+e],o["scroll"+e],t.body["offset"+e],o["offset"+e],o["client"+e])):void 0===i?w.css(t,n,s):w.style(t,n,i,s)},t,a?i:void 0,a)}})}),w.each("blur focus focusin focusout resize scroll click dblclick mousedown mouseup mousemove mouseover mouseout mouseenter mouseleave change select submit keydown keypress keyup contextmenu".split(" "),function(e,t){w.fn[t]=function(e,n){return arguments.length>0?this.on(t,null,e,n):this.trigger(t)}}),w.fn.extend({hover:function(e,t){return this.mouseenter(e).mouseleave(t||e)}}),w.fn.extend({bind:function(e,t,n){return this.on(e,null,t,n)},unbind:function(e,t){return this.off(e,null,t)},delegate:function(e,t,n,r){return this.on(t,e,n,r)},undelegate:function(e,t,n){return 1===arguments.length?this.off(e,"**"):this.off(t,e||"**",n)}}),w.proxy=function(e,t){var n,r,i;if("string"==typeof t&&(n=e[t],t=e,e=n),g(e))return r=o.call(arguments,2),i=function(){return e.apply(t||this,r.concat(o.call(arguments)))},i.guid=e.guid=e.guid||w.guid++,i},w.holdReady=function(e){e?w.readyWait++:w.ready(!0)},w.isArray=Array.isArray,w.parseJSON=JSON.parse,w.nodeName=N,w.isFunction=g,w.isWindow=y,w.camelCase=G,w.type=x,w.now=Date.now,w.isNumeric=function(e){var t=w.type(e);return("number"===t||"string"===t)&&!isNaN(e-parseFloat(e))},"function"==typeof define&&define.amd&&define("jquery",[],function(){return w});var Jt=e.jQuery,Kt=e.$;return w.noConflict=function(t){return e.$===w&&(e.$=Kt),t&&e.jQuery===w&&(e.jQuery=Jt),w},t||(e.jQuery=e.$=w),w});

/* End */
;
; /* Start:"a:4:{s:4:"full";s:83:"/local/templates/.default/markup/new/js/jquery-migrate-3.3.1.min.js?159957690710976";s:6:"source";s:67:"/local/templates/.default/markup/new/js/jquery-migrate-3.3.1.min.js";s:3:"min";s:0:"";s:3:"map";s:0:"";}"*/
/*! jQuery Migrate v3.3.1 | (c) OpenJS Foundation and other contributors | jquery.org/license */
"undefined"==typeof jQuery.migrateMute&&(jQuery.migrateMute=!0),function(t){"use strict";"function"==typeof define&&define.amd?define(["jquery"],function(e){return t(e,window)}):"object"==typeof module&&module.exports?module.exports=t(require("jquery"),window):t(jQuery,window)}(function(s,n){"use strict";function e(e){return 0<=function(e,t){var r,n=/^(\d+)\.(\d+)\.(\d+)/,i=n.exec(e)||[],o=n.exec(t)||[];for(r=1;r<=3;r++){if(+i[r]>+o[r])return 1;if(+i[r]<+o[r])return-1}return 0}(s.fn.jquery,e)}s.migrateVersion="3.3.1",n.console&&n.console.log&&(s&&e("3.0.0")||n.console.log("JQMIGRATE: jQuery 3.0.0+ REQUIRED"),s.migrateWarnings&&n.console.log("JQMIGRATE: Migrate plugin loaded multiple times"),n.console.log("JQMIGRATE: Migrate is installed"+(s.migrateMute?"":" with logging active")+", version "+s.migrateVersion));var r={};function u(e){var t=n.console;s.migrateDeduplicateWarnings&&r[e]||(r[e]=!0,s.migrateWarnings.push(e),t&&t.warn&&!s.migrateMute&&(t.warn("JQMIGRATE: "+e),s.migrateTrace&&t.trace&&t.trace()))}function t(e,t,r,n){Object.defineProperty(e,t,{configurable:!0,enumerable:!0,get:function(){return u(n),r},set:function(e){u(n),r=e}})}function i(e,t,r,n){e[t]=function(){return u(n),r.apply(this,arguments)}}s.migrateDeduplicateWarnings=!0,s.migrateWarnings=[],void 0===s.migrateTrace&&(s.migrateTrace=!0),s.migrateReset=function(){r={},s.migrateWarnings.length=0},"BackCompat"===n.document.compatMode&&u("jQuery is not compatible with Quirks Mode");var o,a={},c=s.fn.init,d=s.find,l=/\[(\s*[-\w]+\s*)([~|^$*]?=)\s*([-\w#]*?#[-\w#]*)\s*\]/,p=/\[(\s*[-\w]+\s*)([~|^$*]?=)\s*([-\w#]*?#[-\w#]*)\s*\]/g,f=/^[\s\uFEFF\xA0]+|[\s\uFEFF\xA0]+$/g;for(o in s.fn.init=function(e){var t=Array.prototype.slice.call(arguments);return"string"==typeof e&&"#"===e&&(u("jQuery( '#' ) is not a valid selector"),t[0]=[]),c.apply(this,t)},s.fn.init.prototype=s.fn,s.find=function(t){var r=Array.prototype.slice.call(arguments);if("string"==typeof t&&l.test(t))try{n.document.querySelector(t)}catch(e){t=t.replace(p,function(e,t,r,n){return"["+t+r+'"'+n+'"]'});try{n.document.querySelector(t),u("Attribute selector with '#' must be quoted: "+r[0]),r[0]=t}catch(e){u("Attribute selector with '#' was not fixed: "+r[0])}}return d.apply(this,r)},d)Object.prototype.hasOwnProperty.call(d,o)&&(s.find[o]=d[o]);if(i(s.fn,"size",function(){return this.length},"jQuery.fn.size() is deprecated and removed; use the .length property"),i(s,"parseJSON",function(){return JSON.parse.apply(null,arguments)},"jQuery.parseJSON is deprecated; use JSON.parse"),i(s,"holdReady",s.holdReady,"jQuery.holdReady is deprecated"),i(s,"unique",s.uniqueSort,"jQuery.unique is deprecated; use jQuery.uniqueSort"),t(s.expr,"filters",s.expr.pseudos,"jQuery.expr.filters is deprecated; use jQuery.expr.pseudos"),t(s.expr,":",s.expr.pseudos,"jQuery.expr[':'] is deprecated; use jQuery.expr.pseudos"),e("3.1.1")&&i(s,"trim",function(e){return null==e?"":(e+"").replace(f,"")},"jQuery.trim is deprecated; use String.prototype.trim"),e("3.2.0")&&i(s,"nodeName",function(e,t){return e.nodeName&&e.nodeName.toLowerCase()===t.toLowerCase()},"jQuery.nodeName is deprecated"),e("3.3.0")&&(i(s,"isNumeric",function(e){var t=typeof e;return("number"==t||"string"==t)&&!isNaN(e-parseFloat(e))},"jQuery.isNumeric() is deprecated"),s.each("Boolean Number String Function Array Date RegExp Object Error Symbol".split(" "),function(e,t){a["[object "+t+"]"]=t.toLowerCase()}),i(s,"type",function(e){return null==e?e+"":"object"==typeof e||"function"==typeof e?a[Object.prototype.toString.call(e)]||"object":typeof e},"jQuery.type is deprecated"),i(s,"isFunction",function(e){return"function"==typeof e},"jQuery.isFunction() is deprecated"),i(s,"isWindow",function(e){return null!=e&&e===e.window},"jQuery.isWindow() is deprecated"),i(s,"isArray",Array.isArray,"jQuery.isArray is deprecated; use Array.isArray")),s.ajax){var y=s.ajax;s.ajax=function(){var e=y.apply(this,arguments);return e.promise&&(i(e,"success",e.done,"jQXHR.success is deprecated and removed"),i(e,"error",e.fail,"jQXHR.error is deprecated and removed"),i(e,"complete",e.always,"jQXHR.complete is deprecated and removed")),e}}var m=s.fn.removeAttr,g=s.fn.toggleClass,h=/\S+/g;function v(e){return e.replace(/-([a-z])/g,function(e,t){return t.toUpperCase()})}s.fn.removeAttr=function(e){var r=this;return s.each(e.match(h),function(e,t){s.expr.match.bool.test(t)&&(u("jQuery.fn.removeAttr no longer sets boolean properties: "+t),r.prop(t,!1))}),m.apply(this,arguments)};var j,Q=!(s.fn.toggleClass=function(t){return void 0!==t&&"boolean"!=typeof t?g.apply(this,arguments):(u("jQuery.fn.toggleClass( boolean ) is deprecated"),this.each(function(){var e=this.getAttribute&&this.getAttribute("class")||"";e&&s.data(this,"__className__",e),this.setAttribute&&this.setAttribute("class",e||!1===t?"":s.data(this,"__className__")||"")}))}),b=/^[a-z]/,w=/^(?:Border(?:Top|Right|Bottom|Left)?(?:Width|)|(?:Margin|Padding)?(?:Top|Right|Bottom|Left)?|(?:Min|Max)?(?:Width|Height))$/;s.swap&&s.each(["height","width","reliableMarginRight"],function(e,t){var r=s.cssHooks[t]&&s.cssHooks[t].get;r&&(s.cssHooks[t].get=function(){var e;return Q=!0,e=r.apply(this,arguments),Q=!1,e})}),s.swap=function(e,t,r,n){var i,o,a={};for(o in Q||u("jQuery.swap() is undocumented and deprecated"),t)a[o]=e.style[o],e.style[o]=t[o];for(o in i=r.apply(e,n||[]),t)e.style[o]=a[o];return i},e("3.4.0")&&"undefined"!=typeof Proxy&&(s.cssProps=new Proxy(s.cssProps||{},{set:function(){return u("JQMIGRATE: jQuery.cssProps is deprecated"),Reflect.set.apply(this,arguments)}})),s.cssNumber||(s.cssNumber={}),j=s.fn.css,s.fn.css=function(e,t){var r,n=this;return e&&"object"==typeof e&&!Array.isArray(e)&&s.each(e,function(e,t){s.fn.css.call(n,e,t)}),"number"==typeof t&&(function(e){return b.test(e)&&w.test(e[0].toUpperCase()+e.slice(1))}(r=v(e))||s.cssNumber[r]||u('Number-typed values are deprecated for jQuery.fn.css( "'+e+'", value )')),j.apply(this,arguments)};var x=s.data;if(s.data=function(e,t,r){var n,i,o;if(t&&"object"==typeof t&&2===arguments.length){for(o in n=s.hasData(e)&&x.call(this,e),i={},t)o!==v(o)?(u("jQuery.data() always sets/gets camelCased names: "+o),n[o]=t[o]):i[o]=t[o];return x.call(this,e,i),t}return t&&"string"==typeof t&&t!==v(t)&&(n=s.hasData(e)&&x.call(this,e))&&t in n?(u("jQuery.data() always sets/gets camelCased names: "+t),2<arguments.length&&(n[t]=r),n[t]):x.apply(this,arguments)},s.fx){var A,k,M=s.Tween.prototype.run,S=function(e){return e};s.Tween.prototype.run=function(){1<s.easing[this.easing].length&&(u("'jQuery.easing."+this.easing.toString()+"' should use only one argument"),s.easing[this.easing]=S),M.apply(this,arguments)},A=s.fx.interval||13,k="jQuery.fx.interval is deprecated",n.requestAnimationFrame&&Object.defineProperty(s.fx,"interval",{configurable:!0,enumerable:!0,get:function(){return n.document.hidden||u(k),A},set:function(e){u(k),A=e}})}var N=s.fn.load,R=s.event.add,H=s.event.fix;s.event.props=[],s.event.fixHooks={},t(s.event.props,"concat",s.event.props.concat,"jQuery.event.props.concat() is deprecated and removed"),s.event.fix=function(e){var t,r=e.type,n=this.fixHooks[r],i=s.event.props;if(i.length){u("jQuery.event.props are deprecated and removed: "+i.join());while(i.length)s.event.addProp(i.pop())}if(n&&!n._migrated_&&(n._migrated_=!0,u("jQuery.event.fixHooks are deprecated and removed: "+r),(i=n.props)&&i.length))while(i.length)s.event.addProp(i.pop());return t=H.call(this,e),n&&n.filter?n.filter(t,e):t},s.event.add=function(e,t){return e===n&&"load"===t&&"complete"===n.document.readyState&&u("jQuery(window).on('load'...) called after load event occurred"),R.apply(this,arguments)},s.each(["load","unload","error"],function(e,t){s.fn[t]=function(){var e=Array.prototype.slice.call(arguments,0);return"load"===t&&"string"==typeof e[0]?N.apply(this,e):(u("jQuery.fn."+t+"() is deprecated"),e.splice(0,0,t),arguments.length?this.on.apply(this,e):(this.triggerHandler.apply(this,e),this))}}),s.each("blur focus focusin focusout resize scroll click dblclick mousedown mouseup mousemove mouseover mouseout mouseenter mouseleave change select submit keydown keypress keyup contextmenu".split(" "),function(e,r){s.fn[r]=function(e,t){return u("jQuery.fn."+r+"() event shorthand is deprecated"),0<arguments.length?this.on(r,null,e,t):this.trigger(r)}}),s(function(){s(n.document).triggerHandler("ready")}),s.event.special.ready={setup:function(){this===n.document&&u("'ready' event is deprecated")}},s.fn.extend({bind:function(e,t,r){return u("jQuery.fn.bind() is deprecated"),this.on(e,null,t,r)},unbind:function(e,t){return u("jQuery.fn.unbind() is deprecated"),this.off(e,null,t)},delegate:function(e,t,r,n){return u("jQuery.fn.delegate() is deprecated"),this.on(t,e,r,n)},undelegate:function(e,t,r){return u("jQuery.fn.undelegate() is deprecated"),1===arguments.length?this.off(e,"**"):this.off(t,e||"**",r)},hover:function(e,t){return u("jQuery.fn.hover() is deprecated"),this.on("mouseenter",e).on("mouseleave",t||e)}});function C(e){var t=n.document.implementation.createHTMLDocument("");return t.body.innerHTML=e,t.body&&t.body.innerHTML}function T(e){var t=e.replace(P,"<$1></$2>");t!==e&&C(e)!==C(t)&&u("HTML tags must be properly nested and closed: "+e)}var P=/<(?!area|br|col|embed|hr|img|input|link|meta|param)(([a-z][^\/\0>\x20\t\r\n\f]*)[^>]*)\/>/gi,q=s.htmlPrefilter;s.UNSAFE_restoreLegacyHtmlPrefilter=function(){s.htmlPrefilter=function(e){return T(e),e.replace(P,"<$1></$2>")}},s.htmlPrefilter=function(e){return T(e),q(e)};var D=s.fn.offset;if(s.fn.offset=function(){var e=this[0];return!e||e.nodeType&&e.getBoundingClientRect?D.apply(this,arguments):(u("jQuery.fn.offset() requires a valid DOM element"),arguments.length?this:void 0)},s.ajax){var _=s.param;s.param=function(e,t){var r=s.ajaxSettings&&s.ajaxSettings.traditional;return void 0===t&&r&&(u("jQuery.param() no longer uses jQuery.ajaxSettings.traditional"),t=r),_.call(this,e,t)}}var E=s.fn.andSelf||s.fn.addBack;if(s.fn.andSelf=function(){return u("jQuery.fn.andSelf() is deprecated and removed, use jQuery.fn.addBack()"),E.apply(this,arguments)},s.Deferred){var F=s.Deferred,O=[["resolve","done",s.Callbacks("once memory"),s.Callbacks("once memory"),"resolved"],["reject","fail",s.Callbacks("once memory"),s.Callbacks("once memory"),"rejected"],["notify","progress",s.Callbacks("memory"),s.Callbacks("memory")]];s.Deferred=function(e){var o=F(),a=o.promise();return o.pipe=a.pipe=function(){var i=arguments;return u("deferred.pipe() is deprecated"),s.Deferred(function(n){s.each(O,function(e,t){var r="function"==typeof i[e]&&i[e];o[t[1]](function(){var e=r&&r.apply(this,arguments);e&&"function"==typeof e.promise?e.promise().done(n.resolve).fail(n.reject).progress(n.notify):n[t[0]+"With"](this===a?n.promise():this,r?[e]:arguments)})}),i=null}).promise()},e&&e.call(o,o),o},s.Deferred.exceptionHook=F.exceptionHook}return s});
/* End */
;
; /* Start:"a:4:{s:4:"full";s:69:"/local/templates/.default/markup/new/js/popper.min.js?164206818521257";s:6:"source";s:53:"/local/templates/.default/markup/new/js/popper.min.js";s:3:"min";s:53:"/local/templates/.default/markup/new/js/popper.min.js";s:3:"map";s:57:"/local/templates/.default/markup/new/js/popper.min.js.map";}"*/
/*
 Copyright (C) Federico Zivolo 2019
 Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
 */(function(e,t){'object'==typeof exports&&'undefined'!=typeof module?module.exports=t():'function'==typeof define&&define.amd?define(t):e.Popper=t()})(this,function(){'use strict';function e(e){return e&&'[object Function]'==={}.toString.call(e)}function t(e,t){if(1!==e.nodeType)return[];var o=e.ownerDocument.defaultView,n=o.getComputedStyle(e,null);return t?n[t]:n}function o(e){return'HTML'===e.nodeName?e:e.parentNode||e.host}function n(e){if(!e)return document.body;switch(e.nodeName){case'HTML':case'BODY':return e.ownerDocument.body;case'#document':return e.body;}var i=t(e),r=i.overflow,p=i.overflowX,s=i.overflowY;return /(auto|scroll|overlay)/.test(r+s+p)?e:n(o(e))}function i(e){return e&&e.referenceNode?e.referenceNode:e}function r(e){return 11===e?re:10===e?pe:re||pe}function p(e){if(!e)return document.documentElement;for(var o=r(10)?document.body:null,n=e.offsetParent||null;n===o&&e.nextElementSibling;)n=(e=e.nextElementSibling).offsetParent;var i=n&&n.nodeName;return i&&'BODY'!==i&&'HTML'!==i?-1!==['TH','TD','TABLE'].indexOf(n.nodeName)&&'static'===t(n,'position')?p(n):n:e?e.ownerDocument.documentElement:document.documentElement}function s(e){var t=e.nodeName;return'BODY'!==t&&('HTML'===t||p(e.firstElementChild)===e)}function d(e){return null===e.parentNode?e:d(e.parentNode)}function a(e,t){if(!e||!e.nodeType||!t||!t.nodeType)return document.documentElement;var o=e.compareDocumentPosition(t)&Node.DOCUMENT_POSITION_FOLLOWING,n=o?e:t,i=o?t:e,r=document.createRange();r.setStart(n,0),r.setEnd(i,0);var l=r.commonAncestorContainer;if(e!==l&&t!==l||n.contains(i))return s(l)?l:p(l);var f=d(e);return f.host?a(f.host,t):a(e,d(t).host)}function l(e){var t=1<arguments.length&&void 0!==arguments[1]?arguments[1]:'top',o='top'===t?'scrollTop':'scrollLeft',n=e.nodeName;if('BODY'===n||'HTML'===n){var i=e.ownerDocument.documentElement,r=e.ownerDocument.scrollingElement||i;return r[o]}return e[o]}function f(e,t){var o=2<arguments.length&&void 0!==arguments[2]&&arguments[2],n=l(t,'top'),i=l(t,'left'),r=o?-1:1;return e.top+=n*r,e.bottom+=n*r,e.left+=i*r,e.right+=i*r,e}function m(e,t){var o='x'===t?'Left':'Top',n='Left'==o?'Right':'Bottom';return parseFloat(e['border'+o+'Width'],10)+parseFloat(e['border'+n+'Width'],10)}function h(e,t,o,n){return ee(t['offset'+e],t['scroll'+e],o['client'+e],o['offset'+e],o['scroll'+e],r(10)?parseInt(o['offset'+e])+parseInt(n['margin'+('Height'===e?'Top':'Left')])+parseInt(n['margin'+('Height'===e?'Bottom':'Right')]):0)}function c(e){var t=e.body,o=e.documentElement,n=r(10)&&getComputedStyle(o);return{height:h('Height',t,o,n),width:h('Width',t,o,n)}}function g(e){return le({},e,{right:e.left+e.width,bottom:e.top+e.height})}function u(e){var o={};try{if(r(10)){o=e.getBoundingClientRect();var n=l(e,'top'),i=l(e,'left');o.top+=n,o.left+=i,o.bottom+=n,o.right+=i}else o=e.getBoundingClientRect()}catch(t){}var p={left:o.left,top:o.top,width:o.right-o.left,height:o.bottom-o.top},s='HTML'===e.nodeName?c(e.ownerDocument):{},d=s.width||e.clientWidth||p.width,a=s.height||e.clientHeight||p.height,f=e.offsetWidth-d,h=e.offsetHeight-a;if(f||h){var u=t(e);f-=m(u,'x'),h-=m(u,'y'),p.width-=f,p.height-=h}return g(p)}function b(e,o){var i=2<arguments.length&&void 0!==arguments[2]&&arguments[2],p=r(10),s='HTML'===o.nodeName,d=u(e),a=u(o),l=n(e),m=t(o),h=parseFloat(m.borderTopWidth,10),c=parseFloat(m.borderLeftWidth,10);i&&s&&(a.top=ee(a.top,0),a.left=ee(a.left,0));var b=g({top:d.top-a.top-h,left:d.left-a.left-c,width:d.width,height:d.height});if(b.marginTop=0,b.marginLeft=0,!p&&s){var w=parseFloat(m.marginTop,10),y=parseFloat(m.marginLeft,10);b.top-=h-w,b.bottom-=h-w,b.left-=c-y,b.right-=c-y,b.marginTop=w,b.marginLeft=y}return(p&&!i?o.contains(l):o===l&&'BODY'!==l.nodeName)&&(b=f(b,o)),b}function w(e){var t=1<arguments.length&&void 0!==arguments[1]&&arguments[1],o=e.ownerDocument.documentElement,n=b(e,o),i=ee(o.clientWidth,window.innerWidth||0),r=ee(o.clientHeight,window.innerHeight||0),p=t?0:l(o),s=t?0:l(o,'left'),d={top:p-n.top+n.marginTop,left:s-n.left+n.marginLeft,width:i,height:r};return g(d)}function y(e){var n=e.nodeName;if('BODY'===n||'HTML'===n)return!1;if('fixed'===t(e,'position'))return!0;var i=o(e);return!!i&&y(i)}function E(e){if(!e||!e.parentElement||r())return document.documentElement;for(var o=e.parentElement;o&&'none'===t(o,'transform');)o=o.parentElement;return o||document.documentElement}function v(e,t,r,p){var s=4<arguments.length&&void 0!==arguments[4]&&arguments[4],d={top:0,left:0},l=s?E(e):a(e,i(t));if('viewport'===p)d=w(l,s);else{var f;'scrollParent'===p?(f=n(o(t)),'BODY'===f.nodeName&&(f=e.ownerDocument.documentElement)):'window'===p?f=e.ownerDocument.documentElement:f=p;var m=b(f,l,s);if('HTML'===f.nodeName&&!y(l)){var h=c(e.ownerDocument),g=h.height,u=h.width;d.top+=m.top-m.marginTop,d.bottom=g+m.top,d.left+=m.left-m.marginLeft,d.right=u+m.left}else d=m}r=r||0;var v='number'==typeof r;return d.left+=v?r:r.left||0,d.top+=v?r:r.top||0,d.right-=v?r:r.right||0,d.bottom-=v?r:r.bottom||0,d}function x(e){var t=e.width,o=e.height;return t*o}function O(e,t,o,n,i){var r=5<arguments.length&&void 0!==arguments[5]?arguments[5]:0;if(-1===e.indexOf('auto'))return e;var p=v(o,n,r,i),s={top:{width:p.width,height:t.top-p.top},right:{width:p.right-t.right,height:p.height},bottom:{width:p.width,height:p.bottom-t.bottom},left:{width:t.left-p.left,height:p.height}},d=Object.keys(s).map(function(e){return le({key:e},s[e],{area:x(s[e])})}).sort(function(e,t){return t.area-e.area}),a=d.filter(function(e){var t=e.width,n=e.height;return t>=o.clientWidth&&n>=o.clientHeight}),l=0<a.length?a[0].key:d[0].key,f=e.split('-')[1];return l+(f?'-'+f:'')}function L(e,t,o){var n=3<arguments.length&&void 0!==arguments[3]?arguments[3]:null,r=n?E(t):a(t,i(o));return b(o,r,n)}function S(e){var t=e.ownerDocument.defaultView,o=t.getComputedStyle(e),n=parseFloat(o.marginTop||0)+parseFloat(o.marginBottom||0),i=parseFloat(o.marginLeft||0)+parseFloat(o.marginRight||0),r={width:e.offsetWidth+i,height:e.offsetHeight+n};return r}function T(e){var t={left:'right',right:'left',bottom:'top',top:'bottom'};return e.replace(/left|right|bottom|top/g,function(e){return t[e]})}function C(e,t,o){o=o.split('-')[0];var n=S(e),i={width:n.width,height:n.height},r=-1!==['right','left'].indexOf(o),p=r?'top':'left',s=r?'left':'top',d=r?'height':'width',a=r?'width':'height';return i[p]=t[p]+t[d]/2-n[d]/2,i[s]=o===s?t[s]-n[a]:t[T(s)],i}function D(e,t){return Array.prototype.find?e.find(t):e.filter(t)[0]}function N(e,t,o){if(Array.prototype.findIndex)return e.findIndex(function(e){return e[t]===o});var n=D(e,function(e){return e[t]===o});return e.indexOf(n)}function P(t,o,n){var i=void 0===n?t:t.slice(0,N(t,'name',n));return i.forEach(function(t){t['function']&&console.warn('`modifier.function` is deprecated, use `modifier.fn`!');var n=t['function']||t.fn;t.enabled&&e(n)&&(o.offsets.popper=g(o.offsets.popper),o.offsets.reference=g(o.offsets.reference),o=n(o,t))}),o}function k(){if(!this.state.isDestroyed){var e={instance:this,styles:{},arrowStyles:{},attributes:{},flipped:!1,offsets:{}};e.offsets.reference=L(this.state,this.popper,this.reference,this.options.positionFixed),e.placement=O(this.options.placement,e.offsets.reference,this.popper,this.reference,this.options.modifiers.flip.boundariesElement,this.options.modifiers.flip.padding),e.originalPlacement=e.placement,e.positionFixed=this.options.positionFixed,e.offsets.popper=C(this.popper,e.offsets.reference,e.placement),e.offsets.popper.position=this.options.positionFixed?'fixed':'absolute',e=P(this.modifiers,e),this.state.isCreated?this.options.onUpdate(e):(this.state.isCreated=!0,this.options.onCreate(e))}}function W(e,t){return e.some(function(e){var o=e.name,n=e.enabled;return n&&o===t})}function B(e){for(var t=[!1,'ms','Webkit','Moz','O'],o=e.charAt(0).toUpperCase()+e.slice(1),n=0;n<t.length;n++){var i=t[n],r=i?''+i+o:e;if('undefined'!=typeof document.body.style[r])return r}return null}function H(){return this.state.isDestroyed=!0,W(this.modifiers,'applyStyle')&&(this.popper.removeAttribute('x-placement'),this.popper.style.position='',this.popper.style.top='',this.popper.style.left='',this.popper.style.right='',this.popper.style.bottom='',this.popper.style.willChange='',this.popper.style[B('transform')]=''),this.disableEventListeners(),this.options.removeOnDestroy&&this.popper.parentNode.removeChild(this.popper),this}function A(e){var t=e.ownerDocument;return t?t.defaultView:window}function M(e,t,o,i){var r='BODY'===e.nodeName,p=r?e.ownerDocument.defaultView:e;p.addEventListener(t,o,{passive:!0}),r||M(n(p.parentNode),t,o,i),i.push(p)}function F(e,t,o,i){o.updateBound=i,A(e).addEventListener('resize',o.updateBound,{passive:!0});var r=n(e);return M(r,'scroll',o.updateBound,o.scrollParents),o.scrollElement=r,o.eventsEnabled=!0,o}function I(){this.state.eventsEnabled||(this.state=F(this.reference,this.options,this.state,this.scheduleUpdate))}function R(e,t){return A(e).removeEventListener('resize',t.updateBound),t.scrollParents.forEach(function(e){e.removeEventListener('scroll',t.updateBound)}),t.updateBound=null,t.scrollParents=[],t.scrollElement=null,t.eventsEnabled=!1,t}function U(){this.state.eventsEnabled&&(cancelAnimationFrame(this.scheduleUpdate),this.state=R(this.reference,this.state))}function Y(e){return''!==e&&!isNaN(parseFloat(e))&&isFinite(e)}function V(e,t){Object.keys(t).forEach(function(o){var n='';-1!==['width','height','top','right','bottom','left'].indexOf(o)&&Y(t[o])&&(n='px'),e.style[o]=t[o]+n})}function j(e,t){Object.keys(t).forEach(function(o){var n=t[o];!1===n?e.removeAttribute(o):e.setAttribute(o,t[o])})}function q(e,t){var o=e.offsets,n=o.popper,i=o.reference,r=$,p=function(e){return e},s=r(i.width),d=r(n.width),a=-1!==['left','right'].indexOf(e.placement),l=-1!==e.placement.indexOf('-'),f=t?a||l||s%2==d%2?r:Z:p,m=t?r:p;return{left:f(1==s%2&&1==d%2&&!l&&t?n.left-1:n.left),top:m(n.top),bottom:m(n.bottom),right:f(n.right)}}function K(e,t,o){var n=D(e,function(e){var o=e.name;return o===t}),i=!!n&&e.some(function(e){return e.name===o&&e.enabled&&e.order<n.order});if(!i){var r='`'+t+'`';console.warn('`'+o+'`'+' modifier is required by '+r+' modifier in order to work, be sure to include it before '+r+'!')}return i}function z(e){return'end'===e?'start':'start'===e?'end':e}function G(e){var t=1<arguments.length&&void 0!==arguments[1]&&arguments[1],o=he.indexOf(e),n=he.slice(o+1).concat(he.slice(0,o));return t?n.reverse():n}function _(e,t,o,n){var i=e.match(/((?:\-|\+)?\d*\.?\d*)(.*)/),r=+i[1],p=i[2];if(!r)return e;if(0===p.indexOf('%')){var s;switch(p){case'%p':s=o;break;case'%':case'%r':default:s=n;}var d=g(s);return d[t]/100*r}if('vh'===p||'vw'===p){var a;return a='vh'===p?ee(document.documentElement.clientHeight,window.innerHeight||0):ee(document.documentElement.clientWidth,window.innerWidth||0),a/100*r}return r}function X(e,t,o,n){var i=[0,0],r=-1!==['right','left'].indexOf(n),p=e.split(/(\+|\-)/).map(function(e){return e.trim()}),s=p.indexOf(D(p,function(e){return-1!==e.search(/,|\s/)}));p[s]&&-1===p[s].indexOf(',')&&console.warn('Offsets separated by white space(s) are deprecated, use a comma (,) instead.');var d=/\s*,\s*|\s+/,a=-1===s?[p]:[p.slice(0,s).concat([p[s].split(d)[0]]),[p[s].split(d)[1]].concat(p.slice(s+1))];return a=a.map(function(e,n){var i=(1===n?!r:r)?'height':'width',p=!1;return e.reduce(function(e,t){return''===e[e.length-1]&&-1!==['+','-'].indexOf(t)?(e[e.length-1]=t,p=!0,e):p?(e[e.length-1]+=t,p=!1,e):e.concat(t)},[]).map(function(e){return _(e,i,t,o)})}),a.forEach(function(e,t){e.forEach(function(o,n){Y(o)&&(i[t]+=o*('-'===e[n-1]?-1:1))})}),i}function J(e,t){var o,n=t.offset,i=e.placement,r=e.offsets,p=r.popper,s=r.reference,d=i.split('-')[0];return o=Y(+n)?[+n,0]:X(n,p,s,d),'left'===d?(p.top+=o[0],p.left-=o[1]):'right'===d?(p.top+=o[0],p.left+=o[1]):'top'===d?(p.left+=o[0],p.top-=o[1]):'bottom'===d&&(p.left+=o[0],p.top+=o[1]),e.popper=p,e}var Q=Math.min,Z=Math.floor,$=Math.round,ee=Math.max,te='undefined'!=typeof window&&'undefined'!=typeof document&&'undefined'!=typeof navigator,oe=function(){for(var e=['Edge','Trident','Firefox'],t=0;t<e.length;t+=1)if(te&&0<=navigator.userAgent.indexOf(e[t]))return 1;return 0}(),ne=te&&window.Promise,ie=ne?function(e){var t=!1;return function(){t||(t=!0,window.Promise.resolve().then(function(){t=!1,e()}))}}:function(e){var t=!1;return function(){t||(t=!0,setTimeout(function(){t=!1,e()},oe))}},re=te&&!!(window.MSInputMethodContext&&document.documentMode),pe=te&&/MSIE 10/.test(navigator.userAgent),se=function(e,t){if(!(e instanceof t))throw new TypeError('Cannot call a class as a function')},de=function(){function e(e,t){for(var o,n=0;n<t.length;n++)o=t[n],o.enumerable=o.enumerable||!1,o.configurable=!0,'value'in o&&(o.writable=!0),Object.defineProperty(e,o.key,o)}return function(t,o,n){return o&&e(t.prototype,o),n&&e(t,n),t}}(),ae=function(e,t,o){return t in e?Object.defineProperty(e,t,{value:o,enumerable:!0,configurable:!0,writable:!0}):e[t]=o,e},le=Object.assign||function(e){for(var t,o=1;o<arguments.length;o++)for(var n in t=arguments[o],t)Object.prototype.hasOwnProperty.call(t,n)&&(e[n]=t[n]);return e},fe=te&&/Firefox/i.test(navigator.userAgent),me=['auto-start','auto','auto-end','top-start','top','top-end','right-start','right','right-end','bottom-end','bottom','bottom-start','left-end','left','left-start'],he=me.slice(3),ce={FLIP:'flip',CLOCKWISE:'clockwise',COUNTERCLOCKWISE:'counterclockwise'},ge=function(){function t(o,n){var i=this,r=2<arguments.length&&void 0!==arguments[2]?arguments[2]:{};se(this,t),this.scheduleUpdate=function(){return requestAnimationFrame(i.update)},this.update=ie(this.update.bind(this)),this.options=le({},t.Defaults,r),this.state={isDestroyed:!1,isCreated:!1,scrollParents:[]},this.reference=o&&o.jquery?o[0]:o,this.popper=n&&n.jquery?n[0]:n,this.options.modifiers={},Object.keys(le({},t.Defaults.modifiers,r.modifiers)).forEach(function(e){i.options.modifiers[e]=le({},t.Defaults.modifiers[e]||{},r.modifiers?r.modifiers[e]:{})}),this.modifiers=Object.keys(this.options.modifiers).map(function(e){return le({name:e},i.options.modifiers[e])}).sort(function(e,t){return e.order-t.order}),this.modifiers.forEach(function(t){t.enabled&&e(t.onLoad)&&t.onLoad(i.reference,i.popper,i.options,t,i.state)}),this.update();var p=this.options.eventsEnabled;p&&this.enableEventListeners(),this.state.eventsEnabled=p}return de(t,[{key:'update',value:function(){return k.call(this)}},{key:'destroy',value:function(){return H.call(this)}},{key:'enableEventListeners',value:function(){return I.call(this)}},{key:'disableEventListeners',value:function(){return U.call(this)}}]),t}();return ge.Utils=('undefined'==typeof window?global:window).PopperUtils,ge.placements=me,ge.Defaults={placement:'bottom',positionFixed:!1,eventsEnabled:!0,removeOnDestroy:!1,onCreate:function(){},onUpdate:function(){},modifiers:{shift:{order:100,enabled:!0,fn:function(e){var t=e.placement,o=t.split('-')[0],n=t.split('-')[1];if(n){var i=e.offsets,r=i.reference,p=i.popper,s=-1!==['bottom','top'].indexOf(o),d=s?'left':'top',a=s?'width':'height',l={start:ae({},d,r[d]),end:ae({},d,r[d]+r[a]-p[a])};e.offsets.popper=le({},p,l[n])}return e}},offset:{order:200,enabled:!0,fn:J,offset:0},preventOverflow:{order:300,enabled:!0,fn:function(e,t){var o=t.boundariesElement||p(e.instance.popper);e.instance.reference===o&&(o=p(o));var n=B('transform'),i=e.instance.popper.style,r=i.top,s=i.left,d=i[n];i.top='',i.left='',i[n]='';var a=v(e.instance.popper,e.instance.reference,t.padding,o,e.positionFixed);i.top=r,i.left=s,i[n]=d,t.boundaries=a;var l=t.priority,f=e.offsets.popper,m={primary:function(e){var o=f[e];return f[e]<a[e]&&!t.escapeWithReference&&(o=ee(f[e],a[e])),ae({},e,o)},secondary:function(e){var o='right'===e?'left':'top',n=f[o];return f[e]>a[e]&&!t.escapeWithReference&&(n=Q(f[o],a[e]-('right'===e?f.width:f.height))),ae({},o,n)}};return l.forEach(function(e){var t=-1===['left','top'].indexOf(e)?'secondary':'primary';f=le({},f,m[t](e))}),e.offsets.popper=f,e},priority:['left','right','top','bottom'],padding:5,boundariesElement:'scrollParent'},keepTogether:{order:400,enabled:!0,fn:function(e){var t=e.offsets,o=t.popper,n=t.reference,i=e.placement.split('-')[0],r=Z,p=-1!==['top','bottom'].indexOf(i),s=p?'right':'bottom',d=p?'left':'top',a=p?'width':'height';return o[s]<r(n[d])&&(e.offsets.popper[d]=r(n[d])-o[a]),o[d]>r(n[s])&&(e.offsets.popper[d]=r(n[s])),e}},arrow:{order:500,enabled:!0,fn:function(e,o){var n;if(!K(e.instance.modifiers,'arrow','keepTogether'))return e;var i=o.element;if('string'==typeof i){if(i=e.instance.popper.querySelector(i),!i)return e;}else if(!e.instance.popper.contains(i))return console.warn('WARNING: `arrow.element` must be child of its popper element!'),e;var r=e.placement.split('-')[0],p=e.offsets,s=p.popper,d=p.reference,a=-1!==['left','right'].indexOf(r),l=a?'height':'width',f=a?'Top':'Left',m=f.toLowerCase(),h=a?'left':'top',c=a?'bottom':'right',u=S(i)[l];d[c]-u<s[m]&&(e.offsets.popper[m]-=s[m]-(d[c]-u)),d[m]+u>s[c]&&(e.offsets.popper[m]+=d[m]+u-s[c]),e.offsets.popper=g(e.offsets.popper);var b=d[m]+d[l]/2-u/2,w=t(e.instance.popper),y=parseFloat(w['margin'+f],10),E=parseFloat(w['border'+f+'Width'],10),v=b-e.offsets.popper[m]-y-E;return v=ee(Q(s[l]-u,v),0),e.arrowElement=i,e.offsets.arrow=(n={},ae(n,m,$(v)),ae(n,h,''),n),e},element:'[x-arrow]'},flip:{order:600,enabled:!0,fn:function(e,t){if(W(e.instance.modifiers,'inner'))return e;if(e.flipped&&e.placement===e.originalPlacement)return e;var o=v(e.instance.popper,e.instance.reference,t.padding,t.boundariesElement,e.positionFixed),n=e.placement.split('-')[0],i=T(n),r=e.placement.split('-')[1]||'',p=[];switch(t.behavior){case ce.FLIP:p=[n,i];break;case ce.CLOCKWISE:p=G(n);break;case ce.COUNTERCLOCKWISE:p=G(n,!0);break;default:p=t.behavior;}return p.forEach(function(s,d){if(n!==s||p.length===d+1)return e;n=e.placement.split('-')[0],i=T(n);var a=e.offsets.popper,l=e.offsets.reference,f=Z,m='left'===n&&f(a.right)>f(l.left)||'right'===n&&f(a.left)<f(l.right)||'top'===n&&f(a.bottom)>f(l.top)||'bottom'===n&&f(a.top)<f(l.bottom),h=f(a.left)<f(o.left),c=f(a.right)>f(o.right),g=f(a.top)<f(o.top),u=f(a.bottom)>f(o.bottom),b='left'===n&&h||'right'===n&&c||'top'===n&&g||'bottom'===n&&u,w=-1!==['top','bottom'].indexOf(n),y=!!t.flipVariations&&(w&&'start'===r&&h||w&&'end'===r&&c||!w&&'start'===r&&g||!w&&'end'===r&&u),E=!!t.flipVariationsByContent&&(w&&'start'===r&&c||w&&'end'===r&&h||!w&&'start'===r&&u||!w&&'end'===r&&g),v=y||E;(m||b||v)&&(e.flipped=!0,(m||b)&&(n=p[d+1]),v&&(r=z(r)),e.placement=n+(r?'-'+r:''),e.offsets.popper=le({},e.offsets.popper,C(e.instance.popper,e.offsets.reference,e.placement)),e=P(e.instance.modifiers,e,'flip'))}),e},behavior:'flip',padding:5,boundariesElement:'viewport',flipVariations:!1,flipVariationsByContent:!1},inner:{order:700,enabled:!1,fn:function(e){var t=e.placement,o=t.split('-')[0],n=e.offsets,i=n.popper,r=n.reference,p=-1!==['left','right'].indexOf(o),s=-1===['top','left'].indexOf(o);return i[p?'left':'top']=r[o]-(s?i[p?'width':'height']:0),e.placement=T(t),e.offsets.popper=g(i),e}},hide:{order:800,enabled:!0,fn:function(e){if(!K(e.instance.modifiers,'hide','preventOverflow'))return e;var t=e.offsets.reference,o=D(e.instance.modifiers,function(e){return'preventOverflow'===e.name}).boundaries;if(t.bottom<o.top||t.left>o.right||t.top>o.bottom||t.right<o.left){if(!0===e.hide)return e;e.hide=!0,e.attributes['x-out-of-boundaries']=''}else{if(!1===e.hide)return e;e.hide=!1,e.attributes['x-out-of-boundaries']=!1}return e}},computeStyle:{order:850,enabled:!0,fn:function(e,t){var o=t.x,n=t.y,i=e.offsets.popper,r=D(e.instance.modifiers,function(e){return'applyStyle'===e.name}).gpuAcceleration;void 0!==r&&console.warn('WARNING: `gpuAcceleration` option moved to `computeStyle` modifier and will not be supported in future versions of Popper.js!');var s,d,a=void 0===r?t.gpuAcceleration:r,l=p(e.instance.popper),f=u(l),m={position:i.position},h=q(e,2>window.devicePixelRatio||!fe),c='bottom'===o?'top':'bottom',g='right'===n?'left':'right',b=B('transform');if(d='bottom'==c?'HTML'===l.nodeName?-l.clientHeight+h.bottom:-f.height+h.bottom:h.top,s='right'==g?'HTML'===l.nodeName?-l.clientWidth+h.right:-f.width+h.right:h.left,a&&b)m[b]='translate3d('+s+'px, '+d+'px, 0)',m[c]=0,m[g]=0,m.willChange='transform';else{var w='bottom'==c?-1:1,y='right'==g?-1:1;m[c]=d*w,m[g]=s*y,m.willChange=c+', '+g}var E={"x-placement":e.placement};return e.attributes=le({},E,e.attributes),e.styles=le({},m,e.styles),e.arrowStyles=le({},e.offsets.arrow,e.arrowStyles),e},gpuAcceleration:!0,x:'bottom',y:'right'},applyStyle:{order:900,enabled:!0,fn:function(e){return V(e.instance.popper,e.styles),j(e.instance.popper,e.attributes),e.arrowElement&&Object.keys(e.arrowStyles).length&&V(e.arrowElement,e.arrowStyles),e},onLoad:function(e,t,o,n,i){var r=L(i,t,e,o.positionFixed),p=O(o.placement,r,t,e,o.modifiers.flip.boundariesElement,o.modifiers.flip.padding);return t.setAttribute('x-placement',p),V(t,{position:o.positionFixed?'fixed':'absolute'}),o},gpuAcceleration:void 0}}},ge});
/* End */
;
; /* Start:"a:4:{s:4:"full";s:72:"/local/templates/.default/markup/new/js/bootstrap.min.js?164206817860010";s:6:"source";s:56:"/local/templates/.default/markup/new/js/bootstrap.min.js";s:3:"min";s:56:"/local/templates/.default/markup/new/js/bootstrap.min.js";s:3:"map";s:60:"/local/templates/.default/markup/new/js/bootstrap.min.js.map";}"*/
/*!
  * Bootstrap v4.4.1 (https://getbootstrap.com/)
  * Copyright 2011-2019 The Bootstrap Authors (https://github.com/twbs/bootstrap/graphs/contributors)
  * Licensed under MIT (https://github.com/twbs/bootstrap/blob/master/LICENSE)
  */
!function(t,e){"object"==typeof exports&&"undefined"!=typeof module?e(exports,require("jquery"),require("popper.js")):"function"==typeof define&&define.amd?define(["exports","jquery","popper.js"],e):e((t=t||self).bootstrap={},t.jQuery,t.Popper)}(this,function(t,g,u){"use strict";function i(t,e){for(var n=0;n<e.length;n++){var i=e[n];i.enumerable=i.enumerable||!1,i.configurable=!0,"value"in i&&(i.writable=!0),Object.defineProperty(t,i.key,i)}}function s(t,e,n){return e&&i(t.prototype,e),n&&i(t,n),t}function e(e,t){var n=Object.keys(e);if(Object.getOwnPropertySymbols){var i=Object.getOwnPropertySymbols(e);t&&(i=i.filter(function(t){return Object.getOwnPropertyDescriptor(e,t).enumerable})),n.push.apply(n,i)}return n}function l(o){for(var t=1;t<arguments.length;t++){var r=null!=arguments[t]?arguments[t]:{};t%2?e(Object(r),!0).forEach(function(t){var e,n,i;e=o,i=r[n=t],n in e?Object.defineProperty(e,n,{value:i,enumerable:!0,configurable:!0,writable:!0}):e[n]=i}):Object.getOwnPropertyDescriptors?Object.defineProperties(o,Object.getOwnPropertyDescriptors(r)):e(Object(r)).forEach(function(t){Object.defineProperty(o,t,Object.getOwnPropertyDescriptor(r,t))})}return o}g=g&&g.hasOwnProperty("default")?g.default:g,u=u&&u.hasOwnProperty("default")?u.default:u;var n="transitionend";function o(t){var e=this,n=!1;return g(this).one(_.TRANSITION_END,function(){n=!0}),setTimeout(function(){n||_.triggerTransitionEnd(e)},t),this}var _={TRANSITION_END:"bsTransitionEnd",getUID:function(t){for(;t+=~~(1e6*Math.random()),document.getElementById(t););return t},getSelectorFromElement:function(t){var e=t.getAttribute("data-target");if(!e||"#"===e){var n=t.getAttribute("href");e=n&&"#"!==n?n.trim():""}try{return document.querySelector(e)?e:null}catch(t){return null}},getTransitionDurationFromElement:function(t){if(!t)return 0;var e=g(t).css("transition-duration"),n=g(t).css("transition-delay"),i=parseFloat(e),o=parseFloat(n);return i||o?(e=e.split(",")[0],n=n.split(",")[0],1e3*(parseFloat(e)+parseFloat(n))):0},reflow:function(t){return t.offsetHeight},triggerTransitionEnd:function(t){g(t).trigger(n)},supportsTransitionEnd:function(){return Boolean(n)},isElement:function(t){return(t[0]||t).nodeType},typeCheckConfig:function(t,e,n){for(var i in n)if(Object.prototype.hasOwnProperty.call(n,i)){var o=n[i],r=e[i],s=r&&_.isElement(r)?"element":(a=r,{}.toString.call(a).match(/\s([a-z]+)/i)[1].toLowerCase());if(!new RegExp(o).test(s))throw new Error(t.toUpperCase()+': Option "'+i+'" provided type "'+s+'" but expected type "'+o+'".')}var a},findShadowRoot:function(t){if(!document.documentElement.attachShadow)return null;if("function"!=typeof t.getRootNode)return t instanceof ShadowRoot?t:t.parentNode?_.findShadowRoot(t.parentNode):null;var e=t.getRootNode();return e instanceof ShadowRoot?e:null},jQueryDetection:function(){if("undefined"==typeof g)throw new TypeError("Bootstrap's JavaScript requires jQuery. jQuery must be included before Bootstrap's JavaScript.");var t=g.fn.jquery.split(" ")[0].split(".");if(t[0]<2&&t[1]<9||1===t[0]&&9===t[1]&&t[2]<1||4<=t[0])throw new Error("Bootstrap's JavaScript requires at least jQuery v1.9.1 but less than v4.0.0")}};_.jQueryDetection(),g.fn.emulateTransitionEnd=o,g.event.special[_.TRANSITION_END]={bindType:n,delegateType:n,handle:function(t){if(g(t.target).is(this))return t.handleObj.handler.apply(this,arguments)}};var r="alert",a="bs.alert",c="."+a,h=g.fn[r],f={CLOSE:"close"+c,CLOSED:"closed"+c,CLICK_DATA_API:"click"+c+".data-api"},d="alert",m="fade",p="show",v=function(){function i(t){this._element=t}var t=i.prototype;return t.close=function(t){var e=this._element;t&&(e=this._getRootElement(t)),this._triggerCloseEvent(e).isDefaultPrevented()||this._removeElement(e)},t.dispose=function(){g.removeData(this._element,a),this._element=null},t._getRootElement=function(t){var e=_.getSelectorFromElement(t),n=!1;return e&&(n=document.querySelector(e)),n=n||g(t).closest("."+d)[0]},t._triggerCloseEvent=function(t){var e=g.Event(f.CLOSE);return g(t).trigger(e),e},t._removeElement=function(e){var n=this;if(g(e).removeClass(p),g(e).hasClass(m)){var t=_.getTransitionDurationFromElement(e);g(e).one(_.TRANSITION_END,function(t){return n._destroyElement(e,t)}).emulateTransitionEnd(t)}else this._destroyElement(e)},t._destroyElement=function(t){g(t).detach().trigger(f.CLOSED).remove()},i._jQueryInterface=function(n){return this.each(function(){var t=g(this),e=t.data(a);e||(e=new i(this),t.data(a,e)),"close"===n&&e[n](this)})},i._handleDismiss=function(e){return function(t){t&&t.preventDefault(),e.close(this)}},s(i,null,[{key:"VERSION",get:function(){return"4.4.1"}}]),i}();g(document).on(f.CLICK_DATA_API,'[data-dismiss="alert"]',v._handleDismiss(new v)),g.fn[r]=v._jQueryInterface,g.fn[r].Constructor=v,g.fn[r].noConflict=function(){return g.fn[r]=h,v._jQueryInterface};var y="button",E="bs.button",C="."+E,T=".data-api",b=g.fn[y],S="active",D="btn",I="focus",w='[data-toggle^="button"]',A='[data-toggle="buttons"]',N='[data-toggle="button"]',O='[data-toggle="buttons"] .btn',k='input:not([type="hidden"])',P=".active",L=".btn",j={CLICK_DATA_API:"click"+C+T,FOCUS_BLUR_DATA_API:"focus"+C+T+" blur"+C+T,LOAD_DATA_API:"load"+C+T},H=function(){function n(t){this._element=t}var t=n.prototype;return t.toggle=function(){var t=!0,e=!0,n=g(this._element).closest(A)[0];if(n){var i=this._element.querySelector(k);if(i){if("radio"===i.type)if(i.checked&&this._element.classList.contains(S))t=!1;else{var o=n.querySelector(P);o&&g(o).removeClass(S)}else"checkbox"===i.type?"LABEL"===this._element.tagName&&i.checked===this._element.classList.contains(S)&&(t=!1):t=!1;t&&(i.checked=!this._element.classList.contains(S),g(i).trigger("change")),i.focus(),e=!1}}this._element.hasAttribute("disabled")||this._element.classList.contains("disabled")||(e&&this._element.setAttribute("aria-pressed",!this._element.classList.contains(S)),t&&g(this._element).toggleClass(S))},t.dispose=function(){g.removeData(this._element,E),this._element=null},n._jQueryInterface=function(e){return this.each(function(){var t=g(this).data(E);t||(t=new n(this),g(this).data(E,t)),"toggle"===e&&t[e]()})},s(n,null,[{key:"VERSION",get:function(){return"4.4.1"}}]),n}();g(document).on(j.CLICK_DATA_API,w,function(t){var e=t.target;if(g(e).hasClass(D)||(e=g(e).closest(L)[0]),!e||e.hasAttribute("disabled")||e.classList.contains("disabled"))t.preventDefault();else{var n=e.querySelector(k);if(n&&(n.hasAttribute("disabled")||n.classList.contains("disabled")))return void t.preventDefault();H._jQueryInterface.call(g(e),"toggle")}}).on(j.FOCUS_BLUR_DATA_API,w,function(t){var e=g(t.target).closest(L)[0];g(e).toggleClass(I,/^focus(in)?$/.test(t.type))}),g(window).on(j.LOAD_DATA_API,function(){for(var t=[].slice.call(document.querySelectorAll(O)),e=0,n=t.length;e<n;e++){var i=t[e],o=i.querySelector(k);o.checked||o.hasAttribute("checked")?i.classList.add(S):i.classList.remove(S)}for(var r=0,s=(t=[].slice.call(document.querySelectorAll(N))).length;r<s;r++){var a=t[r];"true"===a.getAttribute("aria-pressed")?a.classList.add(S):a.classList.remove(S)}}),g.fn[y]=H._jQueryInterface,g.fn[y].Constructor=H,g.fn[y].noConflict=function(){return g.fn[y]=b,H._jQueryInterface};var R="carousel",x="bs.carousel",F="."+x,U=".data-api",W=g.fn[R],q={interval:5e3,keyboard:!0,slide:!1,pause:"hover",wrap:!0,touch:!0},M={interval:"(number|boolean)",keyboard:"boolean",slide:"(boolean|string)",pause:"(string|boolean)",wrap:"boolean",touch:"boolean"},K="next",Q="prev",B="left",V="right",Y={SLIDE:"slide"+F,SLID:"slid"+F,KEYDOWN:"keydown"+F,MOUSEENTER:"mouseenter"+F,MOUSELEAVE:"mouseleave"+F,TOUCHSTART:"touchstart"+F,TOUCHMOVE:"touchmove"+F,TOUCHEND:"touchend"+F,POINTERDOWN:"pointerdown"+F,POINTERUP:"pointerup"+F,DRAG_START:"dragstart"+F,LOAD_DATA_API:"load"+F+U,CLICK_DATA_API:"click"+F+U},z="carousel",X="active",$="slide",G="carousel-item-right",J="carousel-item-left",Z="carousel-item-next",tt="carousel-item-prev",et="pointer-event",nt=".active",it=".active.carousel-item",ot=".carousel-item",rt=".carousel-item img",st=".carousel-item-next, .carousel-item-prev",at=".carousel-indicators",lt="[data-slide], [data-slide-to]",ct='[data-ride="carousel"]',ht={TOUCH:"touch",PEN:"pen"},ut=function(){function r(t,e){this._items=null,this._interval=null,this._activeElement=null,this._isPaused=!1,this._isSliding=!1,this.touchTimeout=null,this.touchStartX=0,this.touchDeltaX=0,this._config=this._getConfig(e),this._element=t,this._indicatorsElement=this._element.querySelector(at),this._touchSupported="ontouchstart"in document.documentElement||0<navigator.maxTouchPoints,this._pointerEvent=Boolean(window.PointerEvent||window.MSPointerEvent),this._addEventListeners()}var t=r.prototype;return t.next=function(){this._isSliding||this._slide(K)},t.nextWhenVisible=function(){!document.hidden&&g(this._element).is(":visible")&&"hidden"!==g(this._element).css("visibility")&&this.next()},t.prev=function(){this._isSliding||this._slide(Q)},t.pause=function(t){t||(this._isPaused=!0),this._element.querySelector(st)&&(_.triggerTransitionEnd(this._element),this.cycle(!0)),clearInterval(this._interval),this._interval=null},t.cycle=function(t){t||(this._isPaused=!1),this._interval&&(clearInterval(this._interval),this._interval=null),this._config.interval&&!this._isPaused&&(this._interval=setInterval((document.visibilityState?this.nextWhenVisible:this.next).bind(this),this._config.interval))},t.to=function(t){var e=this;this._activeElement=this._element.querySelector(it);var n=this._getItemIndex(this._activeElement);if(!(t>this._items.length-1||t<0))if(this._isSliding)g(this._element).one(Y.SLID,function(){return e.to(t)});else{if(n===t)return this.pause(),void this.cycle();var i=n<t?K:Q;this._slide(i,this._items[t])}},t.dispose=function(){g(this._element).off(F),g.removeData(this._element,x),this._items=null,this._config=null,this._element=null,this._interval=null,this._isPaused=null,this._isSliding=null,this._activeElement=null,this._indicatorsElement=null},t._getConfig=function(t){return t=l({},q,{},t),_.typeCheckConfig(R,t,M),t},t._handleSwipe=function(){var t=Math.abs(this.touchDeltaX);if(!(t<=40)){var e=t/this.touchDeltaX;(this.touchDeltaX=0)<e&&this.prev(),e<0&&this.next()}},t._addEventListeners=function(){var e=this;this._config.keyboard&&g(this._element).on(Y.KEYDOWN,function(t){return e._keydown(t)}),"hover"===this._config.pause&&g(this._element).on(Y.MOUSEENTER,function(t){return e.pause(t)}).on(Y.MOUSELEAVE,function(t){return e.cycle(t)}),this._config.touch&&this._addTouchEventListeners()},t._addTouchEventListeners=function(){var e=this;if(this._touchSupported){var n=function(t){e._pointerEvent&&ht[t.originalEvent.pointerType.toUpperCase()]?e.touchStartX=t.originalEvent.clientX:e._pointerEvent||(e.touchStartX=t.originalEvent.touches[0].clientX)},i=function(t){e._pointerEvent&&ht[t.originalEvent.pointerType.toUpperCase()]&&(e.touchDeltaX=t.originalEvent.clientX-e.touchStartX),e._handleSwipe(),"hover"===e._config.pause&&(e.pause(),e.touchTimeout&&clearTimeout(e.touchTimeout),e.touchTimeout=setTimeout(function(t){return e.cycle(t)},500+e._config.interval))};g(this._element.querySelectorAll(rt)).on(Y.DRAG_START,function(t){return t.preventDefault()}),this._pointerEvent?(g(this._element).on(Y.POINTERDOWN,function(t){return n(t)}),g(this._element).on(Y.POINTERUP,function(t){return i(t)}),this._element.classList.add(et)):(g(this._element).on(Y.TOUCHSTART,function(t){return n(t)}),g(this._element).on(Y.TOUCHMOVE,function(t){return function(t){t.originalEvent.touches&&1<t.originalEvent.touches.length?e.touchDeltaX=0:e.touchDeltaX=t.originalEvent.touches[0].clientX-e.touchStartX}(t)}),g(this._element).on(Y.TOUCHEND,function(t){return i(t)}))}},t._keydown=function(t){if(!/input|textarea/i.test(t.target.tagName))switch(t.which){case 37:t.preventDefault(),this.prev();break;case 39:t.preventDefault(),this.next()}},t._getItemIndex=function(t){return this._items=t&&t.parentNode?[].slice.call(t.parentNode.querySelectorAll(ot)):[],this._items.indexOf(t)},t._getItemByDirection=function(t,e){var n=t===K,i=t===Q,o=this._getItemIndex(e),r=this._items.length-1;if((i&&0===o||n&&o===r)&&!this._config.wrap)return e;var s=(o+(t===Q?-1:1))%this._items.length;return-1==s?this._items[this._items.length-1]:this._items[s]},t._triggerSlideEvent=function(t,e){var n=this._getItemIndex(t),i=this._getItemIndex(this._element.querySelector(it)),o=g.Event(Y.SLIDE,{relatedTarget:t,direction:e,from:i,to:n});return g(this._element).trigger(o),o},t._setActiveIndicatorElement=function(t){if(this._indicatorsElement){var e=[].slice.call(this._indicatorsElement.querySelectorAll(nt));g(e).removeClass(X);var n=this._indicatorsElement.children[this._getItemIndex(t)];n&&g(n).addClass(X)}},t._slide=function(t,e){var n,i,o,r=this,s=this._element.querySelector(it),a=this._getItemIndex(s),l=e||s&&this._getItemByDirection(t,s),c=this._getItemIndex(l),h=Boolean(this._interval);if(o=t===K?(n=J,i=Z,B):(n=G,i=tt,V),l&&g(l).hasClass(X))this._isSliding=!1;else if(!this._triggerSlideEvent(l,o).isDefaultPrevented()&&s&&l){this._isSliding=!0,h&&this.pause(),this._setActiveIndicatorElement(l);var u=g.Event(Y.SLID,{relatedTarget:l,direction:o,from:a,to:c});if(g(this._element).hasClass($)){g(l).addClass(i),_.reflow(l),g(s).addClass(n),g(l).addClass(n);var f=parseInt(l.getAttribute("data-interval"),10);f?(this._config.defaultInterval=this._config.defaultInterval||this._config.interval,this._config.interval=f):this._config.interval=this._config.defaultInterval||this._config.interval;var d=_.getTransitionDurationFromElement(s);g(s).one(_.TRANSITION_END,function(){g(l).removeClass(n+" "+i).addClass(X),g(s).removeClass(X+" "+i+" "+n),r._isSliding=!1,setTimeout(function(){return g(r._element).trigger(u)},0)}).emulateTransitionEnd(d)}else g(s).removeClass(X),g(l).addClass(X),this._isSliding=!1,g(this._element).trigger(u);h&&this.cycle()}},r._jQueryInterface=function(i){return this.each(function(){var t=g(this).data(x),e=l({},q,{},g(this).data());"object"==typeof i&&(e=l({},e,{},i));var n="string"==typeof i?i:e.slide;if(t||(t=new r(this,e),g(this).data(x,t)),"number"==typeof i)t.to(i);else if("string"==typeof n){if("undefined"==typeof t[n])throw new TypeError('No method named "'+n+'"');t[n]()}else e.interval&&e.ride&&(t.pause(),t.cycle())})},r._dataApiClickHandler=function(t){var e=_.getSelectorFromElement(this);if(e){var n=g(e)[0];if(n&&g(n).hasClass(z)){var i=l({},g(n).data(),{},g(this).data()),o=this.getAttribute("data-slide-to");o&&(i.interval=!1),r._jQueryInterface.call(g(n),i),o&&g(n).data(x).to(o),t.preventDefault()}}},s(r,null,[{key:"VERSION",get:function(){return"4.4.1"}},{key:"Default",get:function(){return q}}]),r}();g(document).on(Y.CLICK_DATA_API,lt,ut._dataApiClickHandler),g(window).on(Y.LOAD_DATA_API,function(){for(var t=[].slice.call(document.querySelectorAll(ct)),e=0,n=t.length;e<n;e++){var i=g(t[e]);ut._jQueryInterface.call(i,i.data())}}),g.fn[R]=ut._jQueryInterface,g.fn[R].Constructor=ut,g.fn[R].noConflict=function(){return g.fn[R]=W,ut._jQueryInterface};var ft="collapse",dt="bs.collapse",gt="."+dt,_t=g.fn[ft],mt={toggle:!0,parent:""},pt={toggle:"boolean",parent:"(string|element)"},vt={SHOW:"show"+gt,SHOWN:"shown"+gt,HIDE:"hide"+gt,HIDDEN:"hidden"+gt,CLICK_DATA_API:"click"+gt+".data-api"},yt="show",Et="collapse",Ct="collapsing",Tt="collapsed",bt="width",St="height",Dt=".show, .collapsing",It='[data-toggle="collapse"]',wt=function(){function a(e,t){this._isTransitioning=!1,this._element=e,this._config=this._getConfig(t),this._triggerArray=[].slice.call(document.querySelectorAll('[data-toggle="collapse"][href="#'+e.id+'"],[data-toggle="collapse"][data-target="#'+e.id+'"]'));for(var n=[].slice.call(document.querySelectorAll(It)),i=0,o=n.length;i<o;i++){var r=n[i],s=_.getSelectorFromElement(r),a=[].slice.call(document.querySelectorAll(s)).filter(function(t){return t===e});null!==s&&0<a.length&&(this._selector=s,this._triggerArray.push(r))}this._parent=this._config.parent?this._getParent():null,this._config.parent||this._addAriaAndCollapsedClass(this._element,this._triggerArray),this._config.toggle&&this.toggle()}var t=a.prototype;return t.toggle=function(){g(this._element).hasClass(yt)?this.hide():this.show()},t.show=function(){var t,e,n=this;if(!this._isTransitioning&&!g(this._element).hasClass(yt)&&(this._parent&&0===(t=[].slice.call(this._parent.querySelectorAll(Dt)).filter(function(t){return"string"==typeof n._config.parent?t.getAttribute("data-parent")===n._config.parent:t.classList.contains(Et)})).length&&(t=null),!(t&&(e=g(t).not(this._selector).data(dt))&&e._isTransitioning))){var i=g.Event(vt.SHOW);if(g(this._element).trigger(i),!i.isDefaultPrevented()){t&&(a._jQueryInterface.call(g(t).not(this._selector),"hide"),e||g(t).data(dt,null));var o=this._getDimension();g(this._element).removeClass(Et).addClass(Ct),this._element.style[o]=0,this._triggerArray.length&&g(this._triggerArray).removeClass(Tt).attr("aria-expanded",!0),this.setTransitioning(!0);var r="scroll"+(o[0].toUpperCase()+o.slice(1)),s=_.getTransitionDurationFromElement(this._element);g(this._element).one(_.TRANSITION_END,function(){g(n._element).removeClass(Ct).addClass(Et).addClass(yt),n._element.style[o]="",n.setTransitioning(!1),g(n._element).trigger(vt.SHOWN)}).emulateTransitionEnd(s),this._element.style[o]=this._element[r]+"px"}}},t.hide=function(){var t=this;if(!this._isTransitioning&&g(this._element).hasClass(yt)){var e=g.Event(vt.HIDE);if(g(this._element).trigger(e),!e.isDefaultPrevented()){var n=this._getDimension();this._element.style[n]=this._element.getBoundingClientRect()[n]+"px",_.reflow(this._element),g(this._element).addClass(Ct).removeClass(Et).removeClass(yt);var i=this._triggerArray.length;if(0<i)for(var o=0;o<i;o++){var r=this._triggerArray[o],s=_.getSelectorFromElement(r);if(null!==s)g([].slice.call(document.querySelectorAll(s))).hasClass(yt)||g(r).addClass(Tt).attr("aria-expanded",!1)}this.setTransitioning(!0);this._element.style[n]="";var a=_.getTransitionDurationFromElement(this._element);g(this._element).one(_.TRANSITION_END,function(){t.setTransitioning(!1),g(t._element).removeClass(Ct).addClass(Et).trigger(vt.HIDDEN)}).emulateTransitionEnd(a)}}},t.setTransitioning=function(t){this._isTransitioning=t},t.dispose=function(){g.removeData(this._element,dt),this._config=null,this._parent=null,this._element=null,this._triggerArray=null,this._isTransitioning=null},t._getConfig=function(t){return(t=l({},mt,{},t)).toggle=Boolean(t.toggle),_.typeCheckConfig(ft,t,pt),t},t._getDimension=function(){return g(this._element).hasClass(bt)?bt:St},t._getParent=function(){var t,n=this;_.isElement(this._config.parent)?(t=this._config.parent,"undefined"!=typeof this._config.parent.jquery&&(t=this._config.parent[0])):t=document.querySelector(this._config.parent);var e='[data-toggle="collapse"][data-parent="'+this._config.parent+'"]',i=[].slice.call(t.querySelectorAll(e));return g(i).each(function(t,e){n._addAriaAndCollapsedClass(a._getTargetFromElement(e),[e])}),t},t._addAriaAndCollapsedClass=function(t,e){var n=g(t).hasClass(yt);e.length&&g(e).toggleClass(Tt,!n).attr("aria-expanded",n)},a._getTargetFromElement=function(t){var e=_.getSelectorFromElement(t);return e?document.querySelector(e):null},a._jQueryInterface=function(i){return this.each(function(){var t=g(this),e=t.data(dt),n=l({},mt,{},t.data(),{},"object"==typeof i&&i?i:{});if(!e&&n.toggle&&/show|hide/.test(i)&&(n.toggle=!1),e||(e=new a(this,n),t.data(dt,e)),"string"==typeof i){if("undefined"==typeof e[i])throw new TypeError('No method named "'+i+'"');e[i]()}})},s(a,null,[{key:"VERSION",get:function(){return"4.4.1"}},{key:"Default",get:function(){return mt}}]),a}();g(document).on(vt.CLICK_DATA_API,It,function(t){"A"===t.currentTarget.tagName&&t.preventDefault();var n=g(this),e=_.getSelectorFromElement(this),i=[].slice.call(document.querySelectorAll(e));g(i).each(function(){var t=g(this),e=t.data(dt)?"toggle":n.data();wt._jQueryInterface.call(t,e)})}),g.fn[ft]=wt._jQueryInterface,g.fn[ft].Constructor=wt,g.fn[ft].noConflict=function(){return g.fn[ft]=_t,wt._jQueryInterface};var At="dropdown",Nt="bs.dropdown",Ot="."+Nt,kt=".data-api",Pt=g.fn[At],Lt=new RegExp("38|40|27"),jt={HIDE:"hide"+Ot,HIDDEN:"hidden"+Ot,SHOW:"show"+Ot,SHOWN:"shown"+Ot,CLICK:"click"+Ot,CLICK_DATA_API:"click"+Ot+kt,KEYDOWN_DATA_API:"keydown"+Ot+kt,KEYUP_DATA_API:"keyup"+Ot+kt},Ht="disabled",Rt="show",xt="dropup",Ft="dropright",Ut="dropleft",Wt="dropdown-menu-right",qt="position-static",Mt='[data-toggle="dropdown"]',Kt=".dropdown form",Qt=".dropdown-menu",Bt=".navbar-nav",Vt=".dropdown-menu .dropdown-item:not(.disabled):not(:disabled)",Yt="top-start",zt="top-end",Xt="bottom-start",$t="bottom-end",Gt="right-start",Jt="left-start",Zt={offset:0,flip:!0,boundary:"scrollParent",reference:"toggle",display:"dynamic",popperConfig:null},te={offset:"(number|string|function)",flip:"boolean",boundary:"(string|element)",reference:"(string|element)",display:"string",popperConfig:"(null|object)"},ee=function(){function c(t,e){this._element=t,this._popper=null,this._config=this._getConfig(e),this._menu=this._getMenuElement(),this._inNavbar=this._detectNavbar(),this._addEventListeners()}var t=c.prototype;return t.toggle=function(){if(!this._element.disabled&&!g(this._element).hasClass(Ht)){var t=g(this._menu).hasClass(Rt);c._clearMenus(),t||this.show(!0)}},t.show=function(t){if(void 0===t&&(t=!1),!(this._element.disabled||g(this._element).hasClass(Ht)||g(this._menu).hasClass(Rt))){var e={relatedTarget:this._element},n=g.Event(jt.SHOW,e),i=c._getParentFromElement(this._element);if(g(i).trigger(n),!n.isDefaultPrevented()){if(!this._inNavbar&&t){if("undefined"==typeof u)throw new TypeError("Bootstrap's dropdowns require Popper.js (https://popper.js.org/)");var o=this._element;"parent"===this._config.reference?o=i:_.isElement(this._config.reference)&&(o=this._config.reference,"undefined"!=typeof this._config.reference.jquery&&(o=this._config.reference[0])),"scrollParent"!==this._config.boundary&&g(i).addClass(qt),this._popper=new u(o,this._menu,this._getPopperConfig())}"ontouchstart"in document.documentElement&&0===g(i).closest(Bt).length&&g(document.body).children().on("mouseover",null,g.noop),this._element.focus(),this._element.setAttribute("aria-expanded",!0),g(this._menu).toggleClass(Rt),g(i).toggleClass(Rt).trigger(g.Event(jt.SHOWN,e))}}},t.hide=function(){if(!this._element.disabled&&!g(this._element).hasClass(Ht)&&g(this._menu).hasClass(Rt)){var t={relatedTarget:this._element},e=g.Event(jt.HIDE,t),n=c._getParentFromElement(this._element);g(n).trigger(e),e.isDefaultPrevented()||(this._popper&&this._popper.destroy(),g(this._menu).toggleClass(Rt),g(n).toggleClass(Rt).trigger(g.Event(jt.HIDDEN,t)))}},t.dispose=function(){g.removeData(this._element,Nt),g(this._element).off(Ot),this._element=null,(this._menu=null)!==this._popper&&(this._popper.destroy(),this._popper=null)},t.update=function(){this._inNavbar=this._detectNavbar(),null!==this._popper&&this._popper.scheduleUpdate()},t._addEventListeners=function(){var e=this;g(this._element).on(jt.CLICK,function(t){t.preventDefault(),t.stopPropagation(),e.toggle()})},t._getConfig=function(t){return t=l({},this.constructor.Default,{},g(this._element).data(),{},t),_.typeCheckConfig(At,t,this.constructor.DefaultType),t},t._getMenuElement=function(){if(!this._menu){var t=c._getParentFromElement(this._element);t&&(this._menu=t.querySelector(Qt))}return this._menu},t._getPlacement=function(){var t=g(this._element.parentNode),e=Xt;return t.hasClass(xt)?(e=Yt,g(this._menu).hasClass(Wt)&&(e=zt)):t.hasClass(Ft)?e=Gt:t.hasClass(Ut)?e=Jt:g(this._menu).hasClass(Wt)&&(e=$t),e},t._detectNavbar=function(){return 0<g(this._element).closest(".navbar").length},t._getOffset=function(){var e=this,t={};return"function"==typeof this._config.offset?t.fn=function(t){return t.offsets=l({},t.offsets,{},e._config.offset(t.offsets,e._element)||{}),t}:t.offset=this._config.offset,t},t._getPopperConfig=function(){var t={placement:this._getPlacement(),modifiers:{offset:this._getOffset(),flip:{enabled:this._config.flip},preventOverflow:{boundariesElement:this._config.boundary}}};return"static"===this._config.display&&(t.modifiers.applyStyle={enabled:!1}),l({},t,{},this._config.popperConfig)},c._jQueryInterface=function(e){return this.each(function(){var t=g(this).data(Nt);if(t||(t=new c(this,"object"==typeof e?e:null),g(this).data(Nt,t)),"string"==typeof e){if("undefined"==typeof t[e])throw new TypeError('No method named "'+e+'"');t[e]()}})},c._clearMenus=function(t){if(!t||3!==t.which&&("keyup"!==t.type||9===t.which))for(var e=[].slice.call(document.querySelectorAll(Mt)),n=0,i=e.length;n<i;n++){var o=c._getParentFromElement(e[n]),r=g(e[n]).data(Nt),s={relatedTarget:e[n]};if(t&&"click"===t.type&&(s.clickEvent=t),r){var a=r._menu;if(g(o).hasClass(Rt)&&!(t&&("click"===t.type&&/input|textarea/i.test(t.target.tagName)||"keyup"===t.type&&9===t.which)&&g.contains(o,t.target))){var l=g.Event(jt.HIDE,s);g(o).trigger(l),l.isDefaultPrevented()||("ontouchstart"in document.documentElement&&g(document.body).children().off("mouseover",null,g.noop),e[n].setAttribute("aria-expanded","false"),r._popper&&r._popper.destroy(),g(a).removeClass(Rt),g(o).removeClass(Rt).trigger(g.Event(jt.HIDDEN,s)))}}}},c._getParentFromElement=function(t){var e,n=_.getSelectorFromElement(t);return n&&(e=document.querySelector(n)),e||t.parentNode},c._dataApiKeydownHandler=function(t){if((/input|textarea/i.test(t.target.tagName)?!(32===t.which||27!==t.which&&(40!==t.which&&38!==t.which||g(t.target).closest(Qt).length)):Lt.test(t.which))&&(t.preventDefault(),t.stopPropagation(),!this.disabled&&!g(this).hasClass(Ht))){var e=c._getParentFromElement(this),n=g(e).hasClass(Rt);if(n||27!==t.which)if(n&&(!n||27!==t.which&&32!==t.which)){var i=[].slice.call(e.querySelectorAll(Vt)).filter(function(t){return g(t).is(":visible")});if(0!==i.length){var o=i.indexOf(t.target);38===t.which&&0<o&&o--,40===t.which&&o<i.length-1&&o++,o<0&&(o=0),i[o].focus()}}else{if(27===t.which){var r=e.querySelector(Mt);g(r).trigger("focus")}g(this).trigger("click")}}},s(c,null,[{key:"VERSION",get:function(){return"4.4.1"}},{key:"Default",get:function(){return Zt}},{key:"DefaultType",get:function(){return te}}]),c}();g(document).on(jt.KEYDOWN_DATA_API,Mt,ee._dataApiKeydownHandler).on(jt.KEYDOWN_DATA_API,Qt,ee._dataApiKeydownHandler).on(jt.CLICK_DATA_API+" "+jt.KEYUP_DATA_API,ee._clearMenus).on(jt.CLICK_DATA_API,Mt,function(t){t.preventDefault(),t.stopPropagation(),ee._jQueryInterface.call(g(this),"toggle")}).on(jt.CLICK_DATA_API,Kt,function(t){t.stopPropagation()}),g.fn[At]=ee._jQueryInterface,g.fn[At].Constructor=ee,g.fn[At].noConflict=function(){return g.fn[At]=Pt,ee._jQueryInterface};var ne="modal",ie="bs.modal",oe="."+ie,re=g.fn[ne],se={backdrop:!0,keyboard:!0,focus:!0,show:!0},ae={backdrop:"(boolean|string)",keyboard:"boolean",focus:"boolean",show:"boolean"},le={HIDE:"hide"+oe,HIDE_PREVENTED:"hidePrevented"+oe,HIDDEN:"hidden"+oe,SHOW:"show"+oe,SHOWN:"shown"+oe,FOCUSIN:"focusin"+oe,RESIZE:"resize"+oe,CLICK_DISMISS:"click.dismiss"+oe,KEYDOWN_DISMISS:"keydown.dismiss"+oe,MOUSEUP_DISMISS:"mouseup.dismiss"+oe,MOUSEDOWN_DISMISS:"mousedown.dismiss"+oe,CLICK_DATA_API:"click"+oe+".data-api"},ce="modal-dialog-scrollable",he="modal-scrollbar-measure",ue="modal-backdrop",fe="modal-open",de="fade",ge="show",_e="modal-static",me=".modal-dialog",pe=".modal-body",ve='[data-toggle="modal"]',ye='[data-dismiss="modal"]',Ee=".fixed-top, .fixed-bottom, .is-fixed, .sticky-top",Ce=".sticky-top",Te=function(){function o(t,e){this._config=this._getConfig(e),this._element=t,this._dialog=t.querySelector(me),this._backdrop=null,this._isShown=!1,this._isBodyOverflowing=!1,this._ignoreBackdropClick=!1,this._isTransitioning=!1,this._scrollbarWidth=0}var t=o.prototype;return t.toggle=function(t){return this._isShown?this.hide():this.show(t)},t.show=function(t){var e=this;if(!this._isShown&&!this._isTransitioning){g(this._element).hasClass(de)&&(this._isTransitioning=!0);var n=g.Event(le.SHOW,{relatedTarget:t});g(this._element).trigger(n),this._isShown||n.isDefaultPrevented()||(this._isShown=!0,this._checkScrollbar(),this._setScrollbar(),this._adjustDialog(),this._setEscapeEvent(),this._setResizeEvent(),g(this._element).on(le.CLICK_DISMISS,ye,function(t){return e.hide(t)}),g(this._dialog).on(le.MOUSEDOWN_DISMISS,function(){g(e._element).one(le.MOUSEUP_DISMISS,function(t){g(t.target).is(e._element)&&(e._ignoreBackdropClick=!0)})}),this._showBackdrop(function(){return e._showElement(t)}))}},t.hide=function(t){var e=this;if(t&&t.preventDefault(),this._isShown&&!this._isTransitioning){var n=g.Event(le.HIDE);if(g(this._element).trigger(n),this._isShown&&!n.isDefaultPrevented()){this._isShown=!1;var i=g(this._element).hasClass(de);if(i&&(this._isTransitioning=!0),this._setEscapeEvent(),this._setResizeEvent(),g(document).off(le.FOCUSIN),g(this._element).removeClass(ge),g(this._element).off(le.CLICK_DISMISS),g(this._dialog).off(le.MOUSEDOWN_DISMISS),i){var o=_.getTransitionDurationFromElement(this._element);g(this._element).one(_.TRANSITION_END,function(t){return e._hideModal(t)}).emulateTransitionEnd(o)}else this._hideModal()}}},t.dispose=function(){[window,this._element,this._dialog].forEach(function(t){return g(t).off(oe)}),g(document).off(le.FOCUSIN),g.removeData(this._element,ie),this._config=null,this._element=null,this._dialog=null,this._backdrop=null,this._isShown=null,this._isBodyOverflowing=null,this._ignoreBackdropClick=null,this._isTransitioning=null,this._scrollbarWidth=null},t.handleUpdate=function(){this._adjustDialog()},t._getConfig=function(t){return t=l({},se,{},t),_.typeCheckConfig(ne,t,ae),t},t._triggerBackdropTransition=function(){var t=this;if("static"===this._config.backdrop){var e=g.Event(le.HIDE_PREVENTED);if(g(this._element).trigger(e),e.defaultPrevented)return;this._element.classList.add(_e);var n=_.getTransitionDurationFromElement(this._element);g(this._element).one(_.TRANSITION_END,function(){t._element.classList.remove(_e)}).emulateTransitionEnd(n),this._element.focus()}else this.hide()},t._showElement=function(t){var e=this,n=g(this._element).hasClass(de),i=this._dialog?this._dialog.querySelector(pe):null;this._element.parentNode&&this._element.parentNode.nodeType===Node.ELEMENT_NODE||document.body.appendChild(this._element),this._element.style.display="block",this._element.removeAttribute("aria-hidden"),this._element.setAttribute("aria-modal",!0),g(this._dialog).hasClass(ce)&&i?i.scrollTop=0:this._element.scrollTop=0,n&&_.reflow(this._element),g(this._element).addClass(ge),this._config.focus&&this._enforceFocus();function o(){e._config.focus&&e._element.focus(),e._isTransitioning=!1,g(e._element).trigger(r)}var r=g.Event(le.SHOWN,{relatedTarget:t});if(n){var s=_.getTransitionDurationFromElement(this._dialog);g(this._dialog).one(_.TRANSITION_END,o).emulateTransitionEnd(s)}else o()},t._enforceFocus=function(){var e=this;g(document).off(le.FOCUSIN).on(le.FOCUSIN,function(t){document!==t.target&&e._element!==t.target&&0===g(e._element).has(t.target).length&&e._element.focus()})},t._setEscapeEvent=function(){var e=this;this._isShown&&this._config.keyboard?g(this._element).on(le.KEYDOWN_DISMISS,function(t){27===t.which&&e._triggerBackdropTransition()}):this._isShown||g(this._element).off(le.KEYDOWN_DISMISS)},t._setResizeEvent=function(){var e=this;this._isShown?g(window).on(le.RESIZE,function(t){return e.handleUpdate(t)}):g(window).off(le.RESIZE)},t._hideModal=function(){var t=this;this._element.style.display="none",this._element.setAttribute("aria-hidden",!0),this._element.removeAttribute("aria-modal"),this._isTransitioning=!1,this._showBackdrop(function(){g(document.body).removeClass(fe),t._resetAdjustments(),t._resetScrollbar(),g(t._element).trigger(le.HIDDEN)})},t._removeBackdrop=function(){this._backdrop&&(g(this._backdrop).remove(),this._backdrop=null)},t._showBackdrop=function(t){var e=this,n=g(this._element).hasClass(de)?de:"";if(this._isShown&&this._config.backdrop){if(this._backdrop=document.createElement("div"),this._backdrop.className=ue,n&&this._backdrop.classList.add(n),g(this._backdrop).appendTo(document.body),g(this._element).on(le.CLICK_DISMISS,function(t){e._ignoreBackdropClick?e._ignoreBackdropClick=!1:t.target===t.currentTarget&&e._triggerBackdropTransition()}),n&&_.reflow(this._backdrop),g(this._backdrop).addClass(ge),!t)return;if(!n)return void t();var i=_.getTransitionDurationFromElement(this._backdrop);g(this._backdrop).one(_.TRANSITION_END,t).emulateTransitionEnd(i)}else if(!this._isShown&&this._backdrop){g(this._backdrop).removeClass(ge);var o=function(){e._removeBackdrop(),t&&t()};if(g(this._element).hasClass(de)){var r=_.getTransitionDurationFromElement(this._backdrop);g(this._backdrop).one(_.TRANSITION_END,o).emulateTransitionEnd(r)}else o()}else t&&t()},t._adjustDialog=function(){var t=this._element.scrollHeight>document.documentElement.clientHeight;!this._isBodyOverflowing&&t&&(this._element.style.paddingLeft=this._scrollbarWidth+"px"),this._isBodyOverflowing&&!t&&(this._element.style.paddingRight=this._scrollbarWidth+"px")},t._resetAdjustments=function(){this._element.style.paddingLeft="",this._element.style.paddingRight=""},t._checkScrollbar=function(){var t=document.body.getBoundingClientRect();this._isBodyOverflowing=t.left+t.right<window.innerWidth,this._scrollbarWidth=this._getScrollbarWidth()},t._setScrollbar=function(){var o=this;if(this._isBodyOverflowing){var t=[].slice.call(document.querySelectorAll(Ee)),e=[].slice.call(document.querySelectorAll(Ce));g(t).each(function(t,e){var n=e.style.paddingRight,i=g(e).css("padding-right");g(e).data("padding-right",n).css("padding-right",parseFloat(i)+o._scrollbarWidth+"px")}),g(e).each(function(t,e){var n=e.style.marginRight,i=g(e).css("margin-right");g(e).data("margin-right",n).css("margin-right",parseFloat(i)-o._scrollbarWidth+"px")});var n=document.body.style.paddingRight,i=g(document.body).css("padding-right");g(document.body).data("padding-right",n).css("padding-right",parseFloat(i)+this._scrollbarWidth+"px")}g(document.body).addClass(fe)},t._resetScrollbar=function(){var t=[].slice.call(document.querySelectorAll(Ee));g(t).each(function(t,e){var n=g(e).data("padding-right");g(e).removeData("padding-right"),e.style.paddingRight=n||""});var e=[].slice.call(document.querySelectorAll(""+Ce));g(e).each(function(t,e){var n=g(e).data("margin-right");"undefined"!=typeof n&&g(e).css("margin-right",n).removeData("margin-right")});var n=g(document.body).data("padding-right");g(document.body).removeData("padding-right"),document.body.style.paddingRight=n||""},t._getScrollbarWidth=function(){var t=document.createElement("div");t.className=he,document.body.appendChild(t);var e=t.getBoundingClientRect().width-t.clientWidth;return document.body.removeChild(t),e},o._jQueryInterface=function(n,i){return this.each(function(){var t=g(this).data(ie),e=l({},se,{},g(this).data(),{},"object"==typeof n&&n?n:{});if(t||(t=new o(this,e),g(this).data(ie,t)),"string"==typeof n){if("undefined"==typeof t[n])throw new TypeError('No method named "'+n+'"');t[n](i)}else e.show&&t.show(i)})},s(o,null,[{key:"VERSION",get:function(){return"4.4.1"}},{key:"Default",get:function(){return se}}]),o}();g(document).on(le.CLICK_DATA_API,ve,function(t){var e,n=this,i=_.getSelectorFromElement(this);i&&(e=document.querySelector(i));var o=g(e).data(ie)?"toggle":l({},g(e).data(),{},g(this).data());"A"!==this.tagName&&"AREA"!==this.tagName||t.preventDefault();var r=g(e).one(le.SHOW,function(t){t.isDefaultPrevented()||r.one(le.HIDDEN,function(){g(n).is(":visible")&&n.focus()})});Te._jQueryInterface.call(g(e),o,this)}),g.fn[ne]=Te._jQueryInterface,g.fn[ne].Constructor=Te,g.fn[ne].noConflict=function(){return g.fn[ne]=re,Te._jQueryInterface};var be=["background","cite","href","itemtype","longdesc","poster","src","xlink:href"],Se={"*":["class","dir","id","lang","role",/^aria-[\w-]*$/i],a:["target","href","title","rel"],area:[],b:[],br:[],col:[],code:[],div:[],em:[],hr:[],h1:[],h2:[],h3:[],h4:[],h5:[],h6:[],i:[],img:["src","alt","title","width","height"],li:[],ol:[],p:[],pre:[],s:[],small:[],span:[],sub:[],sup:[],strong:[],u:[],ul:[]},De=/^(?:(?:https?|mailto|ftp|tel|file):|[^&:/?#]*(?:[/?#]|$))/gi,Ie=/^data:(?:image\/(?:bmp|gif|jpeg|jpg|png|tiff|webp)|video\/(?:mpeg|mp4|ogg|webm)|audio\/(?:mp3|oga|ogg|opus));base64,[a-z0-9+/]+=*$/i;function we(t,r,e){if(0===t.length)return t;if(e&&"function"==typeof e)return e(t);for(var n=(new window.DOMParser).parseFromString(t,"text/html"),s=Object.keys(r),a=[].slice.call(n.body.querySelectorAll("*")),i=function(t){var e=a[t],n=e.nodeName.toLowerCase();if(-1===s.indexOf(e.nodeName.toLowerCase()))return e.parentNode.removeChild(e),"continue";var i=[].slice.call(e.attributes),o=[].concat(r["*"]||[],r[n]||[]);i.forEach(function(t){!function(t,e){var n=t.nodeName.toLowerCase();if(-1!==e.indexOf(n))return-1===be.indexOf(n)||Boolean(t.nodeValue.match(De)||t.nodeValue.match(Ie));for(var i=e.filter(function(t){return t instanceof RegExp}),o=0,r=i.length;o<r;o++)if(n.match(i[o]))return!0;return!1}(t,o)&&e.removeAttribute(t.nodeName)})},o=0,l=a.length;o<l;o++)i(o);return n.body.innerHTML}var Ae="tooltip",Ne="bs.tooltip",Oe="."+Ne,ke=g.fn[Ae],Pe="bs-tooltip",Le=new RegExp("(^|\\s)"+Pe+"\\S+","g"),je=["sanitize","whiteList","sanitizeFn"],He={animation:"boolean",template:"string",title:"(string|element|function)",trigger:"string",delay:"(number|object)",html:"boolean",selector:"(string|boolean)",placement:"(string|function)",offset:"(number|string|function)",container:"(string|element|boolean)",fallbackPlacement:"(string|array)",boundary:"(string|element)",sanitize:"boolean",sanitizeFn:"(null|function)",whiteList:"object",popperConfig:"(null|object)"},Re={AUTO:"auto",TOP:"top",RIGHT:"right",BOTTOM:"bottom",LEFT:"left"},xe={animation:!0,template:'<div class="tooltip" role="tooltip"><div class="arrow"></div><div class="tooltip-inner"></div></div>',trigger:"hover focus",title:"",delay:0,html:!1,selector:!1,placement:"top",offset:0,container:!1,fallbackPlacement:"flip",boundary:"scrollParent",sanitize:!0,sanitizeFn:null,whiteList:Se,popperConfig:null},Fe="show",Ue="out",We={HIDE:"hide"+Oe,HIDDEN:"hidden"+Oe,SHOW:"show"+Oe,SHOWN:"shown"+Oe,INSERTED:"inserted"+Oe,CLICK:"click"+Oe,FOCUSIN:"focusin"+Oe,FOCUSOUT:"focusout"+Oe,MOUSEENTER:"mouseenter"+Oe,MOUSELEAVE:"mouseleave"+Oe},qe="fade",Me="show",Ke=".tooltip-inner",Qe=".arrow",Be="hover",Ve="focus",Ye="click",ze="manual",Xe=function(){function i(t,e){if("undefined"==typeof u)throw new TypeError("Bootstrap's tooltips require Popper.js (https://popper.js.org/)");this._isEnabled=!0,this._timeout=0,this._hoverState="",this._activeTrigger={},this._popper=null,this.element=t,this.config=this._getConfig(e),this.tip=null,this._setListeners()}var t=i.prototype;return t.enable=function(){this._isEnabled=!0},t.disable=function(){this._isEnabled=!1},t.toggleEnabled=function(){this._isEnabled=!this._isEnabled},t.toggle=function(t){if(this._isEnabled)if(t){var e=this.constructor.DATA_KEY,n=g(t.currentTarget).data(e);n||(n=new this.constructor(t.currentTarget,this._getDelegateConfig()),g(t.currentTarget).data(e,n)),n._activeTrigger.click=!n._activeTrigger.click,n._isWithActiveTrigger()?n._enter(null,n):n._leave(null,n)}else{if(g(this.getTipElement()).hasClass(Me))return void this._leave(null,this);this._enter(null,this)}},t.dispose=function(){clearTimeout(this._timeout),g.removeData(this.element,this.constructor.DATA_KEY),g(this.element).off(this.constructor.EVENT_KEY),g(this.element).closest(".modal").off("hide.bs.modal",this._hideModalHandler),this.tip&&g(this.tip).remove(),this._isEnabled=null,this._timeout=null,this._hoverState=null,this._activeTrigger=null,this._popper&&this._popper.destroy(),this._popper=null,this.element=null,this.config=null,this.tip=null},t.show=function(){var e=this;if("none"===g(this.element).css("display"))throw new Error("Please use show on visible elements");var t=g.Event(this.constructor.Event.SHOW);if(this.isWithContent()&&this._isEnabled){g(this.element).trigger(t);var n=_.findShadowRoot(this.element),i=g.contains(null!==n?n:this.element.ownerDocument.documentElement,this.element);if(t.isDefaultPrevented()||!i)return;var o=this.getTipElement(),r=_.getUID(this.constructor.NAME);o.setAttribute("id",r),this.element.setAttribute("aria-describedby",r),this.setContent(),this.config.animation&&g(o).addClass(qe);var s="function"==typeof this.config.placement?this.config.placement.call(this,o,this.element):this.config.placement,a=this._getAttachment(s);this.addAttachmentClass(a);var l=this._getContainer();g(o).data(this.constructor.DATA_KEY,this),g.contains(this.element.ownerDocument.documentElement,this.tip)||g(o).appendTo(l),g(this.element).trigger(this.constructor.Event.INSERTED),this._popper=new u(this.element,o,this._getPopperConfig(a)),g(o).addClass(Me),"ontouchstart"in document.documentElement&&g(document.body).children().on("mouseover",null,g.noop);var c=function(){e.config.animation&&e._fixTransition();var t=e._hoverState;e._hoverState=null,g(e.element).trigger(e.constructor.Event.SHOWN),t===Ue&&e._leave(null,e)};if(g(this.tip).hasClass(qe)){var h=_.getTransitionDurationFromElement(this.tip);g(this.tip).one(_.TRANSITION_END,c).emulateTransitionEnd(h)}else c()}},t.hide=function(t){function e(){n._hoverState!==Fe&&i.parentNode&&i.parentNode.removeChild(i),n._cleanTipClass(),n.element.removeAttribute("aria-describedby"),g(n.element).trigger(n.constructor.Event.HIDDEN),null!==n._popper&&n._popper.destroy(),t&&t()}var n=this,i=this.getTipElement(),o=g.Event(this.constructor.Event.HIDE);if(g(this.element).trigger(o),!o.isDefaultPrevented()){if(g(i).removeClass(Me),"ontouchstart"in document.documentElement&&g(document.body).children().off("mouseover",null,g.noop),this._activeTrigger[Ye]=!1,this._activeTrigger[Ve]=!1,this._activeTrigger[Be]=!1,g(this.tip).hasClass(qe)){var r=_.getTransitionDurationFromElement(i);g(i).one(_.TRANSITION_END,e).emulateTransitionEnd(r)}else e();this._hoverState=""}},t.update=function(){null!==this._popper&&this._popper.scheduleUpdate()},t.isWithContent=function(){return Boolean(this.getTitle())},t.addAttachmentClass=function(t){g(this.getTipElement()).addClass(Pe+"-"+t)},t.getTipElement=function(){return this.tip=this.tip||g(this.config.template)[0],this.tip},t.setContent=function(){var t=this.getTipElement();this.setElementContent(g(t.querySelectorAll(Ke)),this.getTitle()),g(t).removeClass(qe+" "+Me)},t.setElementContent=function(t,e){"object"!=typeof e||!e.nodeType&&!e.jquery?this.config.html?(this.config.sanitize&&(e=we(e,this.config.whiteList,this.config.sanitizeFn)),t.html(e)):t.text(e):this.config.html?g(e).parent().is(t)||t.empty().append(e):t.text(g(e).text())},t.getTitle=function(){var t=this.element.getAttribute("data-original-title");return t=t||("function"==typeof this.config.title?this.config.title.call(this.element):this.config.title)},t._getPopperConfig=function(t){var e=this;return l({},{placement:t,modifiers:{offset:this._getOffset(),flip:{behavior:this.config.fallbackPlacement},arrow:{element:Qe},preventOverflow:{boundariesElement:this.config.boundary}},onCreate:function(t){t.originalPlacement!==t.placement&&e._handlePopperPlacementChange(t)},onUpdate:function(t){return e._handlePopperPlacementChange(t)}},{},this.config.popperConfig)},t._getOffset=function(){var e=this,t={};return"function"==typeof this.config.offset?t.fn=function(t){return t.offsets=l({},t.offsets,{},e.config.offset(t.offsets,e.element)||{}),t}:t.offset=this.config.offset,t},t._getContainer=function(){return!1===this.config.container?document.body:_.isElement(this.config.container)?g(this.config.container):g(document).find(this.config.container)},t._getAttachment=function(t){return Re[t.toUpperCase()]},t._setListeners=function(){var i=this;this.config.trigger.split(" ").forEach(function(t){if("click"===t)g(i.element).on(i.constructor.Event.CLICK,i.config.selector,function(t){return i.toggle(t)});else if(t!==ze){var e=t===Be?i.constructor.Event.MOUSEENTER:i.constructor.Event.FOCUSIN,n=t===Be?i.constructor.Event.MOUSELEAVE:i.constructor.Event.FOCUSOUT;g(i.element).on(e,i.config.selector,function(t){return i._enter(t)}).on(n,i.config.selector,function(t){return i._leave(t)})}}),this._hideModalHandler=function(){i.element&&i.hide()},g(this.element).closest(".modal").on("hide.bs.modal",this._hideModalHandler),this.config.selector?this.config=l({},this.config,{trigger:"manual",selector:""}):this._fixTitle()},t._fixTitle=function(){var t=typeof this.element.getAttribute("data-original-title");!this.element.getAttribute("title")&&"string"==t||(this.element.setAttribute("data-original-title",this.element.getAttribute("title")||""),this.element.setAttribute("title",""))},t._enter=function(t,e){var n=this.constructor.DATA_KEY;(e=e||g(t.currentTarget).data(n))||(e=new this.constructor(t.currentTarget,this._getDelegateConfig()),g(t.currentTarget).data(n,e)),t&&(e._activeTrigger["focusin"===t.type?Ve:Be]=!0),g(e.getTipElement()).hasClass(Me)||e._hoverState===Fe?e._hoverState=Fe:(clearTimeout(e._timeout),e._hoverState=Fe,e.config.delay&&e.config.delay.show?e._timeout=setTimeout(function(){e._hoverState===Fe&&e.show()},e.config.delay.show):e.show())},t._leave=function(t,e){var n=this.constructor.DATA_KEY;(e=e||g(t.currentTarget).data(n))||(e=new this.constructor(t.currentTarget,this._getDelegateConfig()),g(t.currentTarget).data(n,e)),t&&(e._activeTrigger["focusout"===t.type?Ve:Be]=!1),e._isWithActiveTrigger()||(clearTimeout(e._timeout),e._hoverState=Ue,e.config.delay&&e.config.delay.hide?e._timeout=setTimeout(function(){e._hoverState===Ue&&e.hide()},e.config.delay.hide):e.hide())},t._isWithActiveTrigger=function(){for(var t in this._activeTrigger)if(this._activeTrigger[t])return!0;return!1},t._getConfig=function(t){var e=g(this.element).data();return Object.keys(e).forEach(function(t){-1!==je.indexOf(t)&&delete e[t]}),"number"==typeof(t=l({},this.constructor.Default,{},e,{},"object"==typeof t&&t?t:{})).delay&&(t.delay={show:t.delay,hide:t.delay}),"number"==typeof t.title&&(t.title=t.title.toString()),"number"==typeof t.content&&(t.content=t.content.toString()),_.typeCheckConfig(Ae,t,this.constructor.DefaultType),t.sanitize&&(t.template=we(t.template,t.whiteList,t.sanitizeFn)),t},t._getDelegateConfig=function(){var t={};if(this.config)for(var e in this.config)this.constructor.Default[e]!==this.config[e]&&(t[e]=this.config[e]);return t},t._cleanTipClass=function(){var t=g(this.getTipElement()),e=t.attr("class").match(Le);null!==e&&e.length&&t.removeClass(e.join(""))},t._handlePopperPlacementChange=function(t){var e=t.instance;this.tip=e.popper,this._cleanTipClass(),this.addAttachmentClass(this._getAttachment(t.placement))},t._fixTransition=function(){var t=this.getTipElement(),e=this.config.animation;null===t.getAttribute("x-placement")&&(g(t).removeClass(qe),this.config.animation=!1,this.hide(),this.show(),this.config.animation=e)},i._jQueryInterface=function(n){return this.each(function(){var t=g(this).data(Ne),e="object"==typeof n&&n;if((t||!/dispose|hide/.test(n))&&(t||(t=new i(this,e),g(this).data(Ne,t)),"string"==typeof n)){if("undefined"==typeof t[n])throw new TypeError('No method named "'+n+'"');t[n]()}})},s(i,null,[{key:"VERSION",get:function(){return"4.4.1"}},{key:"Default",get:function(){return xe}},{key:"NAME",get:function(){return Ae}},{key:"DATA_KEY",get:function(){return Ne}},{key:"Event",get:function(){return We}},{key:"EVENT_KEY",get:function(){return Oe}},{key:"DefaultType",get:function(){return He}}]),i}();g.fn[Ae]=Xe._jQueryInterface,g.fn[Ae].Constructor=Xe,g.fn[Ae].noConflict=function(){return g.fn[Ae]=ke,Xe._jQueryInterface};var $e="popover",Ge="bs.popover",Je="."+Ge,Ze=g.fn[$e],tn="bs-popover",en=new RegExp("(^|\\s)"+tn+"\\S+","g"),nn=l({},Xe.Default,{placement:"right",trigger:"click",content:"",template:'<div class="popover" role="tooltip"><div class="arrow"></div><h3 class="popover-header"></h3><div class="popover-body"></div></div>'}),on=l({},Xe.DefaultType,{content:"(string|element|function)"}),rn="fade",sn="show",an=".popover-header",ln=".popover-body",cn={HIDE:"hide"+Je,HIDDEN:"hidden"+Je,SHOW:"show"+Je,SHOWN:"shown"+Je,INSERTED:"inserted"+Je,CLICK:"click"+Je,FOCUSIN:"focusin"+Je,FOCUSOUT:"focusout"+Je,MOUSEENTER:"mouseenter"+Je,MOUSELEAVE:"mouseleave"+Je},hn=function(t){function i(){return t.apply(this,arguments)||this}!function(t,e){t.prototype=Object.create(e.prototype),(t.prototype.constructor=t).__proto__=e}(i,t);var e=i.prototype;return e.isWithContent=function(){return this.getTitle()||this._getContent()},e.addAttachmentClass=function(t){g(this.getTipElement()).addClass(tn+"-"+t)},e.getTipElement=function(){return this.tip=this.tip||g(this.config.template)[0],this.tip},e.setContent=function(){var t=g(this.getTipElement());this.setElementContent(t.find(an),this.getTitle());var e=this._getContent();"function"==typeof e&&(e=e.call(this.element)),this.setElementContent(t.find(ln),e),t.removeClass(rn+" "+sn)},e._getContent=function(){return this.element.getAttribute("data-content")||this.config.content},e._cleanTipClass=function(){var t=g(this.getTipElement()),e=t.attr("class").match(en);null!==e&&0<e.length&&t.removeClass(e.join(""))},i._jQueryInterface=function(n){return this.each(function(){var t=g(this).data(Ge),e="object"==typeof n?n:null;if((t||!/dispose|hide/.test(n))&&(t||(t=new i(this,e),g(this).data(Ge,t)),"string"==typeof n)){if("undefined"==typeof t[n])throw new TypeError('No method named "'+n+'"');t[n]()}})},s(i,null,[{key:"VERSION",get:function(){return"4.4.1"}},{key:"Default",get:function(){return nn}},{key:"NAME",get:function(){return $e}},{key:"DATA_KEY",get:function(){return Ge}},{key:"Event",get:function(){return cn}},{key:"EVENT_KEY",get:function(){return Je}},{key:"DefaultType",get:function(){return on}}]),i}(Xe);g.fn[$e]=hn._jQueryInterface,g.fn[$e].Constructor=hn,g.fn[$e].noConflict=function(){return g.fn[$e]=Ze,hn._jQueryInterface};var un="scrollspy",fn="bs.scrollspy",dn="."+fn,gn=g.fn[un],_n={offset:10,method:"auto",target:""},mn={offset:"number",method:"string",target:"(string|element)"},pn={ACTIVATE:"activate"+dn,SCROLL:"scroll"+dn,LOAD_DATA_API:"load"+dn+".data-api"},vn="dropdown-item",yn="active",En='[data-spy="scroll"]',Cn=".nav, .list-group",Tn=".nav-link",bn=".nav-item",Sn=".list-group-item",Dn=".dropdown",In=".dropdown-item",wn=".dropdown-toggle",An="offset",Nn="position",On=function(){function n(t,e){var n=this;this._element=t,this._scrollElement="BODY"===t.tagName?window:t,this._config=this._getConfig(e),this._selector=this._config.target+" "+Tn+","+this._config.target+" "+Sn+","+this._config.target+" "+In,this._offsets=[],this._targets=[],this._activeTarget=null,this._scrollHeight=0,g(this._scrollElement).on(pn.SCROLL,function(t){return n._process(t)}),this.refresh(),this._process()}var t=n.prototype;return t.refresh=function(){var e=this,t=this._scrollElement===this._scrollElement.window?An:Nn,o="auto"===this._config.method?t:this._config.method,r=o===Nn?this._getScrollTop():0;this._offsets=[],this._targets=[],this._scrollHeight=this._getScrollHeight(),[].slice.call(document.querySelectorAll(this._selector)).map(function(t){var e,n=_.getSelectorFromElement(t);if(n&&(e=document.querySelector(n)),e){var i=e.getBoundingClientRect();if(i.width||i.height)return[g(e)[o]().top+r,n]}return null}).filter(function(t){return t}).sort(function(t,e){return t[0]-e[0]}).forEach(function(t){e._offsets.push(t[0]),e._targets.push(t[1])})},t.dispose=function(){g.removeData(this._element,fn),g(this._scrollElement).off(dn),this._element=null,this._scrollElement=null,this._config=null,this._selector=null,this._offsets=null,this._targets=null,this._activeTarget=null,this._scrollHeight=null},t._getConfig=function(t){if("string"!=typeof(t=l({},_n,{},"object"==typeof t&&t?t:{})).target){var e=g(t.target).attr("id");e||(e=_.getUID(un),g(t.target).attr("id",e)),t.target="#"+e}return _.typeCheckConfig(un,t,mn),t},t._getScrollTop=function(){return this._scrollElement===window?this._scrollElement.pageYOffset:this._scrollElement.scrollTop},t._getScrollHeight=function(){return this._scrollElement.scrollHeight||Math.max(document.body.scrollHeight,document.documentElement.scrollHeight)},t._getOffsetHeight=function(){return this._scrollElement===window?window.innerHeight:this._scrollElement.getBoundingClientRect().height},t._process=function(){var t=this._getScrollTop()+this._config.offset,e=this._getScrollHeight(),n=this._config.offset+e-this._getOffsetHeight();if(this._scrollHeight!==e&&this.refresh(),n<=t){var i=this._targets[this._targets.length-1];this._activeTarget!==i&&this._activate(i)}else{if(this._activeTarget&&t<this._offsets[0]&&0<this._offsets[0])return this._activeTarget=null,void this._clear();for(var o=this._offsets.length;o--;){this._activeTarget!==this._targets[o]&&t>=this._offsets[o]&&("undefined"==typeof this._offsets[o+1]||t<this._offsets[o+1])&&this._activate(this._targets[o])}}},t._activate=function(e){this._activeTarget=e,this._clear();var t=this._selector.split(",").map(function(t){return t+'[data-target="'+e+'"],'+t+'[href="'+e+'"]'}),n=g([].slice.call(document.querySelectorAll(t.join(","))));n.hasClass(vn)?(n.closest(Dn).find(wn).addClass(yn),n.addClass(yn)):(n.addClass(yn),n.parents(Cn).prev(Tn+", "+Sn).addClass(yn),n.parents(Cn).prev(bn).children(Tn).addClass(yn)),g(this._scrollElement).trigger(pn.ACTIVATE,{relatedTarget:e})},t._clear=function(){[].slice.call(document.querySelectorAll(this._selector)).filter(function(t){return t.classList.contains(yn)}).forEach(function(t){return t.classList.remove(yn)})},n._jQueryInterface=function(e){return this.each(function(){var t=g(this).data(fn);if(t||(t=new n(this,"object"==typeof e&&e),g(this).data(fn,t)),"string"==typeof e){if("undefined"==typeof t[e])throw new TypeError('No method named "'+e+'"');t[e]()}})},s(n,null,[{key:"VERSION",get:function(){return"4.4.1"}},{key:"Default",get:function(){return _n}}]),n}();g(window).on(pn.LOAD_DATA_API,function(){for(var t=[].slice.call(document.querySelectorAll(En)),e=t.length;e--;){var n=g(t[e]);On._jQueryInterface.call(n,n.data())}}),g.fn[un]=On._jQueryInterface,g.fn[un].Constructor=On,g.fn[un].noConflict=function(){return g.fn[un]=gn,On._jQueryInterface};var kn="bs.tab",Pn="."+kn,Ln=g.fn.tab,jn={HIDE:"hide"+Pn,HIDDEN:"hidden"+Pn,SHOW:"show"+Pn,SHOWN:"shown"+Pn,CLICK_DATA_API:"click"+Pn+".data-api"},Hn="dropdown-menu",Rn="active",xn="disabled",Fn="fade",Un="show",Wn=".dropdown",qn=".nav, .list-group",Mn=".active",Kn="> li > .active",Qn='[data-toggle="tab"], [data-toggle="pill"], [data-toggle="list"]',Bn=".dropdown-toggle",Vn="> .dropdown-menu .active",Yn=function(){function i(t){this._element=t}var t=i.prototype;return t.show=function(){var n=this;if(!(this._element.parentNode&&this._element.parentNode.nodeType===Node.ELEMENT_NODE&&g(this._element).hasClass(Rn)||g(this._element).hasClass(xn))){var t,i,e=g(this._element).closest(qn)[0],o=_.getSelectorFromElement(this._element);if(e){var r="UL"===e.nodeName||"OL"===e.nodeName?Kn:Mn;i=(i=g.makeArray(g(e).find(r)))[i.length-1]}var s=g.Event(jn.HIDE,{relatedTarget:this._element}),a=g.Event(jn.SHOW,{relatedTarget:i});if(i&&g(i).trigger(s),g(this._element).trigger(a),!a.isDefaultPrevented()&&!s.isDefaultPrevented()){o&&(t=document.querySelector(o)),this._activate(this._element,e);var l=function(){var t=g.Event(jn.HIDDEN,{relatedTarget:n._element}),e=g.Event(jn.SHOWN,{relatedTarget:i});g(i).trigger(t),g(n._element).trigger(e)};t?this._activate(t,t.parentNode,l):l()}}},t.dispose=function(){g.removeData(this._element,kn),this._element=null},t._activate=function(t,e,n){function i(){return o._transitionComplete(t,r,n)}var o=this,r=(!e||"UL"!==e.nodeName&&"OL"!==e.nodeName?g(e).children(Mn):g(e).find(Kn))[0],s=n&&r&&g(r).hasClass(Fn);if(r&&s){var a=_.getTransitionDurationFromElement(r);g(r).removeClass(Un).one(_.TRANSITION_END,i).emulateTransitionEnd(a)}else i()},t._transitionComplete=function(t,e,n){if(e){g(e).removeClass(Rn);var i=g(e.parentNode).find(Vn)[0];i&&g(i).removeClass(Rn),"tab"===e.getAttribute("role")&&e.setAttribute("aria-selected",!1)}if(g(t).addClass(Rn),"tab"===t.getAttribute("role")&&t.setAttribute("aria-selected",!0),_.reflow(t),t.classList.contains(Fn)&&t.classList.add(Un),t.parentNode&&g(t.parentNode).hasClass(Hn)){var o=g(t).closest(Wn)[0];if(o){var r=[].slice.call(o.querySelectorAll(Bn));g(r).addClass(Rn)}t.setAttribute("aria-expanded",!0)}n&&n()},i._jQueryInterface=function(n){return this.each(function(){var t=g(this),e=t.data(kn);if(e||(e=new i(this),t.data(kn,e)),"string"==typeof n){if("undefined"==typeof e[n])throw new TypeError('No method named "'+n+'"');e[n]()}})},s(i,null,[{key:"VERSION",get:function(){return"4.4.1"}}]),i}();g(document).on(jn.CLICK_DATA_API,Qn,function(t){t.preventDefault(),Yn._jQueryInterface.call(g(this),"show")}),g.fn.tab=Yn._jQueryInterface,g.fn.tab.Constructor=Yn,g.fn.tab.noConflict=function(){return g.fn.tab=Ln,Yn._jQueryInterface};var zn="toast",Xn="bs.toast",$n="."+Xn,Gn=g.fn[zn],Jn={CLICK_DISMISS:"click.dismiss"+$n,HIDE:"hide"+$n,HIDDEN:"hidden"+$n,SHOW:"show"+$n,SHOWN:"shown"+$n},Zn="fade",ti="hide",ei="show",ni="showing",ii={animation:"boolean",autohide:"boolean",delay:"number"},oi={animation:!0,autohide:!0,delay:500},ri='[data-dismiss="toast"]',si=function(){function i(t,e){this._element=t,this._config=this._getConfig(e),this._timeout=null,this._setListeners()}var t=i.prototype;return t.show=function(){var t=this,e=g.Event(Jn.SHOW);if(g(this._element).trigger(e),!e.isDefaultPrevented()){this._config.animation&&this._element.classList.add(Zn);var n=function(){t._element.classList.remove(ni),t._element.classList.add(ei),g(t._element).trigger(Jn.SHOWN),t._config.autohide&&(t._timeout=setTimeout(function(){t.hide()},t._config.delay))};if(this._element.classList.remove(ti),_.reflow(this._element),this._element.classList.add(ni),this._config.animation){var i=_.getTransitionDurationFromElement(this._element);g(this._element).one(_.TRANSITION_END,n).emulateTransitionEnd(i)}else n()}},t.hide=function(){if(this._element.classList.contains(ei)){var t=g.Event(Jn.HIDE);g(this._element).trigger(t),t.isDefaultPrevented()||this._close()}},t.dispose=function(){clearTimeout(this._timeout),this._timeout=null,this._element.classList.contains(ei)&&this._element.classList.remove(ei),g(this._element).off(Jn.CLICK_DISMISS),g.removeData(this._element,Xn),this._element=null,this._config=null},t._getConfig=function(t){return t=l({},oi,{},g(this._element).data(),{},"object"==typeof t&&t?t:{}),_.typeCheckConfig(zn,t,this.constructor.DefaultType),t},t._setListeners=function(){var t=this;g(this._element).on(Jn.CLICK_DISMISS,ri,function(){return t.hide()})},t._close=function(){function t(){e._element.classList.add(ti),g(e._element).trigger(Jn.HIDDEN)}var e=this;if(this._element.classList.remove(ei),this._config.animation){var n=_.getTransitionDurationFromElement(this._element);g(this._element).one(_.TRANSITION_END,t).emulateTransitionEnd(n)}else t()},i._jQueryInterface=function(n){return this.each(function(){var t=g(this),e=t.data(Xn);if(e||(e=new i(this,"object"==typeof n&&n),t.data(Xn,e)),"string"==typeof n){if("undefined"==typeof e[n])throw new TypeError('No method named "'+n+'"');e[n](this)}})},s(i,null,[{key:"VERSION",get:function(){return"4.4.1"}},{key:"DefaultType",get:function(){return ii}},{key:"Default",get:function(){return oi}}]),i}();g.fn[zn]=si._jQueryInterface,g.fn[zn].Constructor=si,g.fn[zn].noConflict=function(){return g.fn[zn]=Gn,si._jQueryInterface},t.Alert=v,t.Button=H,t.Carousel=ut,t.Collapse=wt,t.Dropdown=ee,t.Modal=Te,t.Popover=hn,t.Scrollspy=On,t.Tab=Yn,t.Toast=si,t.Tooltip=Xe,t.Util=_,Object.defineProperty(t,"__esModule",{value:!0})});
/* End */
;
; /* Start:"a:4:{s:4:"full";s:79:"/local/templates/.default/markup/new/js/bootstrap-select.min.js?164872532653644";s:6:"source";s:63:"/local/templates/.default/markup/new/js/bootstrap-select.min.js";s:3:"min";s:63:"/local/templates/.default/markup/new/js/bootstrap-select.min.js";s:3:"map";s:67:"/local/templates/.default/markup/new/js/bootstrap-select.min.js.map";}"*/
/*!
 * Bootstrap-select v1.13.18 (https://developer.snapappointments.com/bootstrap-select)
 *
 * Copyright 2012-2020 SnapAppointments, LLC
 * Licensed under MIT (https://github.com/snapappointments/bootstrap-select/blob/master/LICENSE)
 */

!function(e,t){void 0===e&&void 0!==window&&(e=window),"function"==typeof define&&define.amd?define(["jquery"],function(e){return t(e)}):"object"==typeof module&&module.exports?module.exports=t(require("jquery")):t(e.jQuery)}(this,function(e){!function(P){"use strict";var d=["sanitize","whiteList","sanitizeFn"],r=["background","cite","href","itemtype","longdesc","poster","src","xlink:href"],e={"*":["class","dir","id","lang","role","tabindex","style",/^aria-[\w-]*$/i],a:["target","href","title","rel"],area:[],b:[],br:[],col:[],code:[],div:[],em:[],hr:[],h1:[],h2:[],h3:[],h4:[],h5:[],h6:[],i:[],img:["src","alt","title","width","height"],li:[],ol:[],p:[],pre:[],s:[],small:[],span:[],sub:[],sup:[],strong:[],u:[],ul:[]},l=/^(?:(?:https?|mailto|ftp|tel|file):|[^&:/?#]*(?:[/?#]|$))/gi,a=/^data:(?:image\/(?:bmp|gif|jpeg|jpg|png|tiff|webp)|video\/(?:mpeg|mp4|ogg|webm)|audio\/(?:mp3|oga|ogg|opus));base64,[a-z0-9+/]+=*$/i;function v(e,t){var i=e.nodeName.toLowerCase();if(-1!==P.inArray(i,t))return-1===P.inArray(i,r)||Boolean(e.nodeValue.match(l)||e.nodeValue.match(a));for(var s=P(t).filter(function(e,t){return t instanceof RegExp}),n=0,o=s.length;n<o;n++)if(i.match(s[n]))return!0;return!1}function W(e,t,i){if(i&&"function"==typeof i)return i(e);for(var s=Object.keys(t),n=0,o=e.length;n<o;n++)for(var r=e[n].querySelectorAll("*"),l=0,a=r.length;l<a;l++){var c=r[l],d=c.nodeName.toLowerCase();if(-1!==s.indexOf(d))for(var h=[].slice.call(c.attributes),p=[].concat(t["*"]||[],t[d]||[]),u=0,f=h.length;u<f;u++){var m=h[u];v(m,p)||c.removeAttribute(m.nodeName)}else c.parentNode.removeChild(c)}}"classList"in document.createElement("_")||function(e){if("Element"in e){var t="classList",i="prototype",s=e.Element[i],n=Object,o=function(){var i=P(this);return{add:function(e){return e=Array.prototype.slice.call(arguments).join(" "),i.addClass(e)},remove:function(e){return e=Array.prototype.slice.call(arguments).join(" "),i.removeClass(e)},toggle:function(e,t){return i.toggleClass(e,t)},contains:function(e){return i.hasClass(e)}}};if(n.defineProperty){var r={get:o,enumerable:!0,configurable:!0};try{n.defineProperty(s,t,r)}catch(e){void 0!==e.number&&-2146823252!==e.number||(r.enumerable=!1,n.defineProperty(s,t,r))}}else n[i].__defineGetter__&&s.__defineGetter__(t,o)}}(window);var t,c,i=document.createElement("_");if(i.classList.add("c1","c2"),!i.classList.contains("c2")){var s=DOMTokenList.prototype.add,n=DOMTokenList.prototype.remove;DOMTokenList.prototype.add=function(){Array.prototype.forEach.call(arguments,s.bind(this))},DOMTokenList.prototype.remove=function(){Array.prototype.forEach.call(arguments,n.bind(this))}}if(i.classList.toggle("c3",!1),i.classList.contains("c3")){var o=DOMTokenList.prototype.toggle;DOMTokenList.prototype.toggle=function(e,t){return 1 in arguments&&!this.contains(e)==!t?t:o.call(this,e)}}function h(e){if(null==this)throw new TypeError;var t=String(this);if(e&&"[object RegExp]"==c.call(e))throw new TypeError;var i=t.length,s=String(e),n=s.length,o=1<arguments.length?arguments[1]:void 0,r=o?Number(o):0;r!=r&&(r=0);var l=Math.min(Math.max(r,0),i);if(i<n+l)return!1;for(var a=-1;++a<n;)if(t.charCodeAt(l+a)!=s.charCodeAt(a))return!1;return!0}function O(e,t){var i,s=e.selectedOptions,n=[];if(t){for(var o=0,r=s.length;o<r;o++)(i=s[o]).disabled||"OPTGROUP"===i.parentNode.tagName&&i.parentNode.disabled||n.push(i);return n}return s}function z(e,t){for(var i,s=[],n=t||e.selectedOptions,o=0,r=n.length;o<r;o++)(i=n[o]).disabled||"OPTGROUP"===i.parentNode.tagName&&i.parentNode.disabled||s.push(i.value);return e.multiple?s:s.length?s[0]:null}i=null,String.prototype.startsWith||(t=function(){try{var e={},t=Object.defineProperty,i=t(e,e,e)&&t}catch(e){}return i}(),c={}.toString,t?t(String.prototype,"startsWith",{value:h,configurable:!0,writable:!0}):String.prototype.startsWith=h),Object.keys||(Object.keys=function(e,t,i){for(t in i=[],e)i.hasOwnProperty.call(e,t)&&i.push(t);return i}),HTMLSelectElement&&!HTMLSelectElement.prototype.hasOwnProperty("selectedOptions")&&Object.defineProperty(HTMLSelectElement.prototype,"selectedOptions",{get:function(){return this.querySelectorAll(":checked")}});var p={useDefault:!1,_set:P.valHooks.select.set};P.valHooks.select.set=function(e,t){return t&&!p.useDefault&&P(e).data("selected",!0),p._set.apply(this,arguments)};var T=null,u=function(){try{return new Event("change"),!0}catch(e){return!1}}();function k(e,t,i,s){for(var n=["display","subtext","tokens"],o=!1,r=0;r<n.length;r++){var l=n[r],a=e[l];if(a&&(a=a.toString(),"display"===l&&(a=a.replace(/<[^>]+>/g,"")),s&&(a=w(a)),a=a.toUpperCase(),o="contains"===i?0<=a.indexOf(t):a.startsWith(t)))break}return o}function N(e){return parseInt(e,10)||0}P.fn.triggerNative=function(e){var t,i=this[0];i.dispatchEvent?(u?t=new Event(e,{bubbles:!0}):(t=document.createEvent("Event")).initEvent(e,!0,!1),i.dispatchEvent(t)):i.fireEvent?((t=document.createEventObject()).eventType=e,i.fireEvent("on"+e,t)):this.trigger(e)};var f={"\xc0":"A","\xc1":"A","\xc2":"A","\xc3":"A","\xc4":"A","\xc5":"A","\xe0":"a","\xe1":"a","\xe2":"a","\xe3":"a","\xe4":"a","\xe5":"a","\xc7":"C","\xe7":"c","\xd0":"D","\xf0":"d","\xc8":"E","\xc9":"E","\xca":"E","\xcb":"E","\xe8":"e","\xe9":"e","\xea":"e","\xeb":"e","\xcc":"I","\xcd":"I","\xce":"I","\xcf":"I","\xec":"i","\xed":"i","\xee":"i","\xef":"i","\xd1":"N","\xf1":"n","\xd2":"O","\xd3":"O","\xd4":"O","\xd5":"O","\xd6":"O","\xd8":"O","\xf2":"o","\xf3":"o","\xf4":"o","\xf5":"o","\xf6":"o","\xf8":"o","\xd9":"U","\xda":"U","\xdb":"U","\xdc":"U","\xf9":"u","\xfa":"u","\xfb":"u","\xfc":"u","\xdd":"Y","\xfd":"y","\xff":"y","\xc6":"Ae","\xe6":"ae","\xde":"Th","\xfe":"th","\xdf":"ss","\u0100":"A","\u0102":"A","\u0104":"A","\u0101":"a","\u0103":"a","\u0105":"a","\u0106":"C","\u0108":"C","\u010a":"C","\u010c":"C","\u0107":"c","\u0109":"c","\u010b":"c","\u010d":"c","\u010e":"D","\u0110":"D","\u010f":"d","\u0111":"d","\u0112":"E","\u0114":"E","\u0116":"E","\u0118":"E","\u011a":"E","\u0113":"e","\u0115":"e","\u0117":"e","\u0119":"e","\u011b":"e","\u011c":"G","\u011e":"G","\u0120":"G","\u0122":"G","\u011d":"g","\u011f":"g","\u0121":"g","\u0123":"g","\u0124":"H","\u0126":"H","\u0125":"h","\u0127":"h","\u0128":"I","\u012a":"I","\u012c":"I","\u012e":"I","\u0130":"I","\u0129":"i","\u012b":"i","\u012d":"i","\u012f":"i","\u0131":"i","\u0134":"J","\u0135":"j","\u0136":"K","\u0137":"k","\u0138":"k","\u0139":"L","\u013b":"L","\u013d":"L","\u013f":"L","\u0141":"L","\u013a":"l","\u013c":"l","\u013e":"l","\u0140":"l","\u0142":"l","\u0143":"N","\u0145":"N","\u0147":"N","\u014a":"N","\u0144":"n","\u0146":"n","\u0148":"n","\u014b":"n","\u014c":"O","\u014e":"O","\u0150":"O","\u014d":"o","\u014f":"o","\u0151":"o","\u0154":"R","\u0156":"R","\u0158":"R","\u0155":"r","\u0157":"r","\u0159":"r","\u015a":"S","\u015c":"S","\u015e":"S","\u0160":"S","\u015b":"s","\u015d":"s","\u015f":"s","\u0161":"s","\u0162":"T","\u0164":"T","\u0166":"T","\u0163":"t","\u0165":"t","\u0167":"t","\u0168":"U","\u016a":"U","\u016c":"U","\u016e":"U","\u0170":"U","\u0172":"U","\u0169":"u","\u016b":"u","\u016d":"u","\u016f":"u","\u0171":"u","\u0173":"u","\u0174":"W","\u0175":"w","\u0176":"Y","\u0177":"y","\u0178":"Y","\u0179":"Z","\u017b":"Z","\u017d":"Z","\u017a":"z","\u017c":"z","\u017e":"z","\u0132":"IJ","\u0133":"ij","\u0152":"Oe","\u0153":"oe","\u0149":"'n","\u017f":"s"},m=/[\xc0-\xd6\xd8-\xf6\xf8-\xff\u0100-\u017f]/g,g=RegExp("[\\u0300-\\u036f\\ufe20-\\ufe2f\\u20d0-\\u20ff\\u1ab0-\\u1aff\\u1dc0-\\u1dff]","g");function b(e){return f[e]}function w(e){return(e=e.toString())&&e.replace(m,b).replace(g,"")}var I,x,y,$,S=(I={"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#x27;","`":"&#x60;"},x="(?:"+Object.keys(I).join("|")+")",y=RegExp(x),$=RegExp(x,"g"),function(e){return e=null==e?"":""+e,y.test(e)?e.replace($,E):e});function E(e){return I[e]}var C={32:" ",48:"0",49:"1",50:"2",51:"3",52:"4",53:"5",54:"6",55:"7",56:"8",57:"9",59:";",65:"A",66:"B",67:"C",68:"D",69:"E",70:"F",71:"G",72:"H",73:"I",74:"J",75:"K",76:"L",77:"M",78:"N",79:"O",80:"P",81:"Q",82:"R",83:"S",84:"T",85:"U",86:"V",87:"W",88:"X",89:"Y",90:"Z",96:"0",97:"1",98:"2",99:"3",100:"4",101:"5",102:"6",103:"7",104:"8",105:"9"},A=27,L=13,D=32,H=9,B=38,R=40,M={success:!1,major:"3"};try{M.full=(P.fn.dropdown.Constructor.VERSION||"").split(" ")[0].split("."),M.major=M.full[0],M.success=!0}catch(e){}var U=0,j=".bs.select",V={DISABLED:"disabled",DIVIDER:"divider",SHOW:"open",DROPUP:"dropup",MENU:"dropdown-menu",MENURIGHT:"dropdown-menu-right",MENULEFT:"dropdown-menu-left",BUTTONCLASS:"btn-default",POPOVERHEADER:"popover-title",ICONBASE:"glyphicon",TICKICON:"glyphicon-ok"},F={MENU:"."+V.MENU},_={div:document.createElement("div"),span:document.createElement("span"),i:document.createElement("i"),subtext:document.createElement("small"),a:document.createElement("a"),li:document.createElement("li"),whitespace:document.createTextNode("\xa0"),fragment:document.createDocumentFragment()};_.noResults=_.li.cloneNode(!1),_.noResults.className="no-results",_.a.setAttribute("role","option"),_.a.className="dropdown-item",_.subtext.className="text-muted",_.text=_.span.cloneNode(!1),_.text.className="text",_.checkMark=_.span.cloneNode(!1);var G=new RegExp(B+"|"+R),q=new RegExp("^"+H+"$|"+A),K={li:function(e,t,i){var s=_.li.cloneNode(!1);return e&&(1===e.nodeType||11===e.nodeType?s.appendChild(e):s.innerHTML=e),void 0!==t&&""!==t&&(s.className=t),null!=i&&s.classList.add("optgroup-"+i),s},a:function(e,t,i){var s=_.a.cloneNode(!0);return e&&(11===e.nodeType?s.appendChild(e):s.insertAdjacentHTML("beforeend",e)),void 0!==t&&""!==t&&s.classList.add.apply(s.classList,t.split(/\s+/)),i&&s.setAttribute("style",i),s},text:function(e,t){var i,s,n=_.text.cloneNode(!1);if(e.content)n.innerHTML=e.content;else{if(n.textContent=e.text,e.icon){var o=_.whitespace.cloneNode(!1);(s=(!0===t?_.i:_.span).cloneNode(!1)).className=this.options.iconBase+" "+e.icon,_.fragment.appendChild(s),_.fragment.appendChild(o)}e.subtext&&((i=_.subtext.cloneNode(!1)).textContent=e.subtext,n.appendChild(i))}if(!0===t)for(;0<n.childNodes.length;)_.fragment.appendChild(n.childNodes[0]);else _.fragment.appendChild(n);return _.fragment},label:function(e){var t,i,s=_.text.cloneNode(!1);if(s.innerHTML=e.display,e.icon){var n=_.whitespace.cloneNode(!1);(i=_.span.cloneNode(!1)).className=this.options.iconBase+" "+e.icon,_.fragment.appendChild(i),_.fragment.appendChild(n)}return e.subtext&&((t=_.subtext.cloneNode(!1)).textContent=e.subtext,s.appendChild(t)),_.fragment.appendChild(s),_.fragment}};var Y=function(e,t){var i=this;p.useDefault||(P.valHooks.select.set=p._set,p.useDefault=!0),this.$element=P(e),this.$newElement=null,this.$button=null,this.$menu=null,this.options=t,this.selectpicker={main:{},search:{},current:{},view:{},isSearching:!1,keydown:{keyHistory:"",resetKeyHistory:{start:function(){return setTimeout(function(){i.selectpicker.keydown.keyHistory=""},800)}}}},this.sizeInfo={},null===this.options.title&&(this.options.title=this.$element.attr("title"));var s=this.options.windowPadding;"number"==typeof s&&(this.options.windowPadding=[s,s,s,s]),this.val=Y.prototype.val,this.render=Y.prototype.render,this.refresh=Y.prototype.refresh,this.setStyle=Y.prototype.setStyle,this.selectAll=Y.prototype.selectAll,this.deselectAll=Y.prototype.deselectAll,this.destroy=Y.prototype.destroy,this.remove=Y.prototype.remove,this.show=Y.prototype.show,this.hide=Y.prototype.hide,this.init()};function Z(e){var l,a=arguments,c=e;if([].shift.apply(a),!M.success){try{M.full=(P.fn.dropdown.Constructor.VERSION||"").split(" ")[0].split(".")}catch(e){Y.BootstrapVersion?M.full=Y.BootstrapVersion.split(" ")[0].split("."):(M.full=[M.major,"0","0"],console.warn("There was an issue retrieving Bootstrap's version. Ensure Bootstrap is being loaded before bootstrap-select and there is no namespace collision. If loading Bootstrap asynchronously, the version may need to be manually specified via $.fn.selectpicker.Constructor.BootstrapVersion.",e))}M.major=M.full[0],M.success=!0}if("4"===M.major){var t=[];Y.DEFAULTS.style===V.BUTTONCLASS&&t.push({name:"style",className:"BUTTONCLASS"}),Y.DEFAULTS.iconBase===V.ICONBASE&&t.push({name:"iconBase",className:"ICONBASE"}),Y.DEFAULTS.tickIcon===V.TICKICON&&t.push({name:"tickIcon",className:"TICKICON"}),V.DIVIDER="dropdown-divider",V.SHOW="show",V.BUTTONCLASS="btn-light",V.POPOVERHEADER="popover-header",V.ICONBASE="",V.TICKICON="bs-ok-default";for(var i=0;i<t.length;i++){e=t[i];Y.DEFAULTS[e.name]=V[e.className]}}var s=this.each(function(){var e=P(this);if(e.is("select")){var t=e.data("selectpicker"),i="object"==typeof c&&c;if(t){if(i)for(var s in i)Object.prototype.hasOwnProperty.call(i,s)&&(t.options[s]=i[s])}else{var n=e.data();for(var o in n)Object.prototype.hasOwnProperty.call(n,o)&&-1!==P.inArray(o,d)&&delete n[o];var r=P.extend({},Y.DEFAULTS,P.fn.selectpicker.defaults||{},n,i);r.template=P.extend({},Y.DEFAULTS.template,P.fn.selectpicker.defaults?P.fn.selectpicker.defaults.template:{},n.template,i.template),e.data("selectpicker",t=new Y(this,r))}"string"==typeof c&&(l=t[c]instanceof Function?t[c].apply(t,a):t.options[c])}});return void 0!==l?l:s}Y.VERSION="1.13.18",Y.DEFAULTS={noneSelectedText:"Nothing selected",noneResultsText:"No results matched {0}",countSelectedText:function(e,t){return 1==e?"{0} item selected":"{0} items selected"},maxOptionsText:function(e,t){return[1==e?"Limit reached ({n} item max)":"Limit reached ({n} items max)",1==t?"Group limit reached ({n} item max)":"Group limit reached ({n} items max)"]},selectAllText:"Select All",deselectAllText:"Deselect All",doneButton:!1,doneButtonText:"Close",multipleSeparator:", ",styleBase:"btn",style:V.BUTTONCLASS,size:"auto",title:null,selectedTextFormat:"values",width:!1,container:!1,hideDisabled:!1,showSubtext:!1,showIcon:!0,showContent:!0,dropupAuto:!0,header:!1,liveSearch:!1,liveSearchPlaceholder:null,liveSearchNormalize:!1,liveSearchStyle:"contains",actionsBox:!1,iconBase:V.ICONBASE,tickIcon:V.TICKICON,showTick:!1,template:{caret:'<span class="caret"></span>'},maxOptions:!1,mobile:!1,selectOnTab:!1,dropdownAlignRight:!1,windowPadding:0,virtualScroll:600,display:!1,sanitize:!0,sanitizeFn:null,whiteList:e},Y.prototype={constructor:Y,init:function(){var i=this,e=this.$element.attr("id"),t=this.$element[0],s=t.form;U++,this.selectId="bs-select-"+U,t.classList.add("bs-select-hidden"),this.multiple=this.$element.prop("multiple"),this.autofocus=this.$element.prop("autofocus"),t.classList.contains("show-tick")&&(this.options.showTick=!0),this.$newElement=this.createDropdown(),this.buildData(),this.$element.after(this.$newElement).prependTo(this.$newElement),s&&null===t.form&&(s.id||(s.id="form-"+this.selectId),t.setAttribute("form",s.id)),this.$button=this.$newElement.children("button"),this.$menu=this.$newElement.children(F.MENU),this.$menuInner=this.$menu.children(".inner"),this.$searchbox=this.$menu.find("input"),t.classList.remove("bs-select-hidden"),!0===this.options.dropdownAlignRight&&this.$menu[0].classList.add(V.MENURIGHT),void 0!==e&&this.$button.attr("data-id",e),this.checkDisabled(),this.clickListener(),this.options.liveSearch?(this.liveSearchListener(),this.focusedParent=this.$searchbox[0]):this.focusedParent=this.$menuInner[0],this.setStyle(),this.render(),this.setWidth(),this.options.container?this.selectPosition():this.$element.on("hide"+j,function(){if(i.isVirtual()){var e=i.$menuInner[0],t=e.firstChild.cloneNode(!1);e.replaceChild(t,e.firstChild),e.scrollTop=0}}),this.$menu.data("this",this),this.$newElement.data("this",this),this.options.mobile&&this.mobile(),this.$newElement.on({"hide.bs.dropdown":function(e){i.$element.trigger("hide"+j,e)},"hidden.bs.dropdown":function(e){i.$element.trigger("hidden"+j,e)},"show.bs.dropdown":function(e){i.$element.trigger("show"+j,e)},"shown.bs.dropdown":function(e){i.$element.trigger("shown"+j,e)}}),t.hasAttribute("required")&&this.$element.on("invalid"+j,function(){i.$button[0].classList.add("bs-invalid"),i.$element.on("shown"+j+".invalid",function(){i.$element.val(i.$element.val()).off("shown"+j+".invalid")}).on("rendered"+j,function(){this.validity.valid&&i.$button[0].classList.remove("bs-invalid"),i.$element.off("rendered"+j)}),i.$button.on("blur"+j,function(){i.$element.trigger("focus").trigger("blur"),i.$button.off("blur"+j)})}),setTimeout(function(){i.buildList(),i.$element.trigger("loaded"+j)})},createDropdown:function(){var e=this.multiple||this.options.showTick?" show-tick":"",t=this.multiple?' aria-multiselectable="true"':"",i="",s=this.autofocus?" autofocus":"";M.major<4&&this.$element.parent().hasClass("input-group")&&(i=" input-group-btn");var n,o="",r="",l="",a="";return this.options.header&&(o='<div class="'+V.POPOVERHEADER+'"><button type="button" class="close" aria-hidden="true">&times;</button>'+this.options.header+"</div>"),this.options.liveSearch&&(r='<div class="bs-searchbox"><input type="search" class="form-control" autocomplete="off"'+(null===this.options.liveSearchPlaceholder?"":' placeholder="'+S(this.options.liveSearchPlaceholder)+'"')+' role="combobox" aria-label="Search" aria-controls="'+this.selectId+'" aria-autocomplete="list"></div>'),this.multiple&&this.options.actionsBox&&(l='<div class="bs-actionsbox"><div class="btn-group btn-group-sm btn-block"><button type="button" class="actions-btn bs-select-all btn '+V.BUTTONCLASS+'">'+this.options.selectAllText+'</button><button type="button" class="actions-btn bs-deselect-all btn '+V.BUTTONCLASS+'">'+this.options.deselectAllText+"</button></div></div>"),this.multiple&&this.options.doneButton&&(a='<div class="bs-donebutton"><div class="btn-group btn-block"><button type="button" class="btn btn-sm '+V.BUTTONCLASS+'">'+this.options.doneButtonText+"</button></div></div>"),n='<div class="dropdown bootstrap-select'+e+i+'"><button type="button" tabindex="-1" class="'+this.options.styleBase+' dropdown-toggle" '+("static"===this.options.display?'data-display="static"':"")+'data-toggle="dropdown"'+s+' role="combobox" aria-owns="'+this.selectId+'" aria-haspopup="listbox" aria-expanded="false"><div class="filter-option"><div class="filter-option-inner"><div class="filter-option-inner-inner"></div></div> </div>'+("4"===M.major?"":'<span class="bs-caret">'+this.options.template.caret+"</span>")+'</button><div class="'+V.MENU+" "+("4"===M.major?"":V.SHOW)+'">'+o+r+l+'<div class="inner '+V.SHOW+'" role="listbox" id="'+this.selectId+'" tabindex="-1" '+t+'><ul class="'+V.MENU+" inner "+("4"===M.major?V.SHOW:"")+'" role="presentation"></ul></div>'+a+"</div></div>",P(n)},setPositionData:function(){this.selectpicker.view.canHighlight=[],this.selectpicker.view.size=0,this.selectpicker.view.firstHighlightIndex=!1;for(var e=0;e<this.selectpicker.current.data.length;e++){var t=this.selectpicker.current.data[e],i=!0;"divider"===t.type?(i=!1,t.height=this.sizeInfo.dividerHeight):"optgroup-label"===t.type?(i=!1,t.height=this.sizeInfo.dropdownHeaderHeight):t.height=this.sizeInfo.liHeight,t.disabled&&(i=!1),this.selectpicker.view.canHighlight.push(i),i&&(this.selectpicker.view.size++,t.posinset=this.selectpicker.view.size,!1===this.selectpicker.view.firstHighlightIndex&&(this.selectpicker.view.firstHighlightIndex=e)),t.position=(0===e?0:this.selectpicker.current.data[e-1].position)+t.height}},isVirtual:function(){return!1!==this.options.virtualScroll&&this.selectpicker.main.elements.length>=this.options.virtualScroll||!0===this.options.virtualScroll},createView:function(N,e,t){var A,L,D=this,i=0,H=[];if(this.selectpicker.isSearching=N,this.selectpicker.current=N?this.selectpicker.search:this.selectpicker.main,this.setPositionData(),e)if(t)i=this.$menuInner[0].scrollTop;else if(!D.multiple){var s=D.$element[0],n=(s.options[s.selectedIndex]||{}).liIndex;if("number"==typeof n&&!1!==D.options.size){var o=D.selectpicker.main.data[n],r=o&&o.position;r&&(i=r-(D.sizeInfo.menuInnerHeight+D.sizeInfo.liHeight)/2)}}function l(e,t){var i,s,n,o,r,l,a,c,d=D.selectpicker.current.elements.length,h=[],p=!0,u=D.isVirtual();D.selectpicker.view.scrollTop=e,i=Math.ceil(D.sizeInfo.menuInnerHeight/D.sizeInfo.liHeight*1.5),s=Math.round(d/i)||1;for(var f=0;f<s;f++){var m=(f+1)*i;if(f===s-1&&(m=d),h[f]=[f*i+(f?1:0),m],!d)break;void 0===r&&e-1<=D.selectpicker.current.data[m-1].position-D.sizeInfo.menuInnerHeight&&(r=f)}if(void 0===r&&(r=0),l=[D.selectpicker.view.position0,D.selectpicker.view.position1],n=Math.max(0,r-1),o=Math.min(s-1,r+1),D.selectpicker.view.position0=!1===u?0:Math.max(0,h[n][0])||0,D.selectpicker.view.position1=!1===u?d:Math.min(d,h[o][1])||0,a=l[0]!==D.selectpicker.view.position0||l[1]!==D.selectpicker.view.position1,void 0!==D.activeIndex&&(L=D.selectpicker.main.elements[D.prevActiveIndex],H=D.selectpicker.main.elements[D.activeIndex],A=D.selectpicker.main.elements[D.selectedIndex],t&&(D.activeIndex!==D.selectedIndex&&D.defocusItem(H),D.activeIndex=void 0),D.activeIndex&&D.activeIndex!==D.selectedIndex&&D.defocusItem(A)),void 0!==D.prevActiveIndex&&D.prevActiveIndex!==D.activeIndex&&D.prevActiveIndex!==D.selectedIndex&&D.defocusItem(L),(t||a)&&(c=D.selectpicker.view.visibleElements?D.selectpicker.view.visibleElements.slice():[],D.selectpicker.view.visibleElements=!1===u?D.selectpicker.current.elements:D.selectpicker.current.elements.slice(D.selectpicker.view.position0,D.selectpicker.view.position1),D.setOptionStatus(),(N||!1===u&&t)&&(p=!function(e,i){return e.length===i.length&&e.every(function(e,t){return e===i[t]})}(c,D.selectpicker.view.visibleElements)),(t||!0===u)&&p)){var v,g,b=D.$menuInner[0],w=document.createDocumentFragment(),I=b.firstChild.cloneNode(!1),x=D.selectpicker.view.visibleElements,k=[];b.replaceChild(I,b.firstChild);f=0;for(var y=x.length;f<y;f++){var $,S,E=x[f];D.options.sanitize&&($=E.lastChild)&&(S=D.selectpicker.current.data[f+D.selectpicker.view.position0])&&S.content&&!S.sanitized&&(k.push($),S.sanitized=!0),w.appendChild(E)}if(D.options.sanitize&&k.length&&W(k,D.options.whiteList,D.options.sanitizeFn),!0===u?(v=0===D.selectpicker.view.position0?0:D.selectpicker.current.data[D.selectpicker.view.position0-1].position,g=D.selectpicker.view.position1>d-1?0:D.selectpicker.current.data[d-1].position-D.selectpicker.current.data[D.selectpicker.view.position1-1].position,b.firstChild.style.marginTop=v+"px",b.firstChild.style.marginBottom=g+"px"):(b.firstChild.style.marginTop=0,b.firstChild.style.marginBottom=0),b.firstChild.appendChild(w),!0===u&&D.sizeInfo.hasScrollBar){var C=b.firstChild.offsetWidth;if(t&&C<D.sizeInfo.menuInnerInnerWidth&&D.sizeInfo.totalMenuWidth>D.sizeInfo.selectWidth)b.firstChild.style.minWidth=D.sizeInfo.menuInnerInnerWidth+"px";else if(C>D.sizeInfo.menuInnerInnerWidth){D.$menu[0].style.minWidth=0;var O=b.firstChild.offsetWidth;O>D.sizeInfo.menuInnerInnerWidth&&(D.sizeInfo.menuInnerInnerWidth=O,b.firstChild.style.minWidth=D.sizeInfo.menuInnerInnerWidth+"px"),D.$menu[0].style.minWidth=""}}}if(D.prevActiveIndex=D.activeIndex,D.options.liveSearch){if(N&&t){var z,T=0;D.selectpicker.view.canHighlight[T]||(T=1+D.selectpicker.view.canHighlight.slice(1).indexOf(!0)),z=D.selectpicker.view.visibleElements[T],D.defocusItem(D.selectpicker.view.currentActive),D.activeIndex=(D.selectpicker.current.data[T]||{}).index,D.focusItem(z)}}else D.$menuInner.trigger("focus")}l(i,!0),this.$menuInner.off("scroll.createView").on("scroll.createView",function(e,t){D.noScroll||l(this.scrollTop,t),D.noScroll=!1}),P(window).off("resize"+j+"."+this.selectId+".createView").on("resize"+j+"."+this.selectId+".createView",function(){D.$newElement.hasClass(V.SHOW)&&l(D.$menuInner[0].scrollTop)})},focusItem:function(e,t,i){if(e){t=t||this.selectpicker.main.data[this.activeIndex];var s=e.firstChild;s&&(s.setAttribute("aria-setsize",this.selectpicker.view.size),s.setAttribute("aria-posinset",t.posinset),!0!==i&&(this.focusedParent.setAttribute("aria-activedescendant",s.id),e.classList.add("active"),s.classList.add("active")))}},defocusItem:function(e){e&&(e.classList.remove("active"),e.firstChild&&e.firstChild.classList.remove("active"))},setPlaceholder:function(){var e=this,t=!1;if(this.options.title&&!this.multiple){this.selectpicker.view.titleOption||(this.selectpicker.view.titleOption=document.createElement("option")),t=!0;var i=this.$element[0],s=!1,n=!this.selectpicker.view.titleOption.parentNode,o=i.selectedIndex,r=i.options[o],l=window.performance&&window.performance.getEntriesByType("navigation"),a=l&&l.length?"back_forward"!==l[0].type:2!==window.performance.navigation.type;n&&(this.selectpicker.view.titleOption.className="bs-title-option",this.selectpicker.view.titleOption.value="",s=!r||0===o&&!1===r.defaultSelected&&void 0===this.$element.data("selected")),!n&&0===this.selectpicker.view.titleOption.index||i.insertBefore(this.selectpicker.view.titleOption,i.firstChild),s&&a?i.selectedIndex=0:"complete"!==document.readyState&&window.addEventListener("pageshow",function(){e.selectpicker.view.displayedValue!==i.value&&e.render()})}return t},buildData:function(){var p=':not([hidden]):not([data-hidden="true"])',u=[],f=0,m=this.setPlaceholder()?1:0;this.options.hideDisabled&&(p+=":not(:disabled)");var e=this.$element[0].querySelectorAll("select > *"+p);function v(e){var t=u[u.length-1];t&&"divider"===t.type&&(t.optID||e.optID)||((e=e||{}).type="divider",u.push(e))}function g(e,t){if((t=t||{}).divider="true"===e.getAttribute("data-divider"),t.divider)v({optID:t.optID});else{var i=u.length,s=e.style.cssText,n=s?S(s):"",o=(e.className||"")+(t.optgroupClass||"");t.optID&&(o="opt "+o),t.optionClass=o.trim(),t.inlineStyle=n,t.text=e.textContent,t.content=e.getAttribute("data-content"),t.tokens=e.getAttribute("data-tokens"),t.subtext=e.getAttribute("data-subtext"),t.icon=e.getAttribute("data-icon"),e.liIndex=i,t.display=t.content||t.text,t.type="option",t.index=i,t.option=e,t.selected=!!e.selected,t.disabled=t.disabled||!!e.disabled,u.push(t)}}function t(e,t){var i=t[e],s=!(e-1<m)&&t[e-1],n=t[e+1],o=i.querySelectorAll("option"+p);if(o.length){var r,l,a={display:S(i.label),subtext:i.getAttribute("data-subtext"),icon:i.getAttribute("data-icon"),type:"optgroup-label",optgroupClass:" "+(i.className||"")};f++,s&&v({optID:f}),a.optID=f,u.push(a);for(var c=0,d=o.length;c<d;c++){var h=o[c];0===c&&(l=(r=u.length-1)+d),g(h,{headerIndex:r,lastIndex:l,optID:a.optID,optgroupClass:a.optgroupClass,disabled:i.disabled})}n&&v({optID:f})}}for(var i=e.length,s=m;s<i;s++){var n=e[s];"OPTGROUP"!==n.tagName?g(n,{}):t(s,e)}this.selectpicker.main.data=this.selectpicker.current.data=u},buildList:function(){var s=this,e=this.selectpicker.main.data,n=[],o=0;function t(e){var t,i=0;switch(e.type){case"divider":t=K.li(!1,V.DIVIDER,e.optID?e.optID+"div":void 0);break;case"option":(t=K.li(K.a(K.text.call(s,e),e.optionClass,e.inlineStyle),"",e.optID)).firstChild&&(t.firstChild.id=s.selectId+"-"+e.index);break;case"optgroup-label":t=K.li(K.label.call(s,e),"dropdown-header"+e.optgroupClass,e.optID)}e.element=t,n.push(t),e.display&&(i+=e.display.length),e.subtext&&(i+=e.subtext.length),e.icon&&(i+=1),o<i&&(o=i,s.selectpicker.view.widestOption=n[n.length-1])}!s.options.showTick&&!s.multiple||_.checkMark.parentNode||(_.checkMark.className=this.options.iconBase+" "+s.options.tickIcon+" check-mark",_.a.appendChild(_.checkMark));for(var i=e.length,r=0;r<i;r++){t(e[r])}this.selectpicker.main.elements=this.selectpicker.current.elements=n},findLis:function(){return this.$menuInner.find(".inner > li")},render:function(){var e,t=this,i=this.$element[0],s=this.setPlaceholder()&&0===i.selectedIndex,n=O(i,this.options.hideDisabled),o=n.length,r=this.$button[0],l=r.querySelector(".filter-option-inner-inner"),a=document.createTextNode(this.options.multipleSeparator),c=_.fragment.cloneNode(!1),d=!1;if(r.classList.toggle("bs-placeholder",t.multiple?!o:!z(i,n)),t.multiple||1!==n.length||(t.selectpicker.view.displayedValue=z(i,n)),"static"===this.options.selectedTextFormat)c=K.text.call(this,{text:this.options.title},!0);else if(!1===(this.multiple&&-1!==this.options.selectedTextFormat.indexOf("count")&&1<o&&(1<(e=this.options.selectedTextFormat.split(">")).length&&o>e[1]||1===e.length&&2<=o))){if(!s){for(var h=0;h<o&&h<50;h++){var p=n[h],u=this.selectpicker.main.data[p.liIndex],f={};this.multiple&&0<h&&c.appendChild(a.cloneNode(!1)),p.title?f.text=p.title:u&&(u.content&&t.options.showContent?(f.content=u.content.toString(),d=!0):(t.options.showIcon&&(f.icon=u.icon),t.options.showSubtext&&!t.multiple&&u.subtext&&(f.subtext=" "+u.subtext),f.text=p.textContent.trim())),c.appendChild(K.text.call(this,f,!0))}49<o&&c.appendChild(document.createTextNode("..."))}}else{var m=':not([hidden]):not([data-hidden="true"]):not([data-divider="true"])';this.options.hideDisabled&&(m+=":not(:disabled)");var v=this.$element[0].querySelectorAll("select > option"+m+", optgroup"+m+" option"+m).length,g="function"==typeof this.options.countSelectedText?this.options.countSelectedText(o,v):this.options.countSelectedText;c=K.text.call(this,{text:g.replace("{0}",o.toString()).replace("{1}",v.toString())},!0)}if(null==this.options.title&&(this.options.title=this.$element.attr("title")),c.childNodes.length||(c=K.text.call(this,{text:void 0!==this.options.title?this.options.title:this.options.noneSelectedText},!0)),r.title=c.textContent.replace(/<[^>]*>?/g,"").trim(),this.options.sanitize&&d&&W([c],t.options.whiteList,t.options.sanitizeFn),l.innerHTML="",l.appendChild(c),M.major<4&&this.$newElement[0].classList.contains("bs3-has-addon")){var b=r.querySelector(".filter-expand"),w=l.cloneNode(!0);w.className="filter-expand",b?r.replaceChild(w,b):r.appendChild(w)}this.$element.trigger("rendered"+j)},setStyle:function(e,t){var i,s=this.$button[0],n=this.$newElement[0],o=this.options.style.trim();this.$element.attr("class")&&this.$newElement.addClass(this.$element.attr("class").replace(/selectpicker|mobile-device|bs-select-hidden|validate\[.*\]/gi,"")),M.major<4&&(n.classList.add("bs3"),n.parentNode.classList&&n.parentNode.classList.contains("input-group")&&(n.previousElementSibling||n.nextElementSibling)&&(n.previousElementSibling||n.nextElementSibling).classList.contains("input-group-addon")&&n.classList.add("bs3-has-addon")),i=e?e.trim():o,"add"==t?i&&s.classList.add.apply(s.classList,i.split(" ")):"remove"==t?i&&s.classList.remove.apply(s.classList,i.split(" ")):(o&&s.classList.remove.apply(s.classList,o.split(" ")),i&&s.classList.add.apply(s.classList,i.split(" ")))},liHeight:function(e){if(e||!1!==this.options.size&&!Object.keys(this.sizeInfo).length){var t,i=_.div.cloneNode(!1),s=_.div.cloneNode(!1),n=_.div.cloneNode(!1),o=document.createElement("ul"),r=_.li.cloneNode(!1),l=_.li.cloneNode(!1),a=_.a.cloneNode(!1),c=_.span.cloneNode(!1),d=this.options.header&&0<this.$menu.find("."+V.POPOVERHEADER).length?this.$menu.find("."+V.POPOVERHEADER)[0].cloneNode(!0):null,h=this.options.liveSearch?_.div.cloneNode(!1):null,p=this.options.actionsBox&&this.multiple&&0<this.$menu.find(".bs-actionsbox").length?this.$menu.find(".bs-actionsbox")[0].cloneNode(!0):null,u=this.options.doneButton&&this.multiple&&0<this.$menu.find(".bs-donebutton").length?this.$menu.find(".bs-donebutton")[0].cloneNode(!0):null,f=this.$element.find("option")[0];if(this.sizeInfo.selectWidth=this.$newElement[0].offsetWidth,c.className="text",a.className="dropdown-item "+(f?f.className:""),i.className=this.$menu[0].parentNode.className+" "+V.SHOW,i.style.width=0,"auto"===this.options.width&&(s.style.minWidth=0),s.className=V.MENU+" "+V.SHOW,n.className="inner "+V.SHOW,o.className=V.MENU+" inner "+("4"===M.major?V.SHOW:""),r.className=V.DIVIDER,l.className="dropdown-header",c.appendChild(document.createTextNode("\u200b")),this.selectpicker.current.data.length)for(var m=0;m<this.selectpicker.current.data.length;m++){var v=this.selectpicker.current.data[m];if("option"===v.type){t=v.element;break}}else t=_.li.cloneNode(!1),a.appendChild(c),t.appendChild(a);if(l.appendChild(c.cloneNode(!0)),this.selectpicker.view.widestOption&&o.appendChild(this.selectpicker.view.widestOption.cloneNode(!0)),o.appendChild(t),o.appendChild(r),o.appendChild(l),d&&s.appendChild(d),h){var g=document.createElement("input");h.className="bs-searchbox",g.className="form-control",h.appendChild(g),s.appendChild(h)}p&&s.appendChild(p),n.appendChild(o),s.appendChild(n),u&&s.appendChild(u),i.appendChild(s),document.body.appendChild(i);var b,w=t.offsetHeight,I=l?l.offsetHeight:0,x=d?d.offsetHeight:0,k=h?h.offsetHeight:0,y=p?p.offsetHeight:0,$=u?u.offsetHeight:0,S=P(r).outerHeight(!0),E=!!window.getComputedStyle&&window.getComputedStyle(s),C=s.offsetWidth,O=E?null:P(s),z={vert:N(E?E.paddingTop:O.css("paddingTop"))+N(E?E.paddingBottom:O.css("paddingBottom"))+N(E?E.borderTopWidth:O.css("borderTopWidth"))+N(E?E.borderBottomWidth:O.css("borderBottomWidth")),horiz:N(E?E.paddingLeft:O.css("paddingLeft"))+N(E?E.paddingRight:O.css("paddingRight"))+N(E?E.borderLeftWidth:O.css("borderLeftWidth"))+N(E?E.borderRightWidth:O.css("borderRightWidth"))},T={vert:z.vert+N(E?E.marginTop:O.css("marginTop"))+N(E?E.marginBottom:O.css("marginBottom"))+2,horiz:z.horiz+N(E?E.marginLeft:O.css("marginLeft"))+N(E?E.marginRight:O.css("marginRight"))+2};n.style.overflowY="scroll",b=s.offsetWidth-C,document.body.removeChild(i),this.sizeInfo.liHeight=w,this.sizeInfo.dropdownHeaderHeight=I,this.sizeInfo.headerHeight=x,this.sizeInfo.searchHeight=k,this.sizeInfo.actionsHeight=y,this.sizeInfo.doneButtonHeight=$,this.sizeInfo.dividerHeight=S,this.sizeInfo.menuPadding=z,this.sizeInfo.menuExtras=T,this.sizeInfo.menuWidth=C,this.sizeInfo.menuInnerInnerWidth=C-z.horiz,this.sizeInfo.totalMenuWidth=this.sizeInfo.menuWidth,this.sizeInfo.scrollBarWidth=b,this.sizeInfo.selectHeight=this.$newElement[0].offsetHeight,this.setPositionData()}},getSelectPosition:function(){var e,t=P(window),i=this.$newElement.offset(),s=P(this.options.container);this.options.container&&s.length&&!s.is("body")?((e=s.offset()).top+=parseInt(s.css("borderTopWidth")),e.left+=parseInt(s.css("borderLeftWidth"))):e={top:0,left:0};var n=this.options.windowPadding;this.sizeInfo.selectOffsetTop=i.top-e.top-t.scrollTop(),this.sizeInfo.selectOffsetBot=t.height()-this.sizeInfo.selectOffsetTop-this.sizeInfo.selectHeight-e.top-n[2],this.sizeInfo.selectOffsetLeft=i.left-e.left-t.scrollLeft(),this.sizeInfo.selectOffsetRight=t.width()-this.sizeInfo.selectOffsetLeft-this.sizeInfo.selectWidth-e.left-n[1],this.sizeInfo.selectOffsetTop-=n[0],this.sizeInfo.selectOffsetLeft-=n[3]},setMenuSize:function(e){this.getSelectPosition();var t,i,s,n,o,r,l,a,c=this.sizeInfo.selectWidth,d=this.sizeInfo.liHeight,h=this.sizeInfo.headerHeight,p=this.sizeInfo.searchHeight,u=this.sizeInfo.actionsHeight,f=this.sizeInfo.doneButtonHeight,m=this.sizeInfo.dividerHeight,v=this.sizeInfo.menuPadding,g=0;if(this.options.dropupAuto&&(l=d*this.selectpicker.current.elements.length+v.vert,a=this.sizeInfo.selectOffsetTop-this.sizeInfo.selectOffsetBot>this.sizeInfo.menuExtras.vert&&l+this.sizeInfo.menuExtras.vert+50>this.sizeInfo.selectOffsetBot,!0===this.selectpicker.isSearching&&(a=this.selectpicker.dropup),this.$newElement.toggleClass(V.DROPUP,a),this.selectpicker.dropup=a),"auto"===this.options.size)n=3<this.selectpicker.current.elements.length?3*this.sizeInfo.liHeight+this.sizeInfo.menuExtras.vert-2:0,i=this.sizeInfo.selectOffsetBot-this.sizeInfo.menuExtras.vert,s=n+h+p+u+f,r=Math.max(n-v.vert,0),this.$newElement.hasClass(V.DROPUP)&&(i=this.sizeInfo.selectOffsetTop-this.sizeInfo.menuExtras.vert),t=(o=i)-h-p-u-f-v.vert;else if(this.options.size&&"auto"!=this.options.size&&this.selectpicker.current.elements.length>this.options.size){for(var b=0;b<this.options.size;b++)"divider"===this.selectpicker.current.data[b].type&&g++;t=(i=d*this.options.size+g*m+v.vert)-v.vert,o=i+h+p+u+f,s=r=""}this.$menu.css({"max-height":o+"px",overflow:"hidden","min-height":s+"px"}),this.$menuInner.css({"max-height":t+"px","overflow-y":"auto","min-height":r+"px"}),this.sizeInfo.menuInnerHeight=Math.max(t,1),this.selectpicker.current.data.length&&this.selectpicker.current.data[this.selectpicker.current.data.length-1].position>this.sizeInfo.menuInnerHeight&&(this.sizeInfo.hasScrollBar=!0,this.sizeInfo.totalMenuWidth=this.sizeInfo.menuWidth+this.sizeInfo.scrollBarWidth),"auto"===this.options.dropdownAlignRight&&this.$menu.toggleClass(V.MENURIGHT,this.sizeInfo.selectOffsetLeft>this.sizeInfo.selectOffsetRight&&this.sizeInfo.selectOffsetRight<this.sizeInfo.totalMenuWidth-c),this.dropdown&&this.dropdown._popper&&this.dropdown._popper.update()},setSize:function(e){if(this.liHeight(e),this.options.header&&this.$menu.css("padding-top",0),!1!==this.options.size){var t=this,i=P(window);this.setMenuSize(),this.options.liveSearch&&this.$searchbox.off("input.setMenuSize propertychange.setMenuSize").on("input.setMenuSize propertychange.setMenuSize",function(){return t.setMenuSize()}),"auto"===this.options.size?i.off("resize"+j+"."+this.selectId+".setMenuSize scroll"+j+"."+this.selectId+".setMenuSize").on("resize"+j+"."+this.selectId+".setMenuSize scroll"+j+"."+this.selectId+".setMenuSize",function(){return t.setMenuSize()}):this.options.size&&"auto"!=this.options.size&&this.selectpicker.current.elements.length>this.options.size&&i.off("resize"+j+"."+this.selectId+".setMenuSize scroll"+j+"."+this.selectId+".setMenuSize")}this.createView(!1,!0,e)},setWidth:function(){var i=this;"auto"===this.options.width?requestAnimationFrame(function(){i.$menu.css("min-width","0"),i.$element.on("loaded"+j,function(){i.liHeight(),i.setMenuSize();var e=i.$newElement.clone().appendTo("body"),t=e.css("width","auto").children("button").outerWidth();e.remove(),i.sizeInfo.selectWidth=Math.max(i.sizeInfo.totalMenuWidth,t),i.$newElement.css("width",i.sizeInfo.selectWidth+"px")})}):"fit"===this.options.width?(this.$menu.css("min-width",""),this.$newElement.css("width","").addClass("fit-width")):this.options.width?(this.$menu.css("min-width",""),this.$newElement.css("width",this.options.width)):(this.$menu.css("min-width",""),this.$newElement.css("width","")),this.$newElement.hasClass("fit-width")&&"fit"!==this.options.width&&this.$newElement[0].classList.remove("fit-width")},selectPosition:function(){this.$bsContainer=P('<div class="bs-container" />');function e(e){var t={},i=r.options.display||!!P.fn.dropdown.Constructor.Default&&P.fn.dropdown.Constructor.Default.display;r.$bsContainer.addClass(e.attr("class").replace(/form-control|fit-width/gi,"")).toggleClass(V.DROPUP,e.hasClass(V.DROPUP)),s=e.offset(),l.is("body")?n={top:0,left:0}:((n=l.offset()).top+=parseInt(l.css("borderTopWidth"))-l.scrollTop(),n.left+=parseInt(l.css("borderLeftWidth"))-l.scrollLeft()),o=e.hasClass(V.DROPUP)?0:e[0].offsetHeight,(M.major<4||"static"===i)&&(t.top=s.top-n.top+o,t.left=s.left-n.left),t.width=e[0].offsetWidth,r.$bsContainer.css(t)}var s,n,o,r=this,l=P(this.options.container);this.$button.on("click.bs.dropdown.data-api",function(){r.isDisabled()||(e(r.$newElement),r.$bsContainer.appendTo(r.options.container).toggleClass(V.SHOW,!r.$button.hasClass(V.SHOW)).append(r.$menu))}),P(window).off("resize"+j+"."+this.selectId+" scroll"+j+"."+this.selectId).on("resize"+j+"."+this.selectId+" scroll"+j+"."+this.selectId,function(){r.$newElement.hasClass(V.SHOW)&&e(r.$newElement)}),this.$element.on("hide"+j,function(){r.$menu.data("height",r.$menu.height()),r.$bsContainer.detach()})},setOptionStatus:function(e){var t=this;if(t.noScroll=!1,t.selectpicker.view.visibleElements&&t.selectpicker.view.visibleElements.length)for(var i=0;i<t.selectpicker.view.visibleElements.length;i++){var s=t.selectpicker.current.data[i+t.selectpicker.view.position0],n=s.option;n&&(!0!==e&&t.setDisabled(s.index,s.disabled),t.setSelected(s.index,n.selected))}},setSelected:function(e,t){var i,s,n=this.selectpicker.main.elements[e],o=this.selectpicker.main.data[e],r=void 0!==this.activeIndex,l=this.activeIndex===e||t&&!this.multiple&&!r;o.selected=t,s=n.firstChild,t&&(this.selectedIndex=e),n.classList.toggle("selected",t),l?(this.focusItem(n,o),this.selectpicker.view.currentActive=n,this.activeIndex=e):this.defocusItem(n),s&&(s.classList.toggle("selected",t),t?s.setAttribute("aria-selected",!0):this.multiple?s.setAttribute("aria-selected",!1):s.removeAttribute("aria-selected")),l||r||!t||void 0===this.prevActiveIndex||(i=this.selectpicker.main.elements[this.prevActiveIndex],this.defocusItem(i))},setDisabled:function(e,t){var i,s=this.selectpicker.main.elements[e];this.selectpicker.main.data[e].disabled=t,i=s.firstChild,s.classList.toggle(V.DISABLED,t),i&&("4"===M.major&&i.classList.toggle(V.DISABLED,t),t?(i.setAttribute("aria-disabled",t),i.setAttribute("tabindex",-1)):(i.removeAttribute("aria-disabled"),i.setAttribute("tabindex",0)))},isDisabled:function(){return this.$element[0].disabled},checkDisabled:function(){this.isDisabled()?(this.$newElement[0].classList.add(V.DISABLED),this.$button.addClass(V.DISABLED).attr("aria-disabled",!0)):this.$button[0].classList.contains(V.DISABLED)&&(this.$newElement[0].classList.remove(V.DISABLED),this.$button.removeClass(V.DISABLED).attr("aria-disabled",!1))},clickListener:function(){var C=this,t=P(document);function e(){C.options.liveSearch?C.$searchbox.trigger("focus"):C.$menuInner.trigger("focus")}function i(){C.dropdown&&C.dropdown._popper&&C.dropdown._popper.state.isCreated?e():requestAnimationFrame(i)}t.data("spaceSelect",!1),this.$button.on("keyup",function(e){/(32)/.test(e.keyCode.toString(10))&&t.data("spaceSelect")&&(e.preventDefault(),t.data("spaceSelect",!1))}),this.$newElement.on("show.bs.dropdown",function(){3<M.major&&!C.dropdown&&(C.dropdown=C.$button.data("bs.dropdown"),C.dropdown._menu=C.$menu[0])}),this.$button.on("click.bs.dropdown.data-api",function(){C.$newElement.hasClass(V.SHOW)||C.setSize()}),this.$element.on("shown"+j,function(){C.$menuInner[0].scrollTop!==C.selectpicker.view.scrollTop&&(C.$menuInner[0].scrollTop=C.selectpicker.view.scrollTop),3<M.major?requestAnimationFrame(i):e()}),this.$menuInner.on("mouseenter","li a",function(e){var t=this.parentElement,i=C.isVirtual()?C.selectpicker.view.position0:0,s=Array.prototype.indexOf.call(t.parentElement.children,t),n=C.selectpicker.current.data[s+i];C.focusItem(t,n,!0)}),this.$menuInner.on("click","li a",function(e,t){var i=P(this),s=C.$element[0],n=C.isVirtual()?C.selectpicker.view.position0:0,o=C.selectpicker.current.data[i.parent().index()+n],r=o.index,l=z(s),a=s.selectedIndex,c=s.options[a],d=!0;if(C.multiple&&1!==C.options.maxOptions&&e.stopPropagation(),e.preventDefault(),!C.isDisabled()&&!i.parent().hasClass(V.DISABLED)){var h=o.option,p=P(h),u=h.selected,f=p.parent("optgroup"),m=f.find("option"),v=C.options.maxOptions,g=f.data("maxOptions")||!1;if(r===C.activeIndex&&(t=!0),t||(C.prevActiveIndex=C.activeIndex,C.activeIndex=void 0),C.multiple){if(h.selected=!u,C.setSelected(r,!u),C.focusedParent.focus(),!1!==v||!1!==g){var b=v<O(s).length,w=g<f.find("option:selected").length;if(v&&b||g&&w)if(v&&1==v)s.selectedIndex=-1,h.selected=!0,C.setOptionStatus(!0);else if(g&&1==g){for(var I=0;I<m.length;I++){var x=m[I];x.selected=!1,C.setSelected(x.liIndex,!1)}h.selected=!0,C.setSelected(r,!0)}else{var k="string"==typeof C.options.maxOptionsText?[C.options.maxOptionsText,C.options.maxOptionsText]:C.options.maxOptionsText,y="function"==typeof k?k(v,g):k,$=y[0].replace("{n}",v),S=y[1].replace("{n}",g),E=P('<div class="notify"></div>');y[2]&&($=$.replace("{var}",y[2][1<v?0:1]),S=S.replace("{var}",y[2][1<g?0:1])),h.selected=!1,C.$menu.append(E),v&&b&&(E.append(P("<div>"+$+"</div>")),d=!1,C.$element.trigger("maxReached"+j)),g&&w&&(E.append(P("<div>"+S+"</div>")),d=!1,C.$element.trigger("maxReachedGrp"+j)),setTimeout(function(){C.setSelected(r,!1)},10),E[0].classList.add("fadeOut"),setTimeout(function(){E.remove()},1050)}}}else c&&(c.selected=!1),h.selected=!0,C.setSelected(r,!0);!C.multiple||C.multiple&&1===C.options.maxOptions?C.$button.trigger("focus"):C.options.liveSearch&&C.$searchbox.trigger("focus"),d&&(!C.multiple&&a===s.selectedIndex||(T=[h.index,p.prop("selected"),l],C.$element.triggerNative("change")))}}),this.$menu.on("click","li."+V.DISABLED+" a, ."+V.POPOVERHEADER+", ."+V.POPOVERHEADER+" :not(.close)",function(e){e.currentTarget==this&&(e.preventDefault(),e.stopPropagation(),C.options.liveSearch&&!P(e.target).hasClass("close")?C.$searchbox.trigger("focus"):C.$button.trigger("focus"))}),this.$menuInner.on("click",".divider, .dropdown-header",function(e){e.preventDefault(),e.stopPropagation(),C.options.liveSearch?C.$searchbox.trigger("focus"):C.$button.trigger("focus")}),this.$menu.on("click","."+V.POPOVERHEADER+" .close",function(){C.$button.trigger("click")}),this.$searchbox.on("click",function(e){e.stopPropagation()}),this.$menu.on("click",".actions-btn",function(e){C.options.liveSearch?C.$searchbox.trigger("focus"):C.$button.trigger("focus"),e.preventDefault(),e.stopPropagation(),P(this).hasClass("bs-select-all")?C.selectAll():C.deselectAll()}),this.$button.on("focus"+j,function(e){var t=C.$element[0].getAttribute("tabindex");void 0!==t&&e.originalEvent&&e.originalEvent.isTrusted&&(this.setAttribute("tabindex",t),C.$element[0].setAttribute("tabindex",-1),C.selectpicker.view.tabindex=t)}).on("blur"+j,function(e){void 0!==C.selectpicker.view.tabindex&&e.originalEvent&&e.originalEvent.isTrusted&&(C.$element[0].setAttribute("tabindex",C.selectpicker.view.tabindex),this.setAttribute("tabindex",-1),C.selectpicker.view.tabindex=void 0)}),this.$element.on("change"+j,function(){C.render(),C.$element.trigger("changed"+j,T),T=null}).on("focus"+j,function(){C.options.mobile||C.$button[0].focus()})},liveSearchListener:function(){var u=this;this.$button.on("click.bs.dropdown.data-api",function(){u.$searchbox.val()&&(u.$searchbox.val(""),u.selectpicker.search.previousValue=void 0)}),this.$searchbox.on("click.bs.dropdown.data-api focus.bs.dropdown.data-api touchend.bs.dropdown.data-api",function(e){e.stopPropagation()}),this.$searchbox.on("input propertychange",function(){var e=u.$searchbox[0].value;if(u.selectpicker.search.elements=[],u.selectpicker.search.data=[],e){var t=[],i=e.toUpperCase(),s={},n=[],o=u._searchStyle(),r=u.options.liveSearchNormalize;r&&(i=w(i));for(var l=0;l<u.selectpicker.main.data.length;l++){var a=u.selectpicker.main.data[l];s[l]||(s[l]=k(a,i,o,r)),s[l]&&void 0!==a.headerIndex&&-1===n.indexOf(a.headerIndex)&&(0<a.headerIndex&&(s[a.headerIndex-1]=!0,n.push(a.headerIndex-1)),s[a.headerIndex]=!0,n.push(a.headerIndex),s[a.lastIndex+1]=!0),s[l]&&"optgroup-label"!==a.type&&n.push(l)}l=0;for(var c=n.length;l<c;l++){var d=n[l],h=n[l-1],p=(a=u.selectpicker.main.data[d],u.selectpicker.main.data[h]);("divider"!==a.type||"divider"===a.type&&p&&"divider"!==p.type&&c-1!==l)&&(u.selectpicker.search.data.push(a),t.push(u.selectpicker.main.elements[d]))}u.activeIndex=void 0,u.noScroll=!0,u.$menuInner.scrollTop(0),u.selectpicker.search.elements=t,u.createView(!0),function(e,t){e.length||(_.noResults.innerHTML=this.options.noneResultsText.replace("{0}",'"'+S(t)+'"'),this.$menuInner[0].firstChild.appendChild(_.noResults))}.call(u,t,e)}else u.selectpicker.search.previousValue&&(u.$menuInner.scrollTop(0),u.createView(!1));u.selectpicker.search.previousValue=e})},_searchStyle:function(){return this.options.liveSearchStyle||"contains"},val:function(e){var t=this.$element[0];if(void 0===e)return this.$element.val();var i=z(t);if(T=[null,null,i],this.$element.val(e).trigger("changed"+j,T),this.$newElement.hasClass(V.SHOW))if(this.multiple)this.setOptionStatus(!0);else{var s=(t.options[t.selectedIndex]||{}).liIndex;"number"==typeof s&&(this.setSelected(this.selectedIndex,!1),this.setSelected(s,!0))}return this.render(),T=null,this.$element},changeAll:function(e){if(this.multiple){void 0===e&&(e=!0);var t=this.$element[0],i=0,s=0,n=z(t);t.classList.add("bs-select-hidden");for(var o=0,r=this.selectpicker.current.data,l=r.length;o<l;o++){var a=r[o],c=a.option;c&&!a.disabled&&"divider"!==a.type&&(a.selected&&i++,!0===(c.selected=e)&&s++)}t.classList.remove("bs-select-hidden"),i!==s&&(this.setOptionStatus(),T=[null,null,n],this.$element.triggerNative("change"))}},selectAll:function(){return this.changeAll(!0)},deselectAll:function(){return this.changeAll(!1)},toggle:function(e){(e=e||window.event)&&e.stopPropagation(),this.$button.trigger("click.bs.dropdown.data-api")},keydown:function(e){var t,i,s,n,o,r=P(this),l=r.hasClass("dropdown-toggle"),a=(l?r.closest(".dropdown"):r.closest(F.MENU)).data("this"),c=a.findLis(),d=!1,h=e.which===H&&!l&&!a.options.selectOnTab,p=G.test(e.which)||h,u=a.$menuInner[0].scrollTop,f=!0===a.isVirtual()?a.selectpicker.view.position0:0;if(!(112<=e.which&&e.which<=123))if(!(i=a.$newElement.hasClass(V.SHOW))&&(p||48<=e.which&&e.which<=57||96<=e.which&&e.which<=105||65<=e.which&&e.which<=90)&&(a.$button.trigger("click.bs.dropdown.data-api"),a.options.liveSearch))a.$searchbox.trigger("focus");else{if(e.which===A&&i&&(e.preventDefault(),a.$button.trigger("click.bs.dropdown.data-api").trigger("focus")),p){if(!c.length)return;-1!==(t=(s=a.selectpicker.main.elements[a.activeIndex])?Array.prototype.indexOf.call(s.parentElement.children,s):-1)&&a.defocusItem(s),e.which===B?(-1!==t&&t--,t+f<0&&(t+=c.length),a.selectpicker.view.canHighlight[t+f]||-1===(t=a.selectpicker.view.canHighlight.slice(0,t+f).lastIndexOf(!0)-f)&&(t=c.length-1)):e.which!==R&&!h||(++t+f>=a.selectpicker.view.canHighlight.length&&(t=a.selectpicker.view.firstHighlightIndex),a.selectpicker.view.canHighlight[t+f]||(t=t+1+a.selectpicker.view.canHighlight.slice(t+f+1).indexOf(!0))),e.preventDefault();var m=f+t;e.which===B?0===f&&t===c.length-1?(a.$menuInner[0].scrollTop=a.$menuInner[0].scrollHeight,m=a.selectpicker.current.elements.length-1):d=(o=(n=a.selectpicker.current.data[m]).position-n.height)<u:e.which!==R&&!h||(t===a.selectpicker.view.firstHighlightIndex?(a.$menuInner[0].scrollTop=0,m=a.selectpicker.view.firstHighlightIndex):d=u<(o=(n=a.selectpicker.current.data[m]).position-a.sizeInfo.menuInnerHeight)),s=a.selectpicker.current.elements[m],a.activeIndex=a.selectpicker.current.data[m].index,a.focusItem(s),a.selectpicker.view.currentActive=s,d&&(a.$menuInner[0].scrollTop=o),a.options.liveSearch?a.$searchbox.trigger("focus"):r.trigger("focus")}else if(!r.is("input")&&!q.test(e.which)||e.which===D&&a.selectpicker.keydown.keyHistory){var v,g,b=[];e.preventDefault(),a.selectpicker.keydown.keyHistory+=C[e.which],a.selectpicker.keydown.resetKeyHistory.cancel&&clearTimeout(a.selectpicker.keydown.resetKeyHistory.cancel),a.selectpicker.keydown.resetKeyHistory.cancel=a.selectpicker.keydown.resetKeyHistory.start(),g=a.selectpicker.keydown.keyHistory,/^(.)\1+$/.test(g)&&(g=g.charAt(0));for(var w=0;w<a.selectpicker.current.data.length;w++){var I=a.selectpicker.current.data[w];k(I,g,"startsWith",!0)&&a.selectpicker.view.canHighlight[w]&&b.push(I.index)}if(b.length){var x=0;c.removeClass("active").find("a").removeClass("active"),1===g.length&&(-1===(x=b.indexOf(a.activeIndex))||x===b.length-1?x=0:x++),v=b[x],d=0<u-(n=a.selectpicker.main.data[v]).position?(o=n.position-n.height,!0):(o=n.position-a.sizeInfo.menuInnerHeight,n.position>u+a.sizeInfo.menuInnerHeight),s=a.selectpicker.main.elements[v],a.activeIndex=b[x],a.focusItem(s),s&&s.firstChild.focus(),d&&(a.$menuInner[0].scrollTop=o),r.trigger("focus")}}i&&(e.which===D&&!a.selectpicker.keydown.keyHistory||e.which===L||e.which===H&&a.options.selectOnTab)&&(e.which!==D&&e.preventDefault(),a.options.liveSearch&&e.which===D||(a.$menuInner.find(".active a").trigger("click",!0),r.trigger("focus"),a.options.liveSearch||(e.preventDefault(),P(document).data("spaceSelect",!0))))}},mobile:function(){this.options.mobile=!0,this.$element[0].classList.add("mobile-device")},refresh:function(){var e=P.extend({},this.options,this.$element.data());this.options=e,this.checkDisabled(),this.buildData(),this.setStyle(),this.render(),this.buildList(),this.setWidth(),this.setSize(!0),this.$element.trigger("refreshed"+j)},hide:function(){this.$newElement.hide()},show:function(){this.$newElement.show()},remove:function(){this.$newElement.remove(),this.$element.remove()},destroy:function(){this.$newElement.before(this.$element).remove(),this.$bsContainer?this.$bsContainer.remove():this.$menu.remove(),this.selectpicker.view.titleOption&&this.selectpicker.view.titleOption.parentNode&&this.selectpicker.view.titleOption.parentNode.removeChild(this.selectpicker.view.titleOption),this.$element.off(j).removeData("selectpicker").removeClass("bs-select-hidden selectpicker"),P(window).off(j+"."+this.selectId)}};var J=P.fn.selectpicker;function Q(){if(P.fn.dropdown)return(P.fn.dropdown.Constructor._dataApiKeydownHandler||P.fn.dropdown.Constructor.prototype.keydown).apply(this,arguments)}P.fn.selectpicker=Z,P.fn.selectpicker.Constructor=Y,P.fn.selectpicker.noConflict=function(){return P.fn.selectpicker=J,this},P(document).off("keydown.bs.dropdown.data-api").on("keydown.bs.dropdown.data-api",':not(.bootstrap-select) > [data-toggle="dropdown"]',Q).on("keydown.bs.dropdown.data-api",":not(.bootstrap-select) > .dropdown-menu",Q).on("keydown"+j,'.bootstrap-select [data-toggle="dropdown"], .bootstrap-select [role="listbox"], .bootstrap-select .bs-searchbox input',Y.prototype.keydown).on("focusin.modal",'.bootstrap-select [data-toggle="dropdown"], .bootstrap-select [role="listbox"], .bootstrap-select .bs-searchbox input',function(e){e.stopPropagation()}),P(window).on("load"+j+".data-api",function(){P(".selectpicker").each(function(){var e=P(this);Z.call(e,e.data())})})}(e)});
/* End */
;
; /* Start:"a:4:{s:4:"full";s:68:"/local/templates/.default/markup/new/js/slick.min.js?164206818542863";s:6:"source";s:52:"/local/templates/.default/markup/new/js/slick.min.js";s:3:"min";s:0:"";s:3:"map";s:0:"";}"*/
!function(i){"use strict";"function"==typeof define&&define.amd?define(["jquery"],i):"undefined"!=typeof exports?module.exports=i(require("jquery")):i(jQuery)}(function(i){"use strict";var e=window.Slick||{};(e=function(){var e=0;return function(t,o){var s,n=this;n.defaults={accessibility:!0,adaptiveHeight:!1,appendArrows:i(t),appendDots:i(t),arrows:!0,asNavFor:null,prevArrow:'<button class="slick-prev" aria-label="Previous" type="button">Previous</button>',nextArrow:'<button class="slick-next" aria-label="Next" type="button">Next</button>',autoplay:!1,autoplaySpeed:3e3,centerMode:!1,centerPadding:"50px",cssEase:"ease",customPaging:function(e,t){return i('<button type="button" />').text(t+1)},dots:!1,dotsClass:"slick-dots",draggable:!0,easing:"linear",edgeFriction:.35,fade:!1,focusOnSelect:!1,focusOnChange:!1,infinite:!0,initialSlide:0,lazyLoad:"ondemand",mobileFirst:!1,pauseOnHover:!0,pauseOnFocus:!0,pauseOnDotsHover:!1,respondTo:"window",responsive:null,rows:1,rtl:!1,slide:"",slidesPerRow:1,slidesToShow:1,slidesToScroll:1,speed:500,swipe:!0,swipeToSlide:!1,touchMove:!0,touchThreshold:5,useCSS:!0,useTransform:!0,variableWidth:!1,vertical:!1,verticalSwiping:!1,waitForAnimate:!0,zIndex:1e3},n.initials={animating:!1,dragging:!1,autoPlayTimer:null,currentDirection:0,currentLeft:null,currentSlide:0,direction:1,$dots:null,listWidth:null,listHeight:null,loadIndex:0,$nextArrow:null,$prevArrow:null,scrolling:!1,slideCount:null,slideWidth:null,$slideTrack:null,$slides:null,sliding:!1,slideOffset:0,swipeLeft:null,swiping:!1,$list:null,touchObject:{},transformsEnabled:!1,unslicked:!1},i.extend(n,n.initials),n.activeBreakpoint=null,n.animType=null,n.animProp=null,n.breakpoints=[],n.breakpointSettings=[],n.cssTransitions=!1,n.focussed=!1,n.interrupted=!1,n.hidden="hidden",n.paused=!0,n.positionProp=null,n.respondTo=null,n.rowCount=1,n.shouldClick=!0,n.$slider=i(t),n.$slidesCache=null,n.transformType=null,n.transitionType=null,n.visibilityChange="visibilitychange",n.windowWidth=0,n.windowTimer=null,s=i(t).data("slick")||{},n.options=i.extend({},n.defaults,o,s),n.currentSlide=n.options.initialSlide,n.originalSettings=n.options,void 0!==document.mozHidden?(n.hidden="mozHidden",n.visibilityChange="mozvisibilitychange"):void 0!==document.webkitHidden&&(n.hidden="webkitHidden",n.visibilityChange="webkitvisibilitychange"),n.autoPlay=i.proxy(n.autoPlay,n),n.autoPlayClear=i.proxy(n.autoPlayClear,n),n.autoPlayIterator=i.proxy(n.autoPlayIterator,n),n.changeSlide=i.proxy(n.changeSlide,n),n.clickHandler=i.proxy(n.clickHandler,n),n.selectHandler=i.proxy(n.selectHandler,n),n.setPosition=i.proxy(n.setPosition,n),n.swipeHandler=i.proxy(n.swipeHandler,n),n.dragHandler=i.proxy(n.dragHandler,n),n.keyHandler=i.proxy(n.keyHandler,n),n.instanceUid=e++,n.htmlExpr=/^(?:\s*(<[\w\W]+>)[^>]*)$/,n.registerBreakpoints(),n.init(!0)}}()).prototype.activateADA=function(){this.$slideTrack.find(".slick-active").attr({"aria-hidden":"false"}).find("a, input, button, select").attr({tabindex:"0"})},e.prototype.addSlide=e.prototype.slickAdd=function(e,t,o){var s=this;if("boolean"==typeof t)o=t,t=null;else if(t<0||t>=s.slideCount)return!1;s.unload(),"number"==typeof t?0===t&&0===s.$slides.length?i(e).appendTo(s.$slideTrack):o?i(e).insertBefore(s.$slides.eq(t)):i(e).insertAfter(s.$slides.eq(t)):!0===o?i(e).prependTo(s.$slideTrack):i(e).appendTo(s.$slideTrack),s.$slides=s.$slideTrack.children(this.options.slide),s.$slideTrack.children(this.options.slide).detach(),s.$slideTrack.append(s.$slides),s.$slides.each(function(e,t){i(t).attr("data-slick-index",e)}),s.$slidesCache=s.$slides,s.reinit()},e.prototype.animateHeight=function(){var i=this;if(1===i.options.slidesToShow&&!0===i.options.adaptiveHeight&&!1===i.options.vertical){var e=i.$slides.eq(i.currentSlide).outerHeight(!0);i.$list.animate({height:e},i.options.speed)}},e.prototype.animateSlide=function(e,t){var o={},s=this;s.animateHeight(),!0===s.options.rtl&&!1===s.options.vertical&&(e=-e),!1===s.transformsEnabled?!1===s.options.vertical?s.$slideTrack.animate({left:e},s.options.speed,s.options.easing,t):s.$slideTrack.animate({top:e},s.options.speed,s.options.easing,t):!1===s.cssTransitions?(!0===s.options.rtl&&(s.currentLeft=-s.currentLeft),i({animStart:s.currentLeft}).animate({animStart:e},{duration:s.options.speed,easing:s.options.easing,step:function(i){i=Math.ceil(i),!1===s.options.vertical?(o[s.animType]="translate("+i+"px, 0px)",s.$slideTrack.css(o)):(o[s.animType]="translate(0px,"+i+"px)",s.$slideTrack.css(o))},complete:function(){t&&t.call()}})):(s.applyTransition(),e=Math.ceil(e),!1===s.options.vertical?o[s.animType]="translate3d("+e+"px, 0px, 0px)":o[s.animType]="translate3d(0px,"+e+"px, 0px)",s.$slideTrack.css(o),t&&setTimeout(function(){s.disableTransition(),t.call()},s.options.speed))},e.prototype.getNavTarget=function(){var e=this,t=e.options.asNavFor;return t&&null!==t&&(t=i(t).not(e.$slider)),t},e.prototype.asNavFor=function(e){var t=this.getNavTarget();null!==t&&"object"==typeof t&&t.each(function(){var t=i(this).slick("getSlick");t.unslicked||t.slideHandler(e,!0)})},e.prototype.applyTransition=function(i){var e=this,t={};!1===e.options.fade?t[e.transitionType]=e.transformType+" "+e.options.speed+"ms "+e.options.cssEase:t[e.transitionType]="opacity "+e.options.speed+"ms "+e.options.cssEase,!1===e.options.fade?e.$slideTrack.css(t):e.$slides.eq(i).css(t)},e.prototype.autoPlay=function(){var i=this;i.autoPlayClear(),i.slideCount>i.options.slidesToShow&&(i.autoPlayTimer=setInterval(i.autoPlayIterator,i.options.autoplaySpeed))},e.prototype.autoPlayClear=function(){var i=this;i.autoPlayTimer&&clearInterval(i.autoPlayTimer)},e.prototype.autoPlayIterator=function(){var i=this,e=i.currentSlide+i.options.slidesToScroll;i.paused||i.interrupted||i.focussed||(!1===i.options.infinite&&(1===i.direction&&i.currentSlide+1===i.slideCount-1?i.direction=0:0===i.direction&&(e=i.currentSlide-i.options.slidesToScroll,i.currentSlide-1==0&&(i.direction=1))),i.slideHandler(e))},e.prototype.buildArrows=function(){var e=this;!0===e.options.arrows&&(e.$prevArrow=i(e.options.prevArrow).addClass("slick-arrow"),e.$nextArrow=i(e.options.nextArrow).addClass("slick-arrow"),e.slideCount>e.options.slidesToShow?(e.$prevArrow.removeClass("slick-hidden").removeAttr("aria-hidden tabindex"),e.$nextArrow.removeClass("slick-hidden").removeAttr("aria-hidden tabindex"),e.htmlExpr.test(e.options.prevArrow)&&e.$prevArrow.prependTo(e.options.appendArrows),e.htmlExpr.test(e.options.nextArrow)&&e.$nextArrow.appendTo(e.options.appendArrows),!0!==e.options.infinite&&e.$prevArrow.addClass("slick-disabled").attr("aria-disabled","true")):e.$prevArrow.add(e.$nextArrow).addClass("slick-hidden").attr({"aria-disabled":"true",tabindex:"-1"}))},e.prototype.buildDots=function(){var e,t,o=this;if(!0===o.options.dots){for(o.$slider.addClass("slick-dotted"),t=i("<ul />").addClass(o.options.dotsClass),e=0;e<=o.getDotCount();e+=1)t.append(i("<li />").append(o.options.customPaging.call(this,o,e)));o.$dots=t.appendTo(o.options.appendDots),o.$dots.find("li").first().addClass("slick-active")}},e.prototype.buildOut=function(){var e=this;e.$slides=e.$slider.children(e.options.slide+":not(.slick-cloned)").addClass("slick-slide"),e.slideCount=e.$slides.length,e.$slides.each(function(e,t){i(t).attr("data-slick-index",e).data("originalStyling",i(t).attr("style")||"")}),e.$slider.addClass("slick-slider"),e.$slideTrack=0===e.slideCount?i('<div class="slick-track"/>').appendTo(e.$slider):e.$slides.wrapAll('<div class="slick-track"/>').parent(),e.$list=e.$slideTrack.wrap('<div class="slick-list"/>').parent(),e.$slideTrack.css("opacity",0),!0!==e.options.centerMode&&!0!==e.options.swipeToSlide||(e.options.slidesToScroll=1),i("img[data-lazy]",e.$slider).not("[src]").addClass("slick-loading"),e.setupInfinite(),e.buildArrows(),e.buildDots(),e.updateDots(),e.setSlideClasses("number"==typeof e.currentSlide?e.currentSlide:0),!0===e.options.draggable&&e.$list.addClass("draggable")},e.prototype.buildRows=function(){var i,e,t,o,s,n,r,l=this;if(o=document.createDocumentFragment(),n=l.$slider.children(),l.options.rows>1){for(r=l.options.slidesPerRow*l.options.rows,s=Math.ceil(n.length/r),i=0;i<s;i++){var d=document.createElement("div");for(e=0;e<l.options.rows;e++){var a=document.createElement("div");for(t=0;t<l.options.slidesPerRow;t++){var c=i*r+(e*l.options.slidesPerRow+t);n.get(c)&&a.appendChild(n.get(c))}d.appendChild(a)}o.appendChild(d)}l.$slider.empty().append(o),l.$slider.children().children().children().css({width:100/l.options.slidesPerRow+"%",display:"inline-block"})}},e.prototype.checkResponsive=function(e,t){var o,s,n,r=this,l=!1,d=r.$slider.width(),a=window.innerWidth||i(window).width();if("window"===r.respondTo?n=a:"slider"===r.respondTo?n=d:"min"===r.respondTo&&(n=Math.min(a,d)),r.options.responsive&&r.options.responsive.length&&null!==r.options.responsive){s=null;for(o in r.breakpoints)r.breakpoints.hasOwnProperty(o)&&(!1===r.originalSettings.mobileFirst?n<r.breakpoints[o]&&(s=r.breakpoints[o]):n>r.breakpoints[o]&&(s=r.breakpoints[o]));null!==s?null!==r.activeBreakpoint?(s!==r.activeBreakpoint||t)&&(r.activeBreakpoint=s,"unslick"===r.breakpointSettings[s]?r.unslick(s):(r.options=i.extend({},r.originalSettings,r.breakpointSettings[s]),!0===e&&(r.currentSlide=r.options.initialSlide),r.refresh(e)),l=s):(r.activeBreakpoint=s,"unslick"===r.breakpointSettings[s]?r.unslick(s):(r.options=i.extend({},r.originalSettings,r.breakpointSettings[s]),!0===e&&(r.currentSlide=r.options.initialSlide),r.refresh(e)),l=s):null!==r.activeBreakpoint&&(r.activeBreakpoint=null,r.options=r.originalSettings,!0===e&&(r.currentSlide=r.options.initialSlide),r.refresh(e),l=s),e||!1===l||r.$slider.trigger("breakpoint",[r,l])}},e.prototype.changeSlide=function(e,t){var o,s,n,r=this,l=i(e.currentTarget);switch(l.is("a")&&e.preventDefault(),l.is("li")||(l=l.closest("li")),n=r.slideCount%r.options.slidesToScroll!=0,o=n?0:(r.slideCount-r.currentSlide)%r.options.slidesToScroll,e.data.message){case"previous":s=0===o?r.options.slidesToScroll:r.options.slidesToShow-o,r.slideCount>r.options.slidesToShow&&r.slideHandler(r.currentSlide-s,!1,t);break;case"next":s=0===o?r.options.slidesToScroll:o,r.slideCount>r.options.slidesToShow&&r.slideHandler(r.currentSlide+s,!1,t);break;case"index":var d=0===e.data.index?0:e.data.index||l.index()*r.options.slidesToScroll;r.slideHandler(r.checkNavigable(d),!1,t),l.children().trigger("focus");break;default:return}},e.prototype.checkNavigable=function(i){var e,t;if(e=this.getNavigableIndexes(),t=0,i>e[e.length-1])i=e[e.length-1];else for(var o in e){if(i<e[o]){i=t;break}t=e[o]}return i},e.prototype.cleanUpEvents=function(){var e=this;e.options.dots&&null!==e.$dots&&(i("li",e.$dots).off("click.slick",e.changeSlide).off("mouseenter.slick",i.proxy(e.interrupt,e,!0)).off("mouseleave.slick",i.proxy(e.interrupt,e,!1)),!0===e.options.accessibility&&e.$dots.off("keydown.slick",e.keyHandler)),e.$slider.off("focus.slick blur.slick"),!0===e.options.arrows&&e.slideCount>e.options.slidesToShow&&(e.$prevArrow&&e.$prevArrow.off("click.slick",e.changeSlide),e.$nextArrow&&e.$nextArrow.off("click.slick",e.changeSlide),!0===e.options.accessibility&&(e.$prevArrow&&e.$prevArrow.off("keydown.slick",e.keyHandler),e.$nextArrow&&e.$nextArrow.off("keydown.slick",e.keyHandler))),e.$list.off("touchstart.slick mousedown.slick",e.swipeHandler),e.$list.off("touchmove.slick mousemove.slick",e.swipeHandler),e.$list.off("touchend.slick mouseup.slick",e.swipeHandler),e.$list.off("touchcancel.slick mouseleave.slick",e.swipeHandler),e.$list.off("click.slick",e.clickHandler),i(document).off(e.visibilityChange,e.visibility),e.cleanUpSlideEvents(),!0===e.options.accessibility&&e.$list.off("keydown.slick",e.keyHandler),!0===e.options.focusOnSelect&&i(e.$slideTrack).children().off("click.slick",e.selectHandler),i(window).off("orientationchange.slick.slick-"+e.instanceUid,e.orientationChange),i(window).off("resize.slick.slick-"+e.instanceUid,e.resize),i("[draggable!=true]",e.$slideTrack).off("dragstart",e.preventDefault),i(window).off("load.slick.slick-"+e.instanceUid,e.setPosition)},e.prototype.cleanUpSlideEvents=function(){var e=this;e.$list.off("mouseenter.slick",i.proxy(e.interrupt,e,!0)),e.$list.off("mouseleave.slick",i.proxy(e.interrupt,e,!1))},e.prototype.cleanUpRows=function(){var i,e=this;e.options.rows>1&&((i=e.$slides.children().children()).removeAttr("style"),e.$slider.empty().append(i))},e.prototype.clickHandler=function(i){!1===this.shouldClick&&(i.stopImmediatePropagation(),i.stopPropagation(),i.preventDefault())},e.prototype.destroy=function(e){var t=this;t.autoPlayClear(),t.touchObject={},t.cleanUpEvents(),i(".slick-cloned",t.$slider).detach(),t.$dots&&t.$dots.remove(),t.$prevArrow&&t.$prevArrow.length&&(t.$prevArrow.removeClass("slick-disabled slick-arrow slick-hidden").removeAttr("aria-hidden aria-disabled tabindex").css("display",""),t.htmlExpr.test(t.options.prevArrow)&&t.$prevArrow.remove()),t.$nextArrow&&t.$nextArrow.length&&(t.$nextArrow.removeClass("slick-disabled slick-arrow slick-hidden").removeAttr("aria-hidden aria-disabled tabindex").css("display",""),t.htmlExpr.test(t.options.nextArrow)&&t.$nextArrow.remove()),t.$slides&&(t.$slides.removeClass("slick-slide slick-active slick-center slick-visible slick-current").removeAttr("aria-hidden").removeAttr("data-slick-index").each(function(){i(this).attr("style",i(this).data("originalStyling"))}),t.$slideTrack.children(this.options.slide).detach(),t.$slideTrack.detach(),t.$list.detach(),t.$slider.append(t.$slides)),t.cleanUpRows(),t.$slider.removeClass("slick-slider"),t.$slider.removeClass("slick-initialized"),t.$slider.removeClass("slick-dotted"),t.unslicked=!0,e||t.$slider.trigger("destroy",[t])},e.prototype.disableTransition=function(i){var e=this,t={};t[e.transitionType]="",!1===e.options.fade?e.$slideTrack.css(t):e.$slides.eq(i).css(t)},e.prototype.fadeSlide=function(i,e){var t=this;!1===t.cssTransitions?(t.$slides.eq(i).css({zIndex:t.options.zIndex}),t.$slides.eq(i).animate({opacity:1},t.options.speed,t.options.easing,e)):(t.applyTransition(i),t.$slides.eq(i).css({opacity:1,zIndex:t.options.zIndex}),e&&setTimeout(function(){t.disableTransition(i),e.call()},t.options.speed))},e.prototype.fadeSlideOut=function(i){var e=this;!1===e.cssTransitions?e.$slides.eq(i).animate({opacity:0,zIndex:e.options.zIndex-2},e.options.speed,e.options.easing):(e.applyTransition(i),e.$slides.eq(i).css({opacity:0,zIndex:e.options.zIndex-2}))},e.prototype.filterSlides=e.prototype.slickFilter=function(i){var e=this;null!==i&&(e.$slidesCache=e.$slides,e.unload(),e.$slideTrack.children(this.options.slide).detach(),e.$slidesCache.filter(i).appendTo(e.$slideTrack),e.reinit())},e.prototype.focusHandler=function(){var e=this;e.$slider.off("focus.slick blur.slick").on("focus.slick blur.slick","*",function(t){t.stopImmediatePropagation();var o=i(this);setTimeout(function(){e.options.pauseOnFocus&&(e.focussed=o.is(":focus"),e.autoPlay())},0)})},e.prototype.getCurrent=e.prototype.slickCurrentSlide=function(){return this.currentSlide},e.prototype.getDotCount=function(){var i=this,e=0,t=0,o=0;if(!0===i.options.infinite)if(i.slideCount<=i.options.slidesToShow)++o;else for(;e<i.slideCount;)++o,e=t+i.options.slidesToScroll,t+=i.options.slidesToScroll<=i.options.slidesToShow?i.options.slidesToScroll:i.options.slidesToShow;else if(!0===i.options.centerMode)o=i.slideCount;else if(i.options.asNavFor)for(;e<i.slideCount;)++o,e=t+i.options.slidesToScroll,t+=i.options.slidesToScroll<=i.options.slidesToShow?i.options.slidesToScroll:i.options.slidesToShow;else o=1+Math.ceil((i.slideCount-i.options.slidesToShow)/i.options.slidesToScroll);return o-1},e.prototype.getLeft=function(i){var e,t,o,s,n=this,r=0;return n.slideOffset=0,t=n.$slides.first().outerHeight(!0),!0===n.options.infinite?(n.slideCount>n.options.slidesToShow&&(n.slideOffset=n.slideWidth*n.options.slidesToShow*-1,s=-1,!0===n.options.vertical&&!0===n.options.centerMode&&(2===n.options.slidesToShow?s=-1.5:1===n.options.slidesToShow&&(s=-2)),r=t*n.options.slidesToShow*s),n.slideCount%n.options.slidesToScroll!=0&&i+n.options.slidesToScroll>n.slideCount&&n.slideCount>n.options.slidesToShow&&(i>n.slideCount?(n.slideOffset=(n.options.slidesToShow-(i-n.slideCount))*n.slideWidth*-1,r=(n.options.slidesToShow-(i-n.slideCount))*t*-1):(n.slideOffset=n.slideCount%n.options.slidesToScroll*n.slideWidth*-1,r=n.slideCount%n.options.slidesToScroll*t*-1))):i+n.options.slidesToShow>n.slideCount&&(n.slideOffset=(i+n.options.slidesToShow-n.slideCount)*n.slideWidth,r=(i+n.options.slidesToShow-n.slideCount)*t),n.slideCount<=n.options.slidesToShow&&(n.slideOffset=0,r=0),!0===n.options.centerMode&&n.slideCount<=n.options.slidesToShow?n.slideOffset=n.slideWidth*Math.floor(n.options.slidesToShow)/2-n.slideWidth*n.slideCount/2:!0===n.options.centerMode&&!0===n.options.infinite?n.slideOffset+=n.slideWidth*Math.floor(n.options.slidesToShow/2)-n.slideWidth:!0===n.options.centerMode&&(n.slideOffset=0,n.slideOffset+=n.slideWidth*Math.floor(n.options.slidesToShow/2)),e=!1===n.options.vertical?i*n.slideWidth*-1+n.slideOffset:i*t*-1+r,!0===n.options.variableWidth&&(o=n.slideCount<=n.options.slidesToShow||!1===n.options.infinite?n.$slideTrack.children(".slick-slide").eq(i):n.$slideTrack.children(".slick-slide").eq(i+n.options.slidesToShow),e=!0===n.options.rtl?o[0]?-1*(n.$slideTrack.width()-o[0].offsetLeft-o.width()):0:o[0]?-1*o[0].offsetLeft:0,!0===n.options.centerMode&&(o=n.slideCount<=n.options.slidesToShow||!1===n.options.infinite?n.$slideTrack.children(".slick-slide").eq(i):n.$slideTrack.children(".slick-slide").eq(i+n.options.slidesToShow+1),e=!0===n.options.rtl?o[0]?-1*(n.$slideTrack.width()-o[0].offsetLeft-o.width()):0:o[0]?-1*o[0].offsetLeft:0,e+=(n.$list.width()-o.outerWidth())/2)),e},e.prototype.getOption=e.prototype.slickGetOption=function(i){return this.options[i]},e.prototype.getNavigableIndexes=function(){var i,e=this,t=0,o=0,s=[];for(!1===e.options.infinite?i=e.slideCount:(t=-1*e.options.slidesToScroll,o=-1*e.options.slidesToScroll,i=2*e.slideCount);t<i;)s.push(t),t=o+e.options.slidesToScroll,o+=e.options.slidesToScroll<=e.options.slidesToShow?e.options.slidesToScroll:e.options.slidesToShow;return s},e.prototype.getSlick=function(){return this},e.prototype.getSlideCount=function(){var e,t,o=this;return t=!0===o.options.centerMode?o.slideWidth*Math.floor(o.options.slidesToShow/2):0,!0===o.options.swipeToSlide?(o.$slideTrack.find(".slick-slide").each(function(s,n){if(n.offsetLeft-t+i(n).outerWidth()/2>-1*o.swipeLeft)return e=n,!1}),Math.abs(i(e).attr("data-slick-index")-o.currentSlide)||1):o.options.slidesToScroll},e.prototype.goTo=e.prototype.slickGoTo=function(i,e){this.changeSlide({data:{message:"index",index:parseInt(i)}},e)},e.prototype.init=function(e){var t=this;i(t.$slider).hasClass("slick-initialized")||(i(t.$slider).addClass("slick-initialized"),t.buildRows(),t.buildOut(),t.setProps(),t.startLoad(),t.loadSlider(),t.initializeEvents(),t.updateArrows(),t.updateDots(),t.checkResponsive(!0),t.focusHandler()),e&&t.$slider.trigger("init",[t]),!0===t.options.accessibility&&t.initADA(),t.options.autoplay&&(t.paused=!1,t.autoPlay())},e.prototype.initADA=function(){var e=this,t=Math.ceil(e.slideCount/e.options.slidesToShow),o=e.getNavigableIndexes().filter(function(i){return i>=0&&i<e.slideCount});e.$slides.add(e.$slideTrack.find(".slick-cloned")).attr({"aria-hidden":"true",tabindex:"-1"}).find("a, input, button, select").attr({tabindex:"-1"}),null!==e.$dots&&(e.$slides.not(e.$slideTrack.find(".slick-cloned")).each(function(t){var s=o.indexOf(t);i(this).attr({role:"tabpanel",id:"slick-slide"+e.instanceUid+t,tabindex:-1}),-1!==s&&i(this).attr({"aria-describedby":"slick-slide-control"+e.instanceUid+s})}),e.$dots.attr("role","tablist").find("li").each(function(s){var n=o[s];i(this).attr({role:"presentation"}),i(this).find("button").first().attr({role:"tab",id:"slick-slide-control"+e.instanceUid+s,"aria-controls":"slick-slide"+e.instanceUid+n,"aria-label":s+1+" of "+t,"aria-selected":null,tabindex:"-1"})}).eq(e.currentSlide).find("button").attr({"aria-selected":"true",tabindex:"0"}).end());for(var s=e.currentSlide,n=s+e.options.slidesToShow;s<n;s++)e.$slides.eq(s).attr("tabindex",0);e.activateADA()},e.prototype.initArrowEvents=function(){var i=this;!0===i.options.arrows&&i.slideCount>i.options.slidesToShow&&(i.$prevArrow.off("click.slick").on("click.slick",{message:"previous"},i.changeSlide),i.$nextArrow.off("click.slick").on("click.slick",{message:"next"},i.changeSlide),!0===i.options.accessibility&&(i.$prevArrow.on("keydown.slick",i.keyHandler),i.$nextArrow.on("keydown.slick",i.keyHandler)))},e.prototype.initDotEvents=function(){var e=this;!0===e.options.dots&&(i("li",e.$dots).on("click.slick",{message:"index"},e.changeSlide),!0===e.options.accessibility&&e.$dots.on("keydown.slick",e.keyHandler)),!0===e.options.dots&&!0===e.options.pauseOnDotsHover&&i("li",e.$dots).on("mouseenter.slick",i.proxy(e.interrupt,e,!0)).on("mouseleave.slick",i.proxy(e.interrupt,e,!1))},e.prototype.initSlideEvents=function(){var e=this;e.options.pauseOnHover&&(e.$list.on("mouseenter.slick",i.proxy(e.interrupt,e,!0)),e.$list.on("mouseleave.slick",i.proxy(e.interrupt,e,!1)))},e.prototype.initializeEvents=function(){var e=this;e.initArrowEvents(),e.initDotEvents(),e.initSlideEvents(),e.$list.on("touchstart.slick mousedown.slick",{action:"start"},e.swipeHandler),e.$list.on("touchmove.slick mousemove.slick",{action:"move"},e.swipeHandler),e.$list.on("touchend.slick mouseup.slick",{action:"end"},e.swipeHandler),e.$list.on("touchcancel.slick mouseleave.slick",{action:"end"},e.swipeHandler),e.$list.on("click.slick",e.clickHandler),i(document).on(e.visibilityChange,i.proxy(e.visibility,e)),!0===e.options.accessibility&&e.$list.on("keydown.slick",e.keyHandler),!0===e.options.focusOnSelect&&i(e.$slideTrack).children().on("click.slick",e.selectHandler),i(window).on("orientationchange.slick.slick-"+e.instanceUid,i.proxy(e.orientationChange,e)),i(window).on("resize.slick.slick-"+e.instanceUid,i.proxy(e.resize,e)),i("[draggable!=true]",e.$slideTrack).on("dragstart",e.preventDefault),i(window).on("load.slick.slick-"+e.instanceUid,e.setPosition),i(e.setPosition)},e.prototype.initUI=function(){var i=this;!0===i.options.arrows&&i.slideCount>i.options.slidesToShow&&(i.$prevArrow.show(),i.$nextArrow.show()),!0===i.options.dots&&i.slideCount>i.options.slidesToShow&&i.$dots.show()},e.prototype.keyHandler=function(i){var e=this;i.target.tagName.match("TEXTAREA|INPUT|SELECT")||(37===i.keyCode&&!0===e.options.accessibility?e.changeSlide({data:{message:!0===e.options.rtl?"next":"previous"}}):39===i.keyCode&&!0===e.options.accessibility&&e.changeSlide({data:{message:!0===e.options.rtl?"previous":"next"}}))},e.prototype.lazyLoad=function(){function e(e){i("img[data-lazy]",e).each(function(){var e=i(this),t=i(this).attr("data-lazy"),o=i(this).attr("data-srcset"),s=i(this).attr("data-sizes")||n.$slider.attr("data-sizes"),r=document.createElement("img");r.onload=function(){e.animate({opacity:0},100,function(){o&&(e.attr("srcset",o),s&&e.attr("sizes",s)),e.attr("src",t).animate({opacity:1},200,function(){e.removeAttr("data-lazy data-srcset data-sizes").removeClass("slick-loading")}),n.$slider.trigger("lazyLoaded",[n,e,t])})},r.onerror=function(){e.removeAttr("data-lazy").removeClass("slick-loading").addClass("slick-lazyload-error"),n.$slider.trigger("lazyLoadError",[n,e,t])},r.src=t})}var t,o,s,n=this;if(!0===n.options.centerMode?!0===n.options.infinite?s=(o=n.currentSlide+(n.options.slidesToShow/2+1))+n.options.slidesToShow+2:(o=Math.max(0,n.currentSlide-(n.options.slidesToShow/2+1)),s=n.options.slidesToShow/2+1+2+n.currentSlide):(o=n.options.infinite?n.options.slidesToShow+n.currentSlide:n.currentSlide,s=Math.ceil(o+n.options.slidesToShow),!0===n.options.fade&&(o>0&&o--,s<=n.slideCount&&s++)),t=n.$slider.find(".slick-slide").slice(o,s),"anticipated"===n.options.lazyLoad)for(var r=o-1,l=s,d=n.$slider.find(".slick-slide"),a=0;a<n.options.slidesToScroll;a++)r<0&&(r=n.slideCount-1),t=(t=t.add(d.eq(r))).add(d.eq(l)),r--,l++;e(t),n.slideCount<=n.options.slidesToShow?e(n.$slider.find(".slick-slide")):n.currentSlide>=n.slideCount-n.options.slidesToShow?e(n.$slider.find(".slick-cloned").slice(0,n.options.slidesToShow)):0===n.currentSlide&&e(n.$slider.find(".slick-cloned").slice(-1*n.options.slidesToShow))},e.prototype.loadSlider=function(){var i=this;i.setPosition(),i.$slideTrack.css({opacity:1}),i.$slider.removeClass("slick-loading"),i.initUI(),"progressive"===i.options.lazyLoad&&i.progressiveLazyLoad()},e.prototype.next=e.prototype.slickNext=function(){this.changeSlide({data:{message:"next"}})},e.prototype.orientationChange=function(){var i=this;i.checkResponsive(),i.setPosition()},e.prototype.pause=e.prototype.slickPause=function(){var i=this;i.autoPlayClear(),i.paused=!0},e.prototype.play=e.prototype.slickPlay=function(){var i=this;i.autoPlay(),i.options.autoplay=!0,i.paused=!1,i.focussed=!1,i.interrupted=!1},e.prototype.postSlide=function(e){var t=this;t.unslicked||(t.$slider.trigger("afterChange",[t,e]),t.animating=!1,t.slideCount>t.options.slidesToShow&&t.setPosition(),t.swipeLeft=null,t.options.autoplay&&t.autoPlay(),!0===t.options.accessibility&&(t.initADA(),t.options.focusOnChange&&i(t.$slides.get(t.currentSlide)).attr("tabindex",0).focus()))},e.prototype.prev=e.prototype.slickPrev=function(){this.changeSlide({data:{message:"previous"}})},e.prototype.preventDefault=function(i){i.preventDefault()},e.prototype.progressiveLazyLoad=function(e){e=e||1;var t,o,s,n,r,l=this,d=i("img[data-lazy]",l.$slider);d.length?(t=d.first(),o=t.attr("data-lazy"),s=t.attr("data-srcset"),n=t.attr("data-sizes")||l.$slider.attr("data-sizes"),(r=document.createElement("img")).onload=function(){s&&(t.attr("srcset",s),n&&t.attr("sizes",n)),t.attr("src",o).removeAttr("data-lazy data-srcset data-sizes").removeClass("slick-loading"),!0===l.options.adaptiveHeight&&l.setPosition(),l.$slider.trigger("lazyLoaded",[l,t,o]),l.progressiveLazyLoad()},r.onerror=function(){e<3?setTimeout(function(){l.progressiveLazyLoad(e+1)},500):(t.removeAttr("data-lazy").removeClass("slick-loading").addClass("slick-lazyload-error"),l.$slider.trigger("lazyLoadError",[l,t,o]),l.progressiveLazyLoad())},r.src=o):l.$slider.trigger("allImagesLoaded",[l])},e.prototype.refresh=function(e){var t,o,s=this;o=s.slideCount-s.options.slidesToShow,!s.options.infinite&&s.currentSlide>o&&(s.currentSlide=o),s.slideCount<=s.options.slidesToShow&&(s.currentSlide=0),t=s.currentSlide,s.destroy(!0),i.extend(s,s.initials,{currentSlide:t}),s.init(),e||s.changeSlide({data:{message:"index",index:t}},!1)},e.prototype.registerBreakpoints=function(){var e,t,o,s=this,n=s.options.responsive||null;if("array"===i.type(n)&&n.length){s.respondTo=s.options.respondTo||"window";for(e in n)if(o=s.breakpoints.length-1,n.hasOwnProperty(e)){for(t=n[e].breakpoint;o>=0;)s.breakpoints[o]&&s.breakpoints[o]===t&&s.breakpoints.splice(o,1),o--;s.breakpoints.push(t),s.breakpointSettings[t]=n[e].settings}s.breakpoints.sort(function(i,e){return s.options.mobileFirst?i-e:e-i})}},e.prototype.reinit=function(){var e=this;e.$slides=e.$slideTrack.children(e.options.slide).addClass("slick-slide"),e.slideCount=e.$slides.length,e.currentSlide>=e.slideCount&&0!==e.currentSlide&&(e.currentSlide=e.currentSlide-e.options.slidesToScroll),e.slideCount<=e.options.slidesToShow&&(e.currentSlide=0),e.registerBreakpoints(),e.setProps(),e.setupInfinite(),e.buildArrows(),e.updateArrows(),e.initArrowEvents(),e.buildDots(),e.updateDots(),e.initDotEvents(),e.cleanUpSlideEvents(),e.initSlideEvents(),e.checkResponsive(!1,!0),!0===e.options.focusOnSelect&&i(e.$slideTrack).children().on("click.slick",e.selectHandler),e.setSlideClasses("number"==typeof e.currentSlide?e.currentSlide:0),e.setPosition(),e.focusHandler(),e.paused=!e.options.autoplay,e.autoPlay(),e.$slider.trigger("reInit",[e])},e.prototype.resize=function(){var e=this;i(window).width()!==e.windowWidth&&(clearTimeout(e.windowDelay),e.windowDelay=window.setTimeout(function(){e.windowWidth=i(window).width(),e.checkResponsive(),e.unslicked||e.setPosition()},50))},e.prototype.removeSlide=e.prototype.slickRemove=function(i,e,t){var o=this;if(i="boolean"==typeof i?!0===(e=i)?0:o.slideCount-1:!0===e?--i:i,o.slideCount<1||i<0||i>o.slideCount-1)return!1;o.unload(),!0===t?o.$slideTrack.children().remove():o.$slideTrack.children(this.options.slide).eq(i).remove(),o.$slides=o.$slideTrack.children(this.options.slide),o.$slideTrack.children(this.options.slide).detach(),o.$slideTrack.append(o.$slides),o.$slidesCache=o.$slides,o.reinit()},e.prototype.setCSS=function(i){var e,t,o=this,s={};!0===o.options.rtl&&(i=-i),e="left"==o.positionProp?Math.ceil(i)+"px":"0px",t="top"==o.positionProp?Math.ceil(i)+"px":"0px",s[o.positionProp]=i,!1===o.transformsEnabled?o.$slideTrack.css(s):(s={},!1===o.cssTransitions?(s[o.animType]="translate("+e+", "+t+")",o.$slideTrack.css(s)):(s[o.animType]="translate3d("+e+", "+t+", 0px)",o.$slideTrack.css(s)))},e.prototype.setDimensions=function(){var i=this;!1===i.options.vertical?!0===i.options.centerMode&&i.$list.css({padding:"0px "+i.options.centerPadding}):(i.$list.height(i.$slides.first().outerHeight(!0)*i.options.slidesToShow),!0===i.options.centerMode&&i.$list.css({padding:i.options.centerPadding+" 0px"})),i.listWidth=i.$list.width(),i.listHeight=i.$list.height(),!1===i.options.vertical&&!1===i.options.variableWidth?(i.slideWidth=Math.ceil(i.listWidth/i.options.slidesToShow),i.$slideTrack.width(Math.ceil(i.slideWidth*i.$slideTrack.children(".slick-slide").length))):!0===i.options.variableWidth?i.$slideTrack.width(5e3*i.slideCount):(i.slideWidth=Math.ceil(i.listWidth),i.$slideTrack.height(Math.ceil(i.$slides.first().outerHeight(!0)*i.$slideTrack.children(".slick-slide").length)));var e=i.$slides.first().outerWidth(!0)-i.$slides.first().width();!1===i.options.variableWidth&&i.$slideTrack.children(".slick-slide").width(i.slideWidth-e)},e.prototype.setFade=function(){var e,t=this;t.$slides.each(function(o,s){e=t.slideWidth*o*-1,!0===t.options.rtl?i(s).css({position:"relative",right:e,top:0,zIndex:t.options.zIndex-2,opacity:0}):i(s).css({position:"relative",left:e,top:0,zIndex:t.options.zIndex-2,opacity:0})}),t.$slides.eq(t.currentSlide).css({zIndex:t.options.zIndex-1,opacity:1})},e.prototype.setHeight=function(){var i=this;if(1===i.options.slidesToShow&&!0===i.options.adaptiveHeight&&!1===i.options.vertical){var e=i.$slides.eq(i.currentSlide).outerHeight(!0);i.$list.css("height",e)}},e.prototype.setOption=e.prototype.slickSetOption=function(){var e,t,o,s,n,r=this,l=!1;if("object"===i.type(arguments[0])?(o=arguments[0],l=arguments[1],n="multiple"):"string"===i.type(arguments[0])&&(o=arguments[0],s=arguments[1],l=arguments[2],"responsive"===arguments[0]&&"array"===i.type(arguments[1])?n="responsive":void 0!==arguments[1]&&(n="single")),"single"===n)r.options[o]=s;else if("multiple"===n)i.each(o,function(i,e){r.options[i]=e});else if("responsive"===n)for(t in s)if("array"!==i.type(r.options.responsive))r.options.responsive=[s[t]];else{for(e=r.options.responsive.length-1;e>=0;)r.options.responsive[e].breakpoint===s[t].breakpoint&&r.options.responsive.splice(e,1),e--;r.options.responsive.push(s[t])}l&&(r.unload(),r.reinit())},e.prototype.setPosition=function(){var i=this;i.setDimensions(),i.setHeight(),!1===i.options.fade?i.setCSS(i.getLeft(i.currentSlide)):i.setFade(),i.$slider.trigger("setPosition",[i])},e.prototype.setProps=function(){var i=this,e=document.body.style;i.positionProp=!0===i.options.vertical?"top":"left","top"===i.positionProp?i.$slider.addClass("slick-vertical"):i.$slider.removeClass("slick-vertical"),void 0===e.WebkitTransition&&void 0===e.MozTransition&&void 0===e.msTransition||!0===i.options.useCSS&&(i.cssTransitions=!0),i.options.fade&&("number"==typeof i.options.zIndex?i.options.zIndex<3&&(i.options.zIndex=3):i.options.zIndex=i.defaults.zIndex),void 0!==e.OTransform&&(i.animType="OTransform",i.transformType="-o-transform",i.transitionType="OTransition",void 0===e.perspectiveProperty&&void 0===e.webkitPerspective&&(i.animType=!1)),void 0!==e.MozTransform&&(i.animType="MozTransform",i.transformType="-moz-transform",i.transitionType="MozTransition",void 0===e.perspectiveProperty&&void 0===e.MozPerspective&&(i.animType=!1)),void 0!==e.webkitTransform&&(i.animType="webkitTransform",i.transformType="-webkit-transform",i.transitionType="webkitTransition",void 0===e.perspectiveProperty&&void 0===e.webkitPerspective&&(i.animType=!1)),void 0!==e.msTransform&&(i.animType="msTransform",i.transformType="-ms-transform",i.transitionType="msTransition",void 0===e.msTransform&&(i.animType=!1)),void 0!==e.transform&&!1!==i.animType&&(i.animType="transform",i.transformType="transform",i.transitionType="transition"),i.transformsEnabled=i.options.useTransform&&null!==i.animType&&!1!==i.animType},e.prototype.setSlideClasses=function(i){var e,t,o,s,n=this;if(t=n.$slider.find(".slick-slide").removeClass("slick-active slick-center slick-current").attr("aria-hidden","true"),n.$slides.eq(i).addClass("slick-current"),!0===n.options.centerMode){var r=n.options.slidesToShow%2==0?1:0;e=Math.floor(n.options.slidesToShow/2),!0===n.options.infinite&&(i>=e&&i<=n.slideCount-1-e?n.$slides.slice(i-e+r,i+e+1).addClass("slick-active").attr("aria-hidden","false"):(o=n.options.slidesToShow+i,t.slice(o-e+1+r,o+e+2).addClass("slick-active").attr("aria-hidden","false")),0===i?t.eq(t.length-1-n.options.slidesToShow).addClass("slick-center"):i===n.slideCount-1&&t.eq(n.options.slidesToShow).addClass("slick-center")),n.$slides.eq(i).addClass("slick-center")}else i>=0&&i<=n.slideCount-n.options.slidesToShow?n.$slides.slice(i,i+n.options.slidesToShow).addClass("slick-active").attr("aria-hidden","false"):t.length<=n.options.slidesToShow?t.addClass("slick-active").attr("aria-hidden","false"):(s=n.slideCount%n.options.slidesToShow,o=!0===n.options.infinite?n.options.slidesToShow+i:i,n.options.slidesToShow==n.options.slidesToScroll&&n.slideCount-i<n.options.slidesToShow?t.slice(o-(n.options.slidesToShow-s),o+s).addClass("slick-active").attr("aria-hidden","false"):t.slice(o,o+n.options.slidesToShow).addClass("slick-active").attr("aria-hidden","false"));"ondemand"!==n.options.lazyLoad&&"anticipated"!==n.options.lazyLoad||n.lazyLoad()},e.prototype.setupInfinite=function(){var e,t,o,s=this;if(!0===s.options.fade&&(s.options.centerMode=!1),!0===s.options.infinite&&!1===s.options.fade&&(t=null,s.slideCount>s.options.slidesToShow)){for(o=!0===s.options.centerMode?s.options.slidesToShow+1:s.options.slidesToShow,e=s.slideCount;e>s.slideCount-o;e-=1)t=e-1,i(s.$slides[t]).clone(!0).attr("id","").attr("data-slick-index",t-s.slideCount).prependTo(s.$slideTrack).addClass("slick-cloned");for(e=0;e<o+s.slideCount;e+=1)t=e,i(s.$slides[t]).clone(!0).attr("id","").attr("data-slick-index",t+s.slideCount).appendTo(s.$slideTrack).addClass("slick-cloned");s.$slideTrack.find(".slick-cloned").find("[id]").each(function(){i(this).attr("id","")})}},e.prototype.interrupt=function(i){var e=this;i||e.autoPlay(),e.interrupted=i},e.prototype.selectHandler=function(e){var t=this,o=i(e.target).is(".slick-slide")?i(e.target):i(e.target).parents(".slick-slide"),s=parseInt(o.attr("data-slick-index"));s||(s=0),t.slideCount<=t.options.slidesToShow?t.slideHandler(s,!1,!0):t.slideHandler(s)},e.prototype.slideHandler=function(i,e,t){var o,s,n,r,l,d=null,a=this;if(e=e||!1,!(!0===a.animating&&!0===a.options.waitForAnimate||!0===a.options.fade&&a.currentSlide===i))if(!1===e&&a.asNavFor(i),o=i,d=a.getLeft(o),r=a.getLeft(a.currentSlide),a.currentLeft=null===a.swipeLeft?r:a.swipeLeft,!1===a.options.infinite&&!1===a.options.centerMode&&(i<0||i>a.getDotCount()*a.options.slidesToScroll))!1===a.options.fade&&(o=a.currentSlide,!0!==t?a.animateSlide(r,function(){a.postSlide(o)}):a.postSlide(o));else if(!1===a.options.infinite&&!0===a.options.centerMode&&(i<0||i>a.slideCount-a.options.slidesToScroll))!1===a.options.fade&&(o=a.currentSlide,!0!==t?a.animateSlide(r,function(){a.postSlide(o)}):a.postSlide(o));else{if(a.options.autoplay&&clearInterval(a.autoPlayTimer),s=o<0?a.slideCount%a.options.slidesToScroll!=0?a.slideCount-a.slideCount%a.options.slidesToScroll:a.slideCount+o:o>=a.slideCount?a.slideCount%a.options.slidesToScroll!=0?0:o-a.slideCount:o,a.animating=!0,a.$slider.trigger("beforeChange",[a,a.currentSlide,s]),n=a.currentSlide,a.currentSlide=s,a.setSlideClasses(a.currentSlide),a.options.asNavFor&&(l=(l=a.getNavTarget()).slick("getSlick")).slideCount<=l.options.slidesToShow&&l.setSlideClasses(a.currentSlide),a.updateDots(),a.updateArrows(),!0===a.options.fade)return!0!==t?(a.fadeSlideOut(n),a.fadeSlide(s,function(){a.postSlide(s)})):a.postSlide(s),void a.animateHeight();!0!==t?a.animateSlide(d,function(){a.postSlide(s)}):a.postSlide(s)}},e.prototype.startLoad=function(){var i=this;!0===i.options.arrows&&i.slideCount>i.options.slidesToShow&&(i.$prevArrow.hide(),i.$nextArrow.hide()),!0===i.options.dots&&i.slideCount>i.options.slidesToShow&&i.$dots.hide(),i.$slider.addClass("slick-loading")},e.prototype.swipeDirection=function(){var i,e,t,o,s=this;return i=s.touchObject.startX-s.touchObject.curX,e=s.touchObject.startY-s.touchObject.curY,t=Math.atan2(e,i),(o=Math.round(180*t/Math.PI))<0&&(o=360-Math.abs(o)),o<=45&&o>=0?!1===s.options.rtl?"left":"right":o<=360&&o>=315?!1===s.options.rtl?"left":"right":o>=135&&o<=225?!1===s.options.rtl?"right":"left":!0===s.options.verticalSwiping?o>=35&&o<=135?"down":"up":"vertical"},e.prototype.swipeEnd=function(i){var e,t,o=this;if(o.dragging=!1,o.swiping=!1,o.scrolling)return o.scrolling=!1,!1;if(o.interrupted=!1,o.shouldClick=!(o.touchObject.swipeLength>10),void 0===o.touchObject.curX)return!1;if(!0===o.touchObject.edgeHit&&o.$slider.trigger("edge",[o,o.swipeDirection()]),o.touchObject.swipeLength>=o.touchObject.minSwipe){switch(t=o.swipeDirection()){case"left":case"down":e=o.options.swipeToSlide?o.checkNavigable(o.currentSlide+o.getSlideCount()):o.currentSlide+o.getSlideCount(),o.currentDirection=0;break;case"right":case"up":e=o.options.swipeToSlide?o.checkNavigable(o.currentSlide-o.getSlideCount()):o.currentSlide-o.getSlideCount(),o.currentDirection=1}"vertical"!=t&&(o.slideHandler(e),o.touchObject={},o.$slider.trigger("swipe",[o,t]))}else o.touchObject.startX!==o.touchObject.curX&&(o.slideHandler(o.currentSlide),o.touchObject={})},e.prototype.swipeHandler=function(i){var e=this;if(!(!1===e.options.swipe||"ontouchend"in document&&!1===e.options.swipe||!1===e.options.draggable&&-1!==i.type.indexOf("mouse")))switch(e.touchObject.fingerCount=i.originalEvent&&void 0!==i.originalEvent.touches?i.originalEvent.touches.length:1,e.touchObject.minSwipe=e.listWidth/e.options.touchThreshold,!0===e.options.verticalSwiping&&(e.touchObject.minSwipe=e.listHeight/e.options.touchThreshold),i.data.action){case"start":e.swipeStart(i);break;case"move":e.swipeMove(i);break;case"end":e.swipeEnd(i)}},e.prototype.swipeMove=function(i){var e,t,o,s,n,r,l=this;return n=void 0!==i.originalEvent?i.originalEvent.touches:null,!(!l.dragging||l.scrolling||n&&1!==n.length)&&(e=l.getLeft(l.currentSlide),l.touchObject.curX=void 0!==n?n[0].pageX:i.clientX,l.touchObject.curY=void 0!==n?n[0].pageY:i.clientY,l.touchObject.swipeLength=Math.round(Math.sqrt(Math.pow(l.touchObject.curX-l.touchObject.startX,2))),r=Math.round(Math.sqrt(Math.pow(l.touchObject.curY-l.touchObject.startY,2))),!l.options.verticalSwiping&&!l.swiping&&r>4?(l.scrolling=!0,!1):(!0===l.options.verticalSwiping&&(l.touchObject.swipeLength=r),t=l.swipeDirection(),void 0!==i.originalEvent&&l.touchObject.swipeLength>4&&(l.swiping=!0,i.preventDefault()),s=(!1===l.options.rtl?1:-1)*(l.touchObject.curX>l.touchObject.startX?1:-1),!0===l.options.verticalSwiping&&(s=l.touchObject.curY>l.touchObject.startY?1:-1),o=l.touchObject.swipeLength,l.touchObject.edgeHit=!1,!1===l.options.infinite&&(0===l.currentSlide&&"right"===t||l.currentSlide>=l.getDotCount()&&"left"===t)&&(o=l.touchObject.swipeLength*l.options.edgeFriction,l.touchObject.edgeHit=!0),!1===l.options.vertical?l.swipeLeft=e+o*s:l.swipeLeft=e+o*(l.$list.height()/l.listWidth)*s,!0===l.options.verticalSwiping&&(l.swipeLeft=e+o*s),!0!==l.options.fade&&!1!==l.options.touchMove&&(!0===l.animating?(l.swipeLeft=null,!1):void l.setCSS(l.swipeLeft))))},e.prototype.swipeStart=function(i){var e,t=this;if(t.interrupted=!0,1!==t.touchObject.fingerCount||t.slideCount<=t.options.slidesToShow)return t.touchObject={},!1;void 0!==i.originalEvent&&void 0!==i.originalEvent.touches&&(e=i.originalEvent.touches[0]),t.touchObject.startX=t.touchObject.curX=void 0!==e?e.pageX:i.clientX,t.touchObject.startY=t.touchObject.curY=void 0!==e?e.pageY:i.clientY,t.dragging=!0},e.prototype.unfilterSlides=e.prototype.slickUnfilter=function(){var i=this;null!==i.$slidesCache&&(i.unload(),i.$slideTrack.children(this.options.slide).detach(),i.$slidesCache.appendTo(i.$slideTrack),i.reinit())},e.prototype.unload=function(){var e=this;i(".slick-cloned",e.$slider).remove(),e.$dots&&e.$dots.remove(),e.$prevArrow&&e.htmlExpr.test(e.options.prevArrow)&&e.$prevArrow.remove(),e.$nextArrow&&e.htmlExpr.test(e.options.nextArrow)&&e.$nextArrow.remove(),e.$slides.removeClass("slick-slide slick-active slick-visible slick-current").attr("aria-hidden","true").css("width","")},e.prototype.unslick=function(i){var e=this;e.$slider.trigger("unslick",[e,i]),e.destroy()},e.prototype.updateArrows=function(){var i=this;Math.floor(i.options.slidesToShow/2),!0===i.options.arrows&&i.slideCount>i.options.slidesToShow&&!i.options.infinite&&(i.$prevArrow.removeClass("slick-disabled").attr("aria-disabled","false"),i.$nextArrow.removeClass("slick-disabled").attr("aria-disabled","false"),0===i.currentSlide?(i.$prevArrow.addClass("slick-disabled").attr("aria-disabled","true"),i.$nextArrow.removeClass("slick-disabled").attr("aria-disabled","false")):i.currentSlide>=i.slideCount-i.options.slidesToShow&&!1===i.options.centerMode?(i.$nextArrow.addClass("slick-disabled").attr("aria-disabled","true"),i.$prevArrow.removeClass("slick-disabled").attr("aria-disabled","false")):i.currentSlide>=i.slideCount-1&&!0===i.options.centerMode&&(i.$nextArrow.addClass("slick-disabled").attr("aria-disabled","true"),i.$prevArrow.removeClass("slick-disabled").attr("aria-disabled","false")))},e.prototype.updateDots=function(){var i=this;null!==i.$dots&&(i.$dots.find("li").removeClass("slick-active").end(),i.$dots.find("li").eq(Math.floor(i.currentSlide/i.options.slidesToScroll)).addClass("slick-active"))},e.prototype.visibility=function(){var i=this;i.options.autoplay&&(document[i.hidden]?i.interrupted=!0:i.interrupted=!1)},i.fn.slick=function(){var i,t,o=this,s=arguments[0],n=Array.prototype.slice.call(arguments,1),r=o.length;for(i=0;i<r;i++)if("object"==typeof s||void 0===s?o[i].slick=new e(o[i],s):t=o[i].slick[s].apply(o[i].slick,n),void 0!==t)return t;return o}});

/* End */
;
; /* Start:"a:4:{s:4:"full";s:69:"/local/templates/.default/markup/new/js/datepicker.js?164206818467752";s:6:"source";s:53:"/local/templates/.default/markup/new/js/datepicker.js";s:3:"min";s:0:"";s:3:"map";s:0:"";}"*/
(function(window, $, undefined) {
  (function() {
    const VERSION = '2.2.3';
    const pluginName = 'esdatepicker';
    const autoInitSelector = '.esdatepicker-here';
    let $body;
    let $esdatepickersContainer;
    let containerBuilt = false;
    const baseTemplate = ''
      + '<div class="esdatepicker">'
      + '<i class="esdatepicker--pointer"></i>'
      + '<div class="esdatepicker--nav"></div>'
      + '<div class="esdatepicker--content"></div>'
      + '</div>';
    const defaults = {
      classes: '',
      inline: false,
      language: 'ru',
      startDate: new Date(),
      firstDay: '',
      weekends: [6, 0],
      dateFormat: '',
      altField: '',
      altFieldDateFormat: '@',
      toggleSelected: true,
      keyboardNav: true,

      position: 'bottom left',
      offset: 12,

      view: 'days',
      minView: 'days',

      showOtherMonths: true,
      selectOtherMonths: true,
      moveToOtherMonthsOnSelect: true,

      showOtherYears: true,
      selectOtherYears: true,
      moveToOtherYearsOnSelect: true,

      minDate: '',
      maxDate: '',
      disableNavWhenOutOfRange: true,

      multipleDates: false, // Boolean or Number
      multipleDatesSeparator: ',',
      range: false,

      todayButton: false,
      clearButton: false,

      showEvent: 'focus',
      autoClose: false,

      // navigation
      monthsField: 'monthsShort',
      // prevHtml: '<svg><path d="M 17,12 l -5,5 l 5,5"></path></svg>',
      // nextHtml: '<svg><path d="M 14,12 l 5,5 l -5,5"></path></svg>',
      prevHtml: '<span></span>',
      nextHtml: '<span></span>',
      navTitles: {
        days: 'MM, <i>yyyy</i>',
        months: 'yyyy',
        years: 'yyyy1 - yyyy2',
      },

      // estimepicker
      estimepicker: false,
      onlyTimepicker: false,
      dateTimeSeparator: ' ',
      timeFormat: '',
      minHours: 0,
      maxHours: 24,
      minMinutes: 0,
      maxMinutes: 59,
      hoursStep: 1,
      minutesStep: 1,

      // events
      onSelect: '',
      onShow: '',
      onHide: '',
      onChangeMonth: '',
      onChangeYear: '',
      onChangeDecade: '',
      onChangeView: '',
      onRenderCell: '',
    };
    const hotKeys = {
      ctrlRight: [17, 39],
      ctrlUp: [17, 38],
      ctrlLeft: [17, 37],
      ctrlDown: [17, 40],
      shiftRight: [16, 39],
      shiftUp: [16, 38],
      shiftLeft: [16, 37],
      shiftDown: [16, 40],
      altUp: [18, 38],
      altRight: [18, 39],
      altLeft: [18, 37],
      altDown: [18, 40],
      ctrlShiftUp: [16, 17, 38],
    };
    let esdatepicker;

    const Datepicker = function(el, options) {
      this.el = el;
      this.$el = $(el);

      this.opts = $.extend(true, {}, defaults, options, this.$el.data());

      if ($body == undefined) {
        $body = $('body');
      }

      if (!this.opts.startDate) {
        this.opts.startDate = new Date();
      }

      if (this.el.nodeName == 'INPUT') {
        this.elIsInput = true;
      }

      if (this.opts.altField) {
        this.$altField = typeof this.opts.altField === 'string' ? $(this.opts.altField) : this.opts.altField;
      }

      this.inited = false;
      this.visible = false;
      this.silent = false; // Need to prevent unnecessary rendering

      this.currentDate = this.opts.startDate;
      this.currentView = this.opts.view;
      this._createShortCuts();
      this.selectedDates = [];
      this.views = {};
      this.keys = [];
      this.minRange = '';
      this.maxRange = '';
      this._prevOnSelectValue = '';

      this.init();
    };

    esdatepicker = Datepicker;

    esdatepicker.prototype = {
      VERSION,
      viewIndexes: ['days', 'months', 'years'],

      init() {
        if (!containerBuilt && !this.opts.inline && this.elIsInput) {
          this._buildDatepickersContainer();
        }
        this._buildBaseHtml();
        this._defineLocale(this.opts.language);
        this._syncWithMinMaxDates();

        if (this.elIsInput) {
          if (!this.opts.inline) {
            // Set extra classes for proper transitions
            this._setPositionClasses(this.opts.position);
            this._bindEvents();
          }
          if (this.opts.keyboardNav && !this.opts.onlyTimepicker) {
            this._bindKeyboardEvents();
          }
          this.$esdatepicker.on('mousedown', this._onMouseDownDatepicker.bind(this));
          this.$esdatepicker.on('mouseup', this._onMouseUpDatepicker.bind(this));
        }

        if (this.opts.classes) {
          this.$esdatepicker.addClass(this.opts.classes);
        }

        if (this.opts.estimepicker) {
          this.estimepicker = new $.fn.esdatepicker.Timepicker(this, this.opts);
          this._bindTimepickerEvents();
        }

        if (this.opts.onlyTimepicker) {
          this.$esdatepicker.addClass('-only-estimepicker-');
        }

        this.views[this.currentView] = new $.fn.esdatepicker.Body(
          this,
          this.currentView,
          this.opts
        );
        this.views[this.currentView].show();
        this.nav = new $.fn.esdatepicker.Navigation(this, this.opts);
        this.view = this.currentView;

        this.$el.on('clickCell.adp', this._onClickCell.bind(this));
        this.$esdatepicker.on(
          'mouseenter',
          '.esdatepicker--cell',
          this._onMouseEnterCell.bind(this)
        );
        this.$esdatepicker.on(
          'mouseleave',
          '.esdatepicker--cell',
          this._onMouseLeaveCell.bind(this)
        );

        this.inited = true;
      },

      _createShortCuts() {
        this.minDate = this.opts.minDate ? this.opts.minDate : new Date(-8639999913600000);
        this.maxDate = this.opts.maxDate ? this.opts.maxDate : new Date(8639999913600000);
      },

      _bindEvents() {
        this.$el.on(`${this.opts.showEvent}.adp`, this._onShowEvent.bind(this));
        this.$el.on('mouseup.adp', this._onMouseUpEl.bind(this));
        this.$el.on('blur.adp', this._onBlur.bind(this));
        this.$el.on('keyup.adp', this._onKeyUpGeneral.bind(this));
        $(window).on('resize.adp', this._onResize.bind(this));
        $('body').on('mouseup.adp', this._onMouseUpBody.bind(this));
      },

      _bindKeyboardEvents() {
        this.$el.on('keydown.adp', this._onKeyDown.bind(this));
        this.$el.on('keyup.adp', this._onKeyUp.bind(this));
        this.$el.on('hotKey.adp', this._onHotKey.bind(this));
      },

      _bindTimepickerEvents() {
        this.$el.on('timeChange.adp', this._onTimeChange.bind(this));
      },

      isWeekend(day) {
        return this.opts.weekends.indexOf(day) !== -1;
      },

      _defineLocale(lang) {
        if (typeof lang === 'string') {
          this.loc = $.fn.esdatepicker.language[lang];
          if (!this.loc) {
            console.warn(
              `Can't find language "${lang}" in Datepicker.language, will use "ru" instead`
            );
            this.loc = $.extend(true, {}, $.fn.esdatepicker.language.ru);
          }

          this.loc = $.extend(
            true,
            {},
            $.fn.esdatepicker.language.ru,
            $.fn.esdatepicker.language[lang]
          );
        } else {
          this.loc = $.extend(true, {}, $.fn.esdatepicker.language.ru, lang);
        }

        if (this.opts.dateFormat) {
          this.loc.dateFormat = this.opts.dateFormat;
        }

        if (this.opts.timeFormat) {
          this.loc.timeFormat = this.opts.timeFormat;
        }

        if (this.opts.firstDay !== '') {
          this.loc.firstDay = this.opts.firstDay;
        }

        if (this.opts.estimepicker) {
          this.loc.dateFormat = [this.loc.dateFormat, this.loc.timeFormat].join(
            this.opts.dateTimeSeparator
          );
        }

        if (this.opts.onlyTimepicker) {
          this.loc.dateFormat = this.loc.timeFormat;
        }

        const boundary = this._getWordBoundaryRegExp;
        if (
          this.loc.timeFormat.match(boundary('aa'))
          || this.loc.timeFormat.match(boundary('AA'))
        ) {
          this.ampm = true;
        }
      },

      _buildDatepickersContainer() {
        containerBuilt = true;
        $body.append('<div class="esdatepickers-container" id="esdatepickers-container"></div>');
        $esdatepickersContainer = $('#esdatepickers-container');
      },

      _buildBaseHtml() {
        let $appendTarget;
        const $inline = $('<div class="esdatepicker-inline">');

        if (this.el.nodeName == 'INPUT') {
          if (!this.opts.inline) {
            $appendTarget = $esdatepickersContainer;
          } else {
            $appendTarget = $inline.insertAfter(this.$el);
          }
        } else {
          $appendTarget = $inline.appendTo(this.$el);
        }

        this.$esdatepicker = $(baseTemplate).appendTo($appendTarget);
        this.$content = $('.esdatepicker--content', this.$esdatepicker);
        this.$nav = $('.esdatepicker--nav', this.$esdatepicker);
      },

      _triggerOnChange() {
        if (!this.selectedDates.length) {
          // Prevent from triggering multiple onSelect callback with same argument (empty string) in IE10-11
          if (this._prevOnSelectValue === '') return;
          this._prevOnSelectValue = '';
          return this.opts.onSelect('', '', this);
        }

        const { selectedDates } = this;
        const parsedSelected = esdatepicker.getParsedDate(selectedDates[0]);
        let formattedDates;
        const _this = this;
        let dates = new Date(
          parsedSelected.year,
          parsedSelected.month,
          parsedSelected.date,
          parsedSelected.hours,
          parsedSelected.minutes
        );

        formattedDates = selectedDates
          .map(date => {
            return _this.formatDate(_this.loc.dateFormat, date);
          })
          .join(this.opts.multipleDatesSeparator);

        // Create new dates array, to separate it from original selectedDates
        if (this.opts.multipleDates || this.opts.range) {
          dates = selectedDates.map(date => {
            const parsedDate = esdatepicker.getParsedDate(date);
            return new Date(
              parsedDate.year,
              parsedDate.month,
              parsedDate.date,
              parsedDate.hours,
              parsedDate.minutes
            );
          });
        }

        this._prevOnSelectValue = formattedDates;
        this.opts.onSelect(formattedDates, dates, this);
      },

      next() {
        const d = this.parsedDate;
        const o = this.opts;
        switch (this.view) {
        case 'days':
          this.date = new Date(d.year, d.month + 1, 1);
          if (o.onChangeMonth) o.onChangeMonth(this.parsedDate.month, this.parsedDate.year);
          break;
        case 'months':
          this.date = new Date(d.year + 1, d.month, 1);
          if (o.onChangeYear) o.onChangeYear(this.parsedDate.year);
          break;
        case 'years':
          this.date = new Date(d.year + 10, 0, 1);
          if (o.onChangeDecade) o.onChangeDecade(this.curDecade);
          break;
        }
      },

      prev() {
        const d = this.parsedDate;
        const o = this.opts;
        switch (this.view) {
        case 'days':
          this.date = new Date(d.year, d.month - 1, 1);
          if (o.onChangeMonth) o.onChangeMonth(this.parsedDate.month, this.parsedDate.year);
          break;
        case 'months':
          this.date = new Date(d.year - 1, d.month, 1);
          if (o.onChangeYear) o.onChangeYear(this.parsedDate.year);
          break;
        case 'years':
          this.date = new Date(d.year - 10, 0, 1);
          if (o.onChangeDecade) o.onChangeDecade(this.curDecade);
          break;
        }
      },

      formatDate(string, date) {
        date = date || this.date;
        let result = string;
        const boundary = this._getWordBoundaryRegExp;
        const locale = this.loc;
        const leadingZero = esdatepicker.getLeadingZeroNum;
        const decade = esdatepicker.getDecade(date);
        const d = esdatepicker.getParsedDate(date);
        let { fullHours } = d;
        let { hours } = d;
        const ampm = string.match(boundary('aa')) || string.match(boundary('AA'));
        let dayPeriod = 'am';
        const replacer = this._replacer;
        let validHours;

        if (this.opts.estimepicker && this.estimepicker && ampm) {
          validHours = this.estimepicker._getValidHoursFromDate(date, ampm);
          fullHours = leadingZero(validHours.hours);
          hours = validHours.hours;
          dayPeriod = validHours.dayPeriod;
        }

        switch (true) {
        case /@/.test(result):
          result = result.replace(/@/, date.getTime());
        case /aa/.test(result):
          result = replacer(result, boundary('aa'), dayPeriod);
        case /AA/.test(result):
          result = replacer(result, boundary('AA'), dayPeriod.toUpperCase());
        case /dd/.test(result):
          result = replacer(result, boundary('dd'), d.fullDate);
        case /d/.test(result):
          result = replacer(result, boundary('d'), d.date);
        case /DD/.test(result):
          result = replacer(result, boundary('DD'), locale.days[d.day]);
        case /D/.test(result):
          result = replacer(result, boundary('D'), locale.daysShort[d.day]);
        case /mm/.test(result):
          result = replacer(result, boundary('mm'), d.fullMonth);
        case /m/.test(result):
          result = replacer(result, boundary('m'), d.month + 1);
        case /MM/.test(result):
          result = replacer(result, boundary('MM'), this.loc.months[d.month]);
        case /M/.test(result):
          result = replacer(result, boundary('M'), locale.monthsShort[d.month]);
        case /ii/.test(result):
          result = replacer(result, boundary('ii'), d.fullMinutes);
        case /i/.test(result):
          result = replacer(result, boundary('i'), d.minutes);
        case /hh/.test(result):
          result = replacer(result, boundary('hh'), fullHours);
        case /h/.test(result):
          result = replacer(result, boundary('h'), hours);
        case /yyyy/.test(result):
          result = replacer(result, boundary('yyyy'), d.year);
        case /yyyy1/.test(result):
          result = replacer(result, boundary('yyyy1'), decade[0]);
        case /yyyy2/.test(result):
          result = replacer(result, boundary('yyyy2'), decade[1]);
        case /yy/.test(result):
          result = replacer(result, boundary('yy'), d.year.toString().slice(-2));
        }

        return result;
      },

      _replacer(str, reg, data) {
        return str.replace(reg, (match, p1, p2, p3) => {
          return p1 + data + p3;
        });
      },

      _getWordBoundaryRegExp(sign) {
        const symbols = '\\s|\\.|-|/|\\\\|,|\\$|\\!|\\?|:|;';

        return new RegExp(`(^|>|${symbols})(${sign})($|<|${symbols})`, 'g');
      },

      selectDate(date) {
        const _this = this;
        const { opts } = _this;
        const d = _this.parsedDate;
        const { selectedDates } = _this;
        const len = selectedDates.length;
        let newDate = '';

        if (Array.isArray(date)) {
          date.forEach(d => {
            _this.selectDate(d);
          });
          return;
        }

        if (!(date instanceof Date)) return;

        this.lastSelectedDate = date;

        // Set new time values from Date
        if (this.estimepicker) {
          this.estimepicker._setTime(date);
        }

        // On this step estimepicker will set valid values in it's instance
        _this._trigger('selectDate', date);

        // Set correct time values after estimepicker's validation
        // Prevent from setting hours or minutes which values are lesser then `min` value or
        // greater then `max` value
        if (this.estimepicker) {
          date.setHours(this.estimepicker.hours);
          date.setMinutes(this.estimepicker.minutes);
        }

        if (_this.view == 'days') {
          if (date.getMonth() != d.month && opts.moveToOtherMonthsOnSelect) {
            newDate = new Date(date.getFullYear(), date.getMonth(), 1);
          }
        }

        if (_this.view == 'years') {
          if (date.getFullYear() != d.year && opts.moveToOtherYearsOnSelect) {
            newDate = new Date(date.getFullYear(), 0, 1);
          }
        }

        if (newDate) {
          _this.silent = true;
          _this.date = newDate;
          _this.silent = false;
          _this.nav._render();
        }

        if (opts.multipleDates && !opts.range) {
          // Set priority to range functionality
          if (len === opts.multipleDates) return;
          if (!_this._isSelected(date)) {
            _this.selectedDates.push(date);
          }
        } else if (opts.range) {
          if (len == 2) {
            _this.selectedDates = [date];
            _this.minRange = date;
            _this.maxRange = '';
          } else if (len == 1) {
            _this.selectedDates.push(date);
            if (!_this.maxRange) {
              _this.maxRange = date;
            } else {
              _this.minRange = date;
            }
            // Swap dates if they were selected via dp.selectDate() and second date was smaller then first
            if (esdatepicker.bigger(_this.maxRange, _this.minRange)) {
              _this.maxRange = _this.minRange;
              _this.minRange = date;
            }
            _this.selectedDates = [_this.minRange, _this.maxRange];
          } else {
            _this.selectedDates = [date];
            _this.minRange = date;
          }
        } else {
          _this.selectedDates = [date];
        }

        _this._setInputValue();

        if (opts.onSelect) {
          _this._triggerOnChange();
        }

        if (opts.autoClose && !this.estimepickerIsActive) {
          if (!opts.multipleDates && !opts.range) {
            _this.hide();
          } else if (opts.range && _this.selectedDates.length == 2) {
            _this.hide();
          }
        }

        _this.views[this.currentView]._render();
      },

      removeDate(date) {
        const selected = this.selectedDates;
        const _this = this;

        if (!(date instanceof Date)) return;

        return selected.some((curDate, i) => {
          if (esdatepicker.isSame(curDate, date)) {
            selected.splice(i, 1);

            if (!_this.selectedDates.length) {
              _this.minRange = '';
              _this.maxRange = '';
              _this.lastSelectedDate = '';
            } else {
              _this.lastSelectedDate = _this.selectedDates[_this.selectedDates.length - 1];
            }

            _this.views[_this.currentView]._render();
            _this._setInputValue();

            if (_this.opts.onSelect) {
              _this._triggerOnChange();
            }

            return true;
          }
        });
      },

      today() {
        this.silent = true;
        this.view = this.opts.minView;
        this.silent = false;
        this.date = new Date();

        if (this.opts.todayButton instanceof Date) {
          this.selectDate(this.opts.todayButton);
        }
      },

      clear() {
        this.selectedDates = [];
        this.minRange = '';
        this.maxRange = '';
        this.views[this.currentView]._render();
        this._setInputValue();
        if (this.opts.onSelect) {
          this._triggerOnChange();
        }
      },

      /**
       * Updates esdatepicker options
       * @param {String|Object} param - parameter's name to update. If object then it will extend current options
       * @param {String|Number|Object} [value] - new param value
       */
      update(param, value) {
        const len = arguments.length;
        const { lastSelectedDate } = this;

        if (len == 2) {
          this.opts[param] = value;
        } else if (len == 1 && typeof param === 'object') {
          this.opts = $.extend(true, this.opts, param);
        }

        this._createShortCuts();
        this._syncWithMinMaxDates();
        this._defineLocale(this.opts.language);
        this.nav._addButtonsIfNeed();
        if (!this.opts.onlyTimepicker) this.nav._render();
        this.views[this.currentView]._render();

        if (this.elIsInput && !this.opts.inline) {
          this._setPositionClasses(this.opts.position);
          if (this.visible) {
            this.setPosition(this.opts.position);
          }
        }

        if (this.opts.classes) {
          this.$esdatepicker.addClass(this.opts.classes);
        }

        if (this.opts.onlyTimepicker) {
          this.$esdatepicker.addClass('-only-estimepicker-');
        }

        if (this.opts.estimepicker) {
          if (lastSelectedDate) this.estimepicker._handleDate(lastSelectedDate);
          this.estimepicker._updateRanges();
          this.estimepicker._updateCurrentTime();
          // Change hours and minutes if it's values have been changed through min/max hours/minutes
          if (lastSelectedDate) {
            lastSelectedDate.setHours(this.estimepicker.hours);
            lastSelectedDate.setMinutes(this.estimepicker.minutes);
          }
        }

        this._setInputValue();

        return this;
      },

      _syncWithMinMaxDates() {
        const curTime = this.date.getTime();
        this.silent = true;
        if (this.minTime > curTime) {
          this.date = this.minDate;
        }

        if (this.maxTime < curTime) {
          this.date = this.maxDate;
        }
        this.silent = false;
      },

      _isSelected(checkDate, cellType) {
        let res = false;
        this.selectedDates.some(date => {
          if (esdatepicker.isSame(date, checkDate, cellType)) {
            res = date;
            return true;
          }
        });
        return res;
      },

      _setInputValue() {
        const _this = this;
        const { opts } = _this;
        const format = _this.loc.dateFormat;
        const altFormat = opts.altFieldDateFormat;
        let value = _this.selectedDates.map(date => {
          return _this.formatDate(format, date);
        });
        let altValues;

        if (opts.altField && _this.$altField.length) {
          altValues = this.selectedDates.map(date => {
            return _this.formatDate(altFormat, date);
          });
          altValues = altValues.join(this.opts.multipleDatesSeparator);
          this.$altField.val(altValues);
        }

        value = value.join(this.opts.multipleDatesSeparator);

        this.$el.val(value);
      },

      /**
       * Check if date is between minDate and maxDate
       * @param date {object} - date object
       * @param type {string} - cell type
       * @returns {boolean}
       * @private
       */
      _isInRange(date, type) {
        const time = date.getTime();
        const d = esdatepicker.getParsedDate(date);
        const min = esdatepicker.getParsedDate(this.minDate);
        const max = esdatepicker.getParsedDate(this.maxDate);
        const dMinTime = new Date(d.year, d.month, min.date).getTime();
        const dMaxTime = new Date(d.year, d.month, max.date).getTime();
        const types = {
          day: time >= this.minTime && time <= this.maxTime,
          month: dMinTime >= this.minTime && dMaxTime <= this.maxTime,
          year: d.year >= min.year && d.year <= max.year,
        };
        return type ? types[type] : types.day;
      },

      _getDimensions($el) {
        const offset = $el.offset();

        return {
          width: $el.outerWidth(),
          height: $el.outerHeight(),
          left: offset.left,
          top: offset.top,
        };
      },

      _getDateFromCell(cell) {
        const curDate = this.parsedDate;
        const year = cell.data('year') || curDate.year;
        const month = cell.data('month') == undefined ? curDate.month : cell.data('month');
        const date = cell.data('date') || 1;

        return new Date(year, month, date);
      },

      _setPositionClasses(pos) {
        pos = pos.split(' ');
        const main = pos[0];
        const sec = pos[1];
        let classes = `esdatepicker -${main}-${sec}- -from-${main}-`;

        if (this.visible) classes += ' active';

        this.$esdatepicker.removeAttr('class').addClass(classes);
      },

      setPosition(position) {
        position = position || this.opts.position;

        const dims = this._getDimensions(this.$el);
        const selfDims = this._getDimensions(this.$esdatepicker);
        const pos = position.split(' ');
        let top;
        let left;
        const { offset } = this.opts;
        const main = pos[0];
        const secondary = pos[1];

        switch (main) {
        case 'top':
          top = dims.top - selfDims.height - offset;
          break;
        case 'right':
          left = dims.left + dims.width + offset;
          break;
        case 'bottom':
          top = dims.top + dims.height + offset;
          break;
        case 'left':
          left = dims.left - selfDims.width - offset;
          break;
        }

        switch (secondary) {
        case 'top':
          top = dims.top;
          break;
        case 'right':
          left = dims.left + dims.width - selfDims.width;
          break;
        case 'bottom':
          top = dims.top + dims.height - selfDims.height;
          break;
        case 'left':
          left = dims.left;
          break;
        case 'center':
          if (/left|right/.test(main)) {
            top = dims.top + dims.height / 2 - selfDims.height / 2;
          } else {
            left = dims.left + dims.width / 2 - selfDims.width / 2;
          }
        }

        this.$esdatepicker.css({
          left,
          top,
        });
      },

      show() {
        const { onShow } = this.opts;

        this.setPosition(this.opts.position);
        this.$esdatepicker.addClass('active');
        this.visible = true;

        if (onShow) {
          this._bindVisionEvents(onShow);
        }
      },

      hide() {
        const { onHide } = this.opts;

        this.$esdatepicker.removeClass('active').css({
          left: '-100000px',
        });

        this.focused = '';
        this.keys = [];

        this.inFocus = false;
        this.visible = false;
        this.$el.blur();

        if (onHide) {
          this._bindVisionEvents(onHide);
        }
      },

      down(date) {
        this._changeView(date, 'down');
      },

      up(date) {
        this._changeView(date, 'up');
      },

      _bindVisionEvents(event) {
        this.$esdatepicker.off('transitionend.dp');
        event(this, false);
        this.$esdatepicker.one('transitionend.dp', event.bind(this, this, true));
      },

      _changeView(date, dir) {
        date = date || this.focused || this.date;

        let nextView = dir == 'up' ? this.viewIndex + 1 : this.viewIndex - 1;
        if (nextView > 2) nextView = 2;
        if (nextView < 0) nextView = 0;

        this.silent = true;
        this.date = new Date(date.getFullYear(), date.getMonth(), 1);
        this.silent = false;
        this.view = this.viewIndexes[nextView];
      },

      _handleHotKey(key) {
        const date = esdatepicker.getParsedDate(this._getFocusedDate());
        let focusedParsed;
        const o = this.opts;
        let newDate;
        let totalDaysInNextMonth;
        let monthChanged = false;
        let yearChanged = false;
        let decadeChanged = false;
        let y = date.year;
        let m = date.month;
        let d = date.date;

        switch (key) {
        case 'ctrlRight':
        case 'ctrlUp':
          m += 1;
          monthChanged = true;
          break;
        case 'ctrlLeft':
        case 'ctrlDown':
          m -= 1;
          monthChanged = true;
          break;
        case 'shiftRight':
        case 'shiftUp':
          yearChanged = true;
          y += 1;
          break;
        case 'shiftLeft':
        case 'shiftDown':
          yearChanged = true;
          y -= 1;
          break;
        case 'altRight':
        case 'altUp':
          decadeChanged = true;
          y += 10;
          break;
        case 'altLeft':
        case 'altDown':
          decadeChanged = true;
          y -= 10;
          break;
        case 'ctrlShiftUp':
          this.up();
          break;
        }

        totalDaysInNextMonth = esdatepicker.getDaysCount(new Date(y, m));
        newDate = new Date(y, m, d);

        // If next month has less days than current, set date to total days in that month
        if (totalDaysInNextMonth < d) d = totalDaysInNextMonth;

        // Check if newDate is in valid range
        if (newDate.getTime() < this.minTime) {
          newDate = this.minDate;
        } else if (newDate.getTime() > this.maxTime) {
          newDate = this.maxDate;
        }

        this.focused = newDate;

        focusedParsed = esdatepicker.getParsedDate(newDate);
        if (monthChanged && o.onChangeMonth) {
          o.onChangeMonth(focusedParsed.month, focusedParsed.year);
        }
        if (yearChanged && o.onChangeYear) {
          o.onChangeYear(focusedParsed.year);
        }
        if (decadeChanged && o.onChangeDecade) {
          o.onChangeDecade(this.curDecade);
        }
      },

      _registerKey(key) {
        const exists = this.keys.some(curKey => {
          return curKey == key;
        });

        if (!exists) {
          this.keys.push(key);
        }
      },

      _unRegisterKey(key) {
        const index = this.keys.indexOf(key);

        this.keys.splice(index, 1);
      },

      _isHotKeyPressed() {
        let currentHotKey;
        let found = false;
        const _this = this;
        const pressedKeys = this.keys.sort();

        for (const hotKey in hotKeys) {
          currentHotKey = hotKeys[hotKey];
          if (pressedKeys.length != currentHotKey.length) continue;

          if (
            currentHotKey.every((key, i) => {
              return key == pressedKeys[i];
            })
          ) {
            _this._trigger('hotKey', hotKey);
            found = true;
          }
        }

        return found;
      },

      _trigger(event, args) {
        this.$el.trigger(event, args);
      },

      _focusNextCell(keyCode, type) {
        type = type || this.cellType;

        const date = esdatepicker.getParsedDate(this._getFocusedDate());
        let y = date.year;
        let m = date.month;
        let d = date.date;

        if (this._isHotKeyPressed()) {
          return;
        }

        switch (keyCode) {
        case 37: // left
          type == 'day' ? (d -= 1) : '';
          type == 'month' ? (m -= 1) : '';
          type == 'year' ? (y -= 1) : '';
          break;
        case 38: // up
          type == 'day' ? (d -= 7) : '';
          type == 'month' ? (m -= 3) : '';
          type == 'year' ? (y -= 4) : '';
          break;
        case 39: // right
          type == 'day' ? (d += 1) : '';
          type == 'month' ? (m += 1) : '';
          type == 'year' ? (y += 1) : '';
          break;
        case 40: // down
          type == 'day' ? (d += 7) : '';
          type == 'month' ? (m += 3) : '';
          type == 'year' ? (y += 4) : '';
          break;
        }

        let nd = new Date(y, m, d);
        if (nd.getTime() < this.minTime) {
          nd = this.minDate;
        } else if (nd.getTime() > this.maxTime) {
          nd = this.maxDate;
        }

        this.focused = nd;
      },

      _getFocusedDate() {
        let focused = this.focused || this.selectedDates[this.selectedDates.length - 1];
        const d = this.parsedDate;

        if (!focused) {
          switch (this.view) {
          case 'days':
            focused = new Date(d.year, d.month, new Date().getDate());
            break;
          case 'months':
            focused = new Date(d.year, d.month, 1);
            break;
          case 'years':
            focused = new Date(d.year, 0, 1);
            break;
          }
        }

        return focused;
      },

      _getCell(date, type) {
        type = type || this.cellType;

        const d = esdatepicker.getParsedDate(date);
        let selector = `.esdatepicker--cell[data-year="${d.year}"]`;
        let $cell;

        switch (type) {
        case 'month':
          selector = `[data-month="${d.month}"]`;
          break;
        case 'day':
          selector += `[data-month="${d.month}"][data-date="${d.date}"]`;
          break;
        }
        $cell = this.views[this.currentView].$el.find(selector);

        return $cell.length ? $cell : $('');
      },

      destroy() {
        const _this = this;
        _this.$el.off('.adp').data('esdatepicker', '');

        _this.selectedDates = [];
        _this.focused = '';
        _this.views = {};
        _this.keys = [];
        _this.minRange = '';
        _this.maxRange = '';

        if (_this.opts.inline || !_this.elIsInput) {
          _this.$esdatepicker.closest('.esdatepicker-inline').remove();
        } else {
          _this.$esdatepicker.remove();
        }
      },

      _handleAlreadySelectedDates(alreadySelected, selectedDate) {
        if (this.opts.range) {
          if (!this.opts.toggleSelected) {
            // Add possibility to select same date when range is true
            if (this.selectedDates.length != 2) {
              this._trigger('clickCell', selectedDate);
            }
          } else {
            this.removeDate(selectedDate);
          }
        } else if (this.opts.toggleSelected) {
          this.removeDate(selectedDate);
        }

        // Change last selected date to be able to change time when clicking on this cell
        if (!this.opts.toggleSelected) {
          this.lastSelectedDate = alreadySelected;
          if (this.opts.estimepicker) {
            this.estimepicker._setTime(alreadySelected);
            this.estimepicker.update();
          }
        }
      },

      _onShowEvent(e) {
        if (!this.visible) {
          this.show();
        }
      },

      _onBlur() {
        if (!this.inFocus && this.visible) {
          this.hide();
        }
      },

      _onMouseDownDatepicker(e) {
        this.inFocus = true;
      },

      _onMouseUpDatepicker(e) {
        this.inFocus = false;
        e.originalEvent.inFocus = true;
        if (!e.originalEvent.estimepickerFocus) this.$el.focus();
      },

      _onKeyUpGeneral(e) {
        const val = this.$el.val();

        if (!val) {
          this.clear();
        }
      },

      _onResize() {
        if (this.visible) {
          this.setPosition();
        }
      },

      _onMouseUpBody(e) {
        if (e.originalEvent.inFocus) return;

        if (this.visible && !this.inFocus) {
          this.hide();
        }
      },

      _onMouseUpEl(e) {
        e.originalEvent.inFocus = true;
        setTimeout(this._onKeyUpGeneral.bind(this), 4);
      },

      _onKeyDown(e) {
        const code = e.which;
        this._registerKey(code);

        // Arrows
        if (code >= 37 && code <= 40) {
          e.preventDefault();
          this._focusNextCell(code);
        }

        // Enter
        if (code == 13) {
          if (this.focused) {
            if (this._getCell(this.focused).hasClass('-disabled-')) return;
            if (this.view != this.opts.minView) {
              this.down();
            } else {
              const alreadySelected = this._isSelected(this.focused, this.cellType);

              if (!alreadySelected) {
                if (this.estimepicker) {
                  this.focused.setHours(this.estimepicker.hours);
                  this.focused.setMinutes(this.estimepicker.minutes);
                }
                this.selectDate(this.focused);
                return;
              }
              this._handleAlreadySelectedDates(alreadySelected, this.focused);
            }
          }
        }

        // Esc
        if (code == 27) {
          this.hide();
        }
      },

      _onKeyUp(e) {
        const code = e.which;
        this._unRegisterKey(code);
      },

      _onHotKey(e, hotKey) {
        this._handleHotKey(hotKey);
      },

      _onMouseEnterCell(e) {
        const $cell = $(e.target).closest('.esdatepicker--cell');
        const date = this._getDateFromCell($cell);

        // Prevent from unnecessary rendering and setting new currentDate
        this.silent = true;

        if (this.focused) {
          this.focused = '';
        }

        $cell.addClass('-focus-');

        this.focused = date;
        this.silent = false;

        if (this.opts.range && this.selectedDates.length == 1) {
          this.minRange = this.selectedDates[0];
          this.maxRange = '';
          if (esdatepicker.less(this.minRange, this.focused)) {
            this.maxRange = this.minRange;
            this.minRange = '';
          }
          this.views[this.currentView]._update();
        }
      },

      _onMouseLeaveCell(e) {
        const $cell = $(e.target).closest('.esdatepicker--cell');

        $cell.removeClass('-focus-');

        this.silent = true;
        this.focused = '';
        this.silent = false;
      },

      _onTimeChange(e, h, m) {
        let date = new Date();
        const { selectedDates } = this;
        let selected = false;

        if (selectedDates.length) {
          selected = true;
          date = this.lastSelectedDate;
        }

        date.setHours(h);
        date.setMinutes(m);

        if (!selected && !this._getCell(date).hasClass('-disabled-')) {
          this.selectDate(date);
        } else {
          this._setInputValue();
          if (this.opts.onSelect) {
            this._triggerOnChange();
          }
        }
      },

      _onClickCell(e, date) {
        if (this.estimepicker) {
          date.setHours(this.estimepicker.hours);
          date.setMinutes(this.estimepicker.minutes);
        }
        this.selectDate(date);
      },

      set focused(val) {
        if (!val && this.focused) {
          const $cell = this._getCell(this.focused);

          if ($cell.length) {
            $cell.removeClass('-focus-');
          }
        }
        this._focused = val;
        if (this.opts.range && this.selectedDates.length == 1) {
          this.minRange = this.selectedDates[0];
          this.maxRange = '';
          if (esdatepicker.less(this.minRange, this._focused)) {
            this.maxRange = this.minRange;
            this.minRange = '';
          }
        }
        if (this.silent) return;
        this.date = val;
      },

      get focused() {
        return this._focused;
      },

      get parsedDate() {
        return esdatepicker.getParsedDate(this.date);
      },

      set date(val) {
        if (!(val instanceof Date)) return;

        this.currentDate = val;

        if (this.inited && !this.silent) {
          this.views[this.view]._render();
          this.nav._render();
          if (this.visible && this.elIsInput) {
            this.setPosition();
          }
        }
        return val;
      },

      get date() {
        return this.currentDate;
      },

      set view(val) {
        this.viewIndex = this.viewIndexes.indexOf(val);

        if (this.viewIndex < 0) {
          return;
        }

        this.prevView = this.currentView;
        this.currentView = val;

        if (this.inited) {
          if (!this.views[val]) {
            this.views[val] = new $.fn.esdatepicker.Body(this, val, this.opts);
          } else {
            this.views[val]._render();
          }

          this.views[this.prevView].hide();
          this.views[val].show();
          this.nav._render();

          if (this.opts.onChangeView) {
            this.opts.onChangeView(val);
          }
          if (this.elIsInput && this.visible) this.setPosition();
        }

        return val;
      },

      get view() {
        return this.currentView;
      },

      get cellType() {
        return this.view.substring(0, this.view.length - 1);
      },

      get minTime() {
        const min = esdatepicker.getParsedDate(this.minDate);
        return new Date(min.year, min.month, min.date).getTime();
      },

      get maxTime() {
        const max = esdatepicker.getParsedDate(this.maxDate);
        return new Date(max.year, max.month, max.date).getTime();
      },

      get curDecade() {
        return esdatepicker.getDecade(this.date);
      },
    };

    //  Utils
    // -------------------------------------------------

    esdatepicker.getDaysCount = function(date) {
      return new Date(date.getFullYear(), date.getMonth() + 1, 0).getDate();
    };

    esdatepicker.getParsedDate = function(date) {
      return {
        year: date.getFullYear(),
        month: date.getMonth(),
        fullMonth: date.getMonth() + 1 < 10 ? `0${date.getMonth() + 1}` : date.getMonth() + 1, // One based
        date: date.getDate(),
        fullDate: date.getDate() < 10 ? `0${date.getDate()}` : date.getDate(),
        day: date.getDay(),
        hours: date.getHours(),
        fullHours: date.getHours() < 10 ? `0${date.getHours()}` : date.getHours(),
        minutes: date.getMinutes(),
        fullMinutes: date.getMinutes() < 10 ? `0${date.getMinutes()}` : date.getMinutes(),
      };
    };

    esdatepicker.getDecade = function(date) {
      const firstYear = Math.floor(date.getFullYear() / 10) * 10;

      return [firstYear, firstYear + 9];
    };

    esdatepicker.template = function(str, data) {
      return str.replace(/#\{([\w]+)\}/g, (source, match) => {
        if (data[match] || data[match] === 0) {
          return data[match];
        }
      });
    };

    esdatepicker.isSame = function(date1, date2, type) {
      if (!date1 || !date2) return false;
      const d1 = esdatepicker.getParsedDate(date1);
      const d2 = esdatepicker.getParsedDate(date2);
      const _type = type || 'day';
      const conditions = {
        day: d1.date == d2.date && d1.month == d2.month && d1.year == d2.year,
        month: d1.month == d2.month && d1.year == d2.year,
        year: d1.year == d2.year,
      };

      return conditions[_type];
    };

    esdatepicker.less = function(dateCompareTo, date, type) {
      if (!dateCompareTo || !date) return false;
      return date.getTime() < dateCompareTo.getTime();
    };

    esdatepicker.bigger = function(dateCompareTo, date, type) {
      if (!dateCompareTo || !date) return false;
      return date.getTime() > dateCompareTo.getTime();
    };

    esdatepicker.getLeadingZeroNum = function(num) {
      return parseInt(num) < 10 ? `0${num}` : num;
    };

    /**
     * Returns copy of date with hours and minutes equals to 0
     * @param date {Date}
     */
    esdatepicker.resetTime = function(date) {
      if (typeof date !== 'object') return;
      date = esdatepicker.getParsedDate(date);
      return new Date(date.year, date.month, date.date);
    };

    $.fn.esdatepicker = function(options) {
      return this.each(function() {
        if (!$.data(this, pluginName)) {
          $.data(this, pluginName, new Datepicker(this, options));
        } else {
          const _this = $.data(this, pluginName);

          _this.opts = $.extend(true, _this.opts, options);
          _this.update();
        }
      });
    };

    $.fn.esdatepicker.Constructor = Datepicker;

    $.fn.esdatepicker.language = {
      ru: {
        days: ['Воскресенье', 'Понедельник', 'Вторник', 'Среда', 'Четверг', 'Пятница', 'Суббота'],
        daysShort: ['Вос', 'Пон', 'Вто', 'Сре', 'Чет', 'Пят', 'Суб'],
        daysMin: ['Вс', 'Пн', 'Вт', 'Ср', 'Чт', 'Пт', 'Сб'],
        months: [
          'Январь',
          'Февраль',
          'Март',
          'Апрель',
          'Май',
          'Июнь',
          'Июль',
          'Август',
          'Сентябрь',
          'Октябрь',
          'Ноябрь',
          'Декабрь',
        ],
        monthsShort: [
          'Янв',
          'Фев',
          'Мар',
          'Апр',
          'Май',
          'Июн',
          'Июл',
          'Авг',
          'Сен',
          'Окт',
          'Ноя',
          'Дек',
        ],
        today: 'Сегодня',
        clear: 'Очистить',
        dateFormat: 'dd.mm.yyyy',
        timeFormat: 'hh:ii',
        firstDay: 1,
      },
    };

    $(() => {
      $(autoInitSelector).esdatepicker();
    });
  }());

  (function() {
    const templates = {
      days:
        ''
        + '<div class="esdatepicker--days esdatepicker--body">'
        + '<div class="esdatepicker--days-names"></div>'
        + '<div class="esdatepicker--cells esdatepicker--cells-days"></div>'
        + '</div>',
      months:
        ''
        + '<div class="esdatepicker--months esdatepicker--body">'
        + '<div class="esdatepicker--cells esdatepicker--cells-months"></div>'
        + '</div>',
      years:
        ''
        + '<div class="esdatepicker--years esdatepicker--body">'
        + '<div class="esdatepicker--cells esdatepicker--cells-years"></div>'
        + '</div>',
    };
    const { esdatepicker } = $.fn;
    const dp = esdatepicker.Constructor;

    esdatepicker.Body = function(d, type, opts) {
      this.d = d;
      this.type = type;
      this.opts = opts;
      this.$el = $('');

      if (this.opts.onlyTimepicker) return;
      this.init();
    };

    esdatepicker.Body.prototype = {
      init() {
        this._buildBaseHtml();
        this._render();

        this._bindEvents();
      },

      _bindEvents() {
        this.$el.on('click', '.esdatepicker--cell', $.proxy(this._onClickCell, this));
      },

      _buildBaseHtml() {
        this.$el = $(templates[this.type]).appendTo(this.d.$content);
        this.$names = $('.esdatepicker--days-names', this.$el);
        this.$cells = $('.esdatepicker--cells', this.$el);
      },

      _getDayNamesHtml(firstDay, curDay, html, i) {
        curDay = curDay != undefined ? curDay : firstDay;
        html = html || '';
        i = i != undefined ? i : 0;

        if (i > 7) return html;
        if (curDay == 7) return this._getDayNamesHtml(firstDay, 0, html, ++i);

        html += `<div class="esdatepicker--day-name${
          this.d.isWeekend(curDay) ? ' -weekend-' : ''
        }">${this.d.loc.daysMin[curDay]}</div>`;

        return this._getDayNamesHtml(firstDay, ++curDay, html, ++i);
      },

      _getCellContents(date, type) {
        let classes = `esdatepicker--cell esdatepicker--cell-${type}`;
        const currentDate = new Date();
        const parent = this.d;
        const minRange = dp.resetTime(parent.minRange);
        const maxRange = dp.resetTime(parent.maxRange);
        const { opts } = parent;
        const d = dp.getParsedDate(date);
        let render = {};
        let html = d.date;

        switch (type) {
        case 'day':
          if (parent.isWeekend(d.day)) classes += ' -weekend-';
          if (d.month != this.d.parsedDate.month) {
            classes += ' -other-month-';
            if (!opts.selectOtherMonths) {
              classes += ' -disabled-';
            }
            if (!opts.showOtherMonths) html = '';
          }
          break;
        case 'month':
          html = parent.loc[parent.opts.monthsField][d.month];
          break;
        case 'year':
          var decade = parent.curDecade;
          html = d.year;
          if (d.year < decade[0] || d.year > decade[1]) {
            classes += ' -other-decade-';
            if (!opts.selectOtherYears) {
              classes += ' -disabled-';
            }
            if (!opts.showOtherYears) html = '';
          }
          break;
        }

        if (opts.onRenderCell) {
          render = opts.onRenderCell(date, type) || {};
          html = render.html ? render.html : html;
          classes += render.classes ? ` ${render.classes}` : '';
        }

        if (opts.range) {
          if (dp.isSame(minRange, date, type)) classes += ' -range-from-';
          if (dp.isSame(maxRange, date, type)) classes += ' -range-to-';

          if (parent.selectedDates.length == 1 && parent.focused) {
            if (
              (dp.bigger(minRange, date) && dp.less(parent.focused, date))
              || (dp.less(maxRange, date) && dp.bigger(parent.focused, date))
            ) {
              classes += ' -in-range-';
            }

            if (dp.less(maxRange, date) && dp.isSame(parent.focused, date)) {
              classes += ' -range-from-';
            }
            if (dp.bigger(minRange, date) && dp.isSame(parent.focused, date)) {
              classes += ' -range-to-';
            }
          } else if (parent.selectedDates.length == 2) {
            if (dp.bigger(minRange, date) && dp.less(maxRange, date)) {
              classes += ' -in-range-';
            }
          }
        }

        if (dp.isSame(currentDate, date, type)) classes += ' -current-';
        if (parent.focused && dp.isSame(date, parent.focused, type)) classes += ' -focus-';
        if (parent._isSelected(date, type)) classes += ' -selected-';
        if (!parent._isInRange(date, type) || render.disabled) classes += ' -disabled-';

        return {
          html,
          classes,
        };
      },

      /**
       * Calculates days number to render. Generates days html and returns it.
       * @param {object} date - Date object
       * @returns {string}
       * @private
       */
      _getDaysHtml(date) {
        const totalMonthDays = dp.getDaysCount(date);
        const firstMonthDay = new Date(date.getFullYear(), date.getMonth(), 1).getDay();
        const lastMonthDay = new Date(date.getFullYear(), date.getMonth(), totalMonthDays).getDay();
        let daysFromPevMonth = firstMonthDay - this.d.loc.firstDay;
        let daysFromNextMonth = 6 - lastMonthDay + this.d.loc.firstDay;

        daysFromPevMonth = daysFromPevMonth < 0 ? daysFromPevMonth + 7 : daysFromPevMonth;
        daysFromNextMonth = daysFromNextMonth > 6 ? daysFromNextMonth - 7 : daysFromNextMonth;

        const startDayIndex = -daysFromPevMonth + 1;
        let m;
        let y;
        let html = '';

        for (let i = startDayIndex, max = totalMonthDays + daysFromNextMonth; i <= max; i++) {
          y = date.getFullYear();
          m = date.getMonth();

          html += this._getDayHtml(new Date(y, m, i));
        }

        return html;
      },

      _getDayHtml(date) {
        const content = this._getCellContents(date, 'day');

        return (
          `<div class="${content.classes}" `
          + `data-date="${date.getDate()}" `
          + `data-month="${date.getMonth()}" `
          + `data-year="${date.getFullYear()}">${content.html}</div>`
        );
      },

      /**
       * Generates months html
       * @param {object} date - date instance
       * @returns {string}
       * @private
       */
      _getMonthsHtml(date) {
        let html = '';
        const d = dp.getParsedDate(date);
        let i = 0;

        while (i < 12) {
          html += this._getMonthHtml(new Date(d.year, i));
          i++;
        }

        return html;
      },

      _getMonthHtml(date) {
        const content = this._getCellContents(date, 'month');

        return `<div class="${content.classes}" data-month="${date.getMonth()}">${
          content.html
        }</div>`;
      },

      _getYearsHtml(date) {
        const d = dp.getParsedDate(date);
        const decade = dp.getDecade(date);
        const firstYear = decade[0] - 1;
        let html = '';
        let i = firstYear;

        for (i; i <= decade[1] + 1; i++) {
          html += this._getYearHtml(new Date(i, 0));
        }

        return html;
      },

      _getYearHtml(date) {
        const content = this._getCellContents(date, 'year');

        return `<div class="${content.classes}" data-year="${date.getFullYear()}">${
          content.html
        }</div>`;
      },

      _renderTypes: {
        days() {
          const dayNames = this._getDayNamesHtml(this.d.loc.firstDay);
          const days = this._getDaysHtml(this.d.currentDate);

          this.$cells.html(days);
          this.$names.html(dayNames);
        },
        months() {
          const html = this._getMonthsHtml(this.d.currentDate);

          this.$cells.html(html);
        },
        years() {
          const html = this._getYearsHtml(this.d.currentDate);

          this.$cells.html(html);
        },
      },

      _render() {
        if (this.opts.onlyTimepicker) return;
        this._renderTypes[this.type].bind(this)();
      },

      _update() {
        const $cells = $('.esdatepicker--cell', this.$cells);
        const _this = this;
        let classes;
        let $cell;
        let date;
        $cells.each(function(cell, i) {
          $cell = $(this);
          date = _this.d._getDateFromCell($(this));
          classes = _this._getCellContents(date, _this.d.cellType);
          $cell.attr('class', classes.classes);
        });
      },

      show() {
        if (this.opts.onlyTimepicker) return;
        this.$el.addClass('active');
        this.acitve = true;
      },

      hide() {
        this.$el.removeClass('active');
        this.active = false;
      },

      //  Events
      // -------------------------------------------------

      _handleClick(el) {
        const date = el.data('date') || 1;
        const month = el.data('month') || 0;
        const year = el.data('year') || this.d.parsedDate.year;
        const dp = this.d;
        // Change view if min view does not reach yet
        if (dp.view != this.opts.minView) {
          dp.down(new Date(year, month, date));
          return;
        }
        // Select date if min view is reached
        const selectedDate = new Date(year, month, date);
        const alreadySelected = this.d._isSelected(selectedDate, this.d.cellType);

        if (!alreadySelected) {
          dp._trigger('clickCell', selectedDate);
          return;
        }

        dp._handleAlreadySelectedDates.bind(dp, alreadySelected, selectedDate)();
      },

      _onClickCell(e) {
        const $el = $(e.target).closest('.esdatepicker--cell');

        if ($el.hasClass('-disabled-')) return;

        this._handleClick.bind(this)($el);
      },
    };
  }());

  (function() {
    const template = ''
      + '<div class="esdatepicker--nav-action" data-action="prev">#{prevHtml}</div>'
      + '<div class="esdatepicker--nav-title">#{title}</div>'
      + '<div class="esdatepicker--nav-action" data-action="next">#{nextHtml}</div>';
    const buttonsContainerTemplate = '<div class="esdatepicker--buttons"></div>';
    const button = '<span class="esdatepicker--button" data-action="#{action}">#{label}</span>';
    const { esdatepicker } = $.fn;
    const dp = esdatepicker.Constructor;

    esdatepicker.Navigation = function(d, opts) {
      this.d = d;
      this.opts = opts;

      this.$buttonsContainer = '';

      this.init();
    };

    esdatepicker.Navigation.prototype = {
      init() {
        this._buildBaseHtml();
        this._bindEvents();
      },

      _bindEvents() {
        this.d.$nav.on('click', '.esdatepicker--nav-action', $.proxy(this._onClickNavButton, this));
        this.d.$nav.on('click', '.esdatepicker--nav-title', $.proxy(this._onClickNavTitle, this));
        this.d.$esdatepicker.on(
          'click',
          '.esdatepicker--button',
          $.proxy(this._onClickNavButton, this)
        );
      },

      _buildBaseHtml() {
        if (!this.opts.onlyTimepicker) {
          this._render();
        }
        this._addButtonsIfNeed();
      },

      _addButtonsIfNeed() {
        if (this.opts.todayButton) {
          this._addButton('today');
        }
        if (this.opts.clearButton) {
          this._addButton('clear');
        }
      },

      _render() {
        const title = this._getTitle(this.d.currentDate);
        const html = dp.template(template, $.extend({ title }, this.opts));
        this.d.$nav.html(html);
        if (this.d.view == 'years') {
          $('.esdatepicker--nav-title', this.d.$nav).addClass('-disabled-');
        }
        this.setNavStatus();
      },

      _getTitle(date) {
        return this.d.formatDate(this.opts.navTitles[this.d.view], date);
      },

      _addButton(type) {
        if (!this.$buttonsContainer.length) {
          this._addButtonsContainer();
        }

        const data = {
          action: type,
          label: this.d.loc[type],
        };
        const html = dp.template(button, data);

        if ($(`[data-action=${type}]`, this.$buttonsContainer).length) return;
        this.$buttonsContainer.append(html);
      },

      _addButtonsContainer() {
        this.d.$esdatepicker.append(buttonsContainerTemplate);
        this.$buttonsContainer = $('.esdatepicker--buttons', this.d.$esdatepicker);
      },

      setNavStatus() {
        if (!(this.opts.minDate || this.opts.maxDate) || !this.opts.disableNavWhenOutOfRange) return;

        const date = this.d.parsedDate;
        const m = date.month;
        const y = date.year;
        const d = date.date;

        switch (this.d.view) {
        case 'days':
          if (!this.d._isInRange(new Date(y, m - 1, 1), 'month')) {
            this._disableNav('prev');
          }
          if (!this.d._isInRange(new Date(y, m + 1, 1), 'month')) {
            this._disableNav('next');
          }
          break;
        case 'months':
          if (!this.d._isInRange(new Date(y - 1, m, d), 'year')) {
            this._disableNav('prev');
          }
          if (!this.d._isInRange(new Date(y + 1, m, d), 'year')) {
            this._disableNav('next');
          }
          break;
        case 'years':
          var decade = dp.getDecade(this.d.date);
          if (!this.d._isInRange(new Date(decade[0] - 1, 0, 1), 'year')) {
            this._disableNav('prev');
          }
          if (!this.d._isInRange(new Date(decade[1] + 1, 0, 1), 'year')) {
            this._disableNav('next');
          }
          break;
        }
      },

      _disableNav(nav) {
        $(`[data-action="${nav}"]`, this.d.$nav).addClass('-disabled-');
      },

      _activateNav(nav) {
        $(`[data-action="${nav}"]`, this.d.$nav).removeClass('-disabled-');
      },

      _onClickNavButton(e) {
        const $el = $(e.target).closest('[data-action]');
        const action = $el.data('action');

        this.d[action]();
      },

      _onClickNavTitle(e) {
        if ($(e.target).hasClass('-disabled-')) return;

        if (this.d.view == 'days') {
          return (this.d.view = 'months');
        }

        this.d.view = 'years';
      },
    };
  }());

  (function() {
    const template = '<div class="esdatepicker--time">'
      + '<div class="esdatepicker--time-current">'
      + '   <span class="esdatepicker--time-current-hours">#{hourVisible}</span>'
      + '   <span class="esdatepicker--time-current-colon">:</span>'
      + '   <span class="esdatepicker--time-current-minutes">#{minValue}</span>'
      + '</div>'
      + '<div class="esdatepicker--time-sliders">'
      + '   <div class="esdatepicker--time-row">'
      + '      <input type="range" name="hours" value="#{hourValue}" min="#{hourMin}" max="#{hourMax}" step="#{hourStep}"/>'
      + '   </div>'
      + '   <div class="esdatepicker--time-row">'
      + '      <input type="range" name="minutes" value="#{minValue}" min="#{minMin}" max="#{minMax}" step="#{minStep}"/>'
      + '   </div>'
      + '</div>'
      + '</div>';
    const { esdatepicker } = $.fn;
    const dp = esdatepicker.Constructor;

    esdatepicker.Timepicker = function(inst, opts) {
      this.d = inst;
      this.opts = opts;

      this.init();
    };

    esdatepicker.Timepicker.prototype = {
      init() {
        let input = 'input';
        this._setTime(this.d.date);
        this._buildHTML();

        if (navigator.userAgent.match(/trident/gi)) {
          input = 'change';
        }

        this.d.$el.on('selectDate', this._onSelectDate.bind(this));
        this.$ranges.on(input, this._onChangeRange.bind(this));
        this.$ranges.on('mouseup', this._onMouseUpRange.bind(this));
        this.$ranges.on('mousemove focus ', this._onMouseEnterRange.bind(this));
        this.$ranges.on('mouseout blur', this._onMouseOutRange.bind(this));
      },

      _setTime(date) {
        const _date = dp.getParsedDate(date);

        this._handleDate(date);
        this.hours = _date.hours < this.minHours ? this.minHours : _date.hours;
        this.minutes = _date.minutes < this.minMinutes ? this.minMinutes : _date.minutes;
      },

      /**
       * Sets minHours and minMinutes from date (usually it's a minDate)
       * Also changes minMinutes if current hours are bigger then @date hours
       * @param date {Date}
       * @private
       */
      _setMinTimeFromDate(date) {
        this.minHours = date.getHours();
        this.minMinutes = date.getMinutes();

        // If, for example, min hours are 10, and current hours are 12,
        // update minMinutes to default value, to be able to choose whole range of values
        if (this.d.lastSelectedDate) {
          if (this.d.lastSelectedDate.getHours() > date.getHours()) {
            this.minMinutes = this.opts.minMinutes;
          }
        }
      },

      _setMaxTimeFromDate(date) {
        this.maxHours = date.getHours();
        this.maxMinutes = date.getMinutes();

        if (this.d.lastSelectedDate) {
          if (this.d.lastSelectedDate.getHours() < date.getHours()) {
            this.maxMinutes = this.opts.maxMinutes;
          }
        }
      },

      _setDefaultMinMaxTime() {
        const maxHours = 23;
        const maxMinutes = 59;
        const { opts } = this;

        this.minHours = opts.minHours < 0 || opts.minHours > maxHours ? 0 : opts.minHours;
        this.minMinutes = opts.minMinutes < 0 || opts.minMinutes > maxMinutes ? 0 : opts.minMinutes;
        this.maxHours = opts.maxHours < 0 || opts.maxHours > maxHours ? maxHours : opts.maxHours;
        this.maxMinutes = opts.maxMinutes < 0 || opts.maxMinutes > maxMinutes ? maxMinutes : opts.maxMinutes;
      },

      /**
       * Looks for min/max hours/minutes and if current values
       * are out of range sets valid values.
       * @private
       */
      _validateHoursMinutes(date) {
        if (this.hours < this.minHours) {
          this.hours = this.minHours;
        } else if (this.hours > this.maxHours) {
          this.hours = this.maxHours;
        }

        if (this.minutes < this.minMinutes) {
          this.minutes = this.minMinutes;
        } else if (this.minutes > this.maxMinutes) {
          this.minutes = this.maxMinutes;
        }
      },

      _buildHTML() {
        const lz = dp.getLeadingZeroNum;
        const data = {
          hourMin: this.minHours,
          hourMax: lz(this.maxHours),
          hourStep: this.opts.hoursStep,
          hourValue: this.hours,
          hourVisible: lz(this.displayHours),
          minMin: this.minMinutes,
          minMax: lz(this.maxMinutes),
          minStep: this.opts.minutesStep,
          minValue: lz(this.minutes),
        };
        const _template = dp.template(template, data);

        this.$estimepicker = $(_template).appendTo(this.d.$esdatepicker);
        this.$ranges = $('[type="range"]', this.$estimepicker);
        this.$hours = $('[name="hours"]', this.$estimepicker);
        this.$minutes = $('[name="minutes"]', this.$estimepicker);
        this.$hoursText = $('.esdatepicker--time-current-hours', this.$estimepicker);
        this.$minutesText = $('.esdatepicker--time-current-minutes', this.$estimepicker);

        if (this.d.ampm) {
          this.$ampm = $('<span class="esdatepicker--time-current-ampm">')
            .appendTo($('.esdatepicker--time-current', this.$estimepicker))
            .html(this.dayPeriod);

          this.$estimepicker.addClass('-am-pm-');
        }
      },

      _updateCurrentTime() {
        const h = dp.getLeadingZeroNum(this.displayHours);
        const m = dp.getLeadingZeroNum(this.minutes);

        this.$hoursText.html(h);
        this.$minutesText.html(m);

        if (this.d.ampm) {
          this.$ampm.html(this.dayPeriod);
        }
      },

      _updateRanges() {
        this.$hours
          .attr({
            min: this.minHours,
            max: this.maxHours,
          })
          .val(this.hours);

        this.$minutes
          .attr({
            min: this.minMinutes,
            max: this.maxMinutes,
          })
          .val(this.minutes);
      },

      /**
       * Sets minHours, minMinutes etc. from date. If date is not passed, than sets
       * values from options
       * @param [date] {object} - Date object, to get values from
       * @private
       */
      _handleDate(date) {
        this._setDefaultMinMaxTime();
        if (date) {
          if (dp.isSame(date, this.d.opts.minDate)) {
            this._setMinTimeFromDate(this.d.opts.minDate);
          } else if (dp.isSame(date, this.d.opts.maxDate)) {
            this._setMaxTimeFromDate(this.d.opts.maxDate);
          }
        }

        this._validateHoursMinutes(date);
      },

      update() {
        this._updateRanges();
        this._updateCurrentTime();
      },

      /**
       * Calculates valid hour value to display in text input and esdatepicker's body.
       * @param date {Date|Number} - date or hours
       * @param [ampm] {Boolean} - 12 hours mode
       * @returns {{hours: *, dayPeriod: string}}
       * @private
       */
      _getValidHoursFromDate(date, ampm) {
        let d = date;
        let hours = date;

        if (date instanceof Date) {
          d = dp.getParsedDate(date);
          hours = d.hours;
        }

        const _ampm = ampm || this.d.ampm;
        let dayPeriod = 'am';

        if (_ampm) {
          switch (true) {
          case hours == 0:
            hours = 12;
            break;
          case hours == 12:
            dayPeriod = 'pm';
            break;
          case hours > 11:
            hours -= 12;
            dayPeriod = 'pm';
            break;
          default:
            break;
          }
        }

        return {
          hours,
          dayPeriod,
        };
      },

      set hours(val) {
        this._hours = val;

        const displayHours = this._getValidHoursFromDate(val);

        this.displayHours = displayHours.hours;
        this.dayPeriod = displayHours.dayPeriod;
      },

      get hours() {
        return this._hours;
      },

      //  Events
      // -------------------------------------------------

      _onChangeRange(e) {
        const $target = $(e.target);
        const name = $target.attr('name');

        this.d.estimepickerIsActive = true;

        this[name] = $target.val();
        this._updateCurrentTime();
        this.d._trigger('timeChange', [this.hours, this.minutes]);

        this._handleDate(this.d.lastSelectedDate);
        this.update();
      },

      _onSelectDate(e, data) {
        this._handleDate(data);
        this.update();
      },

      _onMouseEnterRange(e) {
        const name = $(e.target).attr('name');
        $(`.esdatepicker--time-current-${name}`, this.$estimepicker).addClass('-focus-');
      },

      _onMouseOutRange(e) {
        const name = $(e.target).attr('name');
        if (this.d.inFocus) return; // Prevent removing focus when mouse out of range slider
        $(`.esdatepicker--time-current-${name}`, this.$estimepicker).removeClass('-focus-');
      },

      _onMouseUpRange(e) {
        this.d.estimepickerIsActive = false;
      },
    };
  }());
}(window, jQuery));

/* End */
;
; /* Start:"a:4:{s:4:"full";s:80:"/local/templates/.default/markup/new/js/jquery.inputmask.min.js?1642060760111004";s:6:"source";s:63:"/local/templates/.default/markup/new/js/jquery.inputmask.min.js";s:3:"min";s:0:"";s:3:"map";s:0:"";}"*/
/*!
 * dist/jquery.inputmask.min
 * https://github.com/RobinHerbots/Inputmask
 * Copyright (c) 2010 - 2019 Robin Herbots
 * Licensed under the MIT license
 * Version: 5.0.0-beta.276
 */
!function webpackUniversalModuleDefinition(root,factory){if("object"==typeof exports&&"object"==typeof module)module.exports=factory(require("jquery"));else if("function"==typeof define&&define.amd)define(["jquery"],factory);else{var a="object"==typeof exports?factory(require("jquery")):factory(root.jQuery);for(var i in a)("object"==typeof exports?exports:root)[i]=a[i]}}(window,function(__WEBPACK_EXTERNAL_MODULE__2__){return modules=[function(module,exports,__webpack_require__){"use strict";function _typeof(obj){return _typeof="function"==typeof Symbol&&"symbol"==typeof Symbol.iterator?function _typeof(obj){return typeof obj}:function _typeof(obj){return obj&&"function"==typeof Symbol&&obj.constructor===Symbol&&obj!==Symbol.prototype?"symbol":typeof obj},_typeof(obj)}var $=__webpack_require__(1),window=__webpack_require__(3),document=window.document,generateMaskSet=__webpack_require__(4).generateMaskSet,analyseMask=__webpack_require__(4).analyseMask,maskScope=__webpack_require__(7);function Inputmask(alias,options,internal){if(!(this instanceof Inputmask))return new Inputmask(alias,options,internal);this.el=void 0,this.events={},this.maskset=void 0,this.refreshValue=!1,!0!==internal&&($.isPlainObject(alias)?options=alias:(options=options||{},alias&&(options.alias=alias)),this.opts=$.extend(!0,{},this.defaults,options),this.noMasksCache=options&&void 0!==options.definitions,this.userOptions=options||{},resolveAlias(this.opts.alias,options,this.opts),this.isRTL=this.opts.numericInput)}function resolveAlias(aliasStr,options,opts){var aliasDefinition=Inputmask.prototype.aliases[aliasStr];return aliasDefinition?(aliasDefinition.alias&&resolveAlias(aliasDefinition.alias,void 0,opts),$.extend(!0,opts,aliasDefinition),$.extend(!0,opts,options),!0):(null===opts.mask&&(opts.mask=aliasStr),!1)}function importAttributeOptions(npt,opts,userOptions,dataAttribute){function importOption(option,optionData){optionData=void 0!==optionData?optionData:npt.getAttribute(dataAttribute+"-"+option),null!==optionData&&("string"==typeof optionData&&(0===option.indexOf("on")?optionData=window[optionData]:"false"===optionData?optionData=!1:"true"===optionData&&(optionData=!0)),userOptions[option]=optionData)}if(!0===opts.importDataAttributes){var attrOptions=npt.getAttribute(dataAttribute),option,dataoptions,optionData,p;if(attrOptions&&""!==attrOptions&&(attrOptions=attrOptions.replace(/'/g,'"'),dataoptions=JSON.parse("{"+attrOptions+"}")),dataoptions)for(p in optionData=void 0,dataoptions)if("alias"===p.toLowerCase()){optionData=dataoptions[p];break}for(option in importOption("alias",optionData),userOptions.alias&&resolveAlias(userOptions.alias,userOptions,opts),opts){if(dataoptions)for(p in optionData=void 0,dataoptions)if(p.toLowerCase()===option.toLowerCase()){optionData=dataoptions[p];break}importOption(option,optionData)}}return $.extend(!0,opts,userOptions),"rtl"!==npt.dir&&!opts.rightAlign||(npt.style.textAlign="right"),"rtl"!==npt.dir&&!opts.numericInput||(npt.dir="ltr",npt.removeAttribute("dir"),opts.isRTL=!0),Object.keys(userOptions).length}Inputmask.prototype={dataAttribute:"data-inputmask",defaults:{placeholder:"_",optionalmarker:["[","]"],quantifiermarker:["{","}"],groupmarker:["(",")"],alternatormarker:"|",escapeChar:"\\",mask:null,regex:null,oncomplete:$.noop,onincomplete:$.noop,oncleared:$.noop,repeat:0,greedy:!1,autoUnmask:!1,removeMaskOnSubmit:!1,clearMaskOnLostFocus:!0,insertMode:!0,clearIncomplete:!1,alias:null,onKeyDown:$.noop,onBeforeMask:null,onBeforePaste:function onBeforePaste(pastedValue,opts){return $.isFunction(opts.onBeforeMask)?opts.onBeforeMask.call(this,pastedValue,opts):pastedValue},onBeforeWrite:null,onUnMask:null,showMaskOnFocus:!0,showMaskOnHover:!0,onKeyValidation:$.noop,skipOptionalPartCharacter:" ",numericInput:!1,rightAlign:!1,undoOnEscape:!0,radixPoint:"",_radixDance:!1,groupSeparator:"",keepStatic:null,positionCaretOnTab:!0,tabThrough:!1,supportsInputType:["text","tel","url","password","search"],ignorables:[8,9,13,19,27,33,34,35,36,37,38,39,40,45,46,93,112,113,114,115,116,117,118,119,120,121,122,123,0,229],isComplete:null,preValidation:null,postValidation:null,staticDefinitionSymbol:void 0,jitMasking:!1,nullable:!0,inputEventOnly:!1,noValuePatching:!1,positionCaretOnClick:"lvp",casing:null,inputmode:"verbatim",importDataAttributes:!0,shiftPositions:!0},definitions:{9:{validator:"[0-9\uff11-\uff19]",definitionSymbol:"*"},a:{validator:"[A-Za-z\u0410-\u044f\u0401\u0451\xc0-\xff\xb5]",definitionSymbol:"*"},"*":{validator:"[0-9\uff11-\uff19A-Za-z\u0410-\u044f\u0401\u0451\xc0-\xff\xb5]"}},aliases:{},masksCache:{},mask:function mask(elems){var that=this;return"string"==typeof elems&&(elems=document.getElementById(elems)||document.querySelectorAll(elems)),elems=elems.nodeName?[elems]:elems,$.each(elems,function(ndx,el){var scopedOpts=$.extend(!0,{},that.opts);if(importAttributeOptions(el,scopedOpts,$.extend(!0,{},that.userOptions),that.dataAttribute)){var maskset=generateMaskSet(scopedOpts,that.noMasksCache);void 0!==maskset&&(void 0!==el.inputmask&&(el.inputmask.opts.autoUnmask=!0,el.inputmask.remove()),el.inputmask=new Inputmask(void 0,void 0,!0),el.inputmask.opts=scopedOpts,el.inputmask.noMasksCache=that.noMasksCache,el.inputmask.userOptions=$.extend(!0,{},that.userOptions),el.inputmask.isRTL=scopedOpts.isRTL||scopedOpts.numericInput,el.inputmask.el=el,el.inputmask.maskset=maskset,$.data(el,"_inputmask_opts",scopedOpts),maskScope.call(el.inputmask,{action:"mask"}))}}),elems&&elems[0]&&elems[0].inputmask||this},option:function option(options,noremask){return"string"==typeof options?this.opts[options]:"object"===_typeof(options)?($.extend(this.userOptions,options),this.el&&!0!==noremask&&this.mask(this.el),this):void 0},unmaskedvalue:function unmaskedvalue(value){return this.maskset=this.maskset||generateMaskSet(this.opts,this.noMasksCache),maskScope.call(this,{action:"unmaskedvalue",value:value})},remove:function remove(){return maskScope.call(this,{action:"remove"})},getemptymask:function getemptymask(){return this.maskset=this.maskset||generateMaskSet(this.opts,this.noMasksCache),maskScope.call(this,{action:"getemptymask"})},hasMaskedValue:function hasMaskedValue(){return!this.opts.autoUnmask},isComplete:function isComplete(){return this.maskset=this.maskset||generateMaskSet(this.opts,this.noMasksCache),maskScope.call(this,{action:"isComplete"})},getmetadata:function getmetadata(){return this.maskset=this.maskset||generateMaskSet(this.opts,this.noMasksCache),maskScope.call(this,{action:"getmetadata"})},isValid:function isValid(value){return this.maskset=this.maskset||generateMaskSet(this.opts,this.noMasksCache),maskScope.call(this,{action:"isValid",value:value})},format:function format(value,metadata){return this.maskset=this.maskset||generateMaskSet(this.opts,this.noMasksCache),maskScope.call(this,{action:"format",value:value,metadata:metadata})},setValue:function setValue(value){this.el&&$(this.el).trigger("setvalue",[value])},analyseMask:analyseMask},Inputmask.extendDefaults=function(options){$.extend(!0,Inputmask.prototype.defaults,options)},Inputmask.extendDefinitions=function(definition){$.extend(!0,Inputmask.prototype.definitions,definition)},Inputmask.extendAliases=function(alias){$.extend(!0,Inputmask.prototype.aliases,alias)},Inputmask.format=function(value,options,metadata){return Inputmask(options).format(value,metadata)},Inputmask.unmask=function(value,options){return Inputmask(options).unmaskedvalue(value)},Inputmask.isValid=function(value,options){return Inputmask(options).isValid(value)},Inputmask.remove=function(elems){"string"==typeof elems&&(elems=document.getElementById(elems)||document.querySelectorAll(elems)),elems=elems.nodeName?[elems]:elems,$.each(elems,function(ndx,el){el.inputmask&&el.inputmask.remove()})},Inputmask.setValue=function(elems,value){"string"==typeof elems&&(elems=document.getElementById(elems)||document.querySelectorAll(elems)),elems=elems.nodeName?[elems]:elems,$.each(elems,function(ndx,el){el.inputmask?el.inputmask.setValue(value):$(el).trigger("setvalue",[value])})};var escapeRegexRegex=new RegExp("(\\"+["/",".","*","+","?","|","(",")","[","]","{","}","\\","$","^"].join("|\\")+")","gim");Inputmask.escapeRegex=function(str){return str.replace(escapeRegexRegex,"\\$1")},Inputmask.keyCode={BACKSPACE:8,BACKSPACE_SAFARI:127,DELETE:46,DOWN:40,END:35,ENTER:13,ESCAPE:27,HOME:36,INSERT:45,LEFT:37,PAGE_DOWN:34,PAGE_UP:33,RIGHT:39,SPACE:32,TAB:9,UP:38,X:88,CONTROL:17},Inputmask.dependencyLib=$,window.Inputmask=Inputmask,module.exports=Inputmask},function(module,exports,__webpack_require__){"use strict";var jquery=__webpack_require__(2);if(void 0===jquery)throw"jQuery not loaded!";module.exports=jquery},function(module,exports){module.exports=__WEBPACK_EXTERNAL_MODULE__2__},function(module,exports,__webpack_require__){"use strict";var __WEBPACK_AMD_DEFINE_RESULT__;function _typeof(obj){return _typeof="function"==typeof Symbol&&"symbol"==typeof Symbol.iterator?function _typeof(obj){return typeof obj}:function _typeof(obj){return obj&&"function"==typeof Symbol&&obj.constructor===Symbol&&obj!==Symbol.prototype?"symbol":typeof obj},_typeof(obj)}__WEBPACK_AMD_DEFINE_RESULT__=function(){return"undefined"!=typeof window?window:new(eval("require('jsdom').JSDOM"))("").window}.call(exports,__webpack_require__,exports,module),void 0===__WEBPACK_AMD_DEFINE_RESULT__||(module.exports=__WEBPACK_AMD_DEFINE_RESULT__)},function(module,exports,__webpack_require__){"use strict";var $=__webpack_require__(1);function generateMaskSet(opts,nocache){var ms;function generateMask(mask,metadata,opts){var regexMask=!1,masksetDefinition,maskdefKey;if(null!==mask&&""!==mask||(regexMask=null!==opts.regex,mask=regexMask?(mask=opts.regex,mask.replace(/^(\^)(.*)(\$)$/,"$2")):(regexMask=!0,".*")),1===mask.length&&!1===opts.greedy&&0!==opts.repeat&&(opts.placeholder=""),0<opts.repeat||"*"===opts.repeat||"+"===opts.repeat){var repeatStart="*"===opts.repeat?0:"+"===opts.repeat?1:opts.repeat;mask=opts.groupmarker[0]+mask+opts.groupmarker[1]+opts.quantifiermarker[0]+repeatStart+","+opts.repeat+opts.quantifiermarker[1]}return maskdefKey=regexMask?"regex_"+opts.regex:opts.numericInput?mask.split("").reverse().join(""):mask,!1!==opts.keepStatic&&(maskdefKey="ks_"+maskdefKey),void 0===Inputmask.prototype.masksCache[maskdefKey]||!0===nocache?(masksetDefinition={mask:mask,maskToken:Inputmask.prototype.analyseMask(mask,regexMask,opts),validPositions:{},_buffer:void 0,buffer:void 0,tests:{},excludes:{},metadata:metadata,maskLength:void 0,jitOffset:{}},!0!==nocache&&(Inputmask.prototype.masksCache[maskdefKey]=masksetDefinition,masksetDefinition=$.extend(!0,{},Inputmask.prototype.masksCache[maskdefKey]))):masksetDefinition=$.extend(!0,{},Inputmask.prototype.masksCache[maskdefKey]),masksetDefinition}if($.isFunction(opts.mask)&&(opts.mask=opts.mask(opts)),$.isArray(opts.mask)){if(1<opts.mask.length){if(null===opts.keepStatic){opts.keepStatic="auto";for(var i=0;i<opts.mask.length;i++)if(opts.mask[i].charAt(0)!==opts.mask[0].charAt(0)){opts.keepStatic=!0;break}}var altMask=opts.groupmarker[0];return $.each(opts.isRTL?opts.mask.reverse():opts.mask,function(ndx,msk){1<altMask.length&&(altMask+=opts.groupmarker[1]+opts.alternatormarker+opts.groupmarker[0]),void 0===msk.mask||$.isFunction(msk.mask)?altMask+=msk:altMask+=msk.mask}),altMask+=opts.groupmarker[1],generateMask(altMask,opts.mask,opts)}opts.mask=opts.mask.pop()}return null===opts.keepStatic&&(opts.keepStatic=!1),ms=opts.mask&&void 0!==opts.mask.mask&&!$.isFunction(opts.mask.mask)?generateMask(opts.mask.mask,opts.mask,opts):generateMask(opts.mask,opts.mask,opts),ms}function analyseMask(mask,regexMask,opts){var tokenizer=/(?:[?*+]|\{[0-9+*]+(?:,[0-9+*]*)?(?:\|[0-9+*]*)?\})|[^.?*+^${[]()|\\]+|./g,regexTokenizer=/\[\^?]?(?:[^\\\]]+|\\[\S\s]?)*]?|\\(?:0(?:[0-3][0-7]{0,2}|[4-7][0-7]?)?|[1-9][0-9]*|x[0-9A-Fa-f]{2}|u[0-9A-Fa-f]{4}|c[A-Za-z]|[\S\s]?)|\((?:\?[:=!]?)?|(?:[?*+]|\{[0-9]+(?:,[0-9]*)?\})\??|[^.?*+^${[()|\\]+|./g,escaped=!1,currentToken=new MaskToken,match,m,openenings=[],maskTokens=[],openingToken,currentOpeningToken,alternator,lastMatch,closeRegexGroup=!1;function MaskToken(isGroup,isOptional,isQuantifier,isAlternator){this.matches=[],this.openGroup=isGroup||!1,this.alternatorGroup=!1,this.isGroup=isGroup||!1,this.isOptional=isOptional||!1,this.isQuantifier=isQuantifier||!1,this.isAlternator=isAlternator||!1,this.quantifier={min:1,max:1}}function insertTestDefinition(mtoken,element,position){position=void 0!==position?position:mtoken.matches.length;var prevMatch=mtoken.matches[position-1];if(regexMask)0===element.indexOf("[")||escaped&&/\\d|\\s|\\w]/i.test(element)||"."===element?mtoken.matches.splice(position++,0,{fn:new RegExp(element,opts.casing?"i":""),static:!1,optionality:!1,newBlockMarker:void 0===prevMatch?"master":prevMatch.def!==element,casing:null,def:element,placeholder:void 0,nativeDef:element}):(escaped&&(element=element[element.length-1]),$.each(element.split(""),function(ndx,lmnt){prevMatch=mtoken.matches[position-1],mtoken.matches.splice(position++,0,{fn:/[a-z]/i.test(opts.staticDefinitionSymbol||lmnt)?new RegExp("["+(opts.staticDefinitionSymbol||lmnt)+"]",opts.casing?"i":""):null,static:!0,optionality:!1,newBlockMarker:void 0===prevMatch?"master":prevMatch.def!==lmnt&&!0!==prevMatch.static,casing:null,def:opts.staticDefinitionSymbol||lmnt,placeholder:void 0!==opts.staticDefinitionSymbol?lmnt:void 0,nativeDef:(escaped?"'":"")+lmnt})})),escaped=!1;else{var maskdef=(opts.definitions?opts.definitions[element]:void 0)||Inputmask.prototype.definitions[element];maskdef&&!escaped?mtoken.matches.splice(position++,0,{fn:maskdef.validator?"string"==typeof maskdef.validator?new RegExp(maskdef.validator,opts.casing?"i":""):new function(){this.test=maskdef.validator}:new RegExp("."),static:!1,optionality:!1,newBlockMarker:void 0===prevMatch?"master":prevMatch.def!==(maskdef.definitionSymbol||element),casing:maskdef.casing,def:maskdef.definitionSymbol||element,placeholder:maskdef.placeholder,nativeDef:element}):(mtoken.matches.splice(position++,0,{fn:/[a-z]/i.test(opts.staticDefinitionSymbol||element)?new RegExp("["+(opts.staticDefinitionSymbol||element)+"]",opts.casing?"i":""):null,static:!0,optionality:!1,newBlockMarker:void 0===prevMatch?"master":prevMatch.def!==element&&!0!==prevMatch.static,casing:null,def:opts.staticDefinitionSymbol||element,placeholder:void 0!==opts.staticDefinitionSymbol?element:void 0,nativeDef:(escaped?"'":"")+element}),escaped=!1)}}function verifyGroupMarker(maskToken){maskToken&&maskToken.matches&&$.each(maskToken.matches,function(ndx,token){var nextToken=maskToken.matches[ndx+1];(void 0===nextToken||void 0===nextToken.matches||!1===nextToken.isQuantifier)&&token&&token.isGroup&&(token.isGroup=!1,regexMask||(insertTestDefinition(token,opts.groupmarker[0],0),!0!==token.openGroup&&insertTestDefinition(token,opts.groupmarker[1]))),verifyGroupMarker(token)})}function defaultCase(){if(0<openenings.length){if(currentOpeningToken=openenings[openenings.length-1],insertTestDefinition(currentOpeningToken,m),currentOpeningToken.isAlternator){alternator=openenings.pop();for(var mndx=0;mndx<alternator.matches.length;mndx++)alternator.matches[mndx].isGroup&&(alternator.matches[mndx].isGroup=!1);0<openenings.length?(currentOpeningToken=openenings[openenings.length-1],currentOpeningToken.matches.push(alternator)):currentToken.matches.push(alternator)}}else insertTestDefinition(currentToken,m)}function reverseTokens(maskToken){function reverseStatic(st){return st===opts.optionalmarker[0]?st=opts.optionalmarker[1]:st===opts.optionalmarker[1]?st=opts.optionalmarker[0]:st===opts.groupmarker[0]?st=opts.groupmarker[1]:st===opts.groupmarker[1]&&(st=opts.groupmarker[0]),st}for(var match in maskToken.matches=maskToken.matches.reverse(),maskToken.matches)if(Object.prototype.hasOwnProperty.call(maskToken.matches,match)){var intMatch=parseInt(match);if(maskToken.matches[match].isQuantifier&&maskToken.matches[intMatch+1]&&maskToken.matches[intMatch+1].isGroup){var qt=maskToken.matches[match];maskToken.matches.splice(match,1),maskToken.matches.splice(intMatch+1,0,qt)}void 0!==maskToken.matches[match].matches?maskToken.matches[match]=reverseTokens(maskToken.matches[match]):maskToken.matches[match]=reverseStatic(maskToken.matches[match])}return maskToken}function groupify(matches){var groupToken=new MaskToken(!0);return groupToken.openGroup=!1,groupToken.matches=matches,groupToken}function closeGroup(){if(openingToken=openenings.pop(),openingToken.openGroup=!1,void 0!==openingToken)if(0<openenings.length){if(currentOpeningToken=openenings[openenings.length-1],currentOpeningToken.matches.push(openingToken),currentOpeningToken.isAlternator){alternator=openenings.pop();for(var mndx=0;mndx<alternator.matches.length;mndx++)alternator.matches[mndx].isGroup=!1,alternator.matches[mndx].alternatorGroup=!1;0<openenings.length?(currentOpeningToken=openenings[openenings.length-1],currentOpeningToken.matches.push(alternator)):currentToken.matches.push(alternator)}}else currentToken.matches.push(openingToken);else defaultCase()}function groupQuantifier(matches){var lastMatch=matches.pop();return lastMatch.isQuantifier&&(lastMatch=groupify([matches.pop(),lastMatch])),lastMatch}for(regexMask&&(opts.optionalmarker[0]=void 0,opts.optionalmarker[1]=void 0);match=regexMask?regexTokenizer.exec(mask):tokenizer.exec(mask);){if(m=match[0],regexMask)switch(m.charAt(0)){case"?":m="{0,1}";break;case"+":case"*":m="{"+m+"}";break;case"|":if(0===openenings.length){var altRegexGroup=groupify(currentToken.matches);altRegexGroup.openGroup=!0,openenings.push(altRegexGroup),currentToken.matches=[],closeRegexGroup=!0}break}if(escaped)defaultCase();else switch(m.charAt(0)){case"(?=":break;case"(?!":break;case"(?<=":break;case"(?<!":break;case opts.escapeChar:escaped=!0,regexMask&&defaultCase();break;case opts.optionalmarker[1]:case opts.groupmarker[1]:closeGroup();break;case opts.optionalmarker[0]:openenings.push(new MaskToken(!1,!0));break;case opts.groupmarker[0]:openenings.push(new MaskToken(!0));break;case opts.quantifiermarker[0]:var quantifier=new MaskToken(!1,!1,!0);m=m.replace(/[{}]/g,"");var mqj=m.split("|"),mq=mqj[0].split(","),mq0=isNaN(mq[0])?mq[0]:parseInt(mq[0]),mq1=1===mq.length?mq0:isNaN(mq[1])?mq[1]:parseInt(mq[1]);"*"!==mq0&&"+"!==mq0||(mq0="*"===mq1?0:1),quantifier.quantifier={min:mq0,max:mq1,jit:mqj[1]};var matches=0<openenings.length?openenings[openenings.length-1].matches:currentToken.matches;if(match=matches.pop(),match.isAlternator){matches.push(match),matches=match.matches;var groupToken=new MaskToken(!0),tmpMatch=matches.pop();matches.push(groupToken),matches=groupToken.matches,match=tmpMatch}match.isGroup||(match=groupify([match])),matches.push(match),matches.push(quantifier);break;case opts.alternatormarker:if(0<openenings.length){currentOpeningToken=openenings[openenings.length-1];var subToken=currentOpeningToken.matches[currentOpeningToken.matches.length-1];lastMatch=currentOpeningToken.openGroup&&(void 0===subToken.matches||!1===subToken.isGroup&&!1===subToken.isAlternator)?openenings.pop():groupQuantifier(currentOpeningToken.matches)}else lastMatch=groupQuantifier(currentToken.matches);if(lastMatch.isAlternator)openenings.push(lastMatch);else if(lastMatch.alternatorGroup?(alternator=openenings.pop(),lastMatch.alternatorGroup=!1):alternator=new MaskToken(!1,!1,!1,!0),alternator.matches.push(lastMatch),openenings.push(alternator),lastMatch.openGroup){lastMatch.openGroup=!1;var alternatorGroup=new MaskToken(!0);alternatorGroup.alternatorGroup=!0,openenings.push(alternatorGroup)}break;default:defaultCase()}}for(closeRegexGroup&&closeGroup();0<openenings.length;)openingToken=openenings.pop(),currentToken.matches.push(openingToken);return 0<currentToken.matches.length&&(verifyGroupMarker(currentToken),maskTokens.push(currentToken)),(opts.numericInput||opts.isRTL)&&reverseTokens(maskTokens[0]),maskTokens}module.exports={generateMaskSet:generateMaskSet,analyseMask:analyseMask}},function(module,exports,__webpack_require__){"use strict";__webpack_require__(6),__webpack_require__(8),__webpack_require__(9),module.exports=__webpack_require__(0)},function(module,exports,__webpack_require__){"use strict";var Inputmask=__webpack_require__(0);Inputmask.extendDefinitions({A:{validator:"[A-Za-z\u0410-\u044f\u0401\u0451\xc0-\xff\xb5]",casing:"upper"},"&":{validator:"[0-9A-Za-z\u0410-\u044f\u0401\u0451\xc0-\xff\xb5]",casing:"upper"},"#":{validator:"[0-9A-Fa-f]",casing:"upper"}});var ipValidatorRegex=new RegExp("25[0-5]|2[0-4][0-9]|[01][0-9][0-9]");function ipValidator(chrs,maskset,pos,strict,opts){return chrs=-1<pos-1&&"."!==maskset.buffer[pos-1]?(chrs=maskset.buffer[pos-1]+chrs,-1<pos-2&&"."!==maskset.buffer[pos-2]?maskset.buffer[pos-2]+chrs:"0"+chrs):"00"+chrs,ipValidatorRegex.test(chrs)}Inputmask.extendAliases({cssunit:{regex:"[+-]?[0-9]+\\.?([0-9]+)?(px|em|rem|ex|%|in|cm|mm|pt|pc)"},url:{regex:"(https?|ftp)//.*",autoUnmask:!1},ip:{mask:"i[i[i]].j[j[j]].k[k[k]].l[l[l]]",definitions:{i:{validator:ipValidator},j:{validator:ipValidator},k:{validator:ipValidator},l:{validator:ipValidator}},onUnMask:function onUnMask(maskedValue,unmaskedValue,opts){return maskedValue},inputmode:"numeric"},email:{mask:"*{1,64}[.*{1,64}][.*{1,64}][.*{1,63}]@-{1,63}.-{1,63}[.-{1,63}][.-{1,63}]",greedy:!1,casing:"lower",onBeforePaste:function onBeforePaste(pastedValue,opts){return pastedValue=pastedValue.toLowerCase(),pastedValue.replace("mailto:","")},definitions:{"*":{validator:"[0-9\uff11-\uff19A-Za-z\u0410-\u044f\u0401\u0451\xc0-\xff\xb5!#$%&'*+/=?^_`{|}~-]"},"-":{validator:"[0-9A-Za-z-]"}},onUnMask:function onUnMask(maskedValue,unmaskedValue,opts){return maskedValue},inputmode:"email"},mac:{mask:"##:##:##:##:##:##"},vin:{mask:"V{13}9{4}",definitions:{V:{validator:"[A-HJ-NPR-Za-hj-npr-z\\d]",casing:"upper"}},clearIncomplete:!0,autoUnmask:!0}}),module.exports=Inputmask},function(module,exports,__webpack_require__){"use strict";function _typeof(obj){return _typeof="function"==typeof Symbol&&"symbol"==typeof Symbol.iterator?function _typeof(obj){return typeof obj}:function _typeof(obj){return obj&&"function"==typeof Symbol&&obj.constructor===Symbol&&obj!==Symbol.prototype?"symbol":typeof obj},_typeof(obj)}var $=__webpack_require__(1),window=__webpack_require__(3),document=window.document,ua=window.navigator&&window.navigator.userAgent||"",ie=0<ua.indexOf("MSIE ")||0<ua.indexOf("Trident/"),mobile="ontouchstart"in window,iemobile=/iemobile/i.test(ua),iphone=/iphone/i.test(ua)&&!iemobile;module.exports=function maskScope(actionObj,maskset,opts){maskset=maskset||this.maskset,opts=opts||this.opts;var inputmask=this,el=this.el,isRTL=this.isRTL||(this.isRTL=opts.numericInput),undoValue,$el,skipKeyPressEvent=!1,skipInputEvent=!1,validationEvent=!1,ignorable=!1,maxLength,mouseEnter=!1,originalPlaceholder=void 0;function getMaskTemplate(baseOnInput,minimalPos,includeMode,noJit,clearOptionalTail){var greedy=opts.greedy;clearOptionalTail&&(opts.greedy=!1),minimalPos=minimalPos||0;var maskTemplate=[],ndxIntlzr,pos=0,test,testPos;do{if(!0===baseOnInput&&maskset.validPositions[pos])testPos=clearOptionalTail&&!0===maskset.validPositions[pos].match.optionality&&void 0===maskset.validPositions[pos+1]&&(!0===maskset.validPositions[pos].generatedInput||maskset.validPositions[pos].input==opts.skipOptionalPartCharacter&&0<pos)?determineTestTemplate(pos,getTests(pos,ndxIntlzr,pos-1)):maskset.validPositions[pos],test=testPos.match,ndxIntlzr=testPos.locator.slice(),maskTemplate.push(!0===includeMode?testPos.input:!1===includeMode?test.nativeDef:getPlaceholder(pos,test));else{testPos=getTestTemplate(pos,ndxIntlzr,pos-1),test=testPos.match,ndxIntlzr=testPos.locator.slice();var jitMasking=!0!==noJit&&(!1!==opts.jitMasking?opts.jitMasking:test.jit);(!1===jitMasking||void 0===jitMasking||"number"==typeof jitMasking&&isFinite(jitMasking)&&pos<jitMasking)&&maskTemplate.push(!1===includeMode?test.nativeDef:getPlaceholder(pos,test))}"auto"===opts.keepStatic&&test.newBlockMarker&&!0!==test.static&&(opts.keepStatic=pos-1),pos++}while((void 0===maxLength||pos<maxLength)&&(!0!==test.static||""!==test.def)||pos<minimalPos);return""===maskTemplate[maskTemplate.length-1]&&maskTemplate.pop(),!1===includeMode&&void 0!==maskset.maskLength||(maskset.maskLength=pos-1),opts.greedy=greedy,maskTemplate}function resetMaskSet(soft){maskset.buffer=void 0,!0!==soft&&(maskset.validPositions={},maskset.p=0)}function getLastValidPosition(closestTo,strict,validPositions){var before=-1,after=-1,valids=validPositions||maskset.validPositions;for(var posNdx in void 0===closestTo&&(closestTo=-1),valids){var psNdx=parseInt(posNdx);valids[psNdx]&&(strict||!0!==valids[psNdx].generatedInput)&&(psNdx<=closestTo&&(before=psNdx),closestTo<=psNdx&&(after=psNdx))}return-1===before||before==closestTo?after:-1==after?before:closestTo-before<after-closestTo?before:after}function getDecisionTaker(tst){var decisionTaker=tst.locator[tst.alternation];return"string"==typeof decisionTaker&&0<decisionTaker.length&&(decisionTaker=decisionTaker.split(",")[0]),void 0!==decisionTaker?decisionTaker.toString():""}function getLocator(tst,align){var locator=(null!=tst.alternation?tst.mloc[getDecisionTaker(tst)]:tst.locator).join("");if(""!==locator)for(;locator.length<align;)locator+="0";return locator}function determineTestTemplate(pos,tests){pos=0<pos?pos-1:0;for(var altTest=getTest(pos),targetLocator=getLocator(altTest),tstLocator,closest,bestMatch,ndx=0;ndx<tests.length;ndx++){var tst=tests[ndx];tstLocator=getLocator(tst,targetLocator.length);var distance=Math.abs(tstLocator-targetLocator);(void 0===closest||""!==tstLocator&&distance<closest||bestMatch&&!opts.greedy&&bestMatch.match.optionality&&"master"===bestMatch.match.newBlockMarker&&(!tst.match.optionality||!tst.match.newBlockMarker)||bestMatch&&bestMatch.match.optionalQuantifier&&!tst.match.optionalQuantifier)&&(closest=distance,bestMatch=tst)}return bestMatch}function getTestTemplate(pos,ndxIntlzr,tstPs){return maskset.validPositions[pos]||determineTestTemplate(pos,getTests(pos,ndxIntlzr?ndxIntlzr.slice():ndxIntlzr,tstPs))}function getTest(pos,tests){return maskset.validPositions[pos]?maskset.validPositions[pos]:(tests||getTests(pos))[0]}function positionCanMatchDefinition(pos,testDefinition,opts){for(var valid=!1,tests=getTests(pos),defProp=opts.shiftPositions?"def":"nativeDef",tndx=0;tndx<tests.length;tndx++)if(tests[tndx].match&&tests[tndx].match[defProp]===testDefinition.match[defProp]){valid=!0;break}return!1===valid&&void 0!==maskset.jitOffset[pos]&&(valid=positionCanMatchDefinition(pos+maskset.jitOffset[pos],testDefinition,opts)),valid}function getTests(pos,ndxIntlzr,tstPs){var maskTokens=maskset.maskToken,testPos=ndxIntlzr?tstPs:0,ndxInitializer=ndxIntlzr?ndxIntlzr.slice():[0],matches=[],insertStop=!1,latestMatch,cacheDependency=ndxIntlzr?ndxIntlzr.join(""):"";function resolveTestFromToken(maskToken,ndxInitializer,loopNdx,quantifierRecurse){function handleMatch(match,loopNdx,quantifierRecurse){function isFirstMatch(latestMatch,tokenGroup){var firstMatch=0===$.inArray(latestMatch,tokenGroup.matches);return firstMatch||$.each(tokenGroup.matches,function(ndx,match){if(!0===match.isQuantifier?firstMatch=isFirstMatch(latestMatch,tokenGroup.matches[ndx-1]):Object.prototype.hasOwnProperty.call(match,"matches")&&(firstMatch=isFirstMatch(latestMatch,match)),firstMatch)return!1}),firstMatch}function resolveNdxInitializer(pos,alternateNdx,targetAlternation){var bestMatch,indexPos;if((maskset.tests[pos]||maskset.validPositions[pos])&&$.each(maskset.tests[pos]||[maskset.validPositions[pos]],function(ndx,lmnt){if(lmnt.mloc[alternateNdx])return bestMatch=lmnt,!1;var alternation=void 0!==targetAlternation?targetAlternation:lmnt.alternation,ndxPos=void 0!==lmnt.locator[alternation]?lmnt.locator[alternation].toString().indexOf(alternateNdx):-1;(void 0===indexPos||ndxPos<indexPos)&&-1!==ndxPos&&(bestMatch=lmnt,indexPos=ndxPos)}),bestMatch){var bestMatchAltIndex=bestMatch.locator[bestMatch.alternation],locator=bestMatch.mloc[alternateNdx]||bestMatch.mloc[bestMatchAltIndex]||bestMatch.locator;return locator.slice((void 0!==targetAlternation?targetAlternation:bestMatch.alternation)+1)}return void 0!==targetAlternation?resolveNdxInitializer(pos,alternateNdx):void 0}function isSubsetOf(source,target){function expand(pattern){for(var expanded=[],start=-1,end,i=0,l=pattern.length;i<l;i++)if("-"===pattern.charAt(i))for(end=pattern.charCodeAt(i+1);++start<end;)expanded.push(String.fromCharCode(start));else start=pattern.charCodeAt(i),expanded.push(pattern.charAt(i));return expanded.join("")}return source.match.def===target.match.nativeDef||!(!(opts.regex||source.match.fn instanceof RegExp&&target.match.fn instanceof RegExp)||!0===source.match.static||!0===target.match.static)&&-1!==expand(target.match.fn.toString().replace(/[[\]/]/g,"")).indexOf(expand(source.match.fn.toString().replace(/[[\]/]/g,"")))}function staticCanMatchDefinition(source,target){return!0===source.match.static&&!0!==target.match.static&&target.match.fn.test(source.match.def,maskset,pos,!1,opts,!1)}function setMergeLocators(targetMatch,altMatch){if(void 0===altMatch||targetMatch.alternation===altMatch.alternation&&-1===targetMatch.locator[targetMatch.alternation].toString().indexOf(altMatch.locator[altMatch.alternation])){targetMatch.mloc=targetMatch.mloc||{};var locNdx=targetMatch.locator[targetMatch.alternation];if(void 0!==locNdx){if("string"==typeof locNdx&&(locNdx=locNdx.split(",")[0]),void 0===targetMatch.mloc[locNdx]&&(targetMatch.mloc[locNdx]=targetMatch.locator.slice()),void 0!==altMatch){for(var ndx in altMatch.mloc)"string"==typeof ndx&&(ndx=ndx.split(",")[0]),void 0===targetMatch.mloc[ndx]&&(targetMatch.mloc[ndx]=altMatch.mloc[ndx]);targetMatch.locator[targetMatch.alternation]=Object.keys(targetMatch.mloc).join(",")}return!0}targetMatch.alternation=void 0}return!1}if(500<testPos&&void 0!==quantifierRecurse)throw"Inputmask: There is probably an error in your mask definition or in the code. Create an issue on github with an example of the mask you are using. "+maskset.mask;if(testPos===pos&&void 0===match.matches)return matches.push({match:match,locator:loopNdx.reverse(),cd:cacheDependency,mloc:{}}),!0;if(void 0!==match.matches){if(match.isGroup&&quantifierRecurse!==match){if(match=handleMatch(maskToken.matches[$.inArray(match,maskToken.matches)+1],loopNdx,quantifierRecurse),match)return!0}else if(match.isOptional){var optionalToken=match,mtchsNdx=matches.length;if(match=resolveTestFromToken(match,ndxInitializer,loopNdx,quantifierRecurse),match){if($.each(matches,function(ndx,mtch){mtchsNdx<=ndx&&(mtch.match.optionality=!0)}),latestMatch=matches[matches.length-1].match,void 0!==quantifierRecurse||!isFirstMatch(latestMatch,optionalToken))return!0;insertStop=!0,testPos=pos}}else if(match.isAlternator){var alternateToken=match,malternateMatches=[],maltMatches,currentMatches=matches.slice(),loopNdxCnt=loopNdx.length,altIndex=0<ndxInitializer.length?ndxInitializer.shift():-1;if(-1===altIndex||"string"==typeof altIndex){var currentPos=testPos,ndxInitializerClone=ndxInitializer.slice(),altIndexArr=[],amndx;if("string"==typeof altIndex)altIndexArr=altIndex.split(",");else for(amndx=0;amndx<alternateToken.matches.length;amndx++)altIndexArr.push(amndx.toString());if(void 0!==maskset.excludes[pos]){for(var altIndexArrClone=altIndexArr.slice(),i=0,el=maskset.excludes[pos].length;i<el;i++)altIndexArr.splice(altIndexArr.indexOf(maskset.excludes[pos][i].toString()),1);0===altIndexArr.length&&(delete maskset.excludes[pos],altIndexArr=altIndexArrClone)}(!0===opts.keepStatic||isFinite(parseInt(opts.keepStatic))&&currentPos>=opts.keepStatic)&&(altIndexArr=altIndexArr.slice(0,1));for(var unMatchedAlternation=!1,ndx=0;ndx<altIndexArr.length;ndx++){amndx=parseInt(altIndexArr[ndx]),matches=[],ndxInitializer="string"==typeof altIndex&&resolveNdxInitializer(testPos,amndx,loopNdxCnt)||ndxInitializerClone.slice(),alternateToken.matches[amndx]&&handleMatch(alternateToken.matches[amndx],[amndx].concat(loopNdx),quantifierRecurse)?match=!0:0===ndx&&(unMatchedAlternation=!0),maltMatches=matches.slice(),testPos=currentPos,matches=[];for(var ndx1=0;ndx1<maltMatches.length;ndx1++){var altMatch=maltMatches[ndx1],dropMatch=!1;altMatch.match.jit=altMatch.match.jit||unMatchedAlternation,altMatch.alternation=altMatch.alternation||loopNdxCnt,setMergeLocators(altMatch);for(var ndx2=0;ndx2<malternateMatches.length;ndx2++){var altMatch2=malternateMatches[ndx2];if("string"!=typeof altIndex||void 0!==altMatch.alternation&&-1!==$.inArray(altMatch.locator[altMatch.alternation].toString(),altIndexArr)){if(altMatch.match.nativeDef===altMatch2.match.nativeDef){dropMatch=!0,setMergeLocators(altMatch2,altMatch);break}if(isSubsetOf(altMatch,altMatch2)){setMergeLocators(altMatch,altMatch2)&&(dropMatch=!0,malternateMatches.splice(malternateMatches.indexOf(altMatch2),0,altMatch));break}if(isSubsetOf(altMatch2,altMatch)){setMergeLocators(altMatch2,altMatch);break}if(staticCanMatchDefinition(altMatch,altMatch2)){setMergeLocators(altMatch,altMatch2)&&(dropMatch=!0,malternateMatches.splice(malternateMatches.indexOf(altMatch2),0,altMatch));break}}}dropMatch||malternateMatches.push(altMatch)}}matches=currentMatches.concat(malternateMatches),testPos=pos,insertStop=0<matches.length,match=0<malternateMatches.length,ndxInitializer=ndxInitializerClone.slice()}else match=handleMatch(alternateToken.matches[altIndex]||maskToken.matches[altIndex],[altIndex].concat(loopNdx),quantifierRecurse);if(match)return!0}else if(match.isQuantifier&&quantifierRecurse!==maskToken.matches[$.inArray(match,maskToken.matches)-1])for(var qt=match,qndx=0<ndxInitializer.length?ndxInitializer.shift():0;qndx<(isNaN(qt.quantifier.max)?qndx+1:qt.quantifier.max)&&testPos<=pos;qndx++){var tokenGroup=maskToken.matches[$.inArray(qt,maskToken.matches)-1];if(match=handleMatch(tokenGroup,[qndx].concat(loopNdx),tokenGroup),match){if(latestMatch=matches[matches.length-1].match,latestMatch.optionalQuantifier=qndx>=qt.quantifier.min,latestMatch.jit=(qndx||1)*tokenGroup.matches.indexOf(latestMatch)>=qt.quantifier.jit,latestMatch.optionalQuantifier&&isFirstMatch(latestMatch,tokenGroup)){insertStop=!0,testPos=pos;break}return latestMatch.jit&&(maskset.jitOffset[pos]=tokenGroup.matches.length-tokenGroup.matches.indexOf(latestMatch)),!0}}else if(match=resolveTestFromToken(match,ndxInitializer,loopNdx,quantifierRecurse),match)return!0}else testPos++}for(var tndx=0<ndxInitializer.length?ndxInitializer.shift():0;tndx<maskToken.matches.length;tndx++)if(!0!==maskToken.matches[tndx].isQuantifier){var match=handleMatch(maskToken.matches[tndx],[tndx].concat(loopNdx),quantifierRecurse);if(match&&testPos===pos)return match;if(pos<testPos)break}}function mergeLocators(pos,tests){var locator=[];return $.isArray(tests)||(tests=[tests]),0<tests.length&&(void 0===tests[0].alternation||!0===opts.keepStatic?(locator=determineTestTemplate(pos,tests.slice()).locator.slice(),0===locator.length&&(locator=tests[0].locator.slice())):$.each(tests,function(ndx,tst){if(""!==tst.def)if(0===locator.length)locator=tst.locator.slice();else for(var i=0;i<locator.length;i++)tst.locator[i]&&-1===locator[i].toString().indexOf(tst.locator[i])&&(locator[i]+=","+tst.locator[i])})),locator}if(-1<pos&&(void 0===maxLength||pos<maxLength)){if(void 0===ndxIntlzr){for(var previousPos=pos-1,test;void 0===(test=maskset.validPositions[previousPos]||maskset.tests[previousPos])&&-1<previousPos;)previousPos--;void 0!==test&&-1<previousPos&&(ndxInitializer=mergeLocators(previousPos,test),cacheDependency=ndxInitializer.join(""),testPos=previousPos)}if(maskset.tests[pos]&&maskset.tests[pos][0].cd===cacheDependency)return maskset.tests[pos];for(var mtndx=ndxInitializer.shift();mtndx<maskTokens.length;mtndx++){var match=resolveTestFromToken(maskTokens[mtndx],ndxInitializer,[mtndx]);if(match&&testPos===pos||pos<testPos)break}}return 0!==matches.length&&!insertStop||matches.push({match:{fn:null,static:!0,optionality:!1,casing:null,def:"",placeholder:""},locator:[],mloc:{},cd:cacheDependency}),void 0!==ndxIntlzr&&maskset.tests[pos]?$.extend(!0,[],matches):(maskset.tests[pos]=$.extend(!0,[],matches),maskset.tests[pos])}function getBufferTemplate(){return void 0===maskset._buffer&&(maskset._buffer=getMaskTemplate(!1,1),void 0===maskset.buffer&&(maskset.buffer=maskset._buffer.slice())),maskset._buffer}function getBuffer(noCache){return void 0!==maskset.buffer&&!0!==noCache||(maskset.buffer=getMaskTemplate(!0,getLastValidPosition(),!0),void 0===maskset._buffer&&(maskset._buffer=maskset.buffer.slice())),maskset.buffer}function refreshFromBuffer(start,end,buffer){var i,p,skipOptionalPartCharacter=opts.skipOptionalPartCharacter;if(opts.skipOptionalPartCharacter="",!0===start)resetMaskSet(),maskset.tests={},start=0,end=buffer.length;else for(i=start;i<end;i++)delete maskset.validPositions[i];for(p=start,i=start;i<end;i++){var valResult=isValid(p,buffer[i],!0,!0);!1!==valResult&&(p=void 0!==valResult.caret&&valResult.caret>valResult.pos?valResult.caret:valResult.pos+1)}opts.skipOptionalPartCharacter=skipOptionalPartCharacter}function casing(elem,test,pos){switch(opts.casing||test.casing){case"upper":elem=elem.toUpperCase();break;case"lower":elem=elem.toLowerCase();break;case"title":var posBefore=maskset.validPositions[pos-1];elem=0===pos||posBefore&&posBefore.input===String.fromCharCode(Inputmask.keyCode.SPACE)?elem.toUpperCase():elem.toLowerCase();break;default:if($.isFunction(opts.casing)){var args=Array.prototype.slice.call(arguments);args.push(maskset.validPositions),elem=opts.casing.apply(this,args)}}return elem}function checkAlternationMatch(altArr1,altArr2,na){for(var altArrC=opts.greedy?altArr2:altArr2.slice(0,1),isMatch=!1,naArr=void 0!==na?na.split(","):[],naNdx,i=0;i<naArr.length;i++)-1!==(naNdx=altArr1.indexOf(naArr[i]))&&altArr1.splice(naNdx,1);for(var alndx=0;alndx<altArr1.length;alndx++)if(-1!==$.inArray(altArr1[alndx],altArrC)){isMatch=!0;break}return isMatch}function alternate(maskPos,c,strict,fromIsValid,rAltPos,selection){var validPsClone=$.extend(!0,{},maskset.validPositions),tstClone=$.extend(!0,{},maskset.tests),lastAlt,alternation,isValidRslt=!1,returnRslt=!1,altPos,prevAltPos,i,validPos,decisionPos,lAltPos=void 0!==rAltPos?rAltPos:getLastValidPosition(),nextPos,input,begin,end;if(selection&&(begin=selection.begin,end=selection.end,selection.begin>selection.end&&(begin=selection.end,end=selection.begin)),-1===lAltPos&&void 0===rAltPos)lastAlt=0,prevAltPos=getTest(lastAlt),alternation=prevAltPos.alternation;else for(;0<=lAltPos;lAltPos--)if(altPos=maskset.validPositions[lAltPos],altPos&&void 0!==altPos.alternation){if(prevAltPos&&prevAltPos.locator[altPos.alternation]!==altPos.locator[altPos.alternation])break;lastAlt=lAltPos,alternation=maskset.validPositions[lastAlt].alternation,prevAltPos=altPos}if(void 0!==alternation){decisionPos=parseInt(lastAlt),maskset.excludes[decisionPos]=maskset.excludes[decisionPos]||[],!0!==maskPos&&maskset.excludes[decisionPos].push(getDecisionTaker(prevAltPos));var validInputs=[],resultPos=-1;for(i=decisionPos;i<getLastValidPosition(void 0,!0)+1;i++)-1===resultPos&&maskPos<=i&&void 0!==c&&(validInputs.push(c),resultPos=validInputs.length-1),validPos=maskset.validPositions[i],validPos&&!0!==validPos.generatedInput&&(void 0===selection||i<begin||end<=i)&&validInputs.push(validPos.input),delete maskset.validPositions[i];for(-1===resultPos&&void 0!==c&&(validInputs.push(c),resultPos=validInputs.length-1);void 0!==maskset.excludes[decisionPos]&&maskset.excludes[decisionPos].length<10;){for(maskset.tests[decisionPos]=void 0,resetMaskSet(!0),isValidRslt=!0,i=0;i<validInputs.length&&(nextPos=isValidRslt.caret||getLastValidPosition(void 0,!0)+1,input=validInputs[i],isValidRslt=isValid(nextPos,input,!1,fromIsValid,!0));i++)i===resultPos&&(returnRslt=isValidRslt),1==maskPos&&isValidRslt&&(returnRslt={caretPos:i});if(isValidRslt)break;if(resetMaskSet(),prevAltPos=getTest(decisionPos),maskset.validPositions=$.extend(!0,{},validPsClone),maskset.tests=$.extend(!0,{},tstClone),!maskset.excludes[decisionPos]){returnRslt=alternate(maskPos,c,strict,fromIsValid,decisionPos-1,selection);break}var decisionTaker=getDecisionTaker(prevAltPos);if(-1!==maskset.excludes[decisionPos].indexOf(decisionTaker)){returnRslt=alternate(maskPos,c,strict,fromIsValid,decisionPos-1,selection);break}for(maskset.excludes[decisionPos].push(decisionTaker),i=decisionPos;i<getLastValidPosition(void 0,!0)+1;i++)delete maskset.validPositions[i]}}return delete maskset.excludes[decisionPos],returnRslt}function isValid(pos,c,strict,fromIsValid,fromAlternate,validateOnly){function isSelection(posObj){return isRTL?1<posObj.begin-posObj.end||posObj.begin-posObj.end==1:1<posObj.end-posObj.begin||posObj.end-posObj.begin==1}strict=!0===strict;var maskPos=pos;function processCommandObject(commandObj){if(void 0!==commandObj){if(void 0!==commandObj.remove&&($.isArray(commandObj.remove)||(commandObj.remove=[commandObj.remove]),$.each(commandObj.remove.sort(function(a,b){return b.pos-a.pos}),function(ndx,lmnt){revalidateMask({begin:lmnt,end:lmnt+1})}),commandObj.remove=void 0),void 0!==commandObj.insert&&($.isArray(commandObj.insert)||(commandObj.insert=[commandObj.insert]),$.each(commandObj.insert.sort(function(a,b){return a.pos-b.pos}),function(ndx,lmnt){""!==lmnt.c&&isValid(lmnt.pos,lmnt.c,void 0===lmnt.strict||lmnt.strict,void 0!==lmnt.fromIsValid?lmnt.fromIsValid:fromIsValid)}),commandObj.insert=void 0),commandObj.refreshFromBuffer&&commandObj.buffer){var refresh=commandObj.refreshFromBuffer;refreshFromBuffer(!0===refresh?refresh:refresh.start,refresh.end,commandObj.buffer),commandObj.refreshFromBuffer=void 0}void 0!==commandObj.rewritePosition&&(maskPos=commandObj.rewritePosition,commandObj=!0)}return commandObj}function _isValid(position,c,strict){var rslt=!1;return $.each(getTests(position),function(ndx,tst){var test=tst.match;if(getBuffer(!0),rslt=null!=test.fn?test.fn.test(c,maskset,position,strict,opts,isSelection(pos)):(c===test.def||c===opts.skipOptionalPartCharacter)&&""!==test.def&&{c:getPlaceholder(position,test,!0)||test.def,pos:position},!1!==rslt){var elem=void 0!==rslt.c?rslt.c:c,validatedPos=position;return elem=elem===opts.skipOptionalPartCharacter&&!0===test.static?getPlaceholder(position,test,!0)||test.def:elem,rslt=processCommandObject(rslt),!0!==rslt&&void 0!==rslt.pos&&rslt.pos!==position&&(validatedPos=rslt.pos),!0!==rslt&&void 0===rslt.pos&&void 0===rslt.c?!1:(!1===revalidateMask(pos,$.extend({},tst,{input:casing(elem,test,validatedPos)}),fromIsValid,validatedPos)&&(rslt=!1),!1)}}),rslt}void 0!==pos.begin&&(maskPos=isRTL?pos.end:pos.begin);var result=!0,positionsClone=$.extend(!0,{},maskset.validPositions);if($.isFunction(opts.preValidation)&&!0!==fromIsValid&&!0!==validateOnly&&!0!==fromAlternate&&(result=opts.preValidation(getBuffer(),maskPos,c,isSelection(pos),opts,maskset,pos,strict),result=processCommandObject(result)),!0===result){if(void 0===maxLength||maskPos<maxLength){if(result=_isValid(maskPos,c,strict),(!strict||!0===fromIsValid)&&!1===result&&!0!==validateOnly){var currentPosValid=maskset.validPositions[maskPos];if(!currentPosValid||!0!==currentPosValid.match.static||currentPosValid.match.def!==c&&c!==opts.skipOptionalPartCharacter){if(opts.insertMode||void 0===maskset.validPositions[seekNext(maskPos)]||pos.end>maskPos){var skip=!1;if(maskset.jitOffset[maskPos]&&void 0===maskset.validPositions[seekNext(maskPos)]&&(result=isValid(maskPos+maskset.jitOffset[maskPos],c,!0),!1!==result&&(!0!==fromAlternate&&(result.caret=maskPos),skip=!0)),pos.end>maskPos&&(maskset.validPositions[maskPos]=void 0),!skip&&!isMask(maskPos,!0))for(var nPos=maskPos+1,snPos=seekNext(maskPos);nPos<=snPos;nPos++)if(result=_isValid(nPos,c,strict),!1!==result){result=trackbackPositions(maskPos,void 0!==result.pos?result.pos:nPos)||result,maskPos=nPos;break}}}else result={caret:seekNext(maskPos)}}}else result=!1;!1!==result||!1!==opts.keepStatic&&!isComplete(getBuffer())&&0!==maskPos||strict||!0===fromAlternate?isSelection(pos)&&maskset.tests[maskPos]&&1<maskset.tests[maskPos].length&&!0===opts.keepStatic&&!strict&&!0!==fromAlternate&&(result=alternate(!0)):result=alternate(maskPos,c,strict,fromIsValid,void 0,pos),!0===result&&(result={pos:maskPos})}if($.isFunction(opts.postValidation)&&!1!==result&&!0!==fromIsValid&&!0!==validateOnly){var postResult=opts.postValidation(getBuffer(!0),void 0!==pos.begin?isRTL?pos.end:pos.begin:pos,result,opts,maskset,strict);void 0!==postResult&&(result=!0===postResult?result:postResult)}result&&void 0===result.pos&&(result.pos=maskPos),!1===result||!0===validateOnly?(resetMaskSet(!0),maskset.validPositions=$.extend(!0,{},positionsClone)):trackbackPositions(void 0,maskPos,!0);var endResult=processCommandObject(result);return endResult}function trackbackPositions(originalPos,newPos,fillOnly){if(void 0===originalPos)for(originalPos=newPos-1;0<originalPos&&!maskset.validPositions[originalPos];originalPos--);for(var ps=originalPos;ps<newPos;ps++)if(void 0===maskset.validPositions[ps]&&!isMask(ps,!0)){var vp=0==ps?getTest(ps):maskset.validPositions[ps-1];if(vp){var tests=getTests(ps).slice();""===tests[tests.length-1].match.def&&tests.pop();var bestMatch=determineTestTemplate(ps,tests),np;if(bestMatch&&(!0!==bestMatch.match.jit||"master"===bestMatch.match.newBlockMarker&&(np=maskset.validPositions[ps+1])&&!0===np.match.optionalQuantifier)&&(bestMatch=$.extend({},bestMatch,{input:getPlaceholder(ps,bestMatch.match,!0)||bestMatch.match.def}),bestMatch.generatedInput=!0,revalidateMask(ps,bestMatch,!0),!0!==fillOnly)){var cvpInput=maskset.validPositions[newPos].input;return maskset.validPositions[newPos]=void 0,isValid(newPos,cvpInput,!0,!0)}}}}function revalidateMask(pos,validTest,fromIsValid,validatedPos){function IsEnclosedStatic(pos,valids,selection){var posMatch=valids[pos];if(void 0===posMatch||!0!==posMatch.match.static||!0===posMatch.match.optionality||void 0!==valids[0]&&void 0!==valids[0].alternation)return!1;var prevMatch=selection.begin<=pos-1?valids[pos-1]&&!0===valids[pos-1].match.static&&valids[pos-1]:valids[pos-1],nextMatch=selection.end>pos+1?valids[pos+1]&&!0===valids[pos+1].match.static&&valids[pos+1]:valids[pos+1];return prevMatch&&nextMatch}var offset=0,begin=void 0!==pos.begin?pos.begin:pos,end=void 0!==pos.end?pos.end:pos;if(pos.begin>pos.end&&(begin=pos.end,end=pos.begin),void 0===validTest&&!1===opts.insertMode&&end<maskset.maskLength&&(0===begin&&0===end||(begin+=1,end+=1)),validatedPos=void 0!==validatedPos?validatedPos:begin,begin!==end||opts.insertMode&&void 0!==maskset.validPositions[validatedPos]&&void 0===fromIsValid||void 0===validTest){var positionsClone=$.extend(!0,{},maskset.validPositions),lvp=void 0===validTest&&!1===opts.insertMode?1<end?end-1:end:getLastValidPosition(void 0,!0),i;for(maskset.p=begin,i=lvp;begin<=i;i--)delete maskset.validPositions[i],void 0===validTest&&delete maskset.tests[i+1];var valid=!0,j=validatedPos,posMatch=j,t;if(i=j,validTest&&(maskset.validPositions[validatedPos]=$.extend(!0,{},validTest),posMatch++,j++,begin<end&&i++),validTest||opts.insertMode)for(;i<=lvp;i++){if(void 0!==(t=positionsClone[i])&&!0!==t.generatedInput&&(end<=i||begin<=i&&IsEnclosedStatic(i,positionsClone,{begin:begin,end:end}))){for(;""!==getTest(posMatch).match.def;){if(positionCanMatchDefinition(posMatch,t,opts)||"+"===t.match.def){"+"===t.match.def&&getBuffer(!0);var result=isValid(posMatch,t.input,"+"!==t.match.def,"+"!==t.match.def);if(valid=!1!==result,j=(result.pos||posMatch)+1,!valid)break}else valid=!1;if(valid){void 0===validTest&&t.match.static&&i===pos.begin&&offset++;break}if(!valid&&posMatch>maskset.maskLength)break;posMatch++}""==getTest(posMatch).match.def&&(valid=!1),posMatch=j}if(!valid)break}if(!valid)return maskset.validPositions=$.extend(!0,{},positionsClone),resetMaskSet(!0),!1}else validTest&&(maskset.validPositions[validatedPos]=$.extend(!0,{},validTest));return resetMaskSet(!0),offset}function isMask(pos,strict,fuzzy){var test=getTestTemplate(pos).match;if(""===test.def&&(test=getTest(pos).match),!0!==test.static)return test.fn;if(!0===fuzzy&&void 0!==maskset.validPositions[pos]&&!0!==maskset.validPositions[pos].generatedInput)return!0;if(!0!==strict&&-1<pos){var tests=getTests(pos);return tests.length>1+(""===tests[tests.length-1].match.def?1:0)}return!1}function seekNext(pos,newBlock){for(var position=pos+1;""!==getTest(position).match.def&&(!0===newBlock&&(!0!==getTest(position).match.newBlockMarker||!isMask(position,void 0,!0))||!0!==newBlock&&!isMask(position,void 0,!0));)position++;return position}function seekPrevious(pos,newBlock){var position=pos,tests;if(position<=0)return 0;for(;0<--position&&(!0===newBlock&&!0!==getTest(position).match.newBlockMarker||!0!==newBlock&&!isMask(position,void 0,!0)&&(tests=getTests(position),tests.length<2||2===tests.length&&""===tests[1].match.def)););return position}function writeBuffer(input,buffer,caretPos,event,triggerEvents){if(event&&$.isFunction(opts.onBeforeWrite)){var result=opts.onBeforeWrite.call(inputmask,event,buffer,caretPos,opts);if(result){if(result.refreshFromBuffer){var refresh=result.refreshFromBuffer;refreshFromBuffer(!0===refresh?refresh:refresh.start,refresh.end,result.buffer||buffer),buffer=getBuffer(!0)}void 0!==caretPos&&(caretPos=void 0!==result.caret?result.caret:caretPos)}}if(void 0!==input&&(input.inputmask._valueSet(buffer.join("")),void 0===caretPos||void 0!==event&&"blur"===event.type||caret(input,caretPos),!0===triggerEvents)){var $input=$(input),nptVal=input.inputmask._valueGet();skipInputEvent=!0,$input.trigger("input"),setTimeout(function(){nptVal===getBufferTemplate().join("")?$input.trigger("cleared"):!0===isComplete(buffer)&&$input.trigger("complete")},0)}}function getPlaceholder(pos,test,returnPL){if(test=test||getTest(pos).match,void 0!==test.placeholder||!0===returnPL)return $.isFunction(test.placeholder)?test.placeholder(opts):test.placeholder;if(!0!==test.static)return opts.placeholder.charAt(pos%opts.placeholder.length);if(-1<pos&&void 0===maskset.validPositions[pos]){var tests=getTests(pos),staticAlternations=[],prevTest;if(tests.length>1+(""===tests[tests.length-1].match.def?1:0))for(var i=0;i<tests.length;i++)if(!0!==tests[i].match.optionality&&!0!==tests[i].match.optionalQuantifier&&(!0===tests[i].match.static||void 0===prevTest||!1!==tests[i].match.fn.test(prevTest.match.def,maskset,pos,!0,opts))&&(staticAlternations.push(tests[i]),!0===tests[i].match.static&&(prevTest=tests[i]),1<staticAlternations.length&&/[0-9a-bA-Z]/.test(staticAlternations[0].match.def)))return opts.placeholder.charAt(pos%opts.placeholder.length)}return test.def}function HandleNativePlaceholder(npt,value){if(ie){if(npt.inputmask._valueGet()!==value&&(npt.placeholder!==value||""===npt.placeholder)){var buffer=getBuffer().slice(),nptValue=npt.inputmask._valueGet();if(nptValue!==value){var lvp=getLastValidPosition();-1===lvp&&nptValue===getBufferTemplate().join("")?buffer=[]:-1!==lvp&&clearOptionalTail(buffer),writeBuffer(npt,buffer)}}}else npt.placeholder!==value&&(npt.placeholder=value,""===npt.placeholder&&npt.removeAttribute("placeholder"))}function determineNewCaretPosition(selectedCaret,tabbed){function doRadixFocus(clickPos){if(""!==opts.radixPoint&&0!==opts.digits){var vps=maskset.validPositions;if(void 0===vps[clickPos]||vps[clickPos].input===getPlaceholder(clickPos)){if(clickPos<seekNext(-1))return!0;var radixPos=$.inArray(opts.radixPoint,getBuffer());if(-1!==radixPos){for(var vp in vps)if(vps[vp]&&radixPos<vp&&vps[vp].input!==getPlaceholder(vp))return!1;return!0}}}return!1}if(tabbed&&(isRTL?selectedCaret.end=selectedCaret.begin:selectedCaret.begin=selectedCaret.end),selectedCaret.begin===selectedCaret.end)switch(opts.positionCaretOnClick){case"none":break;case"select":return{begin:0,end:getBuffer().length};case"ignore":return seekNext(getLastValidPosition());case"radixFocus":if(doRadixFocus(selectedCaret.begin)){var radixPos=getBuffer().join("").indexOf(opts.radixPoint);return opts.numericInput?seekNext(radixPos):radixPos}default:var clickPosition=selectedCaret.begin,lvclickPosition=getLastValidPosition(clickPosition,!0),lastPosition=seekNext(-1!==lvclickPosition||isMask(0)?lvclickPosition:0);if(clickPosition<lastPosition)return isMask(clickPosition,!0)||isMask(clickPosition-1,!0)?clickPosition:seekNext(clickPosition);var lvp=maskset.validPositions[lvclickPosition],tt=getTestTemplate(lastPosition,lvp?lvp.match.locator:void 0,lvp),placeholder=getPlaceholder(lastPosition,tt.match);if(""!==placeholder&&getBuffer()[lastPosition]!==placeholder&&!0!==tt.match.optionalQuantifier&&!0!==tt.match.newBlockMarker||!isMask(lastPosition,opts.keepStatic)&&tt.match.def===placeholder){var newPos=seekNext(lastPosition);(newPos<=clickPosition||clickPosition===lastPosition)&&(lastPosition=newPos)}return lastPosition}}var EventRuler={on:function on(input,eventName,eventHandler){var ev=function ev(e){e=e.originalEvent||e;var that=this,args;if(void 0===that.inputmask&&"FORM"!==this.nodeName){var imOpts=$.data(that,"_inputmask_opts");imOpts?new Inputmask(imOpts).mask(that):EventRuler.off(that)}else{if("setvalue"===e.type||"FORM"===this.nodeName||!(that.disabled||that.readOnly&&!("keydown"===e.type&&e.ctrlKey&&67===e.keyCode||!1===opts.tabThrough&&e.keyCode===Inputmask.keyCode.TAB))){switch(e.type){case"input":if(!0===skipInputEvent||e.inputType&&"insertCompositionText"===e.inputType)return skipInputEvent=!1,e.preventDefault();if(mobile)return args=arguments,setTimeout(function(){eventHandler.apply(that,args),caret(that,that.inputmask.caretPos,void 0,!0)},0),!1;break;case"keydown":skipKeyPressEvent=!1,skipInputEvent=!1;break;case"keypress":if(!0===skipKeyPressEvent)return e.preventDefault();skipKeyPressEvent=!0;break;case"click":case"focus":return validationEvent?(validationEvent=!1,input.blur(),HandleNativePlaceholder(input,(isRTL?getBuffer().slice().reverse():getBuffer()).join("")),setTimeout(function(){input.focus()},3e3)):(args=arguments,setTimeout(function(){eventHandler.apply(that,args)},0)),!1}var returnVal=eventHandler.apply(that,arguments);return!1===returnVal&&(e.preventDefault(),e.stopPropagation()),returnVal}e.preventDefault()}};input.inputmask.events[eventName]=input.inputmask.events[eventName]||[],input.inputmask.events[eventName].push(ev),-1!==$.inArray(eventName,["submit","reset"])?null!==input.form&&$(input.form).on(eventName,ev):$(input).on(eventName,ev)},off:function off(input,event){var events;input.inputmask&&input.inputmask.events&&(event?(events=[],events[event]=input.inputmask.events[event]):events=input.inputmask.events,$.each(events,function(eventName,evArr){for(;0<evArr.length;){var ev=evArr.pop();-1!==$.inArray(eventName,["submit","reset"])?null!==input.form&&$(input.form).off(eventName,ev):$(input).off(eventName,ev)}delete input.inputmask.events[eventName]}))}},EventHandlers={keydownEvent:function keydownEvent(e){var input=this,$input=$(input),k=e.keyCode,pos=caret(input),kdResult=opts.onKeyDown.call(this,e,getBuffer(),pos,opts);if(void 0!==kdResult)return kdResult;if(k===Inputmask.keyCode.BACKSPACE||k===Inputmask.keyCode.DELETE||iphone&&k===Inputmask.keyCode.BACKSPACE_SAFARI||e.ctrlKey&&k===Inputmask.keyCode.X&&!("oncut"in input))e.preventDefault(),handleRemove(input,k,pos),writeBuffer(input,getBuffer(!0),maskset.p,e,input.inputmask._valueGet()!==getBuffer().join(""));else if(k===Inputmask.keyCode.END||k===Inputmask.keyCode.PAGE_DOWN){e.preventDefault();var caretPos=seekNext(getLastValidPosition());caret(input,e.shiftKey?pos.begin:caretPos,caretPos,!0)}else k===Inputmask.keyCode.HOME&&!e.shiftKey||k===Inputmask.keyCode.PAGE_UP?(e.preventDefault(),caret(input,0,e.shiftKey?pos.begin:0,!0)):(opts.undoOnEscape&&k===Inputmask.keyCode.ESCAPE||90===k&&e.ctrlKey)&&!0!==e.altKey?(checkVal(input,!0,!1,undoValue.split("")),$input.trigger("click")):!0===opts.tabThrough&&k===Inputmask.keyCode.TAB?(!0===e.shiftKey?(!0===getTest(pos.begin).match.static&&(pos.begin=seekNext(pos.begin)),pos.end=seekPrevious(pos.begin,!0),pos.begin=seekPrevious(pos.end,!0)):(pos.begin=seekNext(pos.begin,!0),pos.end=seekNext(pos.begin,!0),pos.end<maskset.maskLength&&pos.end--),pos.begin<maskset.maskLength&&(e.preventDefault(),caret(input,pos.begin,pos.end))):e.shiftKey||!1===opts.insertMode&&(k===Inputmask.keyCode.RIGHT?setTimeout(function(){var caretPos=caret(input);caret(input,caretPos.begin)},0):k===Inputmask.keyCode.LEFT&&setTimeout(function(){var caretPos_begin=translatePosition(input.inputmask.caretPos.begin),caretPos_end=translatePosition(input.inputmask.caretPos.end);caret(input,isRTL?caretPos_begin+(caretPos_begin===maskset.maskLength?0:1):caretPos_begin-(0===caretPos_begin?0:1))},0));ignorable=-1!==$.inArray(k,opts.ignorables)},keypressEvent:function keypressEvent(e,checkval,writeOut,strict,ndx){var input=this,$input=$(input),k=e.which||e.charCode||e.keyCode;if(!(!0===checkval||e.ctrlKey&&e.altKey)&&(e.ctrlKey||e.metaKey||ignorable))return k===Inputmask.keyCode.ENTER&&undoValue!==getBuffer().join("")&&(undoValue=getBuffer().join(""),setTimeout(function(){$input.trigger("change")},0)),!0;if(k){46===k&&!1===e.shiftKey&&""!==opts.radixPoint&&(k=opts.radixPoint.charCodeAt(0));var pos=checkval?{begin:ndx,end:ndx}:caret(input),forwardPosition,c=String.fromCharCode(k);maskset.writeOutBuffer=!0;var valResult=isValid(pos,c,strict);if(!1!==valResult&&(resetMaskSet(!0),forwardPosition=void 0!==valResult.caret?valResult.caret:seekNext(valResult.pos.begin?valResult.pos.begin:valResult.pos),maskset.p=forwardPosition),forwardPosition=opts.numericInput&&void 0===valResult.caret?seekPrevious(forwardPosition):forwardPosition,!1!==writeOut&&(setTimeout(function(){opts.onKeyValidation.call(input,k,valResult,opts)},0),maskset.writeOutBuffer&&!1!==valResult)){var buffer=getBuffer();writeBuffer(input,buffer,forwardPosition,e,!0!==checkval)}if(e.preventDefault(),checkval)return!1!==valResult&&(valResult.forwardPosition=forwardPosition),valResult}},pasteEvent:function pasteEvent(e){var input=this,inputValue=this.inputmask._valueGet(!0),caretPos=caret(this),tempValue;isRTL&&(tempValue=caretPos.end,caretPos.end=caretPos.begin,caretPos.begin=tempValue);var valueBeforeCaret=inputValue.substr(0,caretPos.begin),valueAfterCaret=inputValue.substr(caretPos.end,inputValue.length);if(valueBeforeCaret===(isRTL?getBufferTemplate().reverse():getBufferTemplate()).slice(0,caretPos.begin).join("")&&(valueBeforeCaret=""),valueAfterCaret===(isRTL?getBufferTemplate().reverse():getBufferTemplate()).slice(caretPos.end).join("")&&(valueAfterCaret=""),window.clipboardData&&window.clipboardData.getData)inputValue=valueBeforeCaret+window.clipboardData.getData("Text")+valueAfterCaret;else{if(!e.clipboardData||!e.clipboardData.getData)return!0;inputValue=valueBeforeCaret+e.clipboardData.getData("text/plain")+valueAfterCaret}var pasteValue=inputValue;if($.isFunction(opts.onBeforePaste)){if(pasteValue=opts.onBeforePaste.call(inputmask,inputValue,opts),!1===pasteValue)return e.preventDefault();pasteValue=pasteValue||inputValue}return checkVal(this,!1,!1,pasteValue.toString().split("")),writeBuffer(this,getBuffer(),seekNext(getLastValidPosition()),e,undoValue!==getBuffer().join("")),e.preventDefault()},inputFallBackEvent:function inputFallBackEvent(e){function ieMobileHandler(input,inputValue,caretPos){if(iemobile){var inputChar=inputValue.replace(getBuffer().join(""),"");if(1===inputChar.length){var iv=inputValue.split("");iv.splice(caretPos.begin,0,inputChar),inputValue=iv.join("")}}return inputValue}function analyseChanges(inputValue,buffer,caretPos){for(var frontPart=inputValue.substr(0,caretPos.begin).split(""),backPart=inputValue.substr(caretPos.begin).split(""),frontBufferPart=buffer.substr(0,caretPos.begin).split(""),backBufferPart=buffer.substr(caretPos.begin).split(""),fpl=frontPart.length>=frontBufferPart.length?frontPart.length:frontBufferPart.length,bpl=backPart.length>=backBufferPart.length?backPart.length:backBufferPart.length,bl,i,action="",data=[],marker="~",placeholder;frontPart.length<fpl;)frontPart.push("~");for(;frontBufferPart.length<fpl;)frontBufferPart.push("~");for(;backPart.length<bpl;)backPart.unshift("~");for(;backBufferPart.length<bpl;)backBufferPart.unshift("~");var newBuffer=frontPart.concat(backPart),oldBuffer=frontBufferPart.concat(backBufferPart);for(i=0,bl=newBuffer.length;i<bl;i++)switch(placeholder=getPlaceholder(translatePosition(i)),action){case"insertText":i=bl;break;case"insertReplacementText":"~"===newBuffer[i]?caretPos.end++:i=bl;break;case"deleteContentBackward":"~"===newBuffer[i]?caretPos.end++:i=bl;break;default:newBuffer[i]!==oldBuffer[i]&&(oldBuffer[i]===placeholder&&"~"===oldBuffer[i+1]||"~"===oldBuffer[i]?(action="insertText",data.push(newBuffer[i]),caretPos.begin--,caretPos.end--):"~"===oldBuffer[i+1]&&oldBuffer[i]===newBuffer[i+1]?(action="insertText",data.push(newBuffer[i]),caretPos.begin--,caretPos.end--):newBuffer[i]!==placeholder&&"~"!==newBuffer[i]&&("~"===newBuffer[i+1]||oldBuffer[i]!==newBuffer[i]&&oldBuffer[i+1]===newBuffer[i+1])?(action="insertReplacementText",data.push(newBuffer[i]),caretPos.begin--):"~"===newBuffer[i]?(action="deleteContentBackward",isMask(translatePosition(i),!0)&&caretPos.end++):i=bl);break}return{action:action,data:data,caret:caretPos}}var input=this,inputValue=input.inputmask._valueGet(!0),buffer=(isRTL?getBuffer().slice().reverse():getBuffer()).join(""),caretPos=caret(input,void 0,void 0,!0);if(buffer!==inputValue){inputValue=ieMobileHandler(input,inputValue,caretPos);var changes=analyseChanges(inputValue,buffer,caretPos);switch(document.activeElement!==input&&input.focus(),writeBuffer(input,getBuffer()),caret(input,caretPos.begin,caretPos.end,!0),changes.action){case"insertText":case"insertReplacementText":$.each(changes.data,function(ndx,entry){var keypress=new $.Event("keypress");keypress.which=entry.charCodeAt(0),ignorable=!1,EventHandlers.keypressEvent.call(input,keypress)});break;case"deleteContentBackward":var keydown=new $.Event("keydown");keydown.keyCode=Inputmask.keyCode.BACKSPACE,EventHandlers.keydownEvent.call(input,keydown);break;default:applyInputValue(input,inputValue);break}e.preventDefault()}},compositionendEvent:function compositionendEvent(e){$el.trigger("input")},setValueEvent:function setValueEvent(e,argument_1,argument_2){var input=this,value=e&&e.detail?e.detail[0]:argument_1;void 0===value&&(value=this.inputmask._valueGet(!0)),applyInputValue(this,value),(e.detail&&void 0!==e.detail[1]||void 0!==argument_2)&&caret(this,e.detail?e.detail[1]:argument_2)},focusEvent:function focusEvent(e){var input=this,nptValue=this.inputmask._valueGet();opts.showMaskOnFocus&&nptValue!==getBuffer().join("")&&writeBuffer(this,getBuffer(),seekNext(getLastValidPosition())),!0!==opts.positionCaretOnTab||!1!==mouseEnter||isComplete(getBuffer())&&-1!==getLastValidPosition()||EventHandlers.clickEvent.apply(this,[e,!0]),undoValue=getBuffer().join("")},invalidEvent:function invalidEvent(e){validationEvent=!0},mouseleaveEvent:function mouseleaveEvent(){var input=this;mouseEnter=!1,opts.clearMaskOnLostFocus&&document.activeElement!==this&&HandleNativePlaceholder(this,originalPlaceholder)},clickEvent:function clickEvent(e,tabbed){var input=this;if(document.activeElement===this){var newCaretPosition=determineNewCaretPosition(caret(this),tabbed);void 0!==newCaretPosition&&caret(this,newCaretPosition)}},cutEvent:function cutEvent(e){var input=this,pos=caret(this),clipboardData=window.clipboardData||e.clipboardData,clipData=isRTL?getBuffer().slice(pos.end,pos.begin):getBuffer().slice(pos.begin,pos.end);clipboardData.setData("text",isRTL?clipData.reverse().join(""):clipData.join("")),document.execCommand&&document.execCommand("copy"),handleRemove(this,Inputmask.keyCode.DELETE,pos),writeBuffer(this,getBuffer(),maskset.p,e,undoValue!==getBuffer().join(""))},blurEvent:function blurEvent(e){var $input=$(this),input=this;if(this.inputmask){HandleNativePlaceholder(this,originalPlaceholder);var nptValue=this.inputmask._valueGet(),buffer=getBuffer().slice();""!==nptValue&&(opts.clearMaskOnLostFocus&&(-1===getLastValidPosition()&&nptValue===getBufferTemplate().join("")?buffer=[]:clearOptionalTail(buffer)),!1===isComplete(buffer)&&(setTimeout(function(){$input.trigger("incomplete")},0),opts.clearIncomplete&&(resetMaskSet(),buffer=opts.clearMaskOnLostFocus?[]:getBufferTemplate().slice())),writeBuffer(this,buffer,void 0,e)),undoValue!==getBuffer().join("")&&(undoValue=getBuffer().join(""),$input.trigger("change"))}},mouseenterEvent:function mouseenterEvent(){var input=this;mouseEnter=!0,document.activeElement!==this&&(null==originalPlaceholder&&this.placeholder!==originalPlaceholder&&(originalPlaceholder=this.placeholder),opts.showMaskOnHover&&HandleNativePlaceholder(this,(isRTL?getBuffer().slice().reverse():getBuffer()).join("")))},submitEvent:function submitEvent(){undoValue!==getBuffer().join("")&&$el.trigger("change"),opts.clearMaskOnLostFocus&&-1===getLastValidPosition()&&el.inputmask._valueGet&&el.inputmask._valueGet()===getBufferTemplate().join("")&&el.inputmask._valueSet(""),opts.clearIncomplete&&!1===isComplete(getBuffer())&&el.inputmask._valueSet(""),opts.removeMaskOnSubmit&&(el.inputmask._valueSet(el.inputmask.unmaskedvalue(),!0),setTimeout(function(){writeBuffer(el,getBuffer())},0))},resetEvent:function resetEvent(){el.inputmask.refreshValue=!0,setTimeout(function(){applyInputValue(el,el.inputmask._valueGet(!0))},0)}},valueBuffer;function checkVal(input,writeOut,strict,nptvl,initiatingEvent){var inputmask=this||input.inputmask,inputValue=nptvl.slice(),charCodes="",initialNdx=-1,result=void 0;function isTemplateMatch(ndx,charCodes){if(opts.regex)return!1;for(var targetTemplate=getMaskTemplate(!0,0,!1).slice(ndx,seekNext(ndx)).join("").replace(/'/g,""),charCodeNdx=targetTemplate.indexOf(charCodes);0<charCodeNdx&&" "===targetTemplate[charCodeNdx-1];)charCodeNdx--;var match=0===charCodeNdx&&!isMask(ndx)&&(getTest(ndx).match.nativeDef===charCodes.charAt(0)||!0===getTest(ndx).match.static&&getTest(ndx).match.nativeDef==="'"+charCodes.charAt(0)||" "===getTest(ndx).match.nativeDef&&(getTest(ndx+1).match.nativeDef===charCodes.charAt(0)||!0===getTest(ndx+1).match.static&&getTest(ndx+1).match.nativeDef==="'"+charCodes.charAt(0)));return!match&&0<charCodeNdx&&(inputmask.caretPos={begin:seekNext(charCodeNdx)}),match}resetMaskSet(),maskset.tests={},initialNdx=opts.radixPoint?determineNewCaretPosition(0):0,maskset.p=initialNdx,inputmask.caretPos={begin:initialNdx};var staticMatches=[],prevCaretPos=inputmask.caretPos,sndx,validPos,nextValid;if($.each(inputValue,function(ndx,charCode){if(void 0!==charCode)if(void 0===maskset.validPositions[ndx]&&inputValue[ndx]===getPlaceholder(ndx)&&isMask(ndx,!0)&&!1===isValid(ndx,inputValue[ndx],!0,void 0,void 0,!0))maskset.p++;else{var keypress=new $.Event("_checkval");keypress.which=charCode.toString().charCodeAt(0),charCodes+=charCode;var lvp=getLastValidPosition(void 0,!0);isTemplateMatch(initialNdx,charCodes)?result=EventHandlers.keypressEvent.call(input,keypress,!0,!1,strict,lvp+1):(result=EventHandlers.keypressEvent.call(input,keypress,!0,!1,strict,inputmask.caretPos.begin),result&&(initialNdx=inputmask.caretPos.begin+1,charCodes="")),result?(void 0!==result.pos&&maskset.validPositions[result.pos]&&!0===maskset.validPositions[result.pos].match.static&&(staticMatches.push(result.pos),isRTL||(result.forwardPosition=result.pos+1)),writeBuffer(void 0,getBuffer(),result.forwardPosition,keypress,!1),inputmask.caretPos={begin:result.forwardPosition,end:result.forwardPosition},prevCaretPos=inputmask.caretPos):inputmask.caretPos=prevCaretPos}}),0<staticMatches.length)if(!isComplete(getBuffer())||staticMatches.length<seekNext(0)){for(;void 0!==(sndx=staticMatches.pop());)if(sndx!==staticMatches.length){var keypress=new $.Event("_checkval"),nextSndx=sndx+1;for(validPos=maskset.validPositions[sndx],validPos.generatedInput=!0,keypress.which=validPos.input.charCodeAt(0);(nextValid=maskset.validPositions[nextSndx])&&nextValid.input===validPos.input;)nextSndx++;result=EventHandlers.keypressEvent.call(input,keypress,!0,!1,strict,nextSndx),result&&void 0!==result.pos&&result.pos!==sndx&&maskset.validPositions[result.pos]&&!0===maskset.validPositions[result.pos].match.static&&staticMatches.push(result.pos)}}else for(;sndx=staticMatches.pop();)validPos=maskset.validPositions[sndx],validPos&&(validPos.generatedInput=!0);writeOut&&writeBuffer(input,getBuffer(),result?result.forwardPosition:void 0,initiatingEvent||new $.Event("checkval"),initiatingEvent&&"input"===initiatingEvent.type)}function unmaskedvalue(input){if(input){if(void 0===input.inputmask)return input.value;input.inputmask&&input.inputmask.refreshValue&&applyInputValue(input,input.inputmask._valueGet(!0))}var umValue=[],vps=maskset.validPositions;for(var pndx in vps)vps[pndx]&&vps[pndx].match&&1!=vps[pndx].match.static&&umValue.push(vps[pndx].input);var unmaskedValue=0===umValue.length?"":(isRTL?umValue.reverse():umValue).join("");if($.isFunction(opts.onUnMask)){var bufferValue=(isRTL?getBuffer().slice().reverse():getBuffer()).join("");unmaskedValue=opts.onUnMask.call(inputmask,bufferValue,unmaskedValue,opts)}return unmaskedValue}function translatePosition(pos){return!isRTL||"number"!=typeof pos||opts.greedy&&""===opts.placeholder||!el||(pos=el.inputmask._valueGet().length-pos),pos}function caret(input,begin,end,notranslate){var range;if(void 0===begin)return"selectionStart"in input&&"selectionEnd"in input?(begin=input.selectionStart,end=input.selectionEnd):window.getSelection?(range=window.getSelection().getRangeAt(0),range.commonAncestorContainer.parentNode!==input&&range.commonAncestorContainer!==input||(begin=range.startOffset,end=range.endOffset)):document.selection&&document.selection.createRange&&(range=document.selection.createRange(),begin=0-range.duplicate().moveStart("character",-input.inputmask._valueGet().length),end=begin+range.text.length),!1===opts.insertMode&&begin===end-1&&end--,{begin:notranslate?begin:translatePosition(begin),end:notranslate?end:translatePosition(end)};if($.isArray(begin)&&(end=isRTL?begin[0]:begin[1],begin=isRTL?begin[1]:begin[0]),void 0!==begin.begin&&(end=isRTL?begin.begin:begin.end,begin=isRTL?begin.end:begin.begin),"number"==typeof begin){begin=notranslate?begin:translatePosition(begin),end=notranslate?end:translatePosition(end),end="number"==typeof end?end:begin;var scrollCalc=parseInt(((input.ownerDocument.defaultView||window).getComputedStyle?(input.ownerDocument.defaultView||window).getComputedStyle(input,null):input.currentStyle).fontSize)*end;if(input.scrollLeft=scrollCalc>input.scrollWidth?scrollCalc:0,input.inputmask.caretPos={begin:begin,end:end},!1===opts.insertMode&&begin===end&&end++,input===document.activeElement)if("setSelectionRange"in input)input.setSelectionRange(begin,end);else if(window.getSelection){if(range=document.createRange(),void 0===input.firstChild||null===input.firstChild){var textNode=document.createTextNode("");input.appendChild(textNode)}range.setStart(input.firstChild,begin<input.inputmask._valueGet().length?begin:input.inputmask._valueGet().length),range.setEnd(input.firstChild,end<input.inputmask._valueGet().length?end:input.inputmask._valueGet().length),range.collapse(!0);var sel=window.getSelection();sel.removeAllRanges(),sel.addRange(range)}else input.createTextRange&&(range=input.createTextRange(),range.collapse(!0),range.moveEnd("character",end),range.moveStart("character",begin),range.select())}}function determineLastRequiredPosition(returnDefinition){var buffer=getMaskTemplate(!0,getLastValidPosition(),!0,!0),bl=buffer.length,pos,lvp=getLastValidPosition(),positions={},lvTest=maskset.validPositions[lvp],ndxIntlzr=void 0!==lvTest?lvTest.locator.slice():void 0,testPos;for(pos=lvp+1;pos<buffer.length;pos++)testPos=getTestTemplate(pos,ndxIntlzr,pos-1),ndxIntlzr=testPos.locator.slice(),positions[pos]=$.extend(!0,{},testPos);var lvTestAlt=lvTest&&void 0!==lvTest.alternation?lvTest.locator[lvTest.alternation]:void 0;for(pos=bl-1;lvp<pos&&(testPos=positions[pos],(testPos.match.optionality||testPos.match.optionalQuantifier&&testPos.match.newBlockMarker||lvTestAlt&&(lvTestAlt!==positions[pos].locator[lvTest.alternation]&&1!=testPos.match.static||!0===testPos.match.static&&testPos.locator[lvTest.alternation]&&checkAlternationMatch(testPos.locator[lvTest.alternation].toString().split(","),lvTestAlt.toString().split(","))&&""!==getTests(pos)[0].def))&&buffer[pos]===getPlaceholder(pos,testPos.match));pos--)bl--;return returnDefinition?{l:bl,def:positions[bl]?positions[bl].match:void 0}:bl}function clearOptionalTail(buffer){buffer.length=0;for(var template=getMaskTemplate(!0,0,!0,void 0,!0),lmnt;void 0!==(lmnt=template.shift());)buffer.push(lmnt);return buffer}function isComplete(buffer){if($.isFunction(opts.isComplete))return opts.isComplete(buffer,opts);if("*"!==opts.repeat){var complete=!1,lrp=determineLastRequiredPosition(!0),aml=seekPrevious(lrp.l);if(void 0===lrp.def||lrp.def.newBlockMarker||lrp.def.optionality||lrp.def.optionalQuantifier){complete=!0;for(var i=0;i<=aml;i++){var test=getTestTemplate(i).match;if(!0!==test.static&&void 0===maskset.validPositions[i]&&!0!==test.optionality&&!0!==test.optionalQuantifier||!0===test.static&&buffer[i]!==getPlaceholder(i,test)){complete=!1;break}}}return complete}}function handleRemove(input,k,pos,strict,fromIsValid){if((opts.numericInput||isRTL)&&(k===Inputmask.keyCode.BACKSPACE?k=Inputmask.keyCode.DELETE:k===Inputmask.keyCode.DELETE&&(k=Inputmask.keyCode.BACKSPACE),isRTL)){var pend=pos.end;pos.end=pos.begin,pos.begin=pend}var offset;if(k===Inputmask.keyCode.BACKSPACE||k===Inputmask.keyCode.DELETE&&!1===opts.insertMode?pos.end-pos.begin<1&&(pos.begin=seekPrevious(pos.begin),void 0!==maskset.validPositions[pos.begin]&&maskset.validPositions[pos.begin].input===opts.groupSeparator&&pos.begin--):k===Inputmask.keyCode.DELETE&&pos.begin===pos.end&&(pos.end=isMask(pos.end,!0,!0)?pos.end+1:seekNext(pos.end)+1,void 0!==maskset.validPositions[pos.begin]&&maskset.validPositions[pos.begin].input===opts.groupSeparator&&pos.end++),!1!==(offset=revalidateMask(pos))){if(!0!==strict&&!1!==opts.keepStatic||null!==opts.regex&&-1!==getTest(pos.begin).match.def.indexOf("|")){var result=alternate(!0);if(result){var newPos=void 0!==result.caret?result.caret:result.pos?seekNext(result.pos.begin?result.pos.begin:result.pos):getLastValidPosition(-1,!0);(k!==Inputmask.keyCode.DELETE||pos.begin>newPos)&&pos.begin}}var lvp=getLastValidPosition(pos.end,!0);lvp<pos.begin?maskset.p=!1===opts.insertMode?seekPrevious(lvp+1):seekNext(lvp):!0!==strict&&(maskset.p=k===Inputmask.keyCode.DELETE?pos.begin+offset:pos.begin,!1===opts.insertMode&&k===Inputmask.keyCode.DELETE&&(maskset.p=pos.end+1,void 0===maskset.validPositions[maskset.p]&&getLastValidPosition(maskset.maskLength,!0)<maskset.p&&(maskset.p=seekPrevious(lvp+1))))}}function applyInputValue(input,value){input.inputmask.refreshValue=!1,$.isFunction(opts.onBeforeMask)&&(value=opts.onBeforeMask.call(inputmask,value,opts)||value),value=value.toString().split(""),checkVal(input,!0,!1,value),undoValue=getBuffer().join(""),(opts.clearMaskOnLostFocus||opts.clearIncomplete)&&input.inputmask._valueGet()===getBufferTemplate().join("")&&-1===getLastValidPosition()&&input.inputmask._valueSet("")}function mask(elem){function isElementTypeSupported(input,opts){function patchValueProperty(npt){var valueGet,valueSet;function patchValhook(type){if($.valHooks&&(void 0===$.valHooks[type]||!0!==$.valHooks[type].inputmaskpatch)){var valhookGet=$.valHooks[type]&&$.valHooks[type].get?$.valHooks[type].get:function(elem){return elem.value},valhookSet=$.valHooks[type]&&$.valHooks[type].set?$.valHooks[type].set:function(elem,value){return elem.value=value,elem};$.valHooks[type]={get:function get(elem){if(elem.inputmask){if(elem.inputmask.opts.autoUnmask)return elem.inputmask.unmaskedvalue();var result=valhookGet(elem);return-1!==getLastValidPosition(void 0,void 0,elem.inputmask.maskset.validPositions)||!0!==opts.nullable?result:""}return valhookGet(elem)},set:function set(elem,value){var result=valhookSet(elem,value);return elem.inputmask&&applyInputValue(elem,value),result},inputmaskpatch:!0}}}function getter(){return this.inputmask?this.inputmask.opts.autoUnmask?this.inputmask.unmaskedvalue():-1!==getLastValidPosition()||!0!==opts.nullable?document.activeElement===this&&opts.clearMaskOnLostFocus?(isRTL?clearOptionalTail(getBuffer().slice()).reverse():clearOptionalTail(getBuffer().slice())).join(""):valueGet.call(this):"":valueGet.call(this)}function setter(value){valueSet.call(this,value),this.inputmask&&applyInputValue(this,value)}function installNativeValueSetFallback(npt){EventRuler.on(npt,"mouseenter",function(){var input=this,value=this.inputmask._valueGet(!0);value!==(isRTL?getBuffer().reverse():getBuffer()).join("")&&applyInputValue(this,value)})}if(!npt.inputmask.__valueGet){if(!0!==opts.noValuePatching){if(Object.getOwnPropertyDescriptor){"function"!=typeof Object.getPrototypeOf&&(Object.getPrototypeOf="object"===_typeof("test".__proto__)?function(object){return object.__proto__}:function(object){return object.constructor.prototype});var valueProperty=Object.getPrototypeOf?Object.getOwnPropertyDescriptor(Object.getPrototypeOf(npt),"value"):void 0;valueProperty&&valueProperty.get&&valueProperty.set?(valueGet=valueProperty.get,valueSet=valueProperty.set,Object.defineProperty(npt,"value",{get:getter,set:setter,configurable:!0})):"input"!==npt.tagName.toLowerCase()&&(valueGet=function valueGet(){return this.textContent},valueSet=function valueSet(value){this.textContent=value},Object.defineProperty(npt,"value",{get:getter,set:setter,configurable:!0}))}else document.__lookupGetter__&&npt.__lookupGetter__("value")&&(valueGet=npt.__lookupGetter__("value"),valueSet=npt.__lookupSetter__("value"),npt.__defineGetter__("value",getter),npt.__defineSetter__("value",setter));npt.inputmask.__valueGet=valueGet,npt.inputmask.__valueSet=valueSet}npt.inputmask._valueGet=function(overruleRTL){return isRTL&&!0!==overruleRTL?valueGet.call(this.el).split("").reverse().join(""):valueGet.call(this.el)},npt.inputmask._valueSet=function(value,overruleRTL){valueSet.call(this.el,null==value?"":!0!==overruleRTL&&isRTL?value.split("").reverse().join(""):value)},void 0===valueGet&&(valueGet=function valueGet(){return this.value},valueSet=function valueSet(value){this.value=value},patchValhook(npt.type),installNativeValueSetFallback(npt))}}var elementType=input.getAttribute("type"),isSupported="input"===input.tagName.toLowerCase()&&-1!==$.inArray(elementType,opts.supportsInputType)||input.isContentEditable||"textarea"===input.tagName.toLowerCase();if(!isSupported)if("input"===input.tagName.toLowerCase()){var el=document.createElement("input");el.setAttribute("type",elementType),isSupported="text"===el.type,el=null}else isSupported="partial";return!1!==isSupported?patchValueProperty(input):input.inputmask=void 0,isSupported}EventRuler.off(elem);var isSupported=isElementTypeSupported(elem,opts);if(!1!==isSupported&&(el=elem,$el=$(el),originalPlaceholder=el.placeholder,maxLength=void 0!==el?el.maxLength:void 0,-1===maxLength&&(maxLength=void 0),"inputMode"in el&&null===el.getAttribute("inputmode")&&(el.inputMode=opts.inputmode,el.setAttribute("inputmode",opts.inputmode)),!0===isSupported&&(opts.showMaskOnFocus=opts.showMaskOnFocus&&-1===["cc-number","cc-exp"].indexOf(el.autocomplete),EventRuler.on(el,"submit",EventHandlers.submitEvent),EventRuler.on(el,"reset",EventHandlers.resetEvent),EventRuler.on(el,"blur",EventHandlers.blurEvent),EventRuler.on(el,"focus",EventHandlers.focusEvent),EventRuler.on(el,"invalid",EventHandlers.invalidEvent),EventRuler.on(el,"click",EventHandlers.clickEvent),EventRuler.on(el,"mouseleave",EventHandlers.mouseleaveEvent),EventRuler.on(el,"mouseenter",EventHandlers.mouseenterEvent),EventRuler.on(el,"paste",EventHandlers.pasteEvent),EventRuler.on(el,"cut",EventHandlers.cutEvent),EventRuler.on(el,"complete",opts.oncomplete),EventRuler.on(el,"incomplete",opts.onincomplete),EventRuler.on(el,"cleared",opts.oncleared),mobile||!0===opts.inputEventOnly?el.removeAttribute("maxLength"):(EventRuler.on(el,"keydown",EventHandlers.keydownEvent),EventRuler.on(el,"keypress",EventHandlers.keypressEvent)),EventRuler.on(el,"input",EventHandlers.inputFallBackEvent),EventRuler.on(el,"compositionend",EventHandlers.compositionendEvent)),EventRuler.on(el,"setvalue",EventHandlers.setValueEvent),undoValue=getBufferTemplate().join(""),""!==el.inputmask._valueGet(!0)||!1===opts.clearMaskOnLostFocus||document.activeElement===el)){applyInputValue(el,el.inputmask._valueGet(!0),opts);var buffer=getBuffer().slice();!1===isComplete(buffer)&&opts.clearIncomplete&&resetMaskSet(),opts.clearMaskOnLostFocus&&document.activeElement!==el&&(-1===getLastValidPosition()?buffer=[]:clearOptionalTail(buffer)),(!1===opts.clearMaskOnLostFocus||opts.showMaskOnFocus&&document.activeElement===el||""!==el.inputmask._valueGet(!0))&&writeBuffer(el,buffer),document.activeElement===el&&caret(el,seekNext(getLastValidPosition()))}}if(void 0!==actionObj)switch(actionObj.action){case"isComplete":return el=actionObj.el,isComplete(getBuffer());case"unmaskedvalue":return void 0!==el&&void 0===actionObj.value||(valueBuffer=actionObj.value,valueBuffer=($.isFunction(opts.onBeforeMask)&&opts.onBeforeMask.call(inputmask,valueBuffer,opts)||valueBuffer).split(""),checkVal.call(this,void 0,!1,!1,valueBuffer),$.isFunction(opts.onBeforeWrite)&&opts.onBeforeWrite.call(inputmask,void 0,getBuffer(),0,opts)),unmaskedvalue(el);case"mask":mask(el);break;case"format":return valueBuffer=($.isFunction(opts.onBeforeMask)&&opts.onBeforeMask.call(inputmask,actionObj.value,opts)||actionObj.value).split(""),checkVal.call(this,void 0,!0,!1,valueBuffer),actionObj.metadata?{value:isRTL?getBuffer().slice().reverse().join(""):getBuffer().join(""),metadata:maskScope.call(this,{action:"getmetadata"},maskset,opts)}:isRTL?getBuffer().slice().reverse().join(""):getBuffer().join("");case"isValid":actionObj.value?(valueBuffer=($.isFunction(opts.onBeforeMask)&&opts.onBeforeMask.call(inputmask,actionObj.value,opts)||actionObj.value).split(""),checkVal.call(this,void 0,!0,!1,valueBuffer)):actionObj.value=isRTL?getBuffer().slice().reverse().join(""):getBuffer().join("");for(var buffer=getBuffer(),rl=determineLastRequiredPosition(),lmib=buffer.length-1;rl<lmib&&!isMask(lmib);lmib--);return buffer.splice(rl,lmib+1-rl),isComplete(buffer)&&actionObj.value===(isRTL?getBuffer().slice().reverse().join(""):getBuffer().join(""));case"getemptymask":return getBufferTemplate().join("");case"remove":if(el&&el.inputmask){$.data(el,"_inputmask_opts",null),$el=$(el);var cv=opts.autoUnmask?unmaskedvalue(el):el.inputmask._valueGet(opts.autoUnmask),valueProperty;cv!==getBufferTemplate().join("")?el.inputmask._valueSet(cv,opts.autoUnmask):el.inputmask._valueSet(""),EventRuler.off(el),Object.getOwnPropertyDescriptor&&Object.getPrototypeOf?(valueProperty=Object.getOwnPropertyDescriptor(Object.getPrototypeOf(el),"value"),valueProperty&&el.inputmask.__valueGet&&Object.defineProperty(el,"value",{get:el.inputmask.__valueGet,set:el.inputmask.__valueSet,configurable:!0})):document.__lookupGetter__&&el.__lookupGetter__("value")&&el.inputmask.__valueGet&&(el.__defineGetter__("value",el.inputmask.__valueGet),el.__defineSetter__("value",el.inputmask.__valueSet)),el.inputmask=void 0}return el;case"getmetadata":if($.isArray(maskset.metadata)){var maskTarget=getMaskTemplate(!0,0,!1).join("");return $.each(maskset.metadata,function(ndx,mtdt){if(mtdt.mask===maskTarget)return maskTarget=mtdt,!1}),maskTarget}return maskset.metadata}}},function(module,exports,__webpack_require__){"use strict";function _typeof(obj){return _typeof="function"==typeof Symbol&&"symbol"==typeof Symbol.iterator?function _typeof(obj){return typeof obj}:function _typeof(obj){return obj&&"function"==typeof Symbol&&obj.constructor===Symbol&&obj!==Symbol.prototype?"symbol":typeof obj},_typeof(obj)}var Inputmask=__webpack_require__(0),$=Inputmask.dependencyLib,formatCode={d:["[1-9]|[12][0-9]|3[01]",Date.prototype.setDate,"day",Date.prototype.getDate],dd:["0[1-9]|[12][0-9]|3[01]",Date.prototype.setDate,"day",function(){return pad(Date.prototype.getDate.call(this),2)}],ddd:[""],dddd:[""],m:["[1-9]|1[012]",Date.prototype.setMonth,"month",function(){return Date.prototype.getMonth.call(this)+1}],mm:["0[1-9]|1[012]",Date.prototype.setMonth,"month",function(){return pad(Date.prototype.getMonth.call(this)+1,2)}],mmm:[""],mmmm:[""],yy:["[0-9]{2}",Date.prototype.setFullYear,"year",function(){return pad(Date.prototype.getFullYear.call(this),2)}],yyyy:["[0-9]{4}",Date.prototype.setFullYear,"year",function(){return pad(Date.prototype.getFullYear.call(this),4)}],h:["[1-9]|1[0-2]",Date.prototype.setHours,"hours",Date.prototype.getHours],hh:["0[1-9]|1[0-2]",Date.prototype.setHours,"hours",function(){return pad(Date.prototype.getHours.call(this),2)}],hhh:["[0-9]+",Date.prototype.setHours,"hours",Date.prototype.getHours],H:["1?[0-9]|2[0-3]",Date.prototype.setHours,"hours",Date.prototype.getHours],HH:["0[0-9]|1[0-9]|2[0-3]",Date.prototype.setHours,"hours",function(){return pad(Date.prototype.getHours.call(this),2)}],HHH:["[0-9]+",Date.prototype.setHours,"hours",Date.prototype.getHours],M:["[1-5]?[0-9]",Date.prototype.setMinutes,"minutes",Date.prototype.getMinutes],MM:["0[0-9]|1[0-9]|2[0-9]|3[0-9]|4[0-9]|5[0-9]",Date.prototype.setMinutes,"minutes",function(){return pad(Date.prototype.getMinutes.call(this),2)}],s:["[1-5]?[0-9]",Date.prototype.setSeconds,"seconds",Date.prototype.getSeconds],ss:["0[0-9]|1[0-9]|2[0-9]|3[0-9]|4[0-9]|5[0-9]",Date.prototype.setSeconds,"seconds",function(){return pad(Date.prototype.getSeconds.call(this),2)}],l:["[0-9]{3}",Date.prototype.setMilliseconds,"milliseconds",function(){return pad(Date.prototype.getMilliseconds.call(this),3)}],L:["[0-9]{2}",Date.prototype.setMilliseconds,"milliseconds",function(){return pad(Date.prototype.getMilliseconds.call(this),2)}],t:["[ap]"],tt:["[ap]m"],T:["[AP]"],TT:["[AP]M"],Z:[""],o:[""],S:[""]},formatAlias={isoDate:"yyyy-mm-dd",isoTime:"HH:MM:ss",isoDateTime:"yyyy-mm-dd'T'HH:MM:ss",isoUtcDateTime:"UTC:yyyy-mm-dd'T'HH:MM:ss'Z'"};function getTokenizer(opts){if(!opts.tokenizer){var tokens=[];for(var ndx in formatCode)-1===tokens.indexOf(ndx[0])&&tokens.push(ndx[0]);opts.tokenizer="("+tokens.join("+|")+")+?|.",opts.tokenizer=new RegExp(opts.tokenizer,"g")}return opts.tokenizer}function isValidDate(dateParts,currentResult){return(!isFinite(dateParts.rawday)||"29"==dateParts.day&&!isFinite(dateParts.rawyear)||new Date(dateParts.date.getFullYear(),isFinite(dateParts.rawmonth)?dateParts.month:dateParts.date.getMonth()+1,0).getDate()>=dateParts.day)&&currentResult}function isDateInRange(dateParts,opts){var result=!0;if(opts.min){if(dateParts.rawyear){var rawYear=dateParts.rawyear.replace(/[^0-9]/g,""),minYear=opts.min.year.substr(0,rawYear.length);result=minYear<=rawYear}dateParts.year===dateParts.rawyear&&opts.min.date.getTime()==opts.min.date.getTime()&&(result=opts.min.date.getTime()<=dateParts.date.getTime())}return result&&opts.max&&opts.max.date.getTime()==opts.max.date.getTime()&&(result=opts.max.date.getTime()>=dateParts.date.getTime()),result}function parse(format,dateObjValue,opts,raw){var mask="",match;for(getTokenizer(opts).lastIndex=0;match=getTokenizer(opts).exec(format);)if(void 0===dateObjValue)if(formatCode[match[0]])mask+="("+formatCode[match[0]][0]+")";else switch(match[0]){case"[":mask+="(";break;case"]":mask+=")?";break;default:mask+=Inputmask.escapeRegex(match[0])}else if(formatCode[match[0]])if(!0!==raw&&formatCode[match[0]][3]){var getFn=formatCode[match[0]][3];mask+=getFn.call(dateObjValue.date)}else formatCode[match[0]][2]?mask+=dateObjValue["raw"+formatCode[match[0]][2]]:mask+=match[0];else mask+=match[0];return mask}function pad(val,len){for(val=String(val),len=len||2;val.length<len;)val="0"+val;return val}function analyseMask(maskString,format,opts){var dateObj={date:new Date(1,0,1)},targetProp,mask=maskString,match,dateOperation;function extendProperty(value){var correctedValue=value.replace(/[^0-9]/g,"0");return correctedValue}function setValue(dateObj,value,opts){dateObj[targetProp]=extendProperty(value),dateObj["raw"+targetProp]=value,void 0!==dateOperation&&dateOperation.call(dateObj.date,"month"==targetProp?parseInt(dateObj[targetProp])-1:dateObj[targetProp])}if("string"==typeof mask){for(getTokenizer(opts).lastIndex=0;match=getTokenizer(opts).exec(format);){var value=mask.slice(0,match[0].length);formatCode.hasOwnProperty(match[0])&&(targetProp=formatCode[match[0]][2],dateOperation=formatCode[match[0]][1],setValue(dateObj,value,opts)),mask=mask.slice(value.length)}return dateObj}if(mask&&"object"===_typeof(mask)&&mask.hasOwnProperty("date"))return mask}Inputmask.extendAliases({datetime:{mask:function mask(opts){return formatCode.S=opts.i18n.ordinalSuffix.join("|"),opts.inputFormat=formatAlias[opts.inputFormat]||opts.inputFormat,opts.displayFormat=formatAlias[opts.displayFormat]||opts.displayFormat||opts.inputFormat,opts.outputFormat=formatAlias[opts.outputFormat]||opts.outputFormat||opts.inputFormat,opts.placeholder=""!==opts.placeholder?opts.placeholder:opts.inputFormat.replace(/[[\]]/,""),opts.regex=parse(opts.inputFormat,void 0,opts),null},placeholder:"",inputFormat:"isoDateTime",displayFormat:void 0,outputFormat:void 0,min:null,max:null,skipOptionalPartCharacter:"",i18n:{dayNames:["Mon","Tue","Wed","Thu","Fri","Sat","Sun","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"],monthNames:["Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec","January","February","March","April","May","June","July","August","September","October","November","December"],ordinalSuffix:["st","nd","rd","th"]},preValidation:function preValidation(buffer,pos,c,isSelection,opts,maskset,caretPos,strict){if(strict)return!0;var calcPos=0,targetMatch,match;if(isNaN(c)&&buffer[pos]!==c){for(getTokenizer(opts).lastIndex=0;match=getTokenizer(opts).exec(opts.inputFormat);)if(calcPos+=match[0].length,pos<=calcPos){targetMatch=match,match=getTokenizer(opts).exec(opts.inputFormat);break}if(match&&match[0]===c&&1<targetMatch[0].length)return buffer[pos]=buffer[pos-1],buffer[pos-1]="0",{fuzzy:!0,buffer:buffer,refreshFromBuffer:{start:pos-1,end:pos+1},pos:pos+1}}return!0},postValidation:function postValidation(buffer,pos,currentResult,opts,maskset,strict){if(strict)return!0;opts.min=analyseMask(opts.min,opts.inputFormat,opts),opts.max=analyseMask(opts.max,opts.inputFormat,opts),currentResult.fuzzy&&(buffer=currentResult.buffer,pos=currentResult.pos);var calcPos=0,match;for(getTokenizer(opts).lastIndex=0;(match=getTokenizer(opts).exec(opts.inputFormat))&&(calcPos+=match[0].length,!(pos<calcPos)););if(match&&match[0]&&void 0!==formatCode[match[0]]){var validator=formatCode[match[0]][0],part=buffer.slice(match.index,match.index+match[0].length);!1===new RegExp(validator).test(part.join(""))&&2===match[0].length&&maskset.validPositions[match.index]&&maskset.validPositions[match.index+1]&&(maskset.validPositions[match.index+1].input="0")}var result=currentResult,dateParts=analyseMask(buffer.join(""),opts.inputFormat,opts);return result&&dateParts.date.getTime()==dateParts.date.getTime()&&(result=isValidDate(dateParts,result),result=result&&isDateInRange(dateParts,opts)),pos&&result&&currentResult.pos!==pos?{buffer:parse(opts.inputFormat,dateParts,opts).split(""),refreshFromBuffer:{start:pos,end:currentResult.pos}}:result},onKeyDown:function onKeyDown(e,buffer,caretPos,opts){var input=this;if(e.ctrlKey&&e.keyCode===Inputmask.keyCode.RIGHT){var today=new Date,match,date="";for(getTokenizer(opts).lastIndex=0;match=getTokenizer(opts).exec(opts.inputFormat);)"d"===match[0].charAt(0)?date+=pad(today.getDate(),match[0].length):"m"===match[0].charAt(0)?date+=pad(today.getMonth()+1,match[0].length):"yyyy"===match[0]?date+=today.getFullYear().toString():"y"===match[0].charAt(0)&&(date+=pad(today.getYear(),match[0].length));this.inputmask._valueSet(date),$(this).trigger("setvalue")}},onUnMask:function onUnMask(maskedValue,unmaskedValue,opts){return unmaskedValue?parse(opts.outputFormat,analyseMask(maskedValue,opts.inputFormat,opts),opts,!0):unmaskedValue},casing:function casing(elem,test,pos,validPositions){return 0==test.nativeDef.indexOf("[ap]")?elem.toLowerCase():0==test.nativeDef.indexOf("[AP]")?elem.toUpperCase():elem},insertMode:!1,shiftPositions:!1,keepStatic:!1}}),module.exports=Inputmask},function(module,exports,__webpack_require__){"use strict";var Inputmask=__webpack_require__(0),$=Inputmask.dependencyLib;function autoEscape(txt,opts){for(var escapedTxt="",i=0;i<txt.length;i++)Inputmask.prototype.definitions[txt.charAt(i)]||opts.definitions[txt.charAt(i)]||opts.optionalmarker.start===txt.charAt(i)||opts.optionalmarker.end===txt.charAt(i)||opts.quantifiermarker.start===txt.charAt(i)||opts.quantifiermarker.end===txt.charAt(i)||opts.groupmarker.start===txt.charAt(i)||opts.groupmarker.end===txt.charAt(i)||opts.alternatormarker===txt.charAt(i)?escapedTxt+="\\"+txt.charAt(i):escapedTxt+=txt.charAt(i);return escapedTxt}function alignDigits(buffer,digits,opts,force){if(0<digits&&(!opts.digitsOptional||force)){var radixPosition=$.inArray(opts.radixPoint,buffer);-1===radixPosition&&(buffer.push(opts.radixPoint),radixPosition=buffer.length-1);for(var i=1;i<=digits;i++)buffer[radixPosition+i]=buffer[radixPosition+i]||"0"}return buffer}function findValidator(symbol,maskset){var posNdx=0;if("+"===symbol){for(posNdx in maskset.validPositions);posNdx=parseInt(posNdx)}for(var tstNdx in maskset.tests)if(tstNdx=parseInt(tstNdx),posNdx<=tstNdx)for(var ndx=0,ndxl=maskset.tests[tstNdx].length;ndx<ndxl;ndx++)if((void 0===maskset.validPositions[tstNdx]||"-"===symbol)&&maskset.tests[tstNdx][ndx].match.def===symbol)return tstNdx+(void 0!==maskset.validPositions[tstNdx]&&"-"!==symbol?1:0);return posNdx}function findValid(symbol,maskset){var ret=-1;return $.each(maskset.validPositions,function(ndx,tst){if(tst&&tst.match.def===symbol)return ret=parseInt(ndx),!1}),ret}function parseMinMaxOptions(opts){void 0===opts.parseMinMaxOptions&&(null!==opts.min&&(opts.min=opts.min.toString().replace(new RegExp(Inputmask.escapeRegex(opts.groupSeparator),"g"),""),","===opts.radixPoint&&(opts.min=opts.min.replace(opts.radixPoint,".")),opts.min=isFinite(opts.min)?parseFloat(opts.min):NaN,isNaN(opts.min)&&(opts.min=Number.MIN_VALUE)),null!==opts.max&&(opts.max=opts.max.toString().replace(new RegExp(Inputmask.escapeRegex(opts.groupSeparator),"g"),""),","===opts.radixPoint&&(opts.max=opts.max.replace(opts.radixPoint,".")),opts.max=isFinite(opts.max)?parseFloat(opts.max):NaN,isNaN(opts.max)&&(opts.max=Number.MAX_VALUE)),opts.parseMinMaxOptions="done")}function genMask(opts){opts.repeat=0,opts.groupSeparator===opts.radixPoint&&opts.digits&&"0"!==opts.digits&&("."===opts.radixPoint?opts.groupSeparator=",":","===opts.radixPoint?opts.groupSeparator=".":opts.groupSeparator="")," "===opts.groupSeparator&&(opts.skipOptionalPartCharacter=void 0),1<opts.placeholder.length&&(opts.placeholder=opts.placeholder.charAt(0)),"radixFocus"===opts.positionCaretOnClick&&""===opts.placeholder&&(opts.positionCaretOnClick="lvp");var decimalDef="0";!0===opts.numericInput&&void 0===opts.__financeInput?(decimalDef="1",opts.positionCaretOnClick="radixFocus"===opts.positionCaretOnClick?"lvp":opts.positionCaretOnClick,isNaN(opts.digits)&&(opts.digits=2),opts._radixDance=!1):(opts.__financeInput=!1,opts.numericInput=!0);var mask="[+]",altMask;if(mask+=autoEscape(opts.prefix,opts),""!==opts.groupSeparator?mask+=opts._mask(opts):mask+="9{+}",void 0!==opts.digits&&0!==opts.digits){var dq=opts.digits.toString().split(",");isFinite(dq[0])&&dq[1]&&isFinite(dq[1])?mask+=opts.radixPoint+decimalDef+"{"+opts.digits+"}":(isNaN(opts.digits)||0<parseInt(opts.digits))&&(opts.digitsOptional?(altMask=mask+opts.radixPoint+decimalDef+"{0,"+opts.digits+"}",opts.keepStatic=!0):mask+=opts.radixPoint+decimalDef+"{"+opts.digits+"}")}return mask+=autoEscape(opts.suffix,opts),mask+="[-]",altMask&&(mask=[altMask+autoEscape(opts.suffix,opts)+"[-]",mask]),opts.greedy=!1,parseMinMaxOptions(opts),mask}function hanndleRadixDance(pos,c,radixPos,opts){return opts._radixDance&&opts.numericInput&&c!==opts.negationSymbol.back&&pos<=radixPos&&(0<radixPos||c==opts.radixPoint)&&(pos-=1),pos}function decimalValidator(chrs,maskset,pos,strict,opts){var radixPos=maskset.buffer?maskset.buffer.indexOf(opts.radixPoint):-1,result=-1!==radixPos&&new RegExp("[0-9\uff11-\uff19]").test(chrs);return opts._radixDance&&result&&null==maskset.validPositions[radixPos]?{insert:{pos:radixPos===pos?radixPos+1:radixPos,c:opts.radixPoint},pos:pos}:result}function checkForLeadingZeroes(buffer,opts){try{var numberMatches=new RegExp("(^"+(""!==opts.negationSymbol.front?Inputmask.escapeRegex(opts.negationSymbol.front)+"?":"")+Inputmask.escapeRegex(opts.prefix)+")(.*)("+Inputmask.escapeRegex(opts.suffix)+(""!=opts.negationSymbol.back?Inputmask.escapeRegex(opts.negationSymbol.back)+"?":"")+"$)").exec(buffer.slice().reverse().join("")),number=numberMatches?numberMatches[2]:"",leadingzeroes=!1;return number&&(number=number.split(opts.radixPoint.charAt(0))[0],leadingzeroes=new RegExp("^[0"+opts.groupSeparator+"]*").exec(number)),!(!leadingzeroes||!(1<leadingzeroes[0].length||0<leadingzeroes[0].length&&leadingzeroes[0].length<number.length))&&leadingzeroes}catch(e){console.log(buffer.slice().reverse().join(""))}}Inputmask.extendAliases({numeric:{mask:genMask,_mask:function _mask(opts){return"("+opts.groupSeparator+"999){+|1}"},digits:"*",digitsOptional:!0,enforceDigitsOnBlur:!1,radixPoint:".",positionCaretOnClick:"radixFocus",_radixDance:!0,groupSeparator:"",allowMinus:!0,negationSymbol:{front:"-",back:""},prefix:"",suffix:"",min:null,max:null,step:1,unmaskAsNumber:!1,roundingFN:Math.round,inputmode:"numeric",shortcuts:{k:"000",m:"000000"},placeholder:"0",greedy:!1,rightAlign:!0,insertMode:!0,autoUnmask:!1,skipOptionalPartCharacter:"",definitions:{0:{validator:decimalValidator},1:{validator:decimalValidator,definitionSymbol:"*"},"+":{validator:function validator(chrs,maskset,pos,strict,opts){return opts.allowMinus&&("-"===chrs||chrs===opts.negationSymbol.front)}},"-":{validator:function validator(chrs,maskset,pos,strict,opts){return opts.allowMinus&&chrs===opts.negationSymbol.back}}},preValidation:function preValidation(buffer,pos,c,isSelection,opts,maskset,caretPos,strict){if(!1!==opts.__financeInput&&c===opts.radixPoint)return!1;var pattern;if(pattern=opts.shortcuts&&opts.shortcuts[c]){if(1<pattern.length)for(var inserts=[],i=0;i<pattern.length;i++)inserts.push({pos:pos+i,c:pattern[i],strict:!1});return{insert:inserts}}var radixPos=$.inArray(opts.radixPoint,buffer);if(pos=hanndleRadixDance(pos,c,radixPos,opts),"-"!==c&&c!==opts.negationSymbol.front)return!!strict||(-1!==radixPos&&!0===opts._radixDance&&!1===isSelection&&c===opts.radixPoint&&void 0!==opts.digits&&(isNaN(opts.digits)||0<parseInt(opts.digits))&&radixPos!==pos?{caret:opts._radixDance&&pos===radixPos-1?radixPos+1:radixPos}:{rewritePosition:isSelection&&opts.digitsOptional?caretPos.end:pos});if(!0!==opts.allowMinus)return!1;var isNegative=!1,front=findValid("+",maskset),back=findValid("-",maskset);return-1!==front&&(isNegative=[front,back]),!1!==isNegative?{remove:isNegative,caret:pos<radixPos?pos+1:pos}:{insert:[{pos:findValidator("+",maskset),c:opts.negationSymbol.front,fromIsValid:!0},{pos:findValidator("-",maskset),c:opts.negationSymbol.back,fromIsValid:void 0}],caret:pos<radixPos?pos+1:pos}},postValidation:function postValidation(buffer,pos,currentResult,opts,maskset,strict){if(strict)return!0;if(null!==opts.min||null!==opts.max){var unmasked=opts.onUnMask(buffer.slice().reverse().join(""),void 0,$.extend({},opts,{unmaskAsNumber:!0}));if(null!==opts.min&&unmasked<opts.min&&(unmasked.toString().length>=opts.min.toString().length||unmasked<0))return{refreshFromBuffer:!0,buffer:alignDigits(opts.min.toString().replace(".",opts.radixPoint).split(""),opts.digits,opts).reverse()};if(null!==opts.max&&unmasked>opts.max)return{refreshFromBuffer:!0,buffer:alignDigits(opts.max.toString().replace(".",opts.radixPoint).split(""),opts.digits,opts).reverse()}}return currentResult},onUnMask:function onUnMask(maskedValue,unmaskedValue,opts){if(""===unmaskedValue&&!0===opts.nullable)return unmaskedValue;var processValue=maskedValue.replace(opts.prefix,"");return processValue=processValue.replace(opts.suffix,""),processValue=processValue.replace(new RegExp(Inputmask.escapeRegex(opts.groupSeparator),"g"),""),""!==opts.placeholder.charAt(0)&&(processValue=processValue.replace(new RegExp(opts.placeholder.charAt(0),"g"),"0")),opts.unmaskAsNumber?(""!==opts.radixPoint&&-1!==processValue.indexOf(opts.radixPoint)&&(processValue=processValue.replace(Inputmask.escapeRegex.call(this,opts.radixPoint),".")),processValue=processValue.replace(new RegExp("^"+Inputmask.escapeRegex(opts.negationSymbol.front)),"-"),processValue=processValue.replace(new RegExp(Inputmask.escapeRegex(opts.negationSymbol.back)+"$"),""),Number(processValue)):processValue},isComplete:function isComplete(buffer,opts){var maskedValue=(opts.numericInput?buffer.slice().reverse():buffer).join("");return maskedValue=maskedValue.replace(new RegExp("^"+Inputmask.escapeRegex(opts.negationSymbol.front)),"-"),maskedValue=maskedValue.replace(new RegExp(Inputmask.escapeRegex(opts.negationSymbol.back)+"$"),""),maskedValue=maskedValue.replace(opts.prefix,""),maskedValue=maskedValue.replace(opts.suffix,""),maskedValue=maskedValue.replace(new RegExp(Inputmask.escapeRegex(opts.groupSeparator)+"([0-9]{3})","g"),"$1"),","===opts.radixPoint&&(maskedValue=maskedValue.replace(Inputmask.escapeRegex(opts.radixPoint),".")),isFinite(maskedValue)},onBeforeMask:function onBeforeMask(initialValue,opts){var radixPoint=opts.radixPoint||",";"number"!=typeof initialValue&&"number"!==opts.inputType||""===radixPoint||(initialValue=initialValue.toString().replace(".",radixPoint));var valueParts=initialValue.split(radixPoint),integerPart=valueParts[0].replace(/[^\-0-9]/g,""),decimalPart=1<valueParts.length?valueParts[1].replace(/[^0-9]/g,""):"",forceDigits=1<valueParts.length;initialValue=integerPart+(""!==decimalPart?radixPoint+decimalPart:decimalPart);var digits=0;if(""!==radixPoint&&(digits=decimalPart.length,""!==decimalPart)){var digitsFactor=Math.pow(10,digits||1);isFinite(opts.digits)&&(digits=parseInt(opts.digits),digitsFactor=Math.pow(10,digits)),initialValue=initialValue.replace(Inputmask.escapeRegex(radixPoint),"."),isFinite(initialValue)&&(initialValue=opts.roundingFN(parseFloat(initialValue)*digitsFactor)/digitsFactor),initialValue=initialValue.toString().replace(".",radixPoint)}if(0===opts.digits&&-1!==initialValue.indexOf(Inputmask.escapeRegex(radixPoint))&&(initialValue=initialValue.substring(0,initialValue.indexOf(Inputmask.escapeRegex(radixPoint)))),null!==opts.min||null!==opts.max){var numberValue=initialValue.toString().replace(radixPoint,".");null!==opts.min&&numberValue<opts.min?initialValue=opts.min.toString().replace(".",radixPoint):null!==opts.max&&numberValue>opts.max&&(initialValue=opts.max.toString().replace(".",radixPoint))}return alignDigits(initialValue.toString().split(""),digits,opts,forceDigits).join("")},onBeforeWrite:function onBeforeWrite(e,buffer,caretPos,opts){function stripBuffer(buffer,stripRadix){if(!1!==opts.__financeInput||stripRadix){var position=$.inArray(opts.radixPoint,buffer);-1!==position&&buffer.splice(position,1)}if(""!==opts.groupSeparator)for(;-1!==(position=buffer.indexOf(opts.groupSeparator));)buffer.splice(position,1);return buffer}var result,leadingzeroes=checkForLeadingZeroes(buffer,opts);if(leadingzeroes){var buf=buffer.slice().reverse(),caretNdx=buf.join("").indexOf(leadingzeroes[0]);buf.splice(caretNdx,leadingzeroes[0].length);var newCaretPos=buf.length-caretNdx;stripBuffer(buf),result={refreshFromBuffer:!0,buffer:buf.reverse(),caret:caretPos<newCaretPos?caretPos:newCaretPos}}if(e)switch(e.type){case"blur":case"checkval":if(null!==opts.min){var unmasked=opts.onUnMask(buffer.slice().reverse().join(""),void 0,$.extend({},opts,{unmaskAsNumber:!0}));if(null!==opts.min&&unmasked<opts.min)return{refreshFromBuffer:!0,buffer:alignDigits(opts.min.toString().replace(".",opts.radixPoint).split(""),opts.digits,opts).reverse()}}if(""!==opts.radixPoint&&buffer[0]===opts.radixPoint)result&&result.buffer?result.buffer.shift():(buffer.shift(),result={refreshFromBuffer:!0,buffer:stripBuffer(buffer)});else if(buffer[buffer.length-1]===opts.negationSymbol.front){var nmbrMtchs=new RegExp("(^"+(""!=opts.negationSymbol.front?Inputmask.escapeRegex(opts.negationSymbol.front)+"?":"")+Inputmask.escapeRegex(opts.prefix)+")(.*)("+Inputmask.escapeRegex(opts.suffix)+(""!=opts.negationSymbol.back?Inputmask.escapeRegex(opts.negationSymbol.back)+"?":"")+"$)").exec(stripBuffer(buffer.slice(),!0).reverse().join("")),number=nmbrMtchs?nmbrMtchs[2]:"";0==number&&(result={refreshFromBuffer:!0,buffer:[0]})}}return result},onKeyDown:function onKeyDown(e,buffer,caretPos,opts){var $input=$(this),bffr;if(e.ctrlKey)switch(e.keyCode){case Inputmask.keyCode.UP:return this.inputmask.__valueSet.call(this,parseFloat(this.inputmask.unmaskedvalue())+parseInt(opts.step)),$input.trigger("setvalue"),!1;case Inputmask.keyCode.DOWN:return this.inputmask.__valueSet.call(this,parseFloat(this.inputmask.unmaskedvalue())-parseInt(opts.step)),$input.trigger("setvalue"),!1}if(!e.shiftKey&&(e.keyCode===Inputmask.keyCode.DELETE||e.keyCode===Inputmask.keyCode.BACKSPACE||e.keyCode===Inputmask.keyCode.BACKSPACE_SAFARI)){if(buffer[e.keyCode===Inputmask.keyCode.DELETE?caretPos.begin-1:caretPos.end]===opts.negationSymbol.front)return bffr=buffer.slice().reverse(),""!==opts.negationSymbol.front&&bffr.shift(),""!==opts.negationSymbol.back&&bffr.pop(),$input.trigger("setvalue",[bffr.join(""),caretPos.begin]),!1;if(!0===opts._radixDance){var radixPos=$.inArray(opts.radixPoint,buffer);if(opts.digitsOptional){if(0===radixPos)return bffr=buffer.slice().reverse(),bffr.pop(),$input.trigger("setvalue",[bffr.join(""),caretPos.begin>=bffr.length?bffr.length:caretPos.begin]),!1}else if(-1!==radixPos&&(caretPos.begin<radixPos||caretPos.end<radixPos||e.keyCode===Inputmask.keyCode.DELETE&&caretPos.begin===radixPos))return caretPos.begin!==caretPos.end||e.keyCode!==Inputmask.keyCode.BACKSPACE&&e.keyCode!==Inputmask.keyCode.BACKSPACE_SAFARI||caretPos.begin++,bffr=buffer.slice().reverse(),bffr.splice(bffr.length-caretPos.begin,caretPos.begin-caretPos.end+1),bffr=alignDigits(bffr,opts.digits,opts).join(""),$input.trigger("setvalue",[bffr,caretPos.begin>=bffr.length?radixPos+1:caretPos.begin]),!1}}}},currency:{prefix:"$ ",groupSeparator:",",alias:"numeric",digits:2,digitsOptional:!1},decimal:{alias:"numeric"},integer:{alias:"numeric",digits:0},percentage:{alias:"numeric",min:0,max:100,suffix:" %",digits:0,allowMinus:!1},indianns:{alias:"numeric",_mask:function _mask(opts){return"("+opts.groupSeparator+"99){*|1}("+opts.groupSeparator+"999){1|1}"},groupSeparator:",",radixPoint:".",placeholder:"0",digits:2,digitsOptional:!1}}),module.exports=Inputmask},function(module,exports,__webpack_require__){"use strict";function _typeof(obj){return _typeof="function"==typeof Symbol&&"symbol"==typeof Symbol.iterator?function _typeof(obj){return typeof obj}:function _typeof(obj){return obj&&"function"==typeof Symbol&&obj.constructor===Symbol&&obj!==Symbol.prototype?"symbol":typeof obj},_typeof(obj)}var $=__webpack_require__(2),Inputmask=__webpack_require__(0);void 0===$.fn.inputmask&&($.fn.inputmask=function(fn,options){var nptmask,input=this[0];if(void 0===options&&(options={}),"string"==typeof fn)switch(fn){case"unmaskedvalue":return input&&input.inputmask?input.inputmask.unmaskedvalue():$(input).val();case"remove":return this.each(function(){this.inputmask&&this.inputmask.remove()});case"getemptymask":return input&&input.inputmask?input.inputmask.getemptymask():"";case"hasMaskedValue":return!(!input||!input.inputmask)&&input.inputmask.hasMaskedValue();case"isComplete":return!input||!input.inputmask||input.inputmask.isComplete();case"getmetadata":return input&&input.inputmask?input.inputmask.getmetadata():void 0;case"setvalue":Inputmask.setValue(input,options);break;case"option":if("string"!=typeof options)return this.each(function(){if(void 0!==this.inputmask)return this.inputmask.option(options)});if(input&&void 0!==input.inputmask)return input.inputmask.option(options);break;default:return options.alias=fn,nptmask=new Inputmask(options),this.each(function(){nptmask.mask(this)})}else{if(Array.isArray(fn))return options.alias=fn,nptmask=new Inputmask(options),this.each(function(){nptmask.mask(this)});if("object"==_typeof(fn))return nptmask=new Inputmask(fn),void 0===fn.mask&&void 0===fn.alias?this.each(function(){if(void 0!==this.inputmask)return this.inputmask.option(fn);nptmask.mask(this)}):this.each(function(){nptmask.mask(this)});if(void 0===fn)return this.each(function(){nptmask=new Inputmask(options),nptmask.mask(this)})}})},function(module,exports,__webpack_require__){"use strict";var im=__webpack_require__(5),jQuery=__webpack_require__(2);im.dependencyLib===jQuery&&__webpack_require__(10),module.exports=im}],installedModules={},__webpack_require__.m=modules,__webpack_require__.c=installedModules,__webpack_require__.d=function(exports,name,getter){__webpack_require__.o(exports,name)||Object.defineProperty(exports,name,{enumerable:!0,get:getter})},__webpack_require__.r=function(exports){"undefined"!=typeof Symbol&&Symbol.toStringTag&&Object.defineProperty(exports,Symbol.toStringTag,{value:"Module"}),Object.defineProperty(exports,"__esModule",{value:!0})},__webpack_require__.t=function(value,mode){if(1&mode&&(value=__webpack_require__(value)),8&mode)return value;if(4&mode&&"object"==typeof value&&value&&value.__esModule)return value;var ns=Object.create(null);if(__webpack_require__.r(ns),Object.defineProperty(ns,"default",{enumerable:!0,value:value}),2&mode&&"string"!=typeof value)for(var key in value)__webpack_require__.d(ns,key,function(key){return value[key]}.bind(null,key));return ns},__webpack_require__.n=function(module){var getter=module&&module.__esModule?function getDefault(){return module.default}:function getModuleExports(){return module};return __webpack_require__.d(getter,"a",getter),getter},__webpack_require__.o=function(object,property){return Object.prototype.hasOwnProperty.call(object,property)},__webpack_require__.p="",__webpack_require__(__webpack_require__.s=11);function __webpack_require__(moduleId){if(installedModules[moduleId])return installedModules[moduleId].exports;var module=installedModules[moduleId]={i:moduleId,l:!1,exports:{}};return modules[moduleId].call(module.exports,module,module.exports,__webpack_require__),module.l=!0,module.exports}var modules,installedModules});
/* End */
;
; /* Start:"a:4:{s:4:"full";s:57:"/local/templates/.default/litera/litera.js?15384858033738";s:6:"source";s:42:"/local/templates/.default/litera/litera.js";s:3:"min";s:0:"";s:3:"map";s:0:"";}"*/
document.addEventListener('DOMContentLoaded', function(){
    
    var form = document.getElementById("litera-form");
    var shadow = document.getElementById("litera-modal-shadow");
    var modal = document.getElementById("litera-modal-window");
    var send = document.getElementById("litera-send");
    var close = document.getElementById("litera-close");
    var page = document.getElementById("LiteraForm_page");
    var text = document.getElementById("LiteraForm_text");
    var area = document.getElementById("LiteraForm_area");
    var comment = document.getElementById("LiteraForm_comment");
    var folder = document.getElementById("folder");
    var spam = true;
    
    var request = new XMLHttpRequest();
    var url = folder.value+"/litera/litera.php";

    request.onreadystatechange = function() {
        if(request.readyState === 4) {
            if(request.status === 200) {
                alert('Ваше сообщение успешно отправлено!');
            } else {
                alert('При отправке сообщения произошла ошибка. Попробуйте позже.');
            }
        }
    }
    
    send.onclick = function() {

		request.open("POST", url, true);
    	request.setRequestHeader("Content-type", "application/x-www-form-urlencoded");

        var params = 'page=' + page.value +
                    '&text=' + text.value +
                    '&area=' + area.value +
                    '&comment=' + comment.value;
        if(!spam) request.send(params);
        hide();
    };
    
    close.onclick = hide;
    shadow.onclick = hide;
    
    function hide() {
        shadow.style.display = 'none';
        modal.style.display = 'none';
    }
    
    function getSelectedText(){
        var text = "";
        var selection = "";
        if (window.getSelection) {
            selection = window.getSelection().toString();
            modify_selection(window.getSelection());
            text = window.getSelection();
        }else if (document.getSelection) {
            selection = document.getSelection().toString();
            modify_selection(document.getSelection());
            text = window.getSelection();
        }else if (document.selection) {
            selection = document.selection.text;
            text = document.selection.createRange().expand('word').text;
        }
        return [text, selection];
    };

    function modify_selection(sel) {
        var range = document.createRange();
        range.setStart(sel.anchorNode, sel.anchorOffset);
        range.setEnd(sel.focusNode, sel.focusOffset);
        var backwards = range.collapsed;
        range.detach();

        var endNode = sel.focusNode, endOffset = sel.focusOffset;
        sel.collapse(sel.anchorNode, sel.anchorOffset);
        if (backwards) {
            sel.modify("move", "forward", "word");
            sel.extend(endNode, endOffset);
            sel.modify("extend", "backward", "word");
        } else {
            sel.modify("move", "backward", "word");
            sel.extend(endNode, endOffset);
            sel.modify("extend", "forward", "word");
        }
    }
        
    function openWindow(){
          shadow.style.display = 'block';
          modal.style.display = 'block';
          spam = false;
    };
        
    document.addEventListener("keydown", keyDownTextField, false);
    function keyDownTextField(e) {
        if (e.ctrlKey && e.keyCode == 13) {
            var t = getSelectedText();
            if(t && modal.style.display != 'block'){
                text.value = t[1];
                area.value = t[0];
                comment.value='';
                openWindow(); 
            }
        }
    }; 

});

/* End */
;
; /* Start:"a:4:{s:4:"full";s:74:"/local/templates/.default/markup/new/js/social-likes.min.js?16479555399897";s:6:"source";s:59:"/local/templates/.default/markup/new/js/social-likes.min.js";s:3:"min";s:0:"";s:3:"map";s:0:"";}"*/
/*! Social Likes v3.1.3 by Artem Sapegin - http://sapegin.github.com/social-likes - Licensed MIT */

!function(t){"function"==typeof define&&define.amd?define(["jquery"],t):t(jQuery)}(function(t,e){"use strict";function i(t,e){this.container=t,this.options=e,this.init()}function n(e,i){this.widget=e,this.options=t.extend({},i),this.detectService(),this.service&&this.init()}function o(t){var e={},i=t.data();for(var n in i){var o=i[n];"yes"===o?o=!0:"no"===o&&(o=!1),e[n.replace(/-(\w)/g,function(t,e){return e.toUpper()})]=o}return e}function s(t,e){return r(t,e,encodeURIComponent)}function r(t,e,i){return t.replace(/\{([^}]+)\}/g,function(t,n){return n in e?i?i(e[n]):e[n]:t})}function c(t,e){var i=p+t;return i+" "+i+"_"+e}function u(e,i){function n(r){"keydown"===r.type&&27!==r.which||t(r.target).closest(e).length||(e.removeClass(h),o.off(s,n),t.isFunction(i)&&i())}var o=t(document),s="click touchstart keydown";o.on(s,n)}function a(t){if(document.documentElement.getBoundingClientRect){var e=parseInt(t.css("left"),10),i=parseInt(t.css("top"),10),n=t[0].getBoundingClientRect();n.left<10?t.css("left",10-n.left+e):n.right>window.innerWidth-10&&t.css("left",window.innerWidth-n.right-10+e),n.top<10?t.css("top",10-n.top+i):n.bottom>window.innerHeight-10&&t.css("top",window.innerHeight-n.bottom-10+i)}t.addClass(h)}var l="social-likes",p=l+"__",h=l+"_opened",d="https:"===location.protocol?"https:":"http:",f={facebook:{counterUrl:"https://graph.facebook.com/?id={url}",convertNumber:function(t){return t.share.share_count},popupUrl:"https://www.facebook.com/sharer/sharer.php?u={url}",popupWidth:600,popupHeight:359},twitter:{counters:!1,popupUrl:"https://twitter.com/intent/tweet?url={url}&text={title}",popupWidth:600,popupHeight:250,click:function(){return/[.?:\-–—]\s*$/.test(this.options.title)||(this.options.title+=":"),!0}},mailru:{counterUrl:d+"//connect.mail.ru/share_count?url_list={url}&callback=1&func=?",convertNumber:function(t){for(var e in t)if(t.hasOwnProperty(e))return t[e].shares},popupUrl:"https://connect.mail.ru/share?share_url={url}&title={title}",popupWidth:492,popupHeight:500},vkontakte:{counterUrl:"https://vk.com/share.php?act=count&url={url}&index={index}",counter:function(e,i){var n=f.vkontakte;n._||(n._=[],window.VK||(window.VK={}),window.VK.Share={count:function(t,e){n._[t].resolve(e)}});var o=n._.length;n._.push(i),t.getScript(s(e,{index:o})).fail(i.reject)},popupUrl:"https://vk.com/share.php?url={url}&title={title}",popupWidth:655,popupHeight:450},odnoklassniki:{counterUrl:d+"//connect.ok.ru/dk?st.cmd=extLike&ref={url}&uid={index}",counter:function(e,i){var n=f.odnoklassniki;n._||(n._=[],window.ODKL||(window.ODKL={}),window.ODKL.updateCount=function(t,e){n._[t].resolve(e)});var o=n._.length;n._.push(i),t.getScript(s(e,{index:o})).fail(i.reject)},popupUrl:"https://connect.ok.ru/dk?st.cmd=WidgetSharePreview&service=odnoklassniki&st.shareUrl={url}",popupWidth:580,popupHeight:336},plusone:{counters:!1,popupUrl:"https://plus.google.com/share?url={url}",popupWidth:500,popupHeight:550},pinterest:{counterUrl:d+"//api.pinterest.com/v1/urls/count.json?url={url}&callback=?",convertNumber:function(t){return t.count},popupUrl:"https://pinterest.com/pin/create/button/?url={url}&description={title}",popupWidth:740,popupHeight:550},telegram: {counterUrl: undefined,convertNumber: function(data) {return data.data[0].total_count;},popupUrl: 'https://telegram.me/share/url?url={url}&text={title}',popupWidth: 600,popupHeight: 500}},g={promises:{},fetch:function(e,i,n){g.promises[e]||(g.promises[e]={});var o=g.promises[e];if(!n.forceUpdate&&o[i])return o[i];var r=t.extend({},f[e],n),c=t.Deferred(),u=r.counterUrl&&s(r.counterUrl,{url:i});return u&&t.isFunction(r.counter)?r.counter(u,c):r.counterUrl?t.getJSON(u).done(function(e){try{var i=e;t.isFunction(r.convertNumber)&&(i=r.convertNumber(e)),c.resolve(i)}catch(t){c.reject()}}).fail(c.reject):c.reject(),o[i]=c.promise(),o[i]}};t.fn.socialLikes=function(e){return this.each(function(){var n=t(this),s=n.data(l);s?t.isPlainObject(e)&&s.update(e):(s=new i(n,t.extend({},t.fn.socialLikes.defaults,e,o(n))),n.data(l,s))})},t.fn.socialLikes.defaults={url:window.location.href.replace(window.location.hash,""),title:document.title,counters:!0,zeroes:!1,wait:500,timeout:1e4,popupCheckInterval:500,singleTitle:"Share"},i.prototype={init:function(){this.container.addClass(l),this.single=this.container.hasClass(l+"_single"),this.initUserButtons(),this.countersLeft=0,this.number=0,this.container.on("counter."+l,t.proxy(this.updateCounter,this));var e=this.container.children();this.makeSingleButton(),this.buttons=[],e.each(t.proxy(function(e,i){var o=new n(t(i),this.options);this.buttons.push(o),o.options.counterUrl&&this.countersLeft++},this)),this.options.counters?(this.timer=setTimeout(t.proxy(this.appear,this),this.options.wait),this.timeout=setTimeout(t.proxy(this.ready,this,!0),this.options.timeout)):this.appear()},initUserButtons:function(){!this.userButtonInited&&window.socialLikesButtons&&t.extend(!0,f,socialLikesButtons),this.userButtonInited=!0},makeSingleButton:function(){if(this.single){var e=this.container;e.addClass(l+"_vertical"),e.wrap(t("<div>",{class:l+"_single-w"})),e.wrapInner(t("<div>",{class:l+"__single-container"}));var i=e.parent(),n=t("<div>",{class:c("widget","single")}),o=t(r('<div class="{buttonCls}"><span class="{iconCls}"></span>{title}</div>',{buttonCls:c("button","single"),iconCls:c("icon","single"),title:this.options.singleTitle}));n.append(o),i.append(n),n.on("click",function(){var t=l+"__widget_active";return n.toggleClass(t),n.hasClass(t)?(e.css({left:-(e.width()-n.width())/2,top:-e.height()}),a(e),u(e,function(){n.removeClass(t)})):e.removeClass(h),!1}),this.widget=n}},update:function(e){if(e.forceUpdate||e.url!==this.options.url){this.number=0,this.countersLeft=this.buttons.length,this.widget&&this.widget.find("."+l+"__counter").remove(),t.extend(this.options,e);for(var i=0;i<this.buttons.length;i++)this.buttons[i].update(e)}},updateCounter:function(t,e,i){((i=i||0)||this.options.zeroes)&&(this.number+=i,this.single&&this.getCounterElem().text(this.number)),0===--this.countersLeft&&(this.appear(),this.ready())},appear:function(){this.container.addClass(l+"_visible")},ready:function(t){this.timeout&&clearTimeout(this.timeout),this.container.addClass(l+"_ready"),t||this.container.trigger("ready."+l,this.number)},getCounterElem:function(){var e=this.widget.find("."+p+"counter_single");return e.length||(e=t("<span>",{class:c("counter","single")}),this.widget.append(e)),e}},n.prototype={init:function(){this.detectParams(),this.initHtml(),setTimeout(t.proxy(this.initCounter,this),0)},update:function(e){t.extend(this.options,{forceUpdate:!1},e),this.widget.find("."+l+"__counter").remove(),this.initCounter()},detectService:function(){var e=this.widget.data("service");if(!e){for(var i=this.widget[0],n=i.classList||i.className.split(" "),o=0;o<n.length;o++){var s=n[o];if(f[s]){e=s;break}}if(!e)return}this.service=e,t.extend(this.options,f[e])},detectParams:function(){var t=this.widget.data();if(t.counter){var e=parseInt(t.counter,10);isNaN(e)?this.options.counterUrl=t.counter:this.options.counterNumber=e}t.title&&(this.options.title=t.title),t.url&&(this.options.url=t.url)},initHtml:function(){var e=this.options,i=this.widget,n=i.find("a");n.length&&this.cloneDataAttrs(n,i);var o=t("<span>",{class:this.getElementClassNames("button"),html:i.html()});if(e.clickUrl){var r=s(e.clickUrl,{url:e.url,title:e.title}),c=t("<a>",{href:r});this.cloneDataAttrs(i,c),i.replaceWith(c),this.widget=c,i=c}else i.on("click",t.proxy(this.click,this));i.removeClass(this.service),i.addClass(this.getElementClassNames("widget")),o.prepend(t("<span>",{class:this.getElementClassNames("icon")})),i.empty().append(o),this.button=o},initCounter:function(){if(this.options.counters)if(this.options.counterNumber)this.updateCounter(this.options.counterNumber);else{var e={counterUrl:this.options.counterUrl,forceUpdate:this.options.forceUpdate};g.fetch(this.service,this.options.url,e).always(t.proxy(this.updateCounter,this))}},cloneDataAttrs:function(t,e){var i=t.data();for(var n in i)i.hasOwnProperty(n)&&e.data(n,i[n])},getElementClassNames:function(t){return c(t,this.service)},updateCounter:function(e){e=parseInt(e,10)||0;var i={class:this.getElementClassNames("counter"),text:e};e||this.options.zeroes||(i.class+=" "+l+"__counter_empty",i.text="");var n=this.widget.find("."+p+"counter_"+this.service);n.length||(n=t("<span>",i),this.widget.append(n)),this.widget.trigger("counter."+l,[this.service,e])},click:function(e){var i=this.options,n=!0;if(t.isFunction(i.click)&&(n=i.click.call(this,e)),n){var o=s(i.popupUrl,{url:i.url,title:i.title});o=this.addAdditionalParamsToUrl(o),this.openPopup(o,{width:i.popupWidth,height:i.popupHeight})}return!1},addAdditionalParamsToUrl:function(e){var i=t.param(t.extend(this.widget.data(),this.options.data));return t.isEmptyObject(i)?e:e+(-1===e.indexOf("?")?"?":"&")+i},openPopup:function(i,n){var o=window.screenLeft!==e?window.screenLeft:screen.left,s=window.screenTop!==e?window.screenTop:screen.top,r=window.innerWidth?window.innerWidth:document.documentElement.clientWidth?document.documentElement.clientWidth:screen.width,c=window.innerHeight?window.innerHeight:document.documentElement.clientHeight?document.documentElement.clientHeight:screen.height,u=Math.round(r/2-n.width/2)+o,a=0;c>n.height&&(a=Math.round(c/3-n.height/2)+s);var p=window.open(i,"sl_"+this.service,"left="+u+",top="+a+",width="+n.width+",height="+n.height+",personalbar=0,toolbar=0,scrollbars=1,resizable=1");if(p){p.focus(),this.widget.trigger("popup_opened."+l,[this.service,p]);var h=setInterval(t.proxy(function(){p.closed&&(clearInterval(h),this.widget.trigger("popup_closed."+l,this.service))},this),this.options.popupCheckInterval)}else location.href=i}},t(function(){t("."+l).socialLikes()})});
/* End */
;
; /* Start:"a:4:{s:4:"full";s:65:"/local/templates/.default/markup/new/js/special.js?17017833844701";s:6:"source";s:50:"/local/templates/.default/markup/new/js/special.js";s:3:"min";s:0:"";s:3:"map";s:0:"";}"*/
window.onload = funonload;
function funonload() {
var coocka = ReadSpec('special');
//addButton(coocka);
checkSpec(coocka);
}

function checkSpec(spec) {
  setButton(spec);
if (spec == 'y')
{
  addStyleSheet('/local/templates/.default/markup/new/css/poor-vision.css');
}
}

function ReadSpec(name)
{
 var matches = document.cookie.match(new RegExp(
 "(?:^|; )" + name.replace(/([\.$?*|{}\(\)\[\]\\\/\+^])/g, '\\$1') + "=([^;]*)"
  ));
  return matches ? decodeURIComponent(matches[1]) : undefined;
}

function SetCookieSpec(name,value) {
    /*var cookie_date = new Date ( );
    cookie_date.setTime ( cookie_date.getTime() - 1 );
    document.cookie = name + "=; expires=" + cookie_date.toGMTString();*/


    // +1 день от текущей даты
    let date = new Date(Date.now() + 86400e3);
    date = date.toUTCString();

    document.cookie = name + "=" + value + '; expires=' + date + '; domain=' + location.hostname + '; path=/';
}

function addButton() {
    var button = document.createElement('input');
    var parentElem = document.body;
    var s = ReadSpec('special');
    button.type = 'button';
    button.id = 'spec';
    button.value = ' ';
    button.setAttribute("onclick","Spec()");
    button.setAttribute("style","width: 300px; height: 30px; margin-bottom: 5px; background-image: url(/local/templates/.default/markup/images/spec.gif)");
    parentElem.insertBefore(button, parentElem.firstChild);
    setButton(s);
}

function addStyleSheet(url)
{
  // ������ ������� LINK/STYLE � ��������� � ��������
  var style;

//  

//    if (typeof url == 'undefined')
//  {
//    style = document.createElement('style');
//  }
//  else
//  {
    style = document.createElement('link');
   style.rel = 'stylesheet';
   style.type = 'text/css';
    style.href = url;
//    style.id = 'speccss';
//  }
  
   document.getElementsByTagName('head')[0].appendChild(style);
  // ������� ����� ����� � ��������� styleSheets
  style = document.styleSheets[document.styleSheets.length - 1];

  // ������ ������ ������������ � W3C DOM2 (��� IE)
  return StyleSheet_makeCompatible(style);

}
function delStyleSheet()
{
style=document.getElementsByTagName('head')[0].lastChild;//document.styleSheets[document.styleSheets.length - 1]
document.getElementsByTagName('head')[0].removeChild(style);//
}


function Spec() {
var spec = ReadSpec('special');
//alert (spec);

if (spec == 'y')
{
   delStyleSheet();
   SetCookieSpec('special','n');  setButton('n');
}
else {
   addStyleSheet('/local/templates/.default/markup/new/css/poor-vision.css');
   SetCookieSpec('special','y');  setButton('y');

}
}

$(document).on('click', '#spec', function(e) {
  e.preventDefault();
  Spec();
});

function setButton(spec) {
var inputTag = document.getElementById("spec");
if (inputTag != null) {
    if (inputTag.hasAttribute("data-value"))               // ���� ���� ������� value
        defaultValue = inputTag.getAttribute("data-value");

  
    if (spec == 'y') {
        inputTag.text = 'Стандартная версия';
        
    } else {
        inputTag.text = 'Версия для слабовидящих';
        
    }
}
}


function StyleSheet_makeCompatible(style)
{
  // Mozilla �� ��� ������� � cssRules �� �������� �����
  try
  {
    style.cssRules;
  }
  catch (e)
  {
    return style;
  }

  // ������ CSSStyleSheet.cssRules
  if (typeof style.cssRules == 'undefined' && typeof style.rules != 'undefined')
    style.cssRules = style.rules;

  // ������ CSSStyleSheet.insertRule � CSSStyleSheet.deleteRule
  if (typeof style.insertRule == 'undefined' && typeof style.addRule != 'undefined')
    style.insertRule = StyleSheet_insertRule;
  if (typeof style.deleteRule == 'undefined' && typeof style.removeRule != 'undefined')
    style.deleteRule = style.removeRule;

  // ���������, ���������� �� ��� ������ ��������
  if (typeof style.cssRules == 'undefined' || typeof style.insertRule == 'undefined'
      || typeof style.deleteRule == 'undefined')
    return null;
  else
    return style;
}

function StyleSheet_insertRule(rule, index)
{
  // �������� �������� � ����� �� ���������
  if (rule.match(/^([^{]+)\{(.*)\}\s*$/))
  {
    this.addRule(RegExp.$1, RegExp.$2, index);
    return index;
  }
  throw "Syntax error in CSS rule to be added";
}
/* End */
;
; /* Start:"a:4:{s:4:"full";s:78:"/local/templates/.default/markup/new/js/jquery.fancybox.min.js?158459752468253";s:6:"source";s:62:"/local/templates/.default/markup/new/js/jquery.fancybox.min.js";s:3:"min";s:0:"";s:3:"map";s:0:"";}"*/
// ==================================================
// fancyBox v3.5.7
//
// Licensed GPLv3 for open source use
// or fancyBox Commercial License for commercial use
//
// http://fancyapps.com/fancybox/
// Copyright 2019 fancyApps
//
// ==================================================
!function(t,e,n,o){"use strict";function i(t,e){var o,i,a,s=[],r=0;t&&t.isDefaultPrevented()||(t.preventDefault(),e=e||{},t&&t.data&&(e=h(t.data.options,e)),o=e.$target||n(t.currentTarget).trigger("blur"),(a=n.fancybox.getInstance())&&a.$trigger&&a.$trigger.is(o)||(e.selector?s=n(e.selector):(i=o.attr("data-fancybox")||"",i?(s=t.data?t.data.items:[],s=s.length?s.filter('[data-fancybox="'+i+'"]'):n('[data-fancybox="'+i+'"]')):s=[o]),r=n(s).index(o),r<0&&(r=0),a=n.fancybox.open(s,e,r),a.$trigger=o))}if(t.console=t.console||{info:function(t){}},n){if(n.fn.fancybox)return void console.info("fancyBox already initialized");var a={closeExisting:!1,loop:!1,gutter:50,keyboard:!0,preventCaptionOverlap:!0,arrows:!0,infobar:!0,smallBtn:"auto",toolbar:"auto",buttons:["zoom","slideShow","thumbs","close"],idleTime:3,protect:!1,modal:!1,image:{preload:!1},ajax:{settings:{data:{fancybox:!0}}},iframe:{tpl:'<iframe id="fancybox-frame{rnd}" name="fancybox-frame{rnd}" class="fancybox-iframe" allowfullscreen="allowfullscreen" allow="autoplay; fullscreen" src=""></iframe>',preload:!0,css:{},attr:{scrolling:"auto"}},video:{tpl:'<video class="fancybox-video" controls controlsList="nodownload" poster="{{poster}}"><source src="{{src}}" type="{{format}}" />Sorry, your browser doesn\'t support embedded videos, <a href="{{src}}">download</a> and watch with your favorite video player!</video>',format:"",autoStart:!0},defaultType:"image",animationEffect:"zoom",animationDuration:366,zoomOpacity:"auto",transitionEffect:"fade",transitionDuration:366,slideClass:"",baseClass:"",baseTpl:'<div class="fancybox-container" role="dialog" tabindex="-1"><div class="fancybox-bg"></div><div class="fancybox-inner"><div class="fancybox-infobar"><span data-fancybox-index></span>&nbsp;/&nbsp;<span data-fancybox-count></span></div><div class="fancybox-toolbar">{{buttons}}</div><div class="fancybox-navigation">{{arrows}}</div><div class="fancybox-stage"></div><div class="fancybox-caption"><div class="fancybox-caption__body"></div></div></div></div>',spinnerTpl:'<div class="fancybox-loading"></div>',errorTpl:'<div class="fancybox-error"><p>{{ERROR}}</p></div>',btnTpl:{download:'<a download data-fancybox-download class="fancybox-button fancybox-button--download" title="{{DOWNLOAD}}" href="javascript:;"><svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path d="M18.62 17.09V19H5.38v-1.91zm-2.97-6.96L17 11.45l-5 4.87-5-4.87 1.36-1.32 2.68 2.64V5h1.92v7.77z"/></svg></a>',zoom:'<button data-fancybox-zoom class="fancybox-button fancybox-button--zoom" title="{{ZOOM}}"><svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path d="M18.7 17.3l-3-3a5.9 5.9 0 0 0-.6-7.6 5.9 5.9 0 0 0-8.4 0 5.9 5.9 0 0 0 0 8.4 5.9 5.9 0 0 0 7.7.7l3 3a1 1 0 0 0 1.3 0c.4-.5.4-1 0-1.5zM8.1 13.8a4 4 0 0 1 0-5.7 4 4 0 0 1 5.7 0 4 4 0 0 1 0 5.7 4 4 0 0 1-5.7 0z"/></svg></button>',close:'<button data-fancybox-close class="fancybox-button fancybox-button--close" title="{{CLOSE}}"><svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path d="M12 10.6L6.6 5.2 5.2 6.6l5.4 5.4-5.4 5.4 1.4 1.4 5.4-5.4 5.4 5.4 1.4-1.4-5.4-5.4 5.4-5.4-1.4-1.4-5.4 5.4z"/></svg></button>',arrowLeft:'<button data-fancybox-prev class="fancybox-button fancybox-button--arrow_left" title="{{PREV}}"><div><svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path d="M11.28 15.7l-1.34 1.37L5 12l4.94-5.07 1.34 1.38-2.68 2.72H19v1.94H8.6z"/></svg></div></button>',arrowRight:'<button data-fancybox-next class="fancybox-button fancybox-button--arrow_right" title="{{NEXT}}"><div><svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path d="M15.4 12.97l-2.68 2.72 1.34 1.38L19 12l-4.94-5.07-1.34 1.38 2.68 2.72H5v1.94z"/></svg></div></button>',smallBtn:'<button type="button" data-fancybox-close class="fancybox-button fancybox-close-small" title="{{CLOSE}}"><svg xmlns="http://www.w3.org/2000/svg" version="1" viewBox="0 0 24 24"><path d="M13 12l5-5-1-1-5 5-5-5-1 1 5 5-5 5 1 1 5-5 5 5 1-1z"/></svg></button>'},parentEl:"body",hideScrollbar:!0,autoFocus:!0,backFocus:!0,trapFocus:!0,fullScreen:{autoStart:!1},touch:{vertical:!0,momentum:!0},hash:null,media:{},slideShow:{autoStart:!1,speed:3e3},thumbs:{autoStart:!1,hideOnClose:!0,parentEl:".fancybox-container",axis:"y"},wheel:"auto",onInit:n.noop,beforeLoad:n.noop,afterLoad:n.noop,beforeShow:n.noop,afterShow:n.noop,beforeClose:n.noop,afterClose:n.noop,onActivate:n.noop,onDeactivate:n.noop,clickContent:function(t,e){return"image"===t.type&&"zoom"},clickSlide:"close",clickOutside:"close",dblclickContent:!1,dblclickSlide:!1,dblclickOutside:!1,mobile:{preventCaptionOverlap:!1,idleTime:!1,clickContent:function(t,e){return"image"===t.type&&"toggleControls"},clickSlide:function(t,e){return"image"===t.type?"toggleControls":"close"},dblclickContent:function(t,e){return"image"===t.type&&"zoom"},dblclickSlide:function(t,e){return"image"===t.type&&"zoom"}},lang:"en",i18n:{en:{CLOSE:"Close",NEXT:"Next",PREV:"Previous",ERROR:"The requested content cannot be loaded. <br/> Please try again later.",PLAY_START:"Start slideshow",PLAY_STOP:"Pause slideshow",FULL_SCREEN:"Full screen",THUMBS:"Thumbnails",DOWNLOAD:"Download",SHARE:"Share",ZOOM:"Zoom"},de:{CLOSE:"Schlie&szlig;en",NEXT:"Weiter",PREV:"Zur&uuml;ck",ERROR:"Die angeforderten Daten konnten nicht geladen werden. <br/> Bitte versuchen Sie es sp&auml;ter nochmal.",PLAY_START:"Diaschau starten",PLAY_STOP:"Diaschau beenden",FULL_SCREEN:"Vollbild",THUMBS:"Vorschaubilder",DOWNLOAD:"Herunterladen",SHARE:"Teilen",ZOOM:"Vergr&ouml;&szlig;ern"}}},s=n(t),r=n(e),c=0,l=function(t){return t&&t.hasOwnProperty&&t instanceof n},d=function(){return t.requestAnimationFrame||t.webkitRequestAnimationFrame||t.mozRequestAnimationFrame||t.oRequestAnimationFrame||function(e){return t.setTimeout(e,1e3/60)}}(),u=function(){return t.cancelAnimationFrame||t.webkitCancelAnimationFrame||t.mozCancelAnimationFrame||t.oCancelAnimationFrame||function(e){t.clearTimeout(e)}}(),f=function(){var t,n=e.createElement("fakeelement"),o={transition:"transitionend",OTransition:"oTransitionEnd",MozTransition:"transitionend",WebkitTransition:"webkitTransitionEnd"};for(t in o)if(void 0!==n.style[t])return o[t];return"transitionend"}(),p=function(t){return t&&t.length&&t[0].offsetHeight},h=function(t,e){var o=n.extend(!0,{},t,e);return n.each(e,function(t,e){n.isArray(e)&&(o[t]=e)}),o},g=function(t){var o,i;return!(!t||t.ownerDocument!==e)&&(n(".fancybox-container").css("pointer-events","none"),o={x:t.getBoundingClientRect().left+t.offsetWidth/2,y:t.getBoundingClientRect().top+t.offsetHeight/2},i=e.elementFromPoint(o.x,o.y)===t,n(".fancybox-container").css("pointer-events",""),i)},b=function(t,e,o){var i=this;i.opts=h({index:o},n.fancybox.defaults),n.isPlainObject(e)&&(i.opts=h(i.opts,e)),n.fancybox.isMobile&&(i.opts=h(i.opts,i.opts.mobile)),i.id=i.opts.id||++c,i.currIndex=parseInt(i.opts.index,10)||0,i.prevIndex=null,i.prevPos=null,i.currPos=0,i.firstRun=!0,i.group=[],i.slides={},i.addContent(t),i.group.length&&i.init()};n.extend(b.prototype,{init:function(){var o,i,a=this,s=a.group[a.currIndex],r=s.opts;r.closeExisting&&n.fancybox.close(!0),n("body").addClass("fancybox-active"),!n.fancybox.getInstance()&&!1!==r.hideScrollbar&&!n.fancybox.isMobile&&e.body.scrollHeight>t.innerHeight&&(n("head").append('<style id="fancybox-style-noscroll" type="text/css">.compensate-for-scrollbar{margin-right:'+(t.innerWidth-e.documentElement.clientWidth)+"px;}</style>"),n("body").addClass("compensate-for-scrollbar")),i="",n.each(r.buttons,function(t,e){i+=r.btnTpl[e]||""}),o=n(a.translate(a,r.baseTpl.replace("{{buttons}}",i).replace("{{arrows}}",r.btnTpl.arrowLeft+r.btnTpl.arrowRight))).attr("id","fancybox-container-"+a.id).addClass(r.baseClass).data("FancyBox",a).appendTo(r.parentEl),a.$refs={container:o},["bg","inner","infobar","toolbar","stage","caption","navigation"].forEach(function(t){a.$refs[t]=o.find(".fancybox-"+t)}),a.trigger("onInit"),a.activate(),a.jumpTo(a.currIndex)},translate:function(t,e){var n=t.opts.i18n[t.opts.lang]||t.opts.i18n.en;return e.replace(/\{\{(\w+)\}\}/g,function(t,e){return void 0===n[e]?t:n[e]})},addContent:function(t){var e,o=this,i=n.makeArray(t);n.each(i,function(t,e){var i,a,s,r,c,l={},d={};n.isPlainObject(e)?(l=e,d=e.opts||e):"object"===n.type(e)&&n(e).length?(i=n(e),d=i.data()||{},d=n.extend(!0,{},d,d.options),d.$orig=i,l.src=o.opts.src||d.src||i.attr("href"),l.type||l.src||(l.type="inline",l.src=e)):l={type:"html",src:e+""},l.opts=n.extend(!0,{},o.opts,d),n.isArray(d.buttons)&&(l.opts.buttons=d.buttons),n.fancybox.isMobile&&l.opts.mobile&&(l.opts=h(l.opts,l.opts.mobile)),a=l.type||l.opts.type,r=l.src||"",!a&&r&&((s=r.match(/\.(mp4|mov|ogv|webm)((\?|#).*)?$/i))?(a="video",l.opts.video.format||(l.opts.video.format="video/"+("ogv"===s[1]?"ogg":s[1]))):r.match(/(^data:image\/[a-z0-9+\/=]*,)|(\.(jp(e|g|eg)|gif|png|bmp|webp|svg|ico)((\?|#).*)?$)/i)?a="image":r.match(/\.(pdf)((\?|#).*)?$/i)?(a="iframe",l=n.extend(!0,l,{contentType:"pdf",opts:{iframe:{preload:!1}}})):"#"===r.charAt(0)&&(a="inline")),a?l.type=a:o.trigger("objectNeedsType",l),l.contentType||(l.contentType=n.inArray(l.type,["html","inline","ajax"])>-1?"html":l.type),l.index=o.group.length,"auto"==l.opts.smallBtn&&(l.opts.smallBtn=n.inArray(l.type,["html","inline","ajax"])>-1),"auto"===l.opts.toolbar&&(l.opts.toolbar=!l.opts.smallBtn),l.$thumb=l.opts.$thumb||null,l.opts.$trigger&&l.index===o.opts.index&&(l.$thumb=l.opts.$trigger.find("img:first"),l.$thumb.length&&(l.opts.$orig=l.opts.$trigger)),l.$thumb&&l.$thumb.length||!l.opts.$orig||(l.$thumb=l.opts.$orig.find("img:first")),l.$thumb&&!l.$thumb.length&&(l.$thumb=null),l.thumb=l.opts.thumb||(l.$thumb?l.$thumb[0].src:null),"function"===n.type(l.opts.caption)&&(l.opts.caption=l.opts.caption.apply(e,[o,l])),"function"===n.type(o.opts.caption)&&(l.opts.caption=o.opts.caption.apply(e,[o,l])),l.opts.caption instanceof n||(l.opts.caption=void 0===l.opts.caption?"":l.opts.caption+""),"ajax"===l.type&&(c=r.split(/\s+/,2),c.length>1&&(l.src=c.shift(),l.opts.filter=c.shift())),l.opts.modal&&(l.opts=n.extend(!0,l.opts,{trapFocus:!0,infobar:0,toolbar:0,smallBtn:0,keyboard:0,slideShow:0,fullScreen:0,thumbs:0,touch:0,clickContent:!1,clickSlide:!1,clickOutside:!1,dblclickContent:!1,dblclickSlide:!1,dblclickOutside:!1})),o.group.push(l)}),Object.keys(o.slides).length&&(o.updateControls(),(e=o.Thumbs)&&e.isActive&&(e.create(),e.focus()))},addEvents:function(){var e=this;e.removeEvents(),e.$refs.container.on("click.fb-close","[data-fancybox-close]",function(t){t.stopPropagation(),t.preventDefault(),e.close(t)}).on("touchstart.fb-prev click.fb-prev","[data-fancybox-prev]",function(t){t.stopPropagation(),t.preventDefault(),e.previous()}).on("touchstart.fb-next click.fb-next","[data-fancybox-next]",function(t){t.stopPropagation(),t.preventDefault(),e.next()}).on("click.fb","[data-fancybox-zoom]",function(t){e[e.isScaledDown()?"scaleToActual":"scaleToFit"]()}),s.on("orientationchange.fb resize.fb",function(t){t&&t.originalEvent&&"resize"===t.originalEvent.type?(e.requestId&&u(e.requestId),e.requestId=d(function(){e.update(t)})):(e.current&&"iframe"===e.current.type&&e.$refs.stage.hide(),setTimeout(function(){e.$refs.stage.show(),e.update(t)},n.fancybox.isMobile?600:250))}),r.on("keydown.fb",function(t){var o=n.fancybox?n.fancybox.getInstance():null,i=o.current,a=t.keyCode||t.which;if(9==a)return void(i.opts.trapFocus&&e.focus(t));if(!(!i.opts.keyboard||t.ctrlKey||t.altKey||t.shiftKey||n(t.target).is("input,textarea,video,audio,select")))return 8===a||27===a?(t.preventDefault(),void e.close(t)):37===a||38===a?(t.preventDefault(),void e.previous()):39===a||40===a?(t.preventDefault(),void e.next()):void e.trigger("afterKeydown",t,a)}),e.group[e.currIndex].opts.idleTime&&(e.idleSecondsCounter=0,r.on("mousemove.fb-idle mouseleave.fb-idle mousedown.fb-idle touchstart.fb-idle touchmove.fb-idle scroll.fb-idle keydown.fb-idle",function(t){e.idleSecondsCounter=0,e.isIdle&&e.showControls(),e.isIdle=!1}),e.idleInterval=t.setInterval(function(){++e.idleSecondsCounter>=e.group[e.currIndex].opts.idleTime&&!e.isDragging&&(e.isIdle=!0,e.idleSecondsCounter=0,e.hideControls())},1e3))},removeEvents:function(){var e=this;s.off("orientationchange.fb resize.fb"),r.off("keydown.fb .fb-idle"),this.$refs.container.off(".fb-close .fb-prev .fb-next"),e.idleInterval&&(t.clearInterval(e.idleInterval),e.idleInterval=null)},previous:function(t){return this.jumpTo(this.currPos-1,t)},next:function(t){return this.jumpTo(this.currPos+1,t)},jumpTo:function(t,e){var o,i,a,s,r,c,l,d,u,f=this,h=f.group.length;if(!(f.isDragging||f.isClosing||f.isAnimating&&f.firstRun)){if(t=parseInt(t,10),!(a=f.current?f.current.opts.loop:f.opts.loop)&&(t<0||t>=h))return!1;if(o=f.firstRun=!Object.keys(f.slides).length,r=f.current,f.prevIndex=f.currIndex,f.prevPos=f.currPos,s=f.createSlide(t),h>1&&((a||s.index<h-1)&&f.createSlide(t+1),(a||s.index>0)&&f.createSlide(t-1)),f.current=s,f.currIndex=s.index,f.currPos=s.pos,f.trigger("beforeShow",o),f.updateControls(),s.forcedDuration=void 0,n.isNumeric(e)?s.forcedDuration=e:e=s.opts[o?"animationDuration":"transitionDuration"],e=parseInt(e,10),i=f.isMoved(s),s.$slide.addClass("fancybox-slide--current"),o)return s.opts.animationEffect&&e&&f.$refs.container.css("transition-duration",e+"ms"),f.$refs.container.addClass("fancybox-is-open").trigger("focus"),f.loadSlide(s),void f.preload("image");c=n.fancybox.getTranslate(r.$slide),l=n.fancybox.getTranslate(f.$refs.stage),n.each(f.slides,function(t,e){n.fancybox.stop(e.$slide,!0)}),r.pos!==s.pos&&(r.isComplete=!1),r.$slide.removeClass("fancybox-slide--complete fancybox-slide--current"),i?(u=c.left-(r.pos*c.width+r.pos*r.opts.gutter),n.each(f.slides,function(t,o){o.$slide.removeClass("fancybox-animated").removeClass(function(t,e){return(e.match(/(^|\s)fancybox-fx-\S+/g)||[]).join(" ")});var i=o.pos*c.width+o.pos*o.opts.gutter;n.fancybox.setTranslate(o.$slide,{top:0,left:i-l.left+u}),o.pos!==s.pos&&o.$slide.addClass("fancybox-slide--"+(o.pos>s.pos?"next":"previous")),p(o.$slide),n.fancybox.animate(o.$slide,{top:0,left:(o.pos-s.pos)*c.width+(o.pos-s.pos)*o.opts.gutter},e,function(){o.$slide.css({transform:"",opacity:""}).removeClass("fancybox-slide--next fancybox-slide--previous"),o.pos===f.currPos&&f.complete()})})):e&&s.opts.transitionEffect&&(d="fancybox-animated fancybox-fx-"+s.opts.transitionEffect,r.$slide.addClass("fancybox-slide--"+(r.pos>s.pos?"next":"previous")),n.fancybox.animate(r.$slide,d,e,function(){r.$slide.removeClass(d).removeClass("fancybox-slide--next fancybox-slide--previous")},!1)),s.isLoaded?f.revealContent(s):f.loadSlide(s),f.preload("image")}},createSlide:function(t){var e,o,i=this;return o=t%i.group.length,o=o<0?i.group.length+o:o,!i.slides[t]&&i.group[o]&&(e=n('<div class="fancybox-slide"></div>').appendTo(i.$refs.stage),i.slides[t]=n.extend(!0,{},i.group[o],{pos:t,$slide:e,isLoaded:!1}),i.updateSlide(i.slides[t])),i.slides[t]},scaleToActual:function(t,e,o){var i,a,s,r,c,l=this,d=l.current,u=d.$content,f=n.fancybox.getTranslate(d.$slide).width,p=n.fancybox.getTranslate(d.$slide).height,h=d.width,g=d.height;l.isAnimating||l.isMoved()||!u||"image"!=d.type||!d.isLoaded||d.hasError||(l.isAnimating=!0,n.fancybox.stop(u),t=void 0===t?.5*f:t,e=void 0===e?.5*p:e,i=n.fancybox.getTranslate(u),i.top-=n.fancybox.getTranslate(d.$slide).top,i.left-=n.fancybox.getTranslate(d.$slide).left,r=h/i.width,c=g/i.height,a=.5*f-.5*h,s=.5*p-.5*g,h>f&&(a=i.left*r-(t*r-t),a>0&&(a=0),a<f-h&&(a=f-h)),g>p&&(s=i.top*c-(e*c-e),s>0&&(s=0),s<p-g&&(s=p-g)),l.updateCursor(h,g),n.fancybox.animate(u,{top:s,left:a,scaleX:r,scaleY:c},o||366,function(){l.isAnimating=!1}),l.SlideShow&&l.SlideShow.isActive&&l.SlideShow.stop())},scaleToFit:function(t){var e,o=this,i=o.current,a=i.$content;o.isAnimating||o.isMoved()||!a||"image"!=i.type||!i.isLoaded||i.hasError||(o.isAnimating=!0,n.fancybox.stop(a),e=o.getFitPos(i),o.updateCursor(e.width,e.height),n.fancybox.animate(a,{top:e.top,left:e.left,scaleX:e.width/a.width(),scaleY:e.height/a.height()},t||366,function(){o.isAnimating=!1}))},getFitPos:function(t){var e,o,i,a,s=this,r=t.$content,c=t.$slide,l=t.width||t.opts.width,d=t.height||t.opts.height,u={};return!!(t.isLoaded&&r&&r.length)&&(e=n.fancybox.getTranslate(s.$refs.stage).width,o=n.fancybox.getTranslate(s.$refs.stage).height,e-=parseFloat(c.css("paddingLeft"))+parseFloat(c.css("paddingRight"))+parseFloat(r.css("marginLeft"))+parseFloat(r.css("marginRight")),o-=parseFloat(c.css("paddingTop"))+parseFloat(c.css("paddingBottom"))+parseFloat(r.css("marginTop"))+parseFloat(r.css("marginBottom")),l&&d||(l=e,d=o),i=Math.min(1,e/l,o/d),l*=i,d*=i,l>e-.5&&(l=e),d>o-.5&&(d=o),"image"===t.type?(u.top=Math.floor(.5*(o-d))+parseFloat(c.css("paddingTop")),u.left=Math.floor(.5*(e-l))+parseFloat(c.css("paddingLeft"))):"video"===t.contentType&&(a=t.opts.width&&t.opts.height?l/d:t.opts.ratio||16/9,d>l/a?d=l/a:l>d*a&&(l=d*a)),u.width=l,u.height=d,u)},update:function(t){var e=this;n.each(e.slides,function(n,o){e.updateSlide(o,t)})},updateSlide:function(t,e){var o=this,i=t&&t.$content,a=t.width||t.opts.width,s=t.height||t.opts.height,r=t.$slide;o.adjustCaption(t),i&&(a||s||"video"===t.contentType)&&!t.hasError&&(n.fancybox.stop(i),n.fancybox.setTranslate(i,o.getFitPos(t)),t.pos===o.currPos&&(o.isAnimating=!1,o.updateCursor())),o.adjustLayout(t),r.length&&(r.trigger("refresh"),t.pos===o.currPos&&o.$refs.toolbar.add(o.$refs.navigation.find(".fancybox-button--arrow_right")).toggleClass("compensate-for-scrollbar",r.get(0).scrollHeight>r.get(0).clientHeight)),o.trigger("onUpdate",t,e)},centerSlide:function(t){var e=this,o=e.current,i=o.$slide;!e.isClosing&&o&&(i.siblings().css({transform:"",opacity:""}),i.parent().children().removeClass("fancybox-slide--previous fancybox-slide--next"),n.fancybox.animate(i,{top:0,left:0,opacity:1},void 0===t?0:t,function(){i.css({transform:"",opacity:""}),o.isComplete||e.complete()},!1))},isMoved:function(t){var e,o,i=t||this.current;return!!i&&(o=n.fancybox.getTranslate(this.$refs.stage),e=n.fancybox.getTranslate(i.$slide),!i.$slide.hasClass("fancybox-animated")&&(Math.abs(e.top-o.top)>.5||Math.abs(e.left-o.left)>.5))},updateCursor:function(t,e){var o,i,a=this,s=a.current,r=a.$refs.container;s&&!a.isClosing&&a.Guestures&&(r.removeClass("fancybox-is-zoomable fancybox-can-zoomIn fancybox-can-zoomOut fancybox-can-swipe fancybox-can-pan"),o=a.canPan(t,e),i=!!o||a.isZoomable(),r.toggleClass("fancybox-is-zoomable",i),n("[data-fancybox-zoom]").prop("disabled",!i),o?r.addClass("fancybox-can-pan"):i&&("zoom"===s.opts.clickContent||n.isFunction(s.opts.clickContent)&&"zoom"==s.opts.clickContent(s))?r.addClass("fancybox-can-zoomIn"):s.opts.touch&&(s.opts.touch.vertical||a.group.length>1)&&"video"!==s.contentType&&r.addClass("fancybox-can-swipe"))},isZoomable:function(){var t,e=this,n=e.current;if(n&&!e.isClosing&&"image"===n.type&&!n.hasError){if(!n.isLoaded)return!0;if((t=e.getFitPos(n))&&(n.width>t.width||n.height>t.height))return!0}return!1},isScaledDown:function(t,e){var o=this,i=!1,a=o.current,s=a.$content;return void 0!==t&&void 0!==e?i=t<a.width&&e<a.height:s&&(i=n.fancybox.getTranslate(s),i=i.width<a.width&&i.height<a.height),i},canPan:function(t,e){var o=this,i=o.current,a=null,s=!1;return"image"===i.type&&(i.isComplete||t&&e)&&!i.hasError&&(s=o.getFitPos(i),void 0!==t&&void 0!==e?a={width:t,height:e}:i.isComplete&&(a=n.fancybox.getTranslate(i.$content)),a&&s&&(s=Math.abs(a.width-s.width)>1.5||Math.abs(a.height-s.height)>1.5)),s},loadSlide:function(t){var e,o,i,a=this;if(!t.isLoading&&!t.isLoaded){if(t.isLoading=!0,!1===a.trigger("beforeLoad",t))return t.isLoading=!1,!1;switch(e=t.type,o=t.$slide,o.off("refresh").trigger("onReset").addClass(t.opts.slideClass),e){case"image":a.setImage(t);break;case"iframe":a.setIframe(t);break;case"html":a.setContent(t,t.src||t.content);break;case"video":a.setContent(t,t.opts.video.tpl.replace(/\{\{src\}\}/gi,t.src).replace("{{format}}",t.opts.videoFormat||t.opts.video.format||"").replace("{{poster}}",t.thumb||""));break;case"inline":n(t.src).length?a.setContent(t,n(t.src)):a.setError(t);break;case"ajax":a.showLoading(t),i=n.ajax(n.extend({},t.opts.ajax.settings,{url:t.src,success:function(e,n){"success"===n&&a.setContent(t,e)},error:function(e,n){e&&"abort"!==n&&a.setError(t)}})),o.one("onReset",function(){i.abort()});break;default:a.setError(t)}return!0}},setImage:function(t){var o,i=this;setTimeout(function(){var e=t.$image;i.isClosing||!t.isLoading||e&&e.length&&e[0].complete||t.hasError||i.showLoading(t)},50),i.checkSrcset(t),t.$content=n('<div class="fancybox-content"></div>').addClass("fancybox-is-hidden").appendTo(t.$slide.addClass("fancybox-slide--image")),!1!==t.opts.preload&&t.opts.width&&t.opts.height&&t.thumb&&(t.width=t.opts.width,t.height=t.opts.height,o=e.createElement("img"),o.onerror=function(){n(this).remove(),t.$ghost=null},o.onload=function(){i.afterLoad(t)},t.$ghost=n(o).addClass("fancybox-image").appendTo(t.$content).attr("src",t.thumb)),i.setBigImage(t)},checkSrcset:function(e){var n,o,i,a,s=e.opts.srcset||e.opts.image.srcset;if(s){i=t.devicePixelRatio||1,a=t.innerWidth*i,o=s.split(",").map(function(t){var e={};return t.trim().split(/\s+/).forEach(function(t,n){var o=parseInt(t.substring(0,t.length-1),10);if(0===n)return e.url=t;o&&(e.value=o,e.postfix=t[t.length-1])}),e}),o.sort(function(t,e){return t.value-e.value});for(var r=0;r<o.length;r++){var c=o[r];if("w"===c.postfix&&c.value>=a||"x"===c.postfix&&c.value>=i){n=c;break}}!n&&o.length&&(n=o[o.length-1]),n&&(e.src=n.url,e.width&&e.height&&"w"==n.postfix&&(e.height=e.width/e.height*n.value,e.width=n.value),e.opts.srcset=s)}},setBigImage:function(t){var o=this,i=e.createElement("img"),a=n(i);t.$image=a.one("error",function(){o.setError(t)}).one("load",function(){var e;t.$ghost||(o.resolveImageSlideSize(t,this.naturalWidth,this.naturalHeight),o.afterLoad(t)),o.isClosing||(t.opts.srcset&&(e=t.opts.sizes,e&&"auto"!==e||(e=(t.width/t.height>1&&s.width()/s.height()>1?"100":Math.round(t.width/t.height*100))+"vw"),a.attr("sizes",e).attr("srcset",t.opts.srcset)),t.$ghost&&setTimeout(function(){t.$ghost&&!o.isClosing&&t.$ghost.hide()},Math.min(300,Math.max(1e3,t.height/1600))),o.hideLoading(t))}).addClass("fancybox-image").attr("src",t.src).appendTo(t.$content),(i.complete||"complete"==i.readyState)&&a.naturalWidth&&a.naturalHeight?a.trigger("load"):i.error&&a.trigger("error")},resolveImageSlideSize:function(t,e,n){var o=parseInt(t.opts.width,10),i=parseInt(t.opts.height,10);t.width=e,t.height=n,o>0&&(t.width=o,t.height=Math.floor(o*n/e)),i>0&&(t.width=Math.floor(i*e/n),t.height=i)},setIframe:function(t){var e,o=this,i=t.opts.iframe,a=t.$slide;t.$content=n('<div class="fancybox-content'+(i.preload?" fancybox-is-hidden":"")+'"></div>').css(i.css).appendTo(a),a.addClass("fancybox-slide--"+t.contentType),t.$iframe=e=n(i.tpl.replace(/\{rnd\}/g,(new Date).getTime())).attr(i.attr).appendTo(t.$content),i.preload?(o.showLoading(t),e.on("load.fb error.fb",function(e){this.isReady=1,t.$slide.trigger("refresh"),o.afterLoad(t)}),a.on("refresh.fb",function(){var n,o,s=t.$content,r=i.css.width,c=i.css.height;if(1===e[0].isReady){try{n=e.contents(),o=n.find("body")}catch(t){}o&&o.length&&o.children().length&&(a.css("overflow","visible"),s.css({width:"100%","max-width":"100%",height:"9999px"}),void 0===r&&(r=Math.ceil(Math.max(o[0].clientWidth,o.outerWidth(!0)))),s.css("width",r||"").css("max-width",""),void 0===c&&(c=Math.ceil(Math.max(o[0].clientHeight,o.outerHeight(!0)))),s.css("height",c||""),a.css("overflow","auto")),s.removeClass("fancybox-is-hidden")}})):o.afterLoad(t),e.attr("src",t.src),a.one("onReset",function(){try{n(this).find("iframe").hide().unbind().attr("src","//about:blank")}catch(t){}n(this).off("refresh.fb").empty(),t.isLoaded=!1,t.isRevealed=!1})},setContent:function(t,e){var o=this;o.isClosing||(o.hideLoading(t),t.$content&&n.fancybox.stop(t.$content),t.$slide.empty(),l(e)&&e.parent().length?((e.hasClass("fancybox-content")||e.parent().hasClass("fancybox-content"))&&e.parents(".fancybox-slide").trigger("onReset"),t.$placeholder=n("<div>").hide().insertAfter(e),e.css("display","inline-block")):t.hasError||("string"===n.type(e)&&(e=n("<div>").append(n.trim(e)).contents()),t.opts.filter&&(e=n("<div>").html(e).find(t.opts.filter))),t.$slide.one("onReset",function(){n(this).find("video,audio").trigger("pause"),t.$placeholder&&(t.$placeholder.after(e.removeClass("fancybox-content").hide()).remove(),t.$placeholder=null),t.$smallBtn&&(t.$smallBtn.remove(),t.$smallBtn=null),t.hasError||(n(this).empty(),t.isLoaded=!1,t.isRevealed=!1)}),n(e).appendTo(t.$slide),n(e).is("video,audio")&&(n(e).addClass("fancybox-video"),n(e).wrap("<div></div>"),t.contentType="video",t.opts.width=t.opts.width||n(e).attr("width"),t.opts.height=t.opts.height||n(e).attr("height")),t.$content=t.$slide.children().filter("div,form,main,video,audio,article,.fancybox-content").first(),t.$content.siblings().hide(),t.$content.length||(t.$content=t.$slide.wrapInner("<div></div>").children().first()),t.$content.addClass("fancybox-content"),t.$slide.addClass("fancybox-slide--"+t.contentType),o.afterLoad(t))},setError:function(t){t.hasError=!0,t.$slide.trigger("onReset").removeClass("fancybox-slide--"+t.contentType).addClass("fancybox-slide--error"),t.contentType="html",this.setContent(t,this.translate(t,t.opts.errorTpl)),t.pos===this.currPos&&(this.isAnimating=!1)},showLoading:function(t){var e=this;(t=t||e.current)&&!t.$spinner&&(t.$spinner=n(e.translate(e,e.opts.spinnerTpl)).appendTo(t.$slide).hide().fadeIn("fast"))},hideLoading:function(t){var e=this;(t=t||e.current)&&t.$spinner&&(t.$spinner.stop().remove(),delete t.$spinner)},afterLoad:function(t){var e=this;e.isClosing||(t.isLoading=!1,t.isLoaded=!0,e.trigger("afterLoad",t),e.hideLoading(t),!t.opts.smallBtn||t.$smallBtn&&t.$smallBtn.length||(t.$smallBtn=n(e.translate(t,t.opts.btnTpl.smallBtn)).appendTo(t.$content)),t.opts.protect&&t.$content&&!t.hasError&&(t.$content.on("contextmenu.fb",function(t){return 2==t.button&&t.preventDefault(),!0}),"image"===t.type&&n('<div class="fancybox-spaceball"></div>').appendTo(t.$content)),e.adjustCaption(t),e.adjustLayout(t),t.pos===e.currPos&&e.updateCursor(),e.revealContent(t))},adjustCaption:function(t){var e,n=this,o=t||n.current,i=o.opts.caption,a=o.opts.preventCaptionOverlap,s=n.$refs.caption,r=!1;s.toggleClass("fancybox-caption--separate",a),a&&i&&i.length&&(o.pos!==n.currPos?(e=s.clone().appendTo(s.parent()),e.children().eq(0).empty().html(i),r=e.outerHeight(!0),e.empty().remove()):n.$caption&&(r=n.$caption.outerHeight(!0)),o.$slide.css("padding-bottom",r||""))},adjustLayout:function(t){var e,n,o,i,a=this,s=t||a.current;s.isLoaded&&!0!==s.opts.disableLayoutFix&&(s.$content.css("margin-bottom",""),s.$content.outerHeight()>s.$slide.height()+.5&&(o=s.$slide[0].style["padding-bottom"],i=s.$slide.css("padding-bottom"),parseFloat(i)>0&&(e=s.$slide[0].scrollHeight,s.$slide.css("padding-bottom",0),Math.abs(e-s.$slide[0].scrollHeight)<1&&(n=i),s.$slide.css("padding-bottom",o))),s.$content.css("margin-bottom",n))},revealContent:function(t){var e,o,i,a,s=this,r=t.$slide,c=!1,l=!1,d=s.isMoved(t),u=t.isRevealed;return t.isRevealed=!0,e=t.opts[s.firstRun?"animationEffect":"transitionEffect"],i=t.opts[s.firstRun?"animationDuration":"transitionDuration"],i=parseInt(void 0===t.forcedDuration?i:t.forcedDuration,10),!d&&t.pos===s.currPos&&i||(e=!1),"zoom"===e&&(t.pos===s.currPos&&i&&"image"===t.type&&!t.hasError&&(l=s.getThumbPos(t))?c=s.getFitPos(t):e="fade"),"zoom"===e?(s.isAnimating=!0,c.scaleX=c.width/l.width,c.scaleY=c.height/l.height,a=t.opts.zoomOpacity,"auto"==a&&(a=Math.abs(t.width/t.height-l.width/l.height)>.1),a&&(l.opacity=.1,c.opacity=1),n.fancybox.setTranslate(t.$content.removeClass("fancybox-is-hidden"),l),p(t.$content),void n.fancybox.animate(t.$content,c,i,function(){s.isAnimating=!1,s.complete()})):(s.updateSlide(t),e?(n.fancybox.stop(r),o="fancybox-slide--"+(t.pos>=s.prevPos?"next":"previous")+" fancybox-animated fancybox-fx-"+e,r.addClass(o).removeClass("fancybox-slide--current"),t.$content.removeClass("fancybox-is-hidden"),p(r),"image"!==t.type&&t.$content.hide().show(0),void n.fancybox.animate(r,"fancybox-slide--current",i,function(){r.removeClass(o).css({transform:"",opacity:""}),t.pos===s.currPos&&s.complete()},!0)):(t.$content.removeClass("fancybox-is-hidden"),u||!d||"image"!==t.type||t.hasError||t.$content.hide().fadeIn("fast"),void(t.pos===s.currPos&&s.complete())))},getThumbPos:function(t){var e,o,i,a,s,r=!1,c=t.$thumb;return!(!c||!g(c[0]))&&(e=n.fancybox.getTranslate(c),o=parseFloat(c.css("border-top-width")||0),i=parseFloat(c.css("border-right-width")||0),a=parseFloat(c.css("border-bottom-width")||0),s=parseFloat(c.css("border-left-width")||0),r={top:e.top+o,left:e.left+s,width:e.width-i-s,height:e.height-o-a,scaleX:1,scaleY:1},e.width>0&&e.height>0&&r)},complete:function(){var t,e=this,o=e.current,i={};!e.isMoved()&&o.isLoaded&&(o.isComplete||(o.isComplete=!0,o.$slide.siblings().trigger("onReset"),e.preload("inline"),p(o.$slide),o.$slide.addClass("fancybox-slide--complete"),n.each(e.slides,function(t,o){o.pos>=e.currPos-1&&o.pos<=e.currPos+1?i[o.pos]=o:o&&(n.fancybox.stop(o.$slide),o.$slide.off().remove())}),e.slides=i),e.isAnimating=!1,e.updateCursor(),e.trigger("afterShow"),o.opts.video.autoStart&&o.$slide.find("video,audio").filter(":visible:first").trigger("play").one("ended",function(){Document.exitFullscreen?Document.exitFullscreen():this.webkitExitFullscreen&&this.webkitExitFullscreen(),e.next()}),o.opts.autoFocus&&"html"===o.contentType&&(t=o.$content.find("input[autofocus]:enabled:visible:first"),t.length?t.trigger("focus"):e.focus(null,!0)),o.$slide.scrollTop(0).scrollLeft(0))},preload:function(t){var e,n,o=this;o.group.length<2||(n=o.slides[o.currPos+1],e=o.slides[o.currPos-1],e&&e.type===t&&o.loadSlide(e),n&&n.type===t&&o.loadSlide(n))},focus:function(t,o){var i,a,s=this,r=["a[href]","area[href]",'input:not([disabled]):not([type="hidden"]):not([aria-hidden])',"select:not([disabled]):not([aria-hidden])","textarea:not([disabled]):not([aria-hidden])","button:not([disabled]):not([aria-hidden])","iframe","object","embed","video","audio","[contenteditable]",'[tabindex]:not([tabindex^="-"])'].join(",");s.isClosing||(i=!t&&s.current&&s.current.isComplete?s.current.$slide.find("*:visible"+(o?":not(.fancybox-close-small)":"")):s.$refs.container.find("*:visible"),i=i.filter(r).filter(function(){return"hidden"!==n(this).css("visibility")&&!n(this).hasClass("disabled")}),i.length?(a=i.index(e.activeElement),t&&t.shiftKey?(a<0||0==a)&&(t.preventDefault(),i.eq(i.length-1).trigger("focus")):(a<0||a==i.length-1)&&(t&&t.preventDefault(),i.eq(0).trigger("focus"))):s.$refs.container.trigger("focus"))},activate:function(){var t=this;n(".fancybox-container").each(function(){var e=n(this).data("FancyBox");e&&e.id!==t.id&&!e.isClosing&&(e.trigger("onDeactivate"),e.removeEvents(),e.isVisible=!1)}),t.isVisible=!0,(t.current||t.isIdle)&&(t.update(),t.updateControls()),t.trigger("onActivate"),t.addEvents()},close:function(t,e){var o,i,a,s,r,c,l,u=this,f=u.current,h=function(){u.cleanUp(t)};return!u.isClosing&&(u.isClosing=!0,!1===u.trigger("beforeClose",t)?(u.isClosing=!1,d(function(){u.update()}),!1):(u.removeEvents(),a=f.$content,o=f.opts.animationEffect,i=n.isNumeric(e)?e:o?f.opts.animationDuration:0,f.$slide.removeClass("fancybox-slide--complete fancybox-slide--next fancybox-slide--previous fancybox-animated"),!0!==t?n.fancybox.stop(f.$slide):o=!1,f.$slide.siblings().trigger("onReset").remove(),i&&u.$refs.container.removeClass("fancybox-is-open").addClass("fancybox-is-closing").css("transition-duration",i+"ms"),u.hideLoading(f),u.hideControls(!0),u.updateCursor(),"zoom"!==o||a&&i&&"image"===f.type&&!u.isMoved()&&!f.hasError&&(l=u.getThumbPos(f))||(o="fade"),"zoom"===o?(n.fancybox.stop(a),s=n.fancybox.getTranslate(a),c={top:s.top,left:s.left,scaleX:s.width/l.width,scaleY:s.height/l.height,width:l.width,height:l.height},r=f.opts.zoomOpacity,
"auto"==r&&(r=Math.abs(f.width/f.height-l.width/l.height)>.1),r&&(l.opacity=0),n.fancybox.setTranslate(a,c),p(a),n.fancybox.animate(a,l,i,h),!0):(o&&i?n.fancybox.animate(f.$slide.addClass("fancybox-slide--previous").removeClass("fancybox-slide--current"),"fancybox-animated fancybox-fx-"+o,i,h):!0===t?setTimeout(h,i):h(),!0)))},cleanUp:function(e){var o,i,a,s=this,r=s.current.opts.$orig;s.current.$slide.trigger("onReset"),s.$refs.container.empty().remove(),s.trigger("afterClose",e),s.current.opts.backFocus&&(r&&r.length&&r.is(":visible")||(r=s.$trigger),r&&r.length&&(i=t.scrollX,a=t.scrollY,r.trigger("focus"),n("html, body").scrollTop(a).scrollLeft(i))),s.current=null,o=n.fancybox.getInstance(),o?o.activate():(n("body").removeClass("fancybox-active compensate-for-scrollbar"),n("#fancybox-style-noscroll").remove())},trigger:function(t,e){var o,i=Array.prototype.slice.call(arguments,1),a=this,s=e&&e.opts?e:a.current;if(s?i.unshift(s):s=a,i.unshift(a),n.isFunction(s.opts[t])&&(o=s.opts[t].apply(s,i)),!1===o)return o;"afterClose"!==t&&a.$refs?a.$refs.container.trigger(t+".fb",i):r.trigger(t+".fb",i)},updateControls:function(){var t=this,o=t.current,i=o.index,a=t.$refs.container,s=t.$refs.caption,r=o.opts.caption;o.$slide.trigger("refresh"),r&&r.length?(t.$caption=s,s.children().eq(0).html(r)):t.$caption=null,t.hasHiddenControls||t.isIdle||t.showControls(),a.find("[data-fancybox-count]").html(t.group.length),a.find("[data-fancybox-index]").html(i+1),a.find("[data-fancybox-prev]").prop("disabled",!o.opts.loop&&i<=0),a.find("[data-fancybox-next]").prop("disabled",!o.opts.loop&&i>=t.group.length-1),"image"===o.type?a.find("[data-fancybox-zoom]").show().end().find("[data-fancybox-download]").attr("href",o.opts.image.src||o.src).show():o.opts.toolbar&&a.find("[data-fancybox-download],[data-fancybox-zoom]").hide(),n(e.activeElement).is(":hidden,[disabled]")&&t.$refs.container.trigger("focus")},hideControls:function(t){var e=this,n=["infobar","toolbar","nav"];!t&&e.current.opts.preventCaptionOverlap||n.push("caption"),this.$refs.container.removeClass(n.map(function(t){return"fancybox-show-"+t}).join(" ")),this.hasHiddenControls=!0},showControls:function(){var t=this,e=t.current?t.current.opts:t.opts,n=t.$refs.container;t.hasHiddenControls=!1,t.idleSecondsCounter=0,n.toggleClass("fancybox-show-toolbar",!(!e.toolbar||!e.buttons)).toggleClass("fancybox-show-infobar",!!(e.infobar&&t.group.length>1)).toggleClass("fancybox-show-caption",!!t.$caption).toggleClass("fancybox-show-nav",!!(e.arrows&&t.group.length>1)).toggleClass("fancybox-is-modal",!!e.modal)},toggleControls:function(){this.hasHiddenControls?this.showControls():this.hideControls()}}),n.fancybox={version:"3.5.7",defaults:a,getInstance:function(t){var e=n('.fancybox-container:not(".fancybox-is-closing"):last').data("FancyBox"),o=Array.prototype.slice.call(arguments,1);return e instanceof b&&("string"===n.type(t)?e[t].apply(e,o):"function"===n.type(t)&&t.apply(e,o),e)},open:function(t,e,n){return new b(t,e,n)},close:function(t){var e=this.getInstance();e&&(e.close(),!0===t&&this.close(t))},destroy:function(){this.close(!0),r.add("body").off("click.fb-start","**")},isMobile:/Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent),use3d:function(){var n=e.createElement("div");return t.getComputedStyle&&t.getComputedStyle(n)&&t.getComputedStyle(n).getPropertyValue("transform")&&!(e.documentMode&&e.documentMode<11)}(),getTranslate:function(t){var e;return!(!t||!t.length)&&(e=t[0].getBoundingClientRect(),{top:e.top||0,left:e.left||0,width:e.width,height:e.height,opacity:parseFloat(t.css("opacity"))})},setTranslate:function(t,e){var n="",o={};if(t&&e)return void 0===e.left&&void 0===e.top||(n=(void 0===e.left?t.position().left:e.left)+"px, "+(void 0===e.top?t.position().top:e.top)+"px",n=this.use3d?"translate3d("+n+", 0px)":"translate("+n+")"),void 0!==e.scaleX&&void 0!==e.scaleY?n+=" scale("+e.scaleX+", "+e.scaleY+")":void 0!==e.scaleX&&(n+=" scaleX("+e.scaleX+")"),n.length&&(o.transform=n),void 0!==e.opacity&&(o.opacity=e.opacity),void 0!==e.width&&(o.width=e.width),void 0!==e.height&&(o.height=e.height),t.css(o)},animate:function(t,e,o,i,a){var s,r=this;n.isFunction(o)&&(i=o,o=null),r.stop(t),s=r.getTranslate(t),t.on(f,function(c){(!c||!c.originalEvent||t.is(c.originalEvent.target)&&"z-index"!=c.originalEvent.propertyName)&&(r.stop(t),n.isNumeric(o)&&t.css("transition-duration",""),n.isPlainObject(e)?void 0!==e.scaleX&&void 0!==e.scaleY&&r.setTranslate(t,{top:e.top,left:e.left,width:s.width*e.scaleX,height:s.height*e.scaleY,scaleX:1,scaleY:1}):!0!==a&&t.removeClass(e),n.isFunction(i)&&i(c))}),n.isNumeric(o)&&t.css("transition-duration",o+"ms"),n.isPlainObject(e)?(void 0!==e.scaleX&&void 0!==e.scaleY&&(delete e.width,delete e.height,t.parent().hasClass("fancybox-slide--image")&&t.parent().addClass("fancybox-is-scaling")),n.fancybox.setTranslate(t,e)):t.addClass(e),t.data("timer",setTimeout(function(){t.trigger(f)},o+33))},stop:function(t,e){t&&t.length&&(clearTimeout(t.data("timer")),e&&t.trigger(f),t.off(f).css("transition-duration",""),t.parent().removeClass("fancybox-is-scaling"))}},n.fn.fancybox=function(t){var e;return t=t||{},e=t.selector||!1,e?n("body").off("click.fb-start",e).on("click.fb-start",e,{options:t},i):this.off("click.fb-start").on("click.fb-start",{items:this,options:t},i),this},r.on("click.fb-start","[data-fancybox]",i),r.on("click.fb-start","[data-fancybox-trigger]",function(t){n('[data-fancybox="'+n(this).attr("data-fancybox-trigger")+'"]').eq(n(this).attr("data-fancybox-index")||0).trigger("click.fb-start",{$trigger:n(this)})}),function(){var t=null;r.on("mousedown mouseup focus blur",".fancybox-button",function(e){switch(e.type){case"mousedown":t=n(this);break;case"mouseup":t=null;break;case"focusin":n(".fancybox-button").removeClass("fancybox-focus"),n(this).is(t)||n(this).is("[disabled]")||n(this).addClass("fancybox-focus");break;case"focusout":n(".fancybox-button").removeClass("fancybox-focus")}})}()}}(window,document,jQuery),function(t){"use strict";var e={youtube:{matcher:/(youtube\.com|youtu\.be|youtube\-nocookie\.com)\/(watch\?(.*&)?v=|v\/|u\/|embed\/?)?(videoseries\?list=(.*)|[\w-]{11}|\?listType=(.*)&list=(.*))(.*)/i,params:{autoplay:1,autohide:1,fs:1,rel:0,hd:1,wmode:"transparent",enablejsapi:1,html5:1},paramPlace:8,type:"iframe",url:"https://www.youtube-nocookie.com/embed/$4",thumb:"https://img.youtube.com/vi/$4/hqdefault.jpg"},vimeo:{matcher:/^.+vimeo.com\/(.*\/)?([\d]+)(.*)?/,params:{autoplay:1,hd:1,show_title:1,show_byline:1,show_portrait:0,fullscreen:1},paramPlace:3,type:"iframe",url:"//player.vimeo.com/video/$2"},instagram:{matcher:/(instagr\.am|instagram\.com)\/p\/([a-zA-Z0-9_\-]+)\/?/i,type:"image",url:"//$1/p/$2/media/?size=l"},gmap_place:{matcher:/(maps\.)?google\.([a-z]{2,3}(\.[a-z]{2})?)\/(((maps\/(place\/(.*)\/)?\@(.*),(\d+.?\d+?)z))|(\?ll=))(.*)?/i,type:"iframe",url:function(t){return"//maps.google."+t[2]+"/?ll="+(t[9]?t[9]+"&z="+Math.floor(t[10])+(t[12]?t[12].replace(/^\//,"&"):""):t[12]+"").replace(/\?/,"&")+"&output="+(t[12]&&t[12].indexOf("layer=c")>0?"svembed":"embed")}},gmap_search:{matcher:/(maps\.)?google\.([a-z]{2,3}(\.[a-z]{2})?)\/(maps\/search\/)(.*)/i,type:"iframe",url:function(t){return"//maps.google."+t[2]+"/maps?q="+t[5].replace("query=","q=").replace("api=1","")+"&output=embed"}}},n=function(e,n,o){if(e)return o=o||"","object"===t.type(o)&&(o=t.param(o,!0)),t.each(n,function(t,n){e=e.replace("$"+t,n||"")}),o.length&&(e+=(e.indexOf("?")>0?"&":"?")+o),e};t(document).on("objectNeedsType.fb",function(o,i,a){var s,r,c,l,d,u,f,p=a.src||"",h=!1;s=t.extend(!0,{},e,a.opts.media),t.each(s,function(e,o){if(c=p.match(o.matcher)){if(h=o.type,f=e,u={},o.paramPlace&&c[o.paramPlace]){d=c[o.paramPlace],"?"==d[0]&&(d=d.substring(1)),d=d.split("&");for(var i=0;i<d.length;++i){var s=d[i].split("=",2);2==s.length&&(u[s[0]]=decodeURIComponent(s[1].replace(/\+/g," ")))}}return l=t.extend(!0,{},o.params,a.opts[e],u),p="function"===t.type(o.url)?o.url.call(this,c,l,a):n(o.url,c,l),r="function"===t.type(o.thumb)?o.thumb.call(this,c,l,a):n(o.thumb,c),"youtube"===e?p=p.replace(/&t=((\d+)m)?(\d+)s/,function(t,e,n,o){return"&start="+((n?60*parseInt(n,10):0)+parseInt(o,10))}):"vimeo"===e&&(p=p.replace("&%23","#")),!1}}),h?(a.opts.thumb||a.opts.$thumb&&a.opts.$thumb.length||(a.opts.thumb=r),"iframe"===h&&(a.opts=t.extend(!0,a.opts,{iframe:{preload:!1,attr:{scrolling:"no"}}})),t.extend(a,{type:h,src:p,origSrc:a.src,contentSource:f,contentType:"image"===h?"image":"gmap_place"==f||"gmap_search"==f?"map":"video"})):p&&(a.type=a.opts.defaultType)});var o={youtube:{src:"https://www.youtube.com/iframe_api",class:"YT",loading:!1,loaded:!1},vimeo:{src:"https://player.vimeo.com/api/player.js",class:"Vimeo",loading:!1,loaded:!1},load:function(t){var e,n=this;if(this[t].loaded)return void setTimeout(function(){n.done(t)});this[t].loading||(this[t].loading=!0,e=document.createElement("script"),e.type="text/javascript",e.src=this[t].src,"youtube"===t?window.onYouTubeIframeAPIReady=function(){n[t].loaded=!0,n.done(t)}:e.onload=function(){n[t].loaded=!0,n.done(t)},document.body.appendChild(e))},done:function(e){var n,o,i;"youtube"===e&&delete window.onYouTubeIframeAPIReady,(n=t.fancybox.getInstance())&&(o=n.current.$content.find("iframe"),"youtube"===e&&void 0!==YT&&YT?i=new YT.Player(o.attr("id"),{events:{onStateChange:function(t){0==t.data&&n.next()}}}):"vimeo"===e&&void 0!==Vimeo&&Vimeo&&(i=new Vimeo.Player(o),i.on("ended",function(){n.next()})))}};t(document).on({"afterShow.fb":function(t,e,n){e.group.length>1&&("youtube"===n.contentSource||"vimeo"===n.contentSource)&&o.load(n.contentSource)}})}(jQuery),function(t,e,n){"use strict";var o=function(){return t.requestAnimationFrame||t.webkitRequestAnimationFrame||t.mozRequestAnimationFrame||t.oRequestAnimationFrame||function(e){return t.setTimeout(e,1e3/60)}}(),i=function(){return t.cancelAnimationFrame||t.webkitCancelAnimationFrame||t.mozCancelAnimationFrame||t.oCancelAnimationFrame||function(e){t.clearTimeout(e)}}(),a=function(e){var n=[];e=e.originalEvent||e||t.e,e=e.touches&&e.touches.length?e.touches:e.changedTouches&&e.changedTouches.length?e.changedTouches:[e];for(var o in e)e[o].pageX?n.push({x:e[o].pageX,y:e[o].pageY}):e[o].clientX&&n.push({x:e[o].clientX,y:e[o].clientY});return n},s=function(t,e,n){return e&&t?"x"===n?t.x-e.x:"y"===n?t.y-e.y:Math.sqrt(Math.pow(t.x-e.x,2)+Math.pow(t.y-e.y,2)):0},r=function(t){if(t.is('a,area,button,[role="button"],input,label,select,summary,textarea,video,audio,iframe')||n.isFunction(t.get(0).onclick)||t.data("selectable"))return!0;for(var e=0,o=t[0].attributes,i=o.length;e<i;e++)if("data-fancybox-"===o[e].nodeName.substr(0,14))return!0;return!1},c=function(e){var n=t.getComputedStyle(e)["overflow-y"],o=t.getComputedStyle(e)["overflow-x"],i=("scroll"===n||"auto"===n)&&e.scrollHeight>e.clientHeight,a=("scroll"===o||"auto"===o)&&e.scrollWidth>e.clientWidth;return i||a},l=function(t){for(var e=!1;;){if(e=c(t.get(0)))break;if(t=t.parent(),!t.length||t.hasClass("fancybox-stage")||t.is("body"))break}return e},d=function(t){var e=this;e.instance=t,e.$bg=t.$refs.bg,e.$stage=t.$refs.stage,e.$container=t.$refs.container,e.destroy(),e.$container.on("touchstart.fb.touch mousedown.fb.touch",n.proxy(e,"ontouchstart"))};d.prototype.destroy=function(){var t=this;t.$container.off(".fb.touch"),n(e).off(".fb.touch"),t.requestId&&(i(t.requestId),t.requestId=null),t.tapped&&(clearTimeout(t.tapped),t.tapped=null)},d.prototype.ontouchstart=function(o){var i=this,c=n(o.target),d=i.instance,u=d.current,f=u.$slide,p=u.$content,h="touchstart"==o.type;if(h&&i.$container.off("mousedown.fb.touch"),(!o.originalEvent||2!=o.originalEvent.button)&&f.length&&c.length&&!r(c)&&!r(c.parent())&&(c.is("img")||!(o.originalEvent.clientX>c[0].clientWidth+c.offset().left))){if(!u||d.isAnimating||u.$slide.hasClass("fancybox-animated"))return o.stopPropagation(),void o.preventDefault();i.realPoints=i.startPoints=a(o),i.startPoints.length&&(u.touch&&o.stopPropagation(),i.startEvent=o,i.canTap=!0,i.$target=c,i.$content=p,i.opts=u.opts.touch,i.isPanning=!1,i.isSwiping=!1,i.isZooming=!1,i.isScrolling=!1,i.canPan=d.canPan(),i.startTime=(new Date).getTime(),i.distanceX=i.distanceY=i.distance=0,i.canvasWidth=Math.round(f[0].clientWidth),i.canvasHeight=Math.round(f[0].clientHeight),i.contentLastPos=null,i.contentStartPos=n.fancybox.getTranslate(i.$content)||{top:0,left:0},i.sliderStartPos=n.fancybox.getTranslate(f),i.stagePos=n.fancybox.getTranslate(d.$refs.stage),i.sliderStartPos.top-=i.stagePos.top,i.sliderStartPos.left-=i.stagePos.left,i.contentStartPos.top-=i.stagePos.top,i.contentStartPos.left-=i.stagePos.left,n(e).off(".fb.touch").on(h?"touchend.fb.touch touchcancel.fb.touch":"mouseup.fb.touch mouseleave.fb.touch",n.proxy(i,"ontouchend")).on(h?"touchmove.fb.touch":"mousemove.fb.touch",n.proxy(i,"ontouchmove")),n.fancybox.isMobile&&e.addEventListener("scroll",i.onscroll,!0),((i.opts||i.canPan)&&(c.is(i.$stage)||i.$stage.find(c).length)||(c.is(".fancybox-image")&&o.preventDefault(),n.fancybox.isMobile&&c.parents(".fancybox-caption").length))&&(i.isScrollable=l(c)||l(c.parent()),n.fancybox.isMobile&&i.isScrollable||o.preventDefault(),(1===i.startPoints.length||u.hasError)&&(i.canPan?(n.fancybox.stop(i.$content),i.isPanning=!0):i.isSwiping=!0,i.$container.addClass("fancybox-is-grabbing")),2===i.startPoints.length&&"image"===u.type&&(u.isLoaded||u.$ghost)&&(i.canTap=!1,i.isSwiping=!1,i.isPanning=!1,i.isZooming=!0,n.fancybox.stop(i.$content),i.centerPointStartX=.5*(i.startPoints[0].x+i.startPoints[1].x)-n(t).scrollLeft(),i.centerPointStartY=.5*(i.startPoints[0].y+i.startPoints[1].y)-n(t).scrollTop(),i.percentageOfImageAtPinchPointX=(i.centerPointStartX-i.contentStartPos.left)/i.contentStartPos.width,i.percentageOfImageAtPinchPointY=(i.centerPointStartY-i.contentStartPos.top)/i.contentStartPos.height,i.startDistanceBetweenFingers=s(i.startPoints[0],i.startPoints[1]))))}},d.prototype.onscroll=function(t){var n=this;n.isScrolling=!0,e.removeEventListener("scroll",n.onscroll,!0)},d.prototype.ontouchmove=function(t){var e=this;return void 0!==t.originalEvent.buttons&&0===t.originalEvent.buttons?void e.ontouchend(t):e.isScrolling?void(e.canTap=!1):(e.newPoints=a(t),void((e.opts||e.canPan)&&e.newPoints.length&&e.newPoints.length&&(e.isSwiping&&!0===e.isSwiping||t.preventDefault(),e.distanceX=s(e.newPoints[0],e.startPoints[0],"x"),e.distanceY=s(e.newPoints[0],e.startPoints[0],"y"),e.distance=s(e.newPoints[0],e.startPoints[0]),e.distance>0&&(e.isSwiping?e.onSwipe(t):e.isPanning?e.onPan():e.isZooming&&e.onZoom()))))},d.prototype.onSwipe=function(e){var a,s=this,r=s.instance,c=s.isSwiping,l=s.sliderStartPos.left||0;if(!0!==c)"x"==c&&(s.distanceX>0&&(s.instance.group.length<2||0===s.instance.current.index&&!s.instance.current.opts.loop)?l+=Math.pow(s.distanceX,.8):s.distanceX<0&&(s.instance.group.length<2||s.instance.current.index===s.instance.group.length-1&&!s.instance.current.opts.loop)?l-=Math.pow(-s.distanceX,.8):l+=s.distanceX),s.sliderLastPos={top:"x"==c?0:s.sliderStartPos.top+s.distanceY,left:l},s.requestId&&(i(s.requestId),s.requestId=null),s.requestId=o(function(){s.sliderLastPos&&(n.each(s.instance.slides,function(t,e){var o=e.pos-s.instance.currPos;n.fancybox.setTranslate(e.$slide,{top:s.sliderLastPos.top,left:s.sliderLastPos.left+o*s.canvasWidth+o*e.opts.gutter})}),s.$container.addClass("fancybox-is-sliding"))});else if(Math.abs(s.distance)>10){if(s.canTap=!1,r.group.length<2&&s.opts.vertical?s.isSwiping="y":r.isDragging||!1===s.opts.vertical||"auto"===s.opts.vertical&&n(t).width()>800?s.isSwiping="x":(a=Math.abs(180*Math.atan2(s.distanceY,s.distanceX)/Math.PI),s.isSwiping=a>45&&a<135?"y":"x"),"y"===s.isSwiping&&n.fancybox.isMobile&&s.isScrollable)return void(s.isScrolling=!0);r.isDragging=s.isSwiping,s.startPoints=s.newPoints,n.each(r.slides,function(t,e){var o,i;n.fancybox.stop(e.$slide),o=n.fancybox.getTranslate(e.$slide),i=n.fancybox.getTranslate(r.$refs.stage),e.$slide.css({transform:"",opacity:"","transition-duration":""}).removeClass("fancybox-animated").removeClass(function(t,e){return(e.match(/(^|\s)fancybox-fx-\S+/g)||[]).join(" ")}),e.pos===r.current.pos&&(s.sliderStartPos.top=o.top-i.top,s.sliderStartPos.left=o.left-i.left),n.fancybox.setTranslate(e.$slide,{top:o.top-i.top,left:o.left-i.left})}),r.SlideShow&&r.SlideShow.isActive&&r.SlideShow.stop()}},d.prototype.onPan=function(){var t=this;if(s(t.newPoints[0],t.realPoints[0])<(n.fancybox.isMobile?10:5))return void(t.startPoints=t.newPoints);t.canTap=!1,t.contentLastPos=t.limitMovement(),t.requestId&&i(t.requestId),t.requestId=o(function(){n.fancybox.setTranslate(t.$content,t.contentLastPos)})},d.prototype.limitMovement=function(){var t,e,n,o,i,a,s=this,r=s.canvasWidth,c=s.canvasHeight,l=s.distanceX,d=s.distanceY,u=s.contentStartPos,f=u.left,p=u.top,h=u.width,g=u.height;return i=h>r?f+l:f,a=p+d,t=Math.max(0,.5*r-.5*h),e=Math.max(0,.5*c-.5*g),n=Math.min(r-h,.5*r-.5*h),o=Math.min(c-g,.5*c-.5*g),l>0&&i>t&&(i=t-1+Math.pow(-t+f+l,.8)||0),l<0&&i<n&&(i=n+1-Math.pow(n-f-l,.8)||0),d>0&&a>e&&(a=e-1+Math.pow(-e+p+d,.8)||0),d<0&&a<o&&(a=o+1-Math.pow(o-p-d,.8)||0),{top:a,left:i}},d.prototype.limitPosition=function(t,e,n,o){var i=this,a=i.canvasWidth,s=i.canvasHeight;return n>a?(t=t>0?0:t,t=t<a-n?a-n:t):t=Math.max(0,a/2-n/2),o>s?(e=e>0?0:e,e=e<s-o?s-o:e):e=Math.max(0,s/2-o/2),{top:e,left:t}},d.prototype.onZoom=function(){var e=this,a=e.contentStartPos,r=a.width,c=a.height,l=a.left,d=a.top,u=s(e.newPoints[0],e.newPoints[1]),f=u/e.startDistanceBetweenFingers,p=Math.floor(r*f),h=Math.floor(c*f),g=(r-p)*e.percentageOfImageAtPinchPointX,b=(c-h)*e.percentageOfImageAtPinchPointY,m=(e.newPoints[0].x+e.newPoints[1].x)/2-n(t).scrollLeft(),v=(e.newPoints[0].y+e.newPoints[1].y)/2-n(t).scrollTop(),y=m-e.centerPointStartX,x=v-e.centerPointStartY,w=l+(g+y),$=d+(b+x),S={top:$,left:w,scaleX:f,scaleY:f};e.canTap=!1,e.newWidth=p,e.newHeight=h,e.contentLastPos=S,e.requestId&&i(e.requestId),e.requestId=o(function(){n.fancybox.setTranslate(e.$content,e.contentLastPos)})},d.prototype.ontouchend=function(t){var o=this,s=o.isSwiping,r=o.isPanning,c=o.isZooming,l=o.isScrolling;if(o.endPoints=a(t),o.dMs=Math.max((new Date).getTime()-o.startTime,1),o.$container.removeClass("fancybox-is-grabbing"),n(e).off(".fb.touch"),e.removeEventListener("scroll",o.onscroll,!0),o.requestId&&(i(o.requestId),o.requestId=null),o.isSwiping=!1,o.isPanning=!1,o.isZooming=!1,o.isScrolling=!1,o.instance.isDragging=!1,o.canTap)return o.onTap(t);o.speed=100,o.velocityX=o.distanceX/o.dMs*.5,o.velocityY=o.distanceY/o.dMs*.5,r?o.endPanning():c?o.endZooming():o.endSwiping(s,l)},d.prototype.endSwiping=function(t,e){var o=this,i=!1,a=o.instance.group.length,s=Math.abs(o.distanceX),r="x"==t&&a>1&&(o.dMs>130&&s>10||s>50);o.sliderLastPos=null,"y"==t&&!e&&Math.abs(o.distanceY)>50?(n.fancybox.animate(o.instance.current.$slide,{top:o.sliderStartPos.top+o.distanceY+150*o.velocityY,opacity:0},200),i=o.instance.close(!0,250)):r&&o.distanceX>0?i=o.instance.previous(300):r&&o.distanceX<0&&(i=o.instance.next(300)),!1!==i||"x"!=t&&"y"!=t||o.instance.centerSlide(200),o.$container.removeClass("fancybox-is-sliding")},d.prototype.endPanning=function(){var t,e,o,i=this;i.contentLastPos&&(!1===i.opts.momentum||i.dMs>350?(t=i.contentLastPos.left,e=i.contentLastPos.top):(t=i.contentLastPos.left+500*i.velocityX,e=i.contentLastPos.top+500*i.velocityY),o=i.limitPosition(t,e,i.contentStartPos.width,i.contentStartPos.height),o.width=i.contentStartPos.width,o.height=i.contentStartPos.height,n.fancybox.animate(i.$content,o,366))},d.prototype.endZooming=function(){var t,e,o,i,a=this,s=a.instance.current,r=a.newWidth,c=a.newHeight;a.contentLastPos&&(t=a.contentLastPos.left,e=a.contentLastPos.top,i={top:e,left:t,width:r,height:c,scaleX:1,scaleY:1},n.fancybox.setTranslate(a.$content,i),r<a.canvasWidth&&c<a.canvasHeight?a.instance.scaleToFit(150):r>s.width||c>s.height?a.instance.scaleToActual(a.centerPointStartX,a.centerPointStartY,150):(o=a.limitPosition(t,e,r,c),n.fancybox.animate(a.$content,o,150)))},d.prototype.onTap=function(e){var o,i=this,s=n(e.target),r=i.instance,c=r.current,l=e&&a(e)||i.startPoints,d=l[0]?l[0].x-n(t).scrollLeft()-i.stagePos.left:0,u=l[0]?l[0].y-n(t).scrollTop()-i.stagePos.top:0,f=function(t){var o=c.opts[t];if(n.isFunction(o)&&(o=o.apply(r,[c,e])),o)switch(o){case"close":r.close(i.startEvent);break;case"toggleControls":r.toggleControls();break;case"next":r.next();break;case"nextOrClose":r.group.length>1?r.next():r.close(i.startEvent);break;case"zoom":"image"==c.type&&(c.isLoaded||c.$ghost)&&(r.canPan()?r.scaleToFit():r.isScaledDown()?r.scaleToActual(d,u):r.group.length<2&&r.close(i.startEvent))}};if((!e.originalEvent||2!=e.originalEvent.button)&&(s.is("img")||!(d>s[0].clientWidth+s.offset().left))){if(s.is(".fancybox-bg,.fancybox-inner,.fancybox-outer,.fancybox-container"))o="Outside";else if(s.is(".fancybox-slide"))o="Slide";else{if(!r.current.$content||!r.current.$content.find(s).addBack().filter(s).length)return;o="Content"}if(i.tapped){if(clearTimeout(i.tapped),i.tapped=null,Math.abs(d-i.tapX)>50||Math.abs(u-i.tapY)>50)return this;f("dblclick"+o)}else i.tapX=d,i.tapY=u,c.opts["dblclick"+o]&&c.opts["dblclick"+o]!==c.opts["click"+o]?i.tapped=setTimeout(function(){i.tapped=null,r.isAnimating||f("click"+o)},500):f("click"+o);return this}},n(e).on("onActivate.fb",function(t,e){e&&!e.Guestures&&(e.Guestures=new d(e))}).on("beforeClose.fb",function(t,e){e&&e.Guestures&&e.Guestures.destroy()})}(window,document,jQuery),function(t,e){"use strict";e.extend(!0,e.fancybox.defaults,{btnTpl:{slideShow:'<button data-fancybox-play class="fancybox-button fancybox-button--play" title="{{PLAY_START}}"><svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path d="M6.5 5.4v13.2l11-6.6z"/></svg><svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path d="M8.33 5.75h2.2v12.5h-2.2V5.75zm5.15 0h2.2v12.5h-2.2V5.75z"/></svg></button>'},slideShow:{autoStart:!1,speed:3e3,progress:!0}});var n=function(t){this.instance=t,this.init()};e.extend(n.prototype,{timer:null,isActive:!1,$button:null,init:function(){var t=this,n=t.instance,o=n.group[n.currIndex].opts.slideShow;t.$button=n.$refs.toolbar.find("[data-fancybox-play]").on("click",function(){t.toggle()}),n.group.length<2||!o?t.$button.hide():o.progress&&(t.$progress=e('<div class="fancybox-progress"></div>').appendTo(n.$refs.inner))},set:function(t){var n=this,o=n.instance,i=o.current;i&&(!0===t||i.opts.loop||o.currIndex<o.group.length-1)?n.isActive&&"video"!==i.contentType&&(n.$progress&&e.fancybox.animate(n.$progress.show(),{scaleX:1},i.opts.slideShow.speed),n.timer=setTimeout(function(){o.current.opts.loop||o.current.index!=o.group.length-1?o.next():o.jumpTo(0)},i.opts.slideShow.speed)):(n.stop(),o.idleSecondsCounter=0,o.showControls())},clear:function(){var t=this;clearTimeout(t.timer),t.timer=null,t.$progress&&t.$progress.removeAttr("style").hide()},start:function(){var t=this,e=t.instance.current;e&&(t.$button.attr("title",(e.opts.i18n[e.opts.lang]||e.opts.i18n.en).PLAY_STOP).removeClass("fancybox-button--play").addClass("fancybox-button--pause"),t.isActive=!0,e.isComplete&&t.set(!0),t.instance.trigger("onSlideShowChange",!0))},stop:function(){var t=this,e=t.instance.current;t.clear(),t.$button.attr("title",(e.opts.i18n[e.opts.lang]||e.opts.i18n.en).PLAY_START).removeClass("fancybox-button--pause").addClass("fancybox-button--play"),t.isActive=!1,t.instance.trigger("onSlideShowChange",!1),t.$progress&&t.$progress.removeAttr("style").hide()},toggle:function(){var t=this;t.isActive?t.stop():t.start()}}),e(t).on({"onInit.fb":function(t,e){e&&!e.SlideShow&&(e.SlideShow=new n(e))},"beforeShow.fb":function(t,e,n,o){var i=e&&e.SlideShow;o?i&&n.opts.slideShow.autoStart&&i.start():i&&i.isActive&&i.clear()},"afterShow.fb":function(t,e,n){var o=e&&e.SlideShow;o&&o.isActive&&o.set()},"afterKeydown.fb":function(n,o,i,a,s){var r=o&&o.SlideShow;!r||!i.opts.slideShow||80!==s&&32!==s||e(t.activeElement).is("button,a,input")||(a.preventDefault(),r.toggle())},"beforeClose.fb onDeactivate.fb":function(t,e){var n=e&&e.SlideShow;n&&n.stop()}}),e(t).on("visibilitychange",function(){var n=e.fancybox.getInstance(),o=n&&n.SlideShow;o&&o.isActive&&(t.hidden?o.clear():o.set())})}(document,jQuery),function(t,e){"use strict";var n=function(){for(var e=[["requestFullscreen","exitFullscreen","fullscreenElement","fullscreenEnabled","fullscreenchange","fullscreenerror"],["webkitRequestFullscreen","webkitExitFullscreen","webkitFullscreenElement","webkitFullscreenEnabled","webkitfullscreenchange","webkitfullscreenerror"],["webkitRequestFullScreen","webkitCancelFullScreen","webkitCurrentFullScreenElement","webkitCancelFullScreen","webkitfullscreenchange","webkitfullscreenerror"],["mozRequestFullScreen","mozCancelFullScreen","mozFullScreenElement","mozFullScreenEnabled","mozfullscreenchange","mozfullscreenerror"],["msRequestFullscreen","msExitFullscreen","msFullscreenElement","msFullscreenEnabled","MSFullscreenChange","MSFullscreenError"]],n={},o=0;o<e.length;o++){var i=e[o];if(i&&i[1]in t){for(var a=0;a<i.length;a++)n[e[0][a]]=i[a];return n}}return!1}();if(n){var o={request:function(e){e=e||t.documentElement,e[n.requestFullscreen](e.ALLOW_KEYBOARD_INPUT)},exit:function(){t[n.exitFullscreen]()},toggle:function(e){e=e||t.documentElement,this.isFullscreen()?this.exit():this.request(e)},isFullscreen:function(){return Boolean(t[n.fullscreenElement])},enabled:function(){return Boolean(t[n.fullscreenEnabled])}};e.extend(!0,e.fancybox.defaults,{btnTpl:{fullScreen:'<button data-fancybox-fullscreen class="fancybox-button fancybox-button--fsenter" title="{{FULL_SCREEN}}"><svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path d="M7 14H5v5h5v-2H7v-3zm-2-4h2V7h3V5H5v5zm12 7h-3v2h5v-5h-2v3zM14 5v2h3v3h2V5h-5z"/></svg><svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path d="M5 16h3v3h2v-5H5zm3-8H5v2h5V5H8zm6 11h2v-3h3v-2h-5zm2-11V5h-2v5h5V8z"/></svg></button>'},fullScreen:{autoStart:!1}}),e(t).on(n.fullscreenchange,function(){var t=o.isFullscreen(),n=e.fancybox.getInstance();n&&(n.current&&"image"===n.current.type&&n.isAnimating&&(n.isAnimating=!1,n.update(!0,!0,0),n.isComplete||n.complete()),n.trigger("onFullscreenChange",t),n.$refs.container.toggleClass("fancybox-is-fullscreen",t),n.$refs.toolbar.find("[data-fancybox-fullscreen]").toggleClass("fancybox-button--fsenter",!t).toggleClass("fancybox-button--fsexit",t))})}e(t).on({"onInit.fb":function(t,e){var i;if(!n)return void e.$refs.toolbar.find("[data-fancybox-fullscreen]").remove();e&&e.group[e.currIndex].opts.fullScreen?(i=e.$refs.container,i.on("click.fb-fullscreen","[data-fancybox-fullscreen]",function(t){t.stopPropagation(),t.preventDefault(),o.toggle()}),e.opts.fullScreen&&!0===e.opts.fullScreen.autoStart&&o.request(),e.FullScreen=o):e&&e.$refs.toolbar.find("[data-fancybox-fullscreen]").hide()},"afterKeydown.fb":function(t,e,n,o,i){e&&e.FullScreen&&70===i&&(o.preventDefault(),e.FullScreen.toggle())},"beforeClose.fb":function(t,e){e&&e.FullScreen&&e.$refs.container.hasClass("fancybox-is-fullscreen")&&o.exit()}})}(document,jQuery),function(t,e){"use strict";var n="fancybox-thumbs";e.fancybox.defaults=e.extend(!0,{btnTpl:{thumbs:'<button data-fancybox-thumbs class="fancybox-button fancybox-button--thumbs" title="{{THUMBS}}"><svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path d="M14.59 14.59h3.76v3.76h-3.76v-3.76zm-4.47 0h3.76v3.76h-3.76v-3.76zm-4.47 0h3.76v3.76H5.65v-3.76zm8.94-4.47h3.76v3.76h-3.76v-3.76zm-4.47 0h3.76v3.76h-3.76v-3.76zm-4.47 0h3.76v3.76H5.65v-3.76zm8.94-4.47h3.76v3.76h-3.76V5.65zm-4.47 0h3.76v3.76h-3.76V5.65zm-4.47 0h3.76v3.76H5.65V5.65z"/></svg></button>'},thumbs:{autoStart:!1,hideOnClose:!0,parentEl:".fancybox-container",axis:"y"}},e.fancybox.defaults);var o=function(t){this.init(t)};e.extend(o.prototype,{$button:null,$grid:null,$list:null,isVisible:!1,isActive:!1,init:function(t){var e=this,n=t.group,o=0;e.instance=t,e.opts=n[t.currIndex].opts.thumbs,t.Thumbs=e,e.$button=t.$refs.toolbar.find("[data-fancybox-thumbs]");for(var i=0,a=n.length;i<a&&(n[i].thumb&&o++,!(o>1));i++);o>1&&e.opts?(e.$button.removeAttr("style").on("click",function(){e.toggle()}),e.isActive=!0):e.$button.hide()},create:function(){var t,o=this,i=o.instance,a=o.opts.parentEl,s=[];o.$grid||(o.$grid=e('<div class="'+n+" "+n+"-"+o.opts.axis+'"></div>').appendTo(i.$refs.container.find(a).addBack().filter(a)),o.$grid.on("click","a",function(){i.jumpTo(e(this).attr("data-index"))})),o.$list||(o.$list=e('<div class="'+n+'__list">').appendTo(o.$grid)),e.each(i.group,function(e,n){t=n.thumb,t||"image"!==n.type||(t=n.src),s.push('<a href="javascript:;" tabindex="0" data-index="'+e+'"'+(t&&t.length?' style="background-image:url('+t+')"':'class="fancybox-thumbs-missing"')+"></a>")}),o.$list[0].innerHTML=s.join(""),"x"===o.opts.axis&&o.$list.width(parseInt(o.$grid.css("padding-right"),10)+i.group.length*o.$list.children().eq(0).outerWidth(!0))},focus:function(t){var e,n,o=this,i=o.$list,a=o.$grid;o.instance.current&&(e=i.children().removeClass("fancybox-thumbs-active").filter('[data-index="'+o.instance.current.index+'"]').addClass("fancybox-thumbs-active"),n=e.position(),"y"===o.opts.axis&&(n.top<0||n.top>i.height()-e.outerHeight())?i.stop().animate({scrollTop:i.scrollTop()+n.top},t):"x"===o.opts.axis&&(n.left<a.scrollLeft()||n.left>a.scrollLeft()+(a.width()-e.outerWidth()))&&i.parent().stop().animate({scrollLeft:n.left},t))},update:function(){var t=this;t.instance.$refs.container.toggleClass("fancybox-show-thumbs",this.isVisible),t.isVisible?(t.$grid||t.create(),t.instance.trigger("onThumbsShow"),t.focus(0)):t.$grid&&t.instance.trigger("onThumbsHide"),t.instance.update()},hide:function(){this.isVisible=!1,this.update()},show:function(){this.isVisible=!0,this.update()},toggle:function(){this.isVisible=!this.isVisible,this.update()}}),e(t).on({"onInit.fb":function(t,e){var n;e&&!e.Thumbs&&(n=new o(e),n.isActive&&!0===n.opts.autoStart&&n.show())},"beforeShow.fb":function(t,e,n,o){var i=e&&e.Thumbs;i&&i.isVisible&&i.focus(o?0:250)},"afterKeydown.fb":function(t,e,n,o,i){var a=e&&e.Thumbs;a&&a.isActive&&71===i&&(o.preventDefault(),a.toggle())},"beforeClose.fb":function(t,e){var n=e&&e.Thumbs;n&&n.isVisible&&!1!==n.opts.hideOnClose&&n.$grid.hide()}})}(document,jQuery),function(t,e){"use strict";function n(t){var e={"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;","/":"&#x2F;","`":"&#x60;","=":"&#x3D;"};return String(t).replace(/[&<>"'`=\/]/g,function(t){return e[t]})}e.extend(!0,e.fancybox.defaults,{btnTpl:{share:'<button data-fancybox-share class="fancybox-button fancybox-button--share" title="{{SHARE}}"><svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path d="M2.55 19c1.4-8.4 9.1-9.8 11.9-9.8V5l7 7-7 6.3v-3.5c-2.8 0-10.5 2.1-11.9 4.2z"/></svg></button>'},share:{url:function(t,e){return!t.currentHash&&"inline"!==e.type&&"html"!==e.type&&(e.origSrc||e.src)||window.location},
tpl:'<div class="fancybox-share"><h1>{{SHARE}}</h1><p><a class="fancybox-share__button fancybox-share__button--fb" href="https://www.facebook.com/sharer/sharer.php?u={{url}}"><svg viewBox="0 0 512 512" xmlns="http://www.w3.org/2000/svg"><path d="m287 456v-299c0-21 6-35 35-35h38v-63c-7-1-29-3-55-3-54 0-91 33-91 94v306m143-254h-205v72h196" /></svg><span>Facebook</span></a><a class="fancybox-share__button fancybox-share__button--tw" href="https://twitter.com/intent/tweet?url={{url}}&text={{descr}}"><svg viewBox="0 0 512 512" xmlns="http://www.w3.org/2000/svg"><path d="m456 133c-14 7-31 11-47 13 17-10 30-27 37-46-15 10-34 16-52 20-61-62-157-7-141 75-68-3-129-35-169-85-22 37-11 86 26 109-13 0-26-4-37-9 0 39 28 72 65 80-12 3-25 4-37 2 10 33 41 57 77 57-42 30-77 38-122 34 170 111 378-32 359-208 16-11 30-25 41-42z" /></svg><span>Twitter</span></a><a class="fancybox-share__button fancybox-share__button--pt" href="https://www.pinterest.com/pin/create/button/?url={{url}}&description={{descr}}&media={{media}}"><svg viewBox="0 0 512 512" xmlns="http://www.w3.org/2000/svg"><path d="m265 56c-109 0-164 78-164 144 0 39 15 74 47 87 5 2 10 0 12-5l4-19c2-6 1-8-3-13-9-11-15-25-15-45 0-58 43-110 113-110 62 0 96 38 96 88 0 67-30 122-73 122-24 0-42-19-36-44 6-29 20-60 20-81 0-19-10-35-31-35-25 0-44 26-44 60 0 21 7 36 7 36l-30 125c-8 37-1 83 0 87 0 3 4 4 5 2 2-3 32-39 42-75l16-64c8 16 31 29 56 29 74 0 124-67 124-157 0-69-58-132-146-132z" fill="#fff"/></svg><span>Pinterest</span></a></p><p><input class="fancybox-share__input" type="text" value="{{url_raw}}" onclick="select()" /></p></div>'}}),e(t).on("click","[data-fancybox-share]",function(){var t,o,i=e.fancybox.getInstance(),a=i.current||null;a&&("function"===e.type(a.opts.share.url)&&(t=a.opts.share.url.apply(a,[i,a])),o=a.opts.share.tpl.replace(/\{\{media\}\}/g,"image"===a.type?encodeURIComponent(a.src):"").replace(/\{\{url\}\}/g,encodeURIComponent(t)).replace(/\{\{url_raw\}\}/g,n(t)).replace(/\{\{descr\}\}/g,i.$caption?encodeURIComponent(i.$caption.text()):""),e.fancybox.open({src:i.translate(i,o),type:"html",opts:{touch:!1,animationEffect:!1,afterLoad:function(t,e){i.$refs.container.one("beforeClose.fb",function(){t.close(null,0)}),e.$content.find(".fancybox-share__button").click(function(){return window.open(this.href,"Share","width=550, height=450"),!1})},mobile:{autoFocus:!1}}}))})}(document,jQuery),function(t,e,n){"use strict";function o(){var e=t.location.hash.substr(1),n=e.split("-"),o=n.length>1&&/^\+?\d+$/.test(n[n.length-1])?parseInt(n.pop(-1),10)||1:1,i=n.join("-");return{hash:e,index:o<1?1:o,gallery:i}}function i(t){""!==t.gallery&&n("[data-fancybox='"+n.escapeSelector(t.gallery)+"']").eq(t.index-1).focus().trigger("click.fb-start")}function a(t){var e,n;return!!t&&(e=t.current?t.current.opts:t.opts,""!==(n=e.hash||(e.$orig?e.$orig.data("fancybox")||e.$orig.data("fancybox-trigger"):""))&&n)}n.escapeSelector||(n.escapeSelector=function(t){return(t+"").replace(/([\0-\x1f\x7f]|^-?\d)|^-$|[^\x80-\uFFFF\w-]/g,function(t,e){return e?"\0"===t?"�":t.slice(0,-1)+"\\"+t.charCodeAt(t.length-1).toString(16)+" ":"\\"+t})}),n(function(){!1!==n.fancybox.defaults.hash&&(n(e).on({"onInit.fb":function(t,e){var n,i;!1!==e.group[e.currIndex].opts.hash&&(n=o(),(i=a(e))&&n.gallery&&i==n.gallery&&(e.currIndex=n.index-1))},"beforeShow.fb":function(n,o,i,s){var r;i&&!1!==i.opts.hash&&(r=a(o))&&(o.currentHash=r+(o.group.length>1?"-"+(i.index+1):""),t.location.hash!=="#"+o.currentHash&&(s&&!o.origHash&&(o.origHash=t.location.hash),o.hashTimer&&clearTimeout(o.hashTimer),o.hashTimer=setTimeout(function(){"replaceState"in t.history?(t.history[s?"pushState":"replaceState"]({},e.title,t.location.pathname+t.location.search+"#"+o.currentHash),s&&(o.hasCreatedHistory=!0)):t.location.hash=o.currentHash,o.hashTimer=null},300)))},"beforeClose.fb":function(n,o,i){i&&!1!==i.opts.hash&&(clearTimeout(o.hashTimer),o.currentHash&&o.hasCreatedHistory?t.history.back():o.currentHash&&("replaceState"in t.history?t.history.replaceState({},e.title,t.location.pathname+t.location.search+(o.origHash||"")):t.location.hash=o.origHash),o.currentHash=null)}}),n(t).on("hashchange.fb",function(){var t=o(),e=null;n.each(n(".fancybox-container").get().reverse(),function(t,o){var i=n(o).data("FancyBox");if(i&&i.currentHash)return e=i,!1}),e?e.currentHash===t.gallery+"-"+t.index||1===t.index&&e.currentHash==t.gallery||(e.currentHash=null,e.close()):""!==t.gallery&&i(t)}),setTimeout(function(){n.fancybox.getInstance()||i(o())},50))})}(window,document,jQuery),function(t,e){"use strict";var n=(new Date).getTime();e(t).on({"onInit.fb":function(t,e,o){e.$refs.stage.on("mousewheel DOMMouseScroll wheel MozMousePixelScroll",function(t){var o=e.current,i=(new Date).getTime();e.group.length<2||!1===o.opts.wheel||"auto"===o.opts.wheel&&"image"!==o.type||(t.preventDefault(),t.stopPropagation(),o.$slide.hasClass("fancybox-animated")||(t=t.originalEvent||t,i-n<250||(n=i,e[(-t.deltaY||-t.deltaX||t.wheelDelta||-t.detail)<0?"next":"previous"]())))})}})}(document,jQuery);
/* End */
;
; /* Start:"a:4:{s:4:"full";s:84:"/local/templates/.default/markup/new/js/jquery.magnific-popup.min.js?165003251820216";s:6:"source";s:68:"/local/templates/.default/markup/new/js/jquery.magnific-popup.min.js";s:3:"min";s:0:"";s:3:"map";s:0:"";}"*/
/*! Magnific Popup - v1.1.0 - 2016-02-20
* http://dimsemenov.com/plugins/magnific-popup/
* Copyright (c) 2016 Dmitry Semenov; */
!function(a){"function"==typeof define&&define.amd?define(["jquery"],a):a("object"==typeof exports?require("jquery"):window.jQuery||window.Zepto)}(function(a){var b,c,d,e,f,g,h="Close",i="BeforeClose",j="AfterClose",k="BeforeAppend",l="MarkupParse",m="Open",n="Change",o="mfp",p="."+o,q="mfp-ready",r="mfp-removing",s="mfp-prevent-close",t=function(){},u=!!window.jQuery,v=a(window),w=function(a,c){b.ev.on(o+a+p,c)},x=function(b,c,d,e){var f=document.createElement("div");return f.className="mfp-"+b,d&&(f.innerHTML=d),e?c&&c.appendChild(f):(f=a(f),c&&f.appendTo(c)),f},y=function(c,d){b.ev.triggerHandler(o+c,d),b.st.callbacks&&(c=c.charAt(0).toLowerCase()+c.slice(1),b.st.callbacks[c]&&b.st.callbacks[c].apply(b,a.isArray(d)?d:[d]))},z=function(c){return c===g&&b.currTemplate.closeBtn||(b.currTemplate.closeBtn=a(b.st.closeMarkup.replace("%title%",b.st.tClose)),g=c),b.currTemplate.closeBtn},A=function(){a.magnificPopup.instance||(b=new t,b.init(),a.magnificPopup.instance=b)},B=function(){var a=document.createElement("p").style,b=["ms","O","Moz","Webkit"];if(void 0!==a.transition)return!0;for(;b.length;)if(b.pop()+"Transition"in a)return!0;return!1};t.prototype={constructor:t,init:function(){var c=navigator.appVersion;b.isLowIE=b.isIE8=document.all&&!document.addEventListener,b.isAndroid=/android/gi.test(c),b.isIOS=/iphone|ipad|ipod/gi.test(c),b.supportsTransition=B(),b.probablyMobile=b.isAndroid||b.isIOS||/(Opera Mini)|Kindle|webOS|BlackBerry|(Opera Mobi)|(Windows Phone)|IEMobile/i.test(navigator.userAgent),d=a(document),b.popupsCache={}},open:function(c){var e;if(c.isObj===!1){b.items=c.items.toArray(),b.index=0;var g,h=c.items;for(e=0;e<h.length;e++)if(g=h[e],g.parsed&&(g=g.el[0]),g===c.el[0]){b.index=e;break}}else b.items=a.isArray(c.items)?c.items:[c.items],b.index=c.index||0;if(b.isOpen)return void b.updateItemHTML();b.types=[],f="",c.mainEl&&c.mainEl.length?b.ev=c.mainEl.eq(0):b.ev=d,c.key?(b.popupsCache[c.key]||(b.popupsCache[c.key]={}),b.currTemplate=b.popupsCache[c.key]):b.currTemplate={},b.st=a.extend(!0,{},a.magnificPopup.defaults,c),b.fixedContentPos="auto"===b.st.fixedContentPos?!b.probablyMobile:b.st.fixedContentPos,b.st.modal&&(b.st.closeOnContentClick=!1,b.st.closeOnBgClick=!1,b.st.showCloseBtn=!1,b.st.enableEscapeKey=!1),b.bgOverlay||(b.bgOverlay=x("bg").on("click"+p,function(){b.close()}),b.wrap=x("wrap").attr("tabindex",-1).on("click"+p,function(a){b._checkIfClose(a.target)&&b.close()}),b.container=x("container",b.wrap)),b.contentContainer=x("content"),b.st.preloader&&(b.preloader=x("preloader",b.container,b.st.tLoading));var i=a.magnificPopup.modules;for(e=0;e<i.length;e++){var j=i[e];j=j.charAt(0).toUpperCase()+j.slice(1),b["init"+j].call(b)}y("BeforeOpen"),b.st.showCloseBtn&&(b.st.closeBtnInside?(w(l,function(a,b,c,d){c.close_replaceWith=z(d.type)}),f+=" mfp-close-btn-in"):b.wrap.append(z())),b.st.alignTop&&(f+=" mfp-align-top"),b.fixedContentPos?b.wrap.css({overflow:b.st.overflowY,overflowX:"hidden",overflowY:b.st.overflowY}):b.wrap.css({top:v.scrollTop(),position:"absolute"}),(b.st.fixedBgPos===!1||"auto"===b.st.fixedBgPos&&!b.fixedContentPos)&&b.bgOverlay.css({height:d.height(),position:"absolute"}),b.st.enableEscapeKey&&d.on("keyup"+p,function(a){27===a.keyCode&&b.close()}),v.on("resize"+p,function(){b.updateSize()}),b.st.closeOnContentClick||(f+=" mfp-auto-cursor"),f&&b.wrap.addClass(f);var k=b.wH=v.height(),n={};if(b.fixedContentPos&&b._hasScrollBar(k)){var o=b._getScrollbarSize();o&&(n.marginRight=o)}b.fixedContentPos&&(b.isIE7?a("body, html").css("overflow","hidden"):n.overflow="hidden");var r=b.st.mainClass;return b.isIE7&&(r+=" mfp-ie7"),r&&b._addClassToMFP(r),b.updateItemHTML(),y("BuildControls"),a("html").css(n),b.bgOverlay.add(b.wrap).prependTo(b.st.prependTo||a(document.body)),b._lastFocusedEl=document.activeElement,setTimeout(function(){b.content?(b._addClassToMFP(q),b._setFocus()):b.bgOverlay.addClass(q),d.on("focusin"+p,b._onFocusIn)},16),b.isOpen=!0,b.updateSize(k),y(m),c},close:function(){b.isOpen&&(y(i),b.isOpen=!1,b.st.removalDelay&&!b.isLowIE&&b.supportsTransition?(b._addClassToMFP(r),setTimeout(function(){b._close()},b.st.removalDelay)):b._close())},_close:function(){y(h);var c=r+" "+q+" ";if(b.bgOverlay.detach(),b.wrap.detach(),b.container.empty(),b.st.mainClass&&(c+=b.st.mainClass+" "),b._removeClassFromMFP(c),b.fixedContentPos){var e={marginRight:""};b.isIE7?a("body, html").css("overflow",""):e.overflow="",a("html").css(e)}d.off("keyup"+p+" focusin"+p),b.ev.off(p),b.wrap.attr("class","mfp-wrap").removeAttr("style"),b.bgOverlay.attr("class","mfp-bg"),b.container.attr("class","mfp-container"),!b.st.showCloseBtn||b.st.closeBtnInside&&b.currTemplate[b.currItem.type]!==!0||b.currTemplate.closeBtn&&b.currTemplate.closeBtn.detach(),b.st.autoFocusLast&&b._lastFocusedEl&&a(b._lastFocusedEl).focus(),b.currItem=null,b.content=null,b.currTemplate=null,b.prevHeight=0,y(j)},updateSize:function(a){if(b.isIOS){var c=document.documentElement.clientWidth/window.innerWidth,d=window.innerHeight*c;b.wrap.css("height",d),b.wH=d}else b.wH=a||v.height();b.fixedContentPos||b.wrap.css("height",b.wH),y("Resize")},updateItemHTML:function(){var c=b.items[b.index];b.contentContainer.detach(),b.content&&b.content.detach(),c.parsed||(c=b.parseEl(b.index));var d=c.type;if(y("BeforeChange",[b.currItem?b.currItem.type:"",d]),b.currItem=c,!b.currTemplate[d]){var f=b.st[d]?b.st[d].markup:!1;y("FirstMarkupParse",f),f?b.currTemplate[d]=a(f):b.currTemplate[d]=!0}e&&e!==c.type&&b.container.removeClass("mfp-"+e+"-holder");var g=b["get"+d.charAt(0).toUpperCase()+d.slice(1)](c,b.currTemplate[d]);b.appendContent(g,d),c.preloaded=!0,y(n,c),e=c.type,b.container.prepend(b.contentContainer),y("AfterChange")},appendContent:function(a,c){b.content=a,a?b.st.showCloseBtn&&b.st.closeBtnInside&&b.currTemplate[c]===!0?b.content.find(".mfp-close").length||b.content.append(z()):b.content=a:b.content="",y(k),b.container.addClass("mfp-"+c+"-holder"),b.contentContainer.append(b.content)},parseEl:function(c){var d,e=b.items[c];if(e.tagName?e={el:a(e)}:(d=e.type,e={data:e,src:e.src}),e.el){for(var f=b.types,g=0;g<f.length;g++)if(e.el.hasClass("mfp-"+f[g])){d=f[g];break}e.src=e.el.attr("data-mfp-src"),e.src||(e.src=e.el.attr("href"))}return e.type=d||b.st.type||"inline",e.index=c,e.parsed=!0,b.items[c]=e,y("ElementParse",e),b.items[c]},addGroup:function(a,c){var d=function(d){d.mfpEl=this,b._openClick(d,a,c)};c||(c={});var e="click.magnificPopup";c.mainEl=a,c.items?(c.isObj=!0,a.off(e).on(e,d)):(c.isObj=!1,c.delegate?a.off(e).on(e,c.delegate,d):(c.items=a,a.off(e).on(e,d)))},_openClick:function(c,d,e){var f=void 0!==e.midClick?e.midClick:a.magnificPopup.defaults.midClick;if(f||!(2===c.which||c.ctrlKey||c.metaKey||c.altKey||c.shiftKey)){var g=void 0!==e.disableOn?e.disableOn:a.magnificPopup.defaults.disableOn;if(g)if(a.isFunction(g)){if(!g.call(b))return!0}else if(v.width()<g)return!0;c.type&&(c.preventDefault(),b.isOpen&&c.stopPropagation()),e.el=a(c.mfpEl),e.delegate&&(e.items=d.find(e.delegate)),b.open(e)}},updateStatus:function(a,d){if(b.preloader){c!==a&&b.container.removeClass("mfp-s-"+c),d||"loading"!==a||(d=b.st.tLoading);var e={status:a,text:d};y("UpdateStatus",e),a=e.status,d=e.text,b.preloader.html(d),b.preloader.find("a").on("click",function(a){a.stopImmediatePropagation()}),b.container.addClass("mfp-s-"+a),c=a}},_checkIfClose:function(c){if(!a(c).hasClass(s)){var d=b.st.closeOnContentClick,e=b.st.closeOnBgClick;if(d&&e)return!0;if(!b.content||a(c).hasClass("mfp-close")||b.preloader&&c===b.preloader[0])return!0;if(c===b.content[0]||a.contains(b.content[0],c)){if(d)return!0}else if(e&&a.contains(document,c))return!0;return!1}},_addClassToMFP:function(a){b.bgOverlay.addClass(a),b.wrap.addClass(a)},_removeClassFromMFP:function(a){this.bgOverlay.removeClass(a),b.wrap.removeClass(a)},_hasScrollBar:function(a){return(b.isIE7?d.height():document.body.scrollHeight)>(a||v.height())},_setFocus:function(){(b.st.focus?b.content.find(b.st.focus).eq(0):b.wrap).focus()},_onFocusIn:function(c){return c.target===b.wrap[0]||a.contains(b.wrap[0],c.target)?void 0:(b._setFocus(),!1)},_parseMarkup:function(b,c,d){var e;d.data&&(c=a.extend(d.data,c)),y(l,[b,c,d]),a.each(c,function(c,d){if(void 0===d||d===!1)return!0;if(e=c.split("_"),e.length>1){var f=b.find(p+"-"+e[0]);if(f.length>0){var g=e[1];"replaceWith"===g?f[0]!==d[0]&&f.replaceWith(d):"img"===g?f.is("img")?f.attr("src",d):f.replaceWith(a("<img>").attr("src",d).attr("class",f.attr("class"))):f.attr(e[1],d)}}else b.find(p+"-"+c).html(d)})},_getScrollbarSize:function(){if(void 0===b.scrollbarSize){var a=document.createElement("div");a.style.cssText="width: 99px; height: 99px; overflow: scroll; position: absolute; top: -9999px;",document.body.appendChild(a),b.scrollbarSize=a.offsetWidth-a.clientWidth,document.body.removeChild(a)}return b.scrollbarSize}},a.magnificPopup={instance:null,proto:t.prototype,modules:[],open:function(b,c){return A(),b=b?a.extend(!0,{},b):{},b.isObj=!0,b.index=c||0,this.instance.open(b)},close:function(){return a.magnificPopup.instance&&a.magnificPopup.instance.close()},registerModule:function(b,c){c.options&&(a.magnificPopup.defaults[b]=c.options),a.extend(this.proto,c.proto),this.modules.push(b)},defaults:{disableOn:0,key:null,midClick:!1,mainClass:"",preloader:!0,focus:"",closeOnContentClick:!1,closeOnBgClick:!0,closeBtnInside:!0,showCloseBtn:!0,enableEscapeKey:!0,modal:!1,alignTop:!1,removalDelay:0,prependTo:null,fixedContentPos:"auto",fixedBgPos:"auto",overflowY:"auto",closeMarkup:'<button title="%title%" type="button" class="mfp-close">&#215;</button>',tClose:"Close (Esc)",tLoading:"Loading...",autoFocusLast:!0}},a.fn.magnificPopup=function(c){A();var d=a(this);if("string"==typeof c)if("open"===c){var e,f=u?d.data("magnificPopup"):d[0].magnificPopup,g=parseInt(arguments[1],10)||0;f.items?e=f.items[g]:(e=d,f.delegate&&(e=e.find(f.delegate)),e=e.eq(g)),b._openClick({mfpEl:e},d,f)}else b.isOpen&&b[c].apply(b,Array.prototype.slice.call(arguments,1));else c=a.extend(!0,{},c),u?d.data("magnificPopup",c):d[0].magnificPopup=c,b.addGroup(d,c);return d};var C,D,E,F="inline",G=function(){E&&(D.after(E.addClass(C)).detach(),E=null)};a.magnificPopup.registerModule(F,{options:{hiddenClass:"hide",markup:"",tNotFound:"Content not found"},proto:{initInline:function(){b.types.push(F),w(h+"."+F,function(){G()})},getInline:function(c,d){if(G(),c.src){var e=b.st.inline,f=a(c.src);if(f.length){var g=f[0].parentNode;g&&g.tagName&&(D||(C=e.hiddenClass,D=x(C),C="mfp-"+C),E=f.after(D).detach().removeClass(C)),b.updateStatus("ready")}else b.updateStatus("error",e.tNotFound),f=a("<div>");return c.inlineElement=f,f}return b.updateStatus("ready"),b._parseMarkup(d,{},c),d}}});var H,I="ajax",J=function(){H&&a(document.body).removeClass(H)},K=function(){J(),b.req&&b.req.abort()};a.magnificPopup.registerModule(I,{options:{settings:null,cursor:"mfp-ajax-cur",tError:'<a href="%url%">The content</a> could not be loaded.'},proto:{initAjax:function(){b.types.push(I),H=b.st.ajax.cursor,w(h+"."+I,K),w("BeforeChange."+I,K)},getAjax:function(c){H&&a(document.body).addClass(H),b.updateStatus("loading");var d=a.extend({url:c.src,success:function(d,e,f){var g={data:d,xhr:f};y("ParseAjax",g),b.appendContent(a(g.data),I),c.finished=!0,J(),b._setFocus(),setTimeout(function(){b.wrap.addClass(q)},16),b.updateStatus("ready"),y("AjaxContentAdded")},error:function(){J(),c.finished=c.loadError=!0,b.updateStatus("error",b.st.ajax.tError.replace("%url%",c.src))}},b.st.ajax.settings);return b.req=a.ajax(d),""}}});var L,M=function(c){if(c.data&&void 0!==c.data.title)return c.data.title;var d=b.st.image.titleSrc;if(d){if(a.isFunction(d))return d.call(b,c);if(c.el)return c.el.attr(d)||""}return""};a.magnificPopup.registerModule("image",{options:{markup:'<div class="mfp-figure"><div class="mfp-close"></div><figure><div class="mfp-img"></div><figcaption><div class="mfp-bottom-bar"><div class="mfp-title"></div><div class="mfp-counter"></div></div></figcaption></figure></div>',cursor:"mfp-zoom-out-cur",titleSrc:"title",verticalFit:!0,tError:'<a href="%url%">The image</a> could not be loaded.'},proto:{initImage:function(){var c=b.st.image,d=".image";b.types.push("image"),w(m+d,function(){"image"===b.currItem.type&&c.cursor&&a(document.body).addClass(c.cursor)}),w(h+d,function(){c.cursor&&a(document.body).removeClass(c.cursor),v.off("resize"+p)}),w("Resize"+d,b.resizeImage),b.isLowIE&&w("AfterChange",b.resizeImage)},resizeImage:function(){var a=b.currItem;if(a&&a.img&&b.st.image.verticalFit){var c=0;b.isLowIE&&(c=parseInt(a.img.css("padding-top"),10)+parseInt(a.img.css("padding-bottom"),10)),a.img.css("max-height",b.wH-c)}},_onImageHasSize:function(a){a.img&&(a.hasSize=!0,L&&clearInterval(L),a.isCheckingImgSize=!1,y("ImageHasSize",a),a.imgHidden&&(b.content&&b.content.removeClass("mfp-loading"),a.imgHidden=!1))},findImageSize:function(a){var c=0,d=a.img[0],e=function(f){L&&clearInterval(L),L=setInterval(function(){return d.naturalWidth>0?void b._onImageHasSize(a):(c>200&&clearInterval(L),c++,void(3===c?e(10):40===c?e(50):100===c&&e(500)))},f)};e(1)},getImage:function(c,d){var e=0,f=function(){c&&(c.img[0].complete?(c.img.off(".mfploader"),c===b.currItem&&(b._onImageHasSize(c),b.updateStatus("ready")),c.hasSize=!0,c.loaded=!0,y("ImageLoadComplete")):(e++,200>e?setTimeout(f,100):g()))},g=function(){c&&(c.img.off(".mfploader"),c===b.currItem&&(b._onImageHasSize(c),b.updateStatus("error",h.tError.replace("%url%",c.src))),c.hasSize=!0,c.loaded=!0,c.loadError=!0)},h=b.st.image,i=d.find(".mfp-img");if(i.length){var j=document.createElement("img");j.className="mfp-img",c.el&&c.el.find("img").length&&(j.alt=c.el.find("img").attr("alt")),c.img=a(j).on("load.mfploader",f).on("error.mfploader",g),j.src=c.src,i.is("img")&&(c.img=c.img.clone()),j=c.img[0],j.naturalWidth>0?c.hasSize=!0:j.width||(c.hasSize=!1)}return b._parseMarkup(d,{title:M(c),img_replaceWith:c.img},c),b.resizeImage(),c.hasSize?(L&&clearInterval(L),c.loadError?(d.addClass("mfp-loading"),b.updateStatus("error",h.tError.replace("%url%",c.src))):(d.removeClass("mfp-loading"),b.updateStatus("ready")),d):(b.updateStatus("loading"),c.loading=!0,c.hasSize||(c.imgHidden=!0,d.addClass("mfp-loading"),b.findImageSize(c)),d)}}});var N,O=function(){return void 0===N&&(N=void 0!==document.createElement("p").style.MozTransform),N};a.magnificPopup.registerModule("zoom",{options:{enabled:!1,easing:"ease-in-out",duration:300,opener:function(a){return a.is("img")?a:a.find("img")}},proto:{initZoom:function(){var a,c=b.st.zoom,d=".zoom";if(c.enabled&&b.supportsTransition){var e,f,g=c.duration,j=function(a){var b=a.clone().removeAttr("style").removeAttr("class").addClass("mfp-animated-image"),d="all "+c.duration/1e3+"s "+c.easing,e={position:"fixed",zIndex:9999,left:0,top:0,"-webkit-backface-visibility":"hidden"},f="transition";return e["-webkit-"+f]=e["-moz-"+f]=e["-o-"+f]=e[f]=d,b.css(e),b},k=function(){b.content.css("visibility","visible")};w("BuildControls"+d,function(){if(b._allowZoom()){if(clearTimeout(e),b.content.css("visibility","hidden"),a=b._getItemToZoom(),!a)return void k();f=j(a),f.css(b._getOffset()),b.wrap.append(f),e=setTimeout(function(){f.css(b._getOffset(!0)),e=setTimeout(function(){k(),setTimeout(function(){f.remove(),a=f=null,y("ZoomAnimationEnded")},16)},g)},16)}}),w(i+d,function(){if(b._allowZoom()){if(clearTimeout(e),b.st.removalDelay=g,!a){if(a=b._getItemToZoom(),!a)return;f=j(a)}f.css(b._getOffset(!0)),b.wrap.append(f),b.content.css("visibility","hidden"),setTimeout(function(){f.css(b._getOffset())},16)}}),w(h+d,function(){b._allowZoom()&&(k(),f&&f.remove(),a=null)})}},_allowZoom:function(){return"image"===b.currItem.type},_getItemToZoom:function(){return b.currItem.hasSize?b.currItem.img:!1},_getOffset:function(c){var d;d=c?b.currItem.img:b.st.zoom.opener(b.currItem.el||b.currItem);var e=d.offset(),f=parseInt(d.css("padding-top"),10),g=parseInt(d.css("padding-bottom"),10);e.top-=a(window).scrollTop()-f;var h={width:d.width(),height:(u?d.innerHeight():d[0].offsetHeight)-g-f};return O()?h["-moz-transform"]=h.transform="translate("+e.left+"px,"+e.top+"px)":(h.left=e.left,h.top=e.top),h}}});var P="iframe",Q="//about:blank",R=function(a){if(b.currTemplate[P]){var c=b.currTemplate[P].find("iframe");c.length&&(a||(c[0].src=Q),b.isIE8&&c.css("display",a?"block":"none"))}};a.magnificPopup.registerModule(P,{options:{markup:'<div class="mfp-iframe-scaler"><div class="mfp-close"></div><iframe class="mfp-iframe" src="//about:blank" frameborder="0" allowfullscreen></iframe></div>',srcAction:"iframe_src",patterns:{youtube:{index:"youtube.com",id:"v=",src:"//www.youtube.com/embed/%id%?autoplay=1"},vimeo:{index:"vimeo.com/",id:"/",src:"//player.vimeo.com/video/%id%?autoplay=1"},gmaps:{index:"//maps.google.",src:"%id%&output=embed"}}},proto:{initIframe:function(){b.types.push(P),w("BeforeChange",function(a,b,c){b!==c&&(b===P?R():c===P&&R(!0))}),w(h+"."+P,function(){R()})},getIframe:function(c,d){var e=c.src,f=b.st.iframe;a.each(f.patterns,function(){return e.indexOf(this.index)>-1?(this.id&&(e="string"==typeof this.id?e.substr(e.lastIndexOf(this.id)+this.id.length,e.length):this.id.call(this,e)),e=this.src.replace("%id%",e),!1):void 0});var g={};return f.srcAction&&(g[f.srcAction]=e),b._parseMarkup(d,g,c),b.updateStatus("ready"),d}}});var S=function(a){var c=b.items.length;return a>c-1?a-c:0>a?c+a:a},T=function(a,b,c){return a.replace(/%curr%/gi,b+1).replace(/%total%/gi,c)};a.magnificPopup.registerModule("gallery",{options:{enabled:!1,arrowMarkup:'<button title="%title%" type="button" class="mfp-arrow mfp-arrow-%dir%"></button>',preload:[0,2],navigateByImgClick:!0,arrows:!0,tPrev:"Previous (Left arrow key)",tNext:"Next (Right arrow key)",tCounter:"%curr% of %total%"},proto:{initGallery:function(){var c=b.st.gallery,e=".mfp-gallery";return b.direction=!0,c&&c.enabled?(f+=" mfp-gallery",w(m+e,function(){c.navigateByImgClick&&b.wrap.on("click"+e,".mfp-img",function(){return b.items.length>1?(b.next(),!1):void 0}),d.on("keydown"+e,function(a){37===a.keyCode?b.prev():39===a.keyCode&&b.next()})}),w("UpdateStatus"+e,function(a,c){c.text&&(c.text=T(c.text,b.currItem.index,b.items.length))}),w(l+e,function(a,d,e,f){var g=b.items.length;e.counter=g>1?T(c.tCounter,f.index,g):""}),w("BuildControls"+e,function(){if(b.items.length>1&&c.arrows&&!b.arrowLeft){var d=c.arrowMarkup,e=b.arrowLeft=a(d.replace(/%title%/gi,c.tPrev).replace(/%dir%/gi,"left")).addClass(s),f=b.arrowRight=a(d.replace(/%title%/gi,c.tNext).replace(/%dir%/gi,"right")).addClass(s);e.click(function(){b.prev()}),f.click(function(){b.next()}),b.container.append(e.add(f))}}),w(n+e,function(){b._preloadTimeout&&clearTimeout(b._preloadTimeout),b._preloadTimeout=setTimeout(function(){b.preloadNearbyImages(),b._preloadTimeout=null},16)}),void w(h+e,function(){d.off(e),b.wrap.off("click"+e),b.arrowRight=b.arrowLeft=null})):!1},next:function(){b.direction=!0,b.index=S(b.index+1),b.updateItemHTML()},prev:function(){b.direction=!1,b.index=S(b.index-1),b.updateItemHTML()},goTo:function(a){b.direction=a>=b.index,b.index=a,b.updateItemHTML()},preloadNearbyImages:function(){var a,c=b.st.gallery.preload,d=Math.min(c[0],b.items.length),e=Math.min(c[1],b.items.length);for(a=1;a<=(b.direction?e:d);a++)b._preloadItem(b.index+a);for(a=1;a<=(b.direction?d:e);a++)b._preloadItem(b.index-a)},_preloadItem:function(c){if(c=S(c),!b.items[c].preloaded){var d=b.items[c];d.parsed||(d=b.parseEl(c)),y("LazyLoad",d),"image"===d.type&&(d.img=a('<img class="mfp-img" />').on("load.mfploader",function(){d.hasSize=!0}).on("error.mfploader",function(){d.hasSize=!0,d.loadError=!0,y("LazyLoadError",d)}).attr("src",d.src)),d.preloaded=!0}}}});var U="retina";a.magnificPopup.registerModule(U,{options:{replaceSrc:function(a){return a.src.replace(/\.\w+$/,function(a){return"@2x"+a})},ratio:1},proto:{initRetina:function(){if(window.devicePixelRatio>1){var a=b.st.retina,c=a.ratio;c=isNaN(c)?c():c,c>1&&(w("ImageHasSize."+U,function(a,b){b.img.css({"max-width":b.img[0].naturalWidth/c,width:"100%"})}),w("ElementParse."+U,function(b,d){d.src=a.replaceSrc(d,c)}))}}}}),A()});
/* End */
;
; /* Start:"a:4:{s:4:"full";s:67:"/local/templates/.default/markup/new/js/main_new.js?174418421323131";s:6:"source";s:51:"/local/templates/.default/markup/new/js/main_new.js";s:3:"min";s:0:"";s:3:"map";s:0:"";}"*/
! function(n) {
    var o = {};

    function a(e) {
        if (o[e]) return o[e].exports;
        var t = o[e] = {
            i: e,
            l: !1,
            exports: {}
        };
        return n[e].call(t.exports, t, t.exports, a), t.l = !0, t.exports
    }
    a.m = n, a.c = o, a.d = function(e, t, n) {
        a.o(e, t) || Object.defineProperty(e, t, {
            enumerable: !0,
            get: n
        })
    }, a.r = function(e) {
        "undefined" != typeof Symbol && Symbol.toStringTag && Object.defineProperty(e, Symbol.toStringTag, {
            value: "Module"
        }), Object.defineProperty(e, "__esModule", {
            value: !0
        })
    }, a.t = function(t, e) {
        if (1 & e && (t = a(t)), 8 & e) return t;
        if (4 & e && "object" == typeof t && t && t.__esModule) return t;
        var n = Object.create(null);
        if (a.r(n), Object.defineProperty(n, "default", {
                enumerable: !0,
                value: t
            }), 2 & e && "string" != typeof t)
            for (var o in t) a.d(n, o, function(e) {
                return t[e]
            }.bind(null, o));
        return n
    }, a.n = function(e) {
        var t = e && e.__esModule ? function() {
            return e.default
        } : function() {
            return e
        };
        return a.d(t, "a", t), t
    }, a.o = function(e, t) {
        return Object.prototype.hasOwnProperty.call(e, t)
    }, a.p = "", a(a.s = 0)
}([function(e, t, n) {
    "use strict";
    n.r(t);
    t = n(1);
    n(2), n(3), n(4), n(5), n(6), n(7), n(8), n(9), n(10), n(11), $(document).ready(function() {
        $(".bs-select").each(function() {
            $(this).hasClass("bs-select--srch") ? $(this).selectpicker({
                dropupAuto: !0,
                noneResultsText: "Ничего не найдено"
            }) : $(this).selectpicker({
                dropupAuto: !1,
                noneResultsText: "Ничего не найдено"
            })
        })
    }), $(document).on("click", ".spoiler-trigger", function(e) {
        e.preventDefault(), $(this).toggleClass("active")
    })
}, function(e, t, n) {}, function(e, t) {
    $(document).ready(function() {
        $(".button--header-search").on("click", function() {
            $(".form--header-search").addClass("active")
        }), $(document).mouseup(function(e) {
            var t = $(".form--header-search");
            0 === t.has(e.target).length && t.removeClass("active")
        }), $(".button--get-search").on("click", function() {
            $(".form--header-search").submit()
        })
    })
}, function(e, t) {
    $(document).ready(function() {
        var e;

        function t() {
            return window.matchMedia("(max-width: 600px)").matches ? "mobile" : window.matchMedia("(max-width: 1200px)").matches ? "tablet" : "desktop"
        }
        console.log("hello 2"), $(".slick-slider-activation").each(function(e, t) {
            return n = t, o = $(n).find(".slick-slider-container"), a = $(n).find(".slick-slider-prev"), t = $(n).find(".slick-slider-next"), n = $(n).find(".slick-slider-dots"), void($(o).length && $(o).slick({
                dots: !0,
                slidesToShow: 1,
                slidesToScroll: 1,
                arrows: !0,
                infinite: !0,
                variableWidth: !0,
                prevArrow: a,
                nextArrow: t,
                lazyLoad: "ondemand",
                appendDots: n,
                customPaging: function(e, t) {
                    t += 1, e = e.slideCount;
                    return '<span class="dot" role="button" title="'.concat(t, " of ").concat(e, '"></span>')
                }
            }));
            var n, o, a
        }), document.querySelector(".year-reports__slider") && (e = t(), n(), $(window).resize(function() {
            e != t() && (e = t(), $(".year-reports__slider").slick("getSlick") && ($(".year-reports__slider").slick("unslick"), setTimeout(function() {
                n()
            }, 100)))
        })), document.querySelector(".foto-gallery .big-container") && $(".foto-gallery .big-container").each(function(e, t) {
            $(t).addClass("big-container-" + e);
            var a = $(t).closest(".foto-gallery").find(".foto-gallery__status");
            $(t).closest(".foto-gallery").find(".foto-gallery__author"), $(t).find(".foto-gallery .big-container");
            $(".foto-gallery .big-container-" + e).on("init reInit afterChange", function(e, t, n, o) {
                n = (n || 0) + 1;
                a.text("".concat(n, "/").concat(t.slideCount));
                if($('.foto-gallery-big .slick-active img').attr('alt') != undefined){
                	$('.js-photo-name').text($('.foto-gallery-big .slick-active img').attr('alt'));
                }
                
              
            }), $(".foto-gallery .big-container-" + e).slick({
                dots: !0,
                slidesToShow: 1,
                slidesToScroll: 1,
                arrows: !0,
                infinite: !0,
                variableWidth: !0,
                prevArrow: $(t).closest(".foto-gallery").find(".foto-gallery__prev"),
                nextArrow: $(t).closest(".foto-gallery").find(".foto-gallery__next"),
                asNavFor: $(t).closest(".foto-gallery").find(".small-container"),
                lazyLoad: "ondemand",
                appendDots: $(t).find(".foto-gallery__dots"),
                customPaging: function(e, t) {
                    t += 1, e = e.slideCount;
                    return '<span class="dot" role="button" title="'.concat(t, " of ").concat(e, '"></span>')
                }
            })
        }), document.querySelector(".foto-gallery .small-container") && $(".foto-gallery .small-container").each(function(e, t) {
            $(t).addClass("small-container-" + e), $(".foto-gallery .small-container-" + e).slick({
                dots: !1,
                slidesToShow: 1,
                slidesToScroll: 1,
                arrows: !1,
                infinite: !0,
                variableWidth: !0,
                asNavFor: $(t).closest(".foto-gallery").find(".big-container"),
                lazyLoad: "ondemand",
                focusOnSelect: !0
            })
        });
        var n = function() {
            $(".year-reports__slider").slick({
                dots: !0,
                slidesToShow: 1,
                slidesToScroll: 1,
                arrows: !0,
                infinite: !0,
                variableWidth: !0,
                prevArrow: document.querySelector(".year-reports .slick-slider-prev"),
                nextArrow: document.querySelector(".year-reports .slick-slider-next"),
                lazyLoad: "ondemand",
                appendDots: $(".year-reports .slick-slider-dots"),
                customPaging: function(e, t) {
                    t += 1, e = e.slideCount;
                    return '<span class="dot" role="button" title="'.concat(t, " of ").concat(e, '"></span>')
                },
                responsive: [{
                    breakpoint: 1e4,
                    settings: "unslick"
                }, {
                    breakpoint: 1200,
                    settings: "slick"
                }]
            })
        }
    })
}, function(e, t) {
    $(document).ready(function() {
        for (var e, t, n, o, a, i = document.getElementsByClassName("custom-select"), s = 0; s < i.length; s++) {
            for (t = i[s].getElementsByTagName("select")[0], (n = document.createElement("DIV")).setAttribute("class", "select-selected"), n.innerHTML = t.options[t.selectedIndex].innerHTML, i[s].appendChild(n), (o = document.createElement("DIV")).setAttribute("class", "select-items select-hide"), e = 1; e < t.length; e++)(a = document.createElement("DIV")).innerHTML = t.options[e].innerHTML, a.addEventListener("click", function(e) {
                for (var t, n, o = this.parentNode.parentNode.getElementsByTagName("select")[0], a = this.parentNode.previousSibling, i = 0; i < o.length; i++)
                    if (o.options[i].innerHTML == this.innerHTML) {
                        for (o.selectedIndex = i, a.innerHTML = this.innerHTML, t = this.parentNode.getElementsByClassName("same-as-selected"), n = 0; n < t.length; n++) t[n].removeAttribute("class");
                        this.setAttribute("class", "same-as-selected");
                        break
                    }
                a.click()
            }), o.appendChild(a);
            i[s].appendChild(o), n.addEventListener("click", function(e) {
                e.stopPropagation(), c(this), this.nextSibling.classList.toggle("select-hide"), this.classList.toggle("select-arrow-active")
            })
        }

        function c(e) {
            for (var t = [], n = document.getElementsByClassName("select-items"), o = document.getElementsByClassName("select-selected"), a = 0; a < o.length; a++) e == o[a] ? t.push(a) : o[a].classList.remove("select-arrow-active");
            for (a = 0; a < n.length; a++) t.indexOf(a) && n[a].classList.add("select-hide")
        }
        document.addEventListener("click", c), console.log("custom select is ready!")
    })
}, function(e, t) {
    $(document).ready(function() {
        document.querySelector(".bs-mselect-chips-container") && $(".bs-mselect-chips-container").each(function(e, t) {
            var s = $(t).find(".bs-mselect"),
                c = $(t).find(".chip-st__container");
            $(s).on("changed.bs.select", function(e, n, t, o) {
                var a = '.chip-st[data-index="'.concat(n, '"]'),
                    i = $(c).find(a);
                0 < $(i).length ? ($(i).remove(), console.log("удаляю")) : ($(c).append('<div class="chip-st" data-index="'.concat(n, '">\n              <span>').concat(e.currentTarget[n].innerText, '</span>\n            <div class="button-close"></div></div>')), $(c).find(a).find(".button-close").click(function() {
                    console.log("клик и закрываю"), $(c).find(a).remove(), $(s).find("option").each(function(e, t) {
                        e === n && $(t).prop("selected", !1)
                    }), $(s).selectpicker("render"), console.log(n)
                }))
            })
        }), $(".bs-mselect").selectpicker({
            dropupAuto: !0,
            liveSearch: !0,
            noneResultsText: "Ничего не найдено",
            noneSelectedText: "..."
        }), $("form").each(function(e, t) {
            $(t).trigger("reset"), $(t).find(".bs-mselect").each(function(e, t) {
                $(t).val("default").selectpicker("refresh")
            }), $(".bs-mselect-chips-container").each(function(e, t) {
                $(t).find(".chip-st").remove()
            })
        }), console.log("bootstrap multi select is ready!")
    })
}, function(e, t) {
    $(document).ready(function() {
        var c = ["li.item", 3];
        document.querySelector(".show-more-menu") && $(".show-more-menu").each(function(e, t) {
            var o = [],
                a = 0,
                i = !1;

            function n() {
                0
            }

            function s() {
                var n;
                i = !1 === i ? ($(t).find(o[0]).each(function(e, t) {
                    setTimeout(function() {
                        $(t).removeClass("d-none")
                    }, 30 + 30 * e)
                }), !0) : (n = 30 * a, $(t).find(o[0]).each(function(e, t) {
                    e >= o[1] && (time = n - 30 * e, setTimeout(function() {
                        $(t).addClass("d-none")
                    }, time))
                }), !1)
            }
            5 < $(t).find(c[0]).length && (a = $(t).find(c[0]).length, o = c), 5 < a ? (n("количество пунктов меню ".concat(a, ", работаю")), $(t).find(o[0]).addClass("d-none"), n(), 5 < a && $(t).find("div.btn-more-container").length < 1 && $(t).append($('<div class="btn-more-container"><div class="btn-more btn-more-show show-more-menu-btn">Раскрыть меню</div></div>')), $(t).find(o[0]).each(function(e, t) {
                e < o[1] && $(t).removeClass("d-none")
            }), $(t).find(".show-more-menu-btn").click(function() {
                s(), !1 === i ? $(t).find("div.show-more-menu-btn").text("Раскрыть меню") : $(t).find("div.show-more-menu-btn").text("Скрыть меню"), n()
            })) : n()
        }), console.log("show more menu is ready!")
    })
}, function(e, t) {
    $(document).ready(function() {
        $(".scheme-container .mmap_point").each(function(e, t) {
            $(t).mouseenter(function() {
                $(t).addClass("active")
            }), $(t).mouseleave(function() {
                $(t).removeClass("active")
            })
        });
        var e = window.location.href.split("?")[0];

        function t(e) {
            var t = location.search.substr(1).split("&").reduce(function(e, t) {
                t = t.split("=");
                return e[decodeURIComponent(t[0])] = 1 == t.length ? null : decodeURIComponent(t[1]), e
            }, {});
            return t[e] ? t[e] : ""
        }
        e = e.split("#")[0], $(".scheme-container .mmap_a").click(function() {
            window.location.href = "".concat(e, "?mmapPoint=a")
        }), $(".scheme-container .mmap_b").click(function() {
            window.location.href = "".concat(e, "?mmapPoint=b")
        }), $(".scheme-container .mmap_v").click(function() {
            window.location.href = "".concat(e, "?mmapPoint=v")
        }), $(".scheme-container .mmap_l").click(function() {
            window.location.href = "".concat(e, "?mmapPoint=l")
        }), $(".scheme-container .mmap_g").click(function() {
            window.location.href = "".concat(e, "?mmapPoint=g")
        }), $(".scheme-container .mmap_s").click(function() {
            window.location.href = "".concat(e, "?mmapPoint=s")
        }), $(".scheme-container .mmap_d").click(function() {
            window.location.href = "".concat(e, "?mmapPoint=d")
        }), $(".scheme-container .mmap_n").click(function() {
            window.location.href = "".concat(e, "?mmapPoint=n")
        }), $(".scheme-container .mmap_r").click(function() {
            window.location.href = "".concat(e, "?mmapPoint=r")
        }), "a" === t("mmapPoint") ? $(".scheme-container .mmap_a").addClass("selected") : "b" === t("mmapPoint") ? $(".scheme-container .mmap_b").addClass("selected") : "v" === t("mmapPoint") ? $(".scheme-container .mmap_v").addClass("selected") : "l" === t("mmapPoint") ? $(".scheme-container .mmap_l").addClass("selected") : "g" === t("mmapPoint") ? $(".scheme-container .mmap_g").addClass("selected") : "s" === t("mmapPoint") ? $(".scheme-container .mmap_s").addClass("selected") : "d" === t("mmapPoint") ? $(".scheme-container .mmap_d").addClass("selected") : "n" === t("mmapPoint") ? $(".scheme-container .mmap_n").addClass("selected") : "r" === t("mmapPoint") && $(".scheme-container .mmap_r").addClass("selected"), console.log("map is ready!")
    })
}, function(e, t) {
    $(document).ready(function() {
        var o = {
            bibl_1_k1: {
                type: "1",
                text: '<span class="balloon_caption"><i>подробности не указаны</i></span><br><div class="balloon_detail"></div>',
                name: "1"
            },
            bibl_1_k2: {
                type: "1",
                text: '<span class="balloon_caption">Заместитель директора библиотеки</span><br><div class="balloon_detail"><p>Князева Тамара Ивановна</p></div>',
                name: "2"
            },
            bibl_1_k3: {
                type: "1",
                text: '<span class="balloon_caption"><i>подробности не указаны</i></span><br><div class="balloon_detail"></div>',
                name: "3"
            },
            bibl_1_k4: {
                type: "1",
                text: '<span class="balloon_caption">Директор библиотеки</span><br><div class="balloon_detail"><p>Решетникова Марина Вадимовна</p>  <p>Заместитель директора,<br />  Родина Светлана Валериевна</p></div>',
                name: "4"
            },
            bibl_1_uslugi: {
                type: "7",
                text: '<span class="balloon_caption"><i>подробности не указаны</i></span><br><div class="balloon_detail"></div>',
                name: "Отдел<br> платных<br> услуг",
                shift: [0, 0, 0, .8]
            },
            bibl_1_katalog: {
                type: "7",
                text: '<span class="balloon_caption"><i>подробности не указаны</i></span><br><div class="balloon_detail"></div>',
                name: "Каталог",
                shift: [0, 0, 360, 1]
            },
            bibl_1_sxron: {
                type: "7",
                text: '<span class="balloon_caption"><i>подробности не указаны</i></span><br><div class="balloon_detail"></div>',
                name: "Камера <br>хранения",
                shift: [0, 0, 0, .9]
            },
            bibl_2_1: {
                type: "5",
                text: '<span class="balloon_caption">Пожарная лестница</span><br><div class="balloon_detail"></div>',
                name: "&nbsp;"
            },
            go_bibl_1: {
                type: "10",
                text: '<span class="balloon_caption">Вход в библиотеку с кампуса</span><br><div class="balloon_detail"></div>',
                name: "Вход в библиотеку",
                shift: [0, -70, 0, 1]
            },
            go_library_gc: {
                type: "9",
                text: '<span class="balloon_caption"><i>подробности не указаны</i></span><br><div class="balloon_detail"></div>',
                name: "Подземный переход<br> в корпус Г <br>(на цокольный этаж)",
                shift: [350, -20, 360, 1],
                link: "room=go_gc_library"
            },
            biblio_bust_1: {
                type: "5",
                text: '<span class="balloon_caption">Бюст И.Г.Тюлина</span><br><div class="balloon_detail"><p><img class="right-bor" title="" border="0" hspace="0" alt=""  src="/files2/y09_2014/236406/2008-06-30_tulin14.jpg">30 июня 2008 года, в&nbsp;годовщину смерти первого проректора  МГИМО, доктора политических наук, профессора <a href="/tyulin">Ивана  Георгиевича Тюлина</a> в&nbsp;Научной библиотеке, носящей его имя, был  установлен скульптурный бюст.</p></div>',
                name: "&nbsp;"
            },
            dot_book_1_1: {
                type: "5",
                text: '<span class="balloon_caption">Автомат по продаже газировки</span><br><div class="balloon_detail"></div>',
                name: "&nbsp;"
            },
            bibl_1_2: {
                type: "5",
                text: '<span class="balloon_caption">Лестница наверх</span><br><div class="balloon_detail"><p>в читальный зал</p></div>',
                name: "&nbsp;"
            },
            bibl_1_4: {
                type: "5",
                text: '<span class="balloon_caption">Лестница вниз</span><br><div class="balloon_detail"><p>в абонемент</p></div>',
                name: "&nbsp;"
            }
        };
        $(".map-act .result-scheme rect").each(function(e, t) {
            var n;
            $(t).attr("id") && o[$(t).attr("id")] && (n = "".concat(o[$(t).attr("id")].text), $(t).popover({
                container: "body",
                content: n,
                html: !0,
                placement: "top"
            }))
        }), $("html").on("click", function(e) {
            void 0 === $(e.target).attr("data-original-title") ? $("[data-original-title]").popover("hide") : 0 <= $(e.target).attr("data-original-title").length && ($("[data-original-title]").popover("hide"), $(e.target).popover("show"))
        }), console.log("map + popovers ready!")
    })
}, function(e, t) {
    $(document).ready(function() {
        document.querySelector(".date-start-field") && $(".date-start-field").esdatepicker({
            position: "bottom center",
            autoClose: !0,
            classes: "date-start-calendar"
        }), document.querySelector(".date-end-field") && $(".date-end-field").esdatepicker({
            position: "bottom center",
            autoClose: !0,
            classes: "date-end-calendar"
        }), console.log("datepicker is ready!")
    })
}, function(e, t) {
    $(document).ready(function() {
        document.querySelector(".dropdown") && ($(".dropdown").on("shown.bs.dropdown", function(e) {
            e = e.target;
            $(".dropdown").addClass("noActive"), $(e).removeClass("noActive"), $(e).addClass("active")
        }), $(".dropdown").on("hidden.bs.dropdown", function(e) {
            $(".dropdown").removeClass("noActive"), $(".dropdown").removeClass("active")
        })), console.log("nav menu is ready!")
    })
}, function(e, t) {
    $(document).ready(function() {
        var e, t, n, o, a;
        document.querySelector(".button--mobile-menu") && (e = function() {
            a = !1 === a ? (t.addClass("active"), o.addClass("visible"), setTimeout(function() {
                o.addClass("active")
            }, 50), $("body").addClass("menu-overflov-hidden"), !0) : (t.removeClass("active"), o.removeClass("active"), setTimeout(function() {
                o.removeClass("visible")
            }, 400), $("body").removeClass("menu-overflov-hidden"), !1)
        }, t = $(".button--mobile-menu"), n = $(".nav-mob__close"), o = $(".full-menu"), a = !1, t.click(function() {
            e()
        }), n.click(function() {
            e()
        })), console.log("mob menu is ready!")
    })

}]);

$(document).ready(function() {
    if ($('.post iframe[src*="vkvideo.ru"]').length) {
        $('.post iframe[src*="vkvideo.ru"]').wrap("<div class='video-wrapper'></div>");
    }
    if ($(".partner-universities__page-wrapper--.univers>table").length){
        $(".partner-universities__page-wrapper--.univers>table").wrap("<div class='financial-table'></div>");
    }

});


/* End */
;
; /* Start:"a:4:{s:4:"full";s:69:"/local/templates/.default/markup/new/js/script_new.js?175889903412373";s:6:"source";s:53:"/local/templates/.default/markup/new/js/script_new.js";s:3:"min";s:0:"";s:3:"map";s:0:"";}"*/
$.browser = {
    msie: false,
    safari: false,
    mozilla: false,
    opera: false,
    version: 0
};


$(function () {

   

    $('.js-phone-mask').inputmask("+7 (999) 999 99 99",{ "showMaskOnHover": false });


    if ($('input[name=how]').length != 0) {
        if ($(window).width() <= '1100') {
            $('input[name=how]').prop("disabled", true);
            $('input[name=how]').attr("disabled", true);
        }
        else {
            $('select[name=how]').prop("disabled", true);
            $('select[name=how]').attr("disabled", true);
        }
        $(window).resize(function() {
            if ($(window).width() <= '1100') {
                $('input[name=how]').prop("disabled", true);
                $('input[name=how]').attr("disabled", true);
                $('select[name=how]').prop("disabled", false);
                $('select[name=how]').attr("disabled", false);
            }
            else {
                $('select[name=how]').prop("disabled", true);
                $('select[name=how]').attr("disabled", true);
                $('input[name=how]').prop("disabled", false);
                $('input[name=how]').attr("disabled", false);
            }
        });
    }


    if ($('input[name=alphabet]').length != 0) {
        $('input[name=alphabet]').change(function(){
            $('.letter-selector .letter').removeClass('active');
            $(this).parent().parent().addClass('active');
        });
        $('.letter-selector .letter').click(function(e){
            if ($(this).hasClass('active')) {
                e.preventDefault();
                $('.letter-selector .letter').removeClass('active');
                $('input[name=alphabet]').prop("checked", false);
                $('input[name=alphabet]').attr("checked", false);
            }
        });
        $('.selected-alphabet-new').click(function(){
            $('#letterSelectorMobButton').text($(this).text());
            console.log($('input[name=alphabet][value='+$(this).data('value')+']').length);
            $('input[name=alphabet][value='+$(this).data('value')+']').prop("checked", true);
            $('input[name=alphabet][value='+$(this).data('value')+']').attr("checked", true);
            $('input[name=alphabet][value='+$(this).data('value')+']').change();
        });
        $(document).on('change', 'select[name=department]', function(){
            $('.letter-selector .letter').removeClass('active');
            $('input[name=alphabet]').prop("checked", false);
            $('input[name=alphabet]').attr("checked", false);
        });
    }


    $('.collapse .accord-ion').click(function(){
        $(this).toggleClass('open');
        $(this).next('.accord-ion-panel').slideToggle("slow");
    });


    var floors = [];
    $("select.bs-select-map-flor option:not(.default)").each(function() {
        let selected = $(this).prop('selected');
        let value = $(this).val();
        let text = $(this).text();
        let building = $(this).data('building');
        floors.push({selected: selected, value: value, text: text, building: building});
    });
    $("select.bs-select-map-flor option:not(.default)").remove();
    for (let i = 0; i < floors.length; i++) {
        if (floors[i]['building'] == $("select.bs-select-map-corp option:selected").val()) {
            $("select.bs-select-map-flor").append('<option ' + (floors[i]['selected'] == true ? 'selected' : '') + ' data-building="' + floors[i]['building'] + '" class="b_' + floors[i]['building'] + '" value="' + floors[i]['value'] + '">' + floors[i]['text'] + '</option>');
        }
    }
    $("select.bs-select-map-corp").change(function(){
        $("select.bs-select-map-flor option:not(.default)").remove();
        console.log($("select.bs-select-map-corp option:selected").val());
        for (let i = 0; i < floors.length; i++) {
            console.log(floors[i]['building'] + floors[i]['text']);
            if (floors[i]['building'] == $("select.bs-select-map-corp option:selected").val()) {
                $("select.bs-select-map-flor").append('<option ' + (floors[i]['selected'] == true ? 'selected' : '') + ' data-building="' + floors[i]['building'] + '" class="b_' + floors[i]['building'] + '" value="' + floors[i]['value'] + '">' + floors[i]['text'] + '</option>');
            }
        }
        $(".bs-select").selectpicker("refresh");
    });


    $(document).on("click", ".nav__search-link", function(e){
        $(".nav_search-form input[name=q]").focus();
    });

    $(".post > table, .post p > table, .post div > table").wrap("<div class='table_wrapper_new'></div>");

    /*$(".projects__wrapper").slick({
        dots: !0,
        slidesToShow: 1,
        slidesToScroll: 1,
        arrows: !0,
        infinite: !0,
        variableWidth: !0,
        prevArrow: document.querySelector(".applicants__prev-button"),
        nextArrow: document.querySelector(".applicants__next-button"),
        lazyLoad: "ondemand",
        appendDots: $(".applicants__sl-dots"),
        customPaging: function (e, n) {
            var t = n + 1, o = e.slideCount;
            return '<span class="dot" role="button" title="'.concat(t, " of ").concat(o, '"></span>')
        }
    });*/

    if ($('.photograpfer_new').length != 0) {
        $('.foto-gallery .author').text($('.photograpfer_new').text());
    }

    if ($('.library-preamble .grid-4').length != 0) {
        $('.library-preamble').addClass('hide-before');
    }


    $(document).on('change','.submit-active-event', function() {
        $(this).closest('form').submit();
    });
    $(document).on('click','.button-search-event-only', function(e) {
        e.preventDefault();
        $(this).closest('form').find('input:not(input[name=search])').prop("disabled", true);
        $(this).closest('form').find('input:not(input[name=search])').attr("disabled", true);
        $(this).closest('form').find('select').prop("disabled", true);
        $(this).closest('form').find('select').attr("disabled", true);
        $(this).closest('form').submit();
    });
    $(document).on('change','.department-event-only', function(e) {
        e.preventDefault();
        $(this).closest('form').find('input').prop("disabled", true);
        $(this).closest('form').find('input').attr("disabled", true);
        $(this).closest('form').find('select:not(select[name=department])').prop("disabled", true);
        $(this).closest('form').find('select:not(select[name=department])').attr("disabled", true);
        $(this).closest('form').submit();
    });
    $(document).on('change','.alphabet-event-only', function(e) {
        e.preventDefault();
        $(this).closest('form').find('input:not(input[name=alphabet])').prop("disabled", true);
        $(this).closest('form').find('input:not(input[name=alphabet])').attr("disabled", true);
        $(this).closest('form').find('select').prop("disabled", true);
        $(this).closest('form').find('select').attr("disabled", true);
        $(this).closest('form').submit();
    });


    $(document).on('click','.news-navigation-tabs .link, .news-navigation-tabs .dropdown-item', function(e) {
        e.preventDefault();
        var href = $(this).attr('href');
        var $tab = $(href);

        $(".news-navigation .link").removeClass("active");
        $(".news-navigation .dropdown-item").removeClass("active");
        $(".news-navigation .link[href=" + href + "]").addClass("active");
        $(".news-navigation .dropdown-item[href=" + href + "]").addClass("active");
        $("#submenuMobDropdownButton").text($(this).text());

        $('.news-content-container').hide();
        $tab.show().css('display', 'flex');

				// try scroll caro to first slide
				$tab.find(".slick-slider-activation .slick-slider-container").slick('slickGoTo', 0, true);
            });

    var loadedCount = 1;
    $(document).on('click','.button--show-more', function(e) {
        e.preventDefault();
        let button = $(this);
        let link = $(this).data('src');
        let limit = $(this).data("size");
        $.ajax({
            url: link.replace("{{page_no}}", ++loadedCount) ,
            beforeSend: function(){
                button.css({'cursor':'wait'});
                button.prop("disabled", true);
            },
            success: function(html) {
                let elements = $(html).find('.ajax-load-card');
                button.css({'cursor':'pointer'});
                button.prop("disabled", false);
                $('.ajax-load-container').append(elements);
                if (loadedCount >= limit) button.remove();
            }
        });
    });

    if($('.foto-galleries .small-container').length > 0){
        $('.foto-galleries .small-container').each(function(idx, slider){
            $(slider).addClass('small-container-'+idx);

            var a = $(slider).closest('.foto-galleries').find('.foto-gallery__status');
            $(slider).closest('.foto-galleries').find('.foto-gallery__author'); 
            $(slider).find('.foto-galleries .small-container');
            $('.foto-galleries .small-container-' + idx).on("init reInit afterChange", function (e, n, t, o) {
                t = (t || 0) + 1;
                a.text("".concat(t, "/").concat(n.slideCount));
            });

            $('.foto-galleries .small-container-' + idx).on("afterChange", function (slick, currentSlide) {
                var curSlide = currentSlide.currentSlide + 1;
                if(curSlide == currentSlide.slideCount){
                   
                    $('.foto-galleries .small-container-' + idx).slick('slickGoTo', 0)
                }
                console.log('currentSlide '+currentSlide.currentSlide);  
                console.log('direction '+currentSlide.direction); 
                console.log('direction '+currentSlide.direction); 
                console.log(currentSlide); 
            });




            $('.foto-galleries .small-container-' + idx).slick({
                dots: false,
                slidesToShow: 1,
                slidesToScroll: 1,
                arrows: true,
                infinite: false,
                variableWidth: true,

                lazyLoad: "ondemand",
                focusOnSelect: false,
                prevArrow: $(slider).closest('.foto-galleries').find('.foto-gallery__prev'),
                nextArrow: $(slider).closest('.foto-galleries').find('.foto-gallery__next'),
            });
            setTimeout(function(){
                $('.foto-galleries .small-container-' + idx).magnificPopup({
                type: 'image',
                delegate: 'a',
                gallery: {
                    enabled: true,
                    tCounter: '%curr% / %total%',
                    
                },
                image: {
                    titleSrc: function(item) {
                        return item.el.find('img').attr('alt');
                    }
                },
                callbacks: {
                    open: function() {

                    },
                    beforeClose: function() {

                    }
                }
            });        
            },1);
            
        }); 
    }
/*
	$(document).on('change','form.vote-form input.vote_input', function(e) {
	    $(this).closest('form').find('input.vote_result').trigger('click');
	})
*/

    if ($('.nice-select').length != 0) {
        $('.nice-select').niceSelect();    

        $(".js-form-btn-reset").click(function() {

            $("#VoteQuestionsForm")[0].reset();
            $('.nice-select').niceSelect('update'); 

        });
    }

	if($('.expertise .voting-form-box').length == 0) {
		$('.expertise .voting__container').remove();
		$('.expertise .expertise__container').addClass('expertise_full-container');
	}

    
    if ($('.js-mgimo-voting__answer__show-link').length != 0) {    
        $('.js-mgimo-voting__answer__show-link').click(function() {
            $(this).toggleClass('show');
            $(this).parents('.vote-answers-list').find('.js-mgimo-voting__answer__show-list').toggleClass('show'); 
        });
    }
});
document.addEventListener('DOMContentLoaded', function() {
  if (window.location.href.includes('st-council')) {
    var wrapper = document.querySelector('.main-wrapper');
    if (wrapper) {
      wrapper.classList.add('main-wrapper__st-council');
    }
  }
});

/* End */
;; /* /local/templates/.default/markup/new/js/jquery-3.3.1.min.js?164206818586927*/
; /* /local/templates/.default/markup/new/js/jquery-migrate-3.3.1.min.js?159957690710976*/
; /* /local/templates/.default/markup/new/js/popper.min.js?164206818521257*/
; /* /local/templates/.default/markup/new/js/bootstrap.min.js?164206817860010*/
; /* /local/templates/.default/markup/new/js/bootstrap-select.min.js?164872532653644*/
; /* /local/templates/.default/markup/new/js/slick.min.js?164206818542863*/
; /* /local/templates/.default/markup/new/js/datepicker.js?164206818467752*/
; /* /local/templates/.default/markup/new/js/jquery.inputmask.min.js?1642060760111004*/
; /* /local/templates/.default/litera/litera.js?15384858033738*/
; /* /local/templates/.default/markup/new/js/social-likes.min.js?16479555399897*/
; /* /local/templates/.default/markup/new/js/special.js?17017833844701*/
; /* /local/templates/.default/markup/new/js/jquery.fancybox.min.js?158459752468253*/
; /* /local/templates/.default/markup/new/js/jquery.magnific-popup.min.js?165003251820216*/
; /* /local/templates/.default/markup/new/js/main_new.js?174418421323131*/
; /* /local/templates/.default/markup/new/js/script_new.js?175889903412373*/

//# sourceMappingURL=template_06e5f11f299189cd60d536df97f5e167.map.js