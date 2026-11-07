"""WCAG 2.x. Run from any directory; outputs every used text/background pair."""
import re,pathlib
css=pathlib.Path(__file__).with_name('tokens.css').read_text(encoding='utf-8')
light=css.split('@media')[0];dark=css.split(':root[data-theme=dark]')[1]
def luminance(hexcode):
 rgb=[int(hexcode[i:i+2],16)/255 for i in (1,3,5)]
 linear=[v/12.92 if v<=.04045 else ((v+.055)/1.055)**2.4 for v in rgb]
 return sum(v*w for v,w in zip(linear,(.2126,.7152,.0722)))
pairs=[(t,b) for t in ('text','text-2','text-3','primary') for b in ('bg','surface','surface-2')]
pairs += [('on-primary',b) for b in ('primary','primary-hover','primary-pressed')]
pairs += [(s+'-text',s+'-bg') for s in ('success','warning','danger','primary-soft')]
pairs += [(s+'-text','surface') for s in ('success','warning','danger')]
pairs += [('primary-soft-text','surface'),('text','primary-soft-bg'),('text-2','primary-soft-bg'),('bg','text'),('primary','primary-soft-bg')]
failed=0
print('WCAG: c=sRGB/255; c_lin=c/12.92 if c<=0.04045 else ((c+0.055)/1.055)^2.4; L=0.2126R+0.7152G+0.0722B; CR=(Lmax+0.05)/(Lmin+0.05). All text threshold 4.5:1 (including large).')
for theme,source in [('light',light),('dark',dark)]:
 colors=dict(re.findall(r'--([\w-]+):\s*(#[0-9A-Fa-f]{6})',source))
 for fg,bg in pairs:
  a,b=luminance(colors[fg]),luminance(colors[bg]);ratio=(max(a,b)+.05)/(min(a,b)+.05);ok=ratio>=4.5;failed+=not ok
  print(f'{theme} --{fg} {colors[fg]} / --{bg} {colors[bg]}: L={a:.6f}/{b:.6f}; ({max(a,b):.6f}+.05)/({min(a,b):.6f}+.05) = {ratio:.3f}:1 {"PASS" if ok else "FAIL"}')
print(f'FAIL: {failed}')
raise SystemExit(bool(failed))
