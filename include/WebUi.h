#pragma once
#include <Arduino.h>
static const char WEB_UI[] PROGMEM = R"HTML(<!DOCTYPE html>

<html lang="pt"><head><meta charset="utf-8"/><meta content="width=device-width,initial-scale=1" name="viewport"/><title>MySmartHouse · Sensor KNX</title>
<style>:root{color-scheme:dark;--bg:#111820;--panel:#1b2631;--line:#344351;--accent:#78d9c0}*{box-sizing:border-box}body{margin:0 auto;padding:24px;max-width:1050px;font:16px system-ui;background:var(--bg);color:#e8eef5}h1{font-size:28px;margin-bottom:6px}h2{font-size:20px}p,small{color:#adbdcd}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:12px}.card,section{background:var(--panel);border:1px solid var(--line);border-radius:12px;padding:18px;margin:16px 0}.value{font-size:28px;font-weight:600}label{display:block;font-size:14px;margin:10px 0}input,select,button{font:inherit;border-radius:6px;border:1px solid var(--line);padding:9px;background:#101922;color:inherit;width:100%}input[type=checkbox]{width:auto;margin-right:8px}button{background:var(--accent);color:#10251f;cursor:pointer;font-weight:600;width:auto}button:disabled{opacity:.5}summary{cursor:pointer;font-weight:600}pre{white-space:pre-wrap;overflow-wrap:anywhere;font-size:13px}#msg{position:sticky;top:0;background:#253e43;padding:10px;border-radius:6px;display:none}a{color:var(--accent)}fieldset{border:1px solid var(--line);border-radius:8px;margin:12px 0}footer{color:#adbdcd;font-size:13px}input:focus,button:focus{outline:2px solid var(--accent)}.hydronic{display:flex;align-items:center;gap:12px;overflow-x:auto}.hydronic .pump{min-width:135px;max-width:170px;text-align:center;border:4px solid #8a959c;background:#12181d}.manifold{margin:16px 0;overflow-x:auto}.manifold-bar{background:linear-gradient(#d8dde1,#8a959c,#dce2e6);color:#18242c;border:2px solid #aeb8bf;border-radius:12px;padding:11px 18px;font-weight:700;min-width:760px}.branches{display:grid;grid-template-columns:repeat(8,minmax(90px,1fr));gap:7px;min-width:760px;padding:8px 0}.branch{text-align:center;background:#17232d;border:1px solid var(--line);border-radius:10px;padding:9px 4px;min-height:112px}.pipe{width:4px;height:15px;margin:3px auto;background:#b99256}.branch strong{display:block;font-size:13px}.branch small{display:block}.branch.on{border-color:#78d9c0;background:#20483e}.branch.unused{opacity:.5}.state-dot.on{background:#68ef9a;box-shadow:0 0 9px #68ef9a}.state-dot.off{background:#283a32}.state-dot.pending{background:#ffcd66}.state-dot{display:inline-block;width:9px;height:9px;border-radius:50%;background:#89949c;margin-right:4px}.pump{margin:8px 0 12px;border:1px solid var(--line);border-radius:8px;padding:10px;display:inline-block}.plan-fields{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:12px}.plan-fields fieldset{margin:0}.plan-fields select{width:100%}[hidden]{display:none!important}.topnav{display:flex;gap:8px;margin:16px 0}.topnav button{background:#263c48;color:#e8eef5}.topnav button.selected{background:var(--accent);color:#10251f}#rooms{display:grid;grid-template-columns:repeat(5,minmax(0,1fr));gap:12px}#rooms .room{min-width:0;overflow-wrap:anywhere}@media(max-width:760px){#rooms{grid-template-columns:repeat(3,minmax(0,1fr))}}@media(max-width:520px){#rooms{grid-template-columns:repeat(2,minmax(0,1fr))}}@media(max-width:360px){#rooms{grid-template-columns:minmax(0,1fr)}}.power-toggle{display:flex;align-items:center;justify-content:center;flex-direction:column;gap:8px;width:64px;height:64px;border-radius:16px;margin:12px auto;background:#626a73;color:white;border:3px solid #a4abb2;box-shadow:0 3px 7px #0008}.power-toggle.power-on{background:#d42d39;border-color:#ff7d85}.power-toggle svg{width:30px;height:30px}.room-temperature{display:block;font-size:34px;line-height:1.15;font-weight:700;color:#e8eef5;margin:14px 0 8px}.room-detail{margin:5px 0;font-size:14px}.room-auto{padding:4px 8px;font-size:12px;line-height:1.2;min-height:26px}.room{text-align:center;border:1px solid var(--line);border-radius:9px;padding:12px;margin:8px 0}.room h3{margin:0 0 9px}.room-circuits{display:flex;flex-wrap:wrap;gap:8px}.room-circuit{background:#101922;border:1px solid var(--line);border-radius:7px;padding:9px;min-width:170px}.room-circuit button{display:block;margin-top:8px}.page-header{display:flex;align-items:center;justify-content:space-between;gap:16px;margin:0 0 20px}.page-header h1{margin:0}.settings-icon{display:grid;place-items:center;width:42px;height:42px;padding:7px;border:1px solid var(--line);border-radius:12px;background:#263c48;color:#e8eef5}.settings-icon svg{width:23px;height:23px;stroke:currentColor;fill:none;stroke-width:1.8}.top-tiles{grid-template-columns:repeat(4,minmax(0,1fr))}.top-tiles .card{margin:0}.device-meta{margin:30px 0 10px;padding-top:16px;border-top:1px solid var(--line)}.login-panel{max-width:410px;margin:24px auto}.login-panel input{margin:5px 0 12px}.login-panel button{width:100%}
#rooms{grid-template-columns:repeat(5,minmax(0,1fr));gap:10px}.thermostat{--ring:#69d8c3;--ring-rgb:105,216,195;min-width:0;padding:12px;border:1px solid #344854;border-radius:20px;background:radial-gradient(circle at 50% 32%,rgba(var(--ring-rgb),.09),transparent 45%),linear-gradient(145deg,#1e2e39,#132029 75%);box-shadow:0 16px 35px #0005}.thermostat[data-demand="true"]{--ring:#ff705c;--ring-rgb:255,112,92;border-color:#ff705c80}.thermostat[data-powered="false"]{--ring:#71808a;--ring-rgb:113,128,138;opacity:.82}.thermostat .header,.thermostat .tile-footer,.thermostat .setpoint-controls{display:flex;align-items:center}.thermostat .header{justify-content:space-between;gap:5px}.thermostat h3{margin:0;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;font-size:.88rem}.thermostat .status{display:inline-flex;align-items:center;gap:5px;color:#9dafbb;font-size:.58rem;font-weight:700;white-space:nowrap}.thermostat .status::before{width:7px;height:7px;border-radius:50%;background:var(--ring);box-shadow:0 0 10px var(--ring);content:""}.thermostat .dial{position:relative;display:grid;width:min(100%,124px);aspect-ratio:1;margin:13px auto;place-items:center;border-radius:50%;background:radial-gradient(circle,transparent 65%,#091118 66% 72%,transparent 73%),conic-gradient(from 220deg,var(--ring) 0 72%,#344550 72% 86%,transparent 86%);filter:drop-shadow(0 11px 18px #0007)}.thermostat .dial::before{position:absolute;inset:9px;border:1px solid #9cb5c43b;border-radius:50%;background:radial-gradient(circle at 42% 30%,#2b3b46,#17242d 55%,#101a21);box-shadow:inset 0 0 28px #0007,0 0 20px rgba(var(--ring-rgb),.14);content:""}.thermostat .dial-content{position:relative;z-index:1;display:grid;place-items:center;text-align:center}.thermostat .label{color:#9dafbb;font-size:.58rem;font-weight:750;letter-spacing:.1em;text-transform:uppercase}.thermostat .current{margin:2px 0;font-size:1.95rem;font-weight:500;line-height:1;letter-spacing:-.05em;font-variant-numeric:tabular-nums}.thermostat .current small{margin-left:2px;color:#9dafbb;font-size:.65rem;font-weight:700}.thermostat .heat-state{display:flex;align-items:center;gap:3px;min-height:16px;color:var(--ring);font-size:.58rem;font-weight:750}.thermostat .flame{width:11px;fill:currentColor}.thermostat .setpoint{margin:0;padding:9px 6px 7px;border:1px solid #536c7a80;border-radius:15px;background:#070d124d;text-align:center}.thermostat .setpoint-controls{justify-content:space-between;gap:4px}.thermostat .setpoint-value{display:grid;min-width:0;place-items:center}.thermostat .setpoint-value output{font-size:1.04rem;font-weight:780;font-variant-numeric:tabular-nums;white-space:nowrap}.thermostat .step,.thermostat .power,.thermostat .mode{border:1px solid #405461;color:#f4f7f9;cursor:pointer;transition:.15s ease}.thermostat .step{display:grid;width:36px;height:36px;flex:0 0 36px;place-items:center;padding:0;border-radius:50%;background:#293944;font-size:1.4rem;line-height:1}.thermostat .step:hover,.thermostat .step:focus-visible{border-color:var(--ring);background:rgba(var(--ring-rgb),.14);color:var(--ring)}.thermostat .pending{min-height:14px;margin:5px 0 -2px;color:#9dafbb;font-size:.59rem}.thermostat .pending[data-visible="true"]::before{display:inline-block;width:7px;height:7px;margin-right:6px;border:1px solid currentColor;border-right-color:transparent;border-radius:50%;content:"";animation:spin .7s linear infinite}.thermostat .tile-footer{justify-content:space-between;gap:7px;margin-top:10px}.thermostat .power{display:grid;width:36px;height:36px;flex:0 0 36px;place-items:center;padding:0;border-radius:12px;background:#273640;color:#82929c}.thermostat .power svg{width:19px;fill:none;stroke:currentColor;stroke-linecap:round;stroke-width:1.9}.thermostat .power[aria-pressed="true"]{border-color:rgba(var(--ring-rgb),.55);background:rgba(var(--ring-rgb),.12);color:var(--ring)}.thermostat .mode{min-height:36px;flex:1;padding:4px;border-color:#69d8c36b;border-radius:12px;background:#69d8c31a;color:#69d8c3;font-size:.69rem;font-weight:750}.thermostat .mode[data-mode="fault"]{border-color:#536978;background:#263640;color:#bdcad2}.thermostat .mode[data-mode="manual"]{border-color:#536978;background:#263640;color:#bdcad2}.thermostat button:active{transform:scale(.94)}@keyframes spin{to{transform:rotate(360deg)}}@media(max-width:850px){#rooms{grid-template-columns:repeat(3,minmax(0,1fr))}}@media(max-width:550px){#rooms,.top-tiles{grid-template-columns:repeat(2,minmax(0,1fr))}}@media(max-width:350px){#rooms{grid-template-columns:1fr}}@media(prefers-reduced-motion:reduce){.thermostat,.thermostat button,.thermostat .pending::before{animation:none!important;transition:none!important}}

/* Collector inspired by the supplied mockup; all states come from KNX feedback. */
.collector-card{--collector-active:#69d8c3;--collector-warning:#efb35a;--collector-off:#657681;width:100%;min-width:0;margin:18px 0;padding:18px;border:1px solid #344854;border-radius:22px;background:radial-gradient(circle at 17% 42%,#69d8c30c,transparent 22rem),linear-gradient(145deg,#1d2d38,#121f28 75%);box-shadow:0 22px 48px #0005}
.collector-header,.collector-title-group,.collector-system,.collector-rail,.collector-footer,.collector-legend{display:flex;align-items:center}.collector-header,.collector-footer{justify-content:space-between;gap:12px}.collector-header{margin-bottom:16px}.collector-title-group{gap:10px;min-width:0}.collector-title-icon{display:grid;width:39px;height:39px;flex:0 0 39px;place-items:center;border:1px solid #526875;border-radius:12px;background:#263742;color:var(--collector-active)}.collector-title-icon svg{width:22px;fill:none;stroke:currentColor;stroke-linecap:round;stroke-width:1.8}.collector-title-group h2{margin:0;font-size:1rem}.collector-subtitle{margin:3px 0 0;color:#9cafbb;font-size:.69rem}.collector-summary{display:inline-flex;align-items:center;gap:7px;min-height:31px;padding:0 11px;border:1px solid #69d8c35e;border-radius:999px;background:#69d8c314;color:var(--collector-active);font-size:.7rem;font-weight:750;white-space:nowrap}.collector-summary::before{width:7px;height:7px;border-radius:50%;background:currentColor;box-shadow:0 0 9px currentColor;content:""}.collector-system{display:grid;grid-template-columns:138px minmax(0,1fr);gap:12px;min-width:0}.collector-pump{--state:var(--collector-active);--state-rgb:105,216,195;display:grid;min-height:245px;padding:14px 9px 12px;place-items:center;align-content:center;border:1px solid #3d515e;border-radius:18px;background:radial-gradient(circle at 50% 37%,rgba(var(--state-rgb),.1),transparent 54%),#132029;text-align:center}.collector-pump[data-state="off"],.collector-pump[data-state="unknown"]{--state:#72818b;--state-rgb:114,129,139}.collector-pump[data-state="pending"],.collector-pump[data-state="blocked"]{--state:var(--collector-warning);--state-rgb:239,179,90}.collector-pump-graphic{width:108px;height:108px;padding:8px;border:1px solid rgba(var(--state-rgb),.55);border-radius:29px;background:linear-gradient(145deg,#3b4d57,#111a20 70%);color:var(--state);box-shadow:inset 0 1px 1px #fff3,0 14px 26px #0007,0 0 20px rgba(var(--state-rgb),.15)}.collector-pump-shell{display:grid;width:90px;height:90px;place-items:center;border-radius:50%;background:radial-gradient(circle at 35% 25%,#687983,#354550 39%,#11191e 76%);box-shadow:inset 0 0 0 4px #b5c5cd2b,inset 0 0 22px #000b,0 0 0 2px #0a1014}.collector-pump-shell svg{width:80px;filter:drop-shadow(0 3px 5px #0009)}.collector-pump-inner{fill:#162129;stroke:currentColor;stroke-width:2}.collector-pump-rotor{fill:currentColor;opacity:.84;transform-origin:50px 50px}.collector-pump-hub{fill:#334650;stroke:#b7c6cd;stroke-width:1.4}.collector-pump-led{fill:currentColor;filter:drop-shadow(0 0 5px currentColor)}.collector-pump[data-state="running"] .collector-pump-rotor{animation:collector-spin 1.2s linear infinite}.collector-pump[data-state="pending"] .collector-pump-graphic{animation:collector-pulse .9s ease-in-out infinite}.collector-pump-name{margin-top:10px;font-size:.76rem}.collector-pump-status{display:flex;align-items:center;gap:5px;margin-top:3px;color:var(--state);font-size:.7rem;font-weight:750}.collector-pump-status::before{width:7px;height:7px;border-radius:50%;background:currentColor;box-shadow:0 0 8px currentColor;content:""}.collector-pump-detail{min-height:26px;margin-top:5px;color:#9cafbb;font-size:.6rem;line-height:1.35}.collector-manifold{min-width:0;max-width:100%}.collector-rail{--rail:#fff;position:relative;height:45px;justify-content:space-between;gap:6px;padding:0 10px 0 16px;overflow:hidden;border:1px solid #536773;border-radius:14px;background:linear-gradient(180deg,#42535e,#263640 45%,#19252d 100%);box-shadow:inset 0 1px 1px #fff3,inset 0 -8px 14px #0005,0 7px 14px #0003}.collector-rail::before{position:absolute;inset:0 auto 0 0;width:6px;background:var(--rail);box-shadow:0 0 15px var(--rail);content:""}.collector-supply{--rail:#ff715b}.collector-return{--rail:#4db9e7}.collector-rail-label{position:relative;z-index:1;display:flex;align-items:center;gap:5px;min-width:0;font-size:.72rem;font-weight:720}.collector-flow-arrow{color:var(--rail);font-size:1rem}.collector-temperature{position:relative;z-index:1;padding:5px 7px;border:1px solid #657884;border-radius:8px;background:#111c23b8;color:#c3d0d7;font-size:.65rem;white-space:nowrap}.collector-circuits{display:grid;height:154px;grid-template-columns:repeat(8,minmax(0,1fr));gap:2px;min-width:0;padding:0 2px}.collector-circuit{--circuit:var(--collector-off);position:relative;display:grid;min-width:0;padding:5px 0 6px;grid-template-rows:32px 1fr auto auto;justify-items:center;color:#f3f7f9;text-align:center}.collector-circuit[data-active="true"]{--circuit:var(--collector-active)}.collector-circuit[data-pending="true"]{--circuit:var(--collector-warning)}.collector-circuit[data-available="false"]{--circuit:#56636c;opacity:.42}.collector-valve{position:relative;z-index:2;display:grid;width:min(30px,90%);height:24px;place-items:center;border:1px solid #627681;border-radius:7px 7px 5px 5px;background:linear-gradient(145deg,#455963,#1a272f);color:var(--circuit);box-shadow:0 4px 8px #0005}.collector-valve::before{position:absolute;top:-5px;width:15px;height:5px;border-radius:3px 3px 0 0;background:#647681;content:""}.collector-valve-led{width:7px;height:7px;border-radius:50%;background:currentColor;box-shadow:0 0 8px currentColor}.collector-circuit[data-pending="true"] .collector-valve-led{animation:collector-blink .65s ease-in-out infinite}.collector-pipe{position:relative;z-index:1;width:6px;height:100%;min-height:53px;overflow:hidden;border:1px solid #0b1216;border-radius:4px;background:var(--circuit);box-shadow:0 0 8px color-mix(in srgb,var(--circuit),transparent 65%)}.collector-circuit[data-available="false"] .collector-pipe{width:4px;border:0;background:repeating-linear-gradient(to bottom,#65727b 0 6px,transparent 6px 11px);box-shadow:none}.collector-flow{position:absolute;inset:-18px 0 auto;height:20px;background:linear-gradient(transparent,#eafffa,transparent);opacity:0}.collector-circuit[data-active="true"] .collector-flow{opacity:.65;animation:collector-flow-down 1.35s linear infinite}.collector-circuit-id{margin-top:3px;font-size:.68rem;font-weight:800}.collector-room{max-width:100%;overflow:hidden;color:#9cafbb;font-size:.54rem;text-overflow:ellipsis;white-space:nowrap}.collector-footer{margin-top:13px;color:#9cafbb;font-size:.61rem}.collector-legend{gap:11px;flex-wrap:wrap}.collector-legend-item{display:inline-flex;align-items:center;gap:5px}.collector-legend-item::before{width:7px;height:7px;border-radius:50%;background:var(--legend);content:""}.collector-safety{white-space:nowrap;color:#c1ced5}@keyframes collector-spin{to{transform:rotate(360deg)}}@keyframes collector-pulse{50%{box-shadow:0 0 27px rgba(var(--state-rgb),.42)}}@keyframes collector-flow-down{to{transform:translateY(92px)}}@keyframes collector-blink{50%{opacity:.25}}@media(max-width:720px){.collector-header,.collector-footer{align-items:flex-start;flex-direction:column}.collector-system{grid-template-columns:minmax(0,1fr)}.collector-pump{min-height:unset;grid-template-columns:auto 1fr;grid-template-rows:auto auto auto;justify-items:start;column-gap:14px;text-align:left}.collector-pump-graphic{grid-row:1/4}.collector-pump-name{margin-top:0}}@media(max-width:400px){.collector-card{padding:10px}.collector-system{gap:8px}.collector-circuits{gap:1px}.collector-rail-label{font-size:.61rem}.collector-temperature{font-size:.58rem;padding:4px}.collector-room{font-size:.48rem}}@media(prefers-reduced-motion:reduce){.collector-pump-rotor,.collector-pump-graphic,.collector-valve-led,.collector-flow{animation:none!important}}

.ntp-row{display:grid;grid-template-columns:1fr 1fr;align-items:center;gap:20px;padding:14px 0;border-bottom:1px solid var(--orbit-line,#293747)}.ntp-row>input,.ntp-row>select{width:100%}.ntp-summary{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:12px;margin:20px 0}.ntp-summary>div{border:1px solid var(--orbit-line,#293747);border-radius:12px;padding:14px}.ntp-summary small,.ntp-summary b{display:block}.ntp-summary small{color:var(--orbit-muted,#9eafc2);font-size:12px;margin-bottom:6px}.ntp-summary b{font-size:13px;overflow-wrap:anywhere}.ntp-panel .ntp-hint{font-size:12px;color:var(--orbit-muted,#9eafc2)}@media(max-width:600px){.ntp-row{grid-template-columns:1fr;gap:7px}.ntp-summary{grid-template-columns:1fr}}</style>
</style>
<style data-orbit-controls>
.orbit.light :is(.thermostat-hero,.collector-card,.room-thermostat,.thermostat){color:#e8eef3}
.orbit-power-area>#railPower{
 --rail-ring:var(--orbit-accent);--rail-fill:0%;
 position:relative;display:grid;place-items:center;flex:none;
 width:118px;height:118px;min-height:0;margin:0 auto;padding:0;
 border:0;border-radius:50%;background:var(--orbit-bg);color:var(--orbit-ink);
 box-shadow:0 0 0 7px color-mix(in srgb,var(--rail-ring),transparent 94%);
 touch-action:none;user-select:none;-webkit-touch-callout:none;
}
#railPower[data-state=warning]{--rail-ring:var(--orange)}
#railPower[data-state=critical]{--rail-ring:var(--red)}
#railPower:is([data-state=off],[data-state=offline]){--rail-ring:var(--orbit-muted)}
#railPower[data-holding],#railPower[data-pending]{--rail-ring:var(--orbit-accent)}
#railPower:is([data-holding],[data-pending]) .rail-power-ring{visibility:hidden}
#railPower[aria-disabled=true]{cursor:default}
#railPower :is(.rail-power-track,.rail-power-ring,.rail-power-progress){
 position:absolute;inset:0;border-radius:50%;pointer-events:none;
 mask:radial-gradient(farthest-side,transparent calc(100% - 6px),#000 calc(100% - 5px));
}
.rail-power-track{background:var(--orbit-line)}
.rail-power-ring{background:var(--rail-ring)}
#railPower[data-state=ok] .rail-power-ring{
 --dim:color-mix(in srgb,var(--rail-ring),transparent 62%);
 background:conic-gradient(var(--dim),var(--rail-ring) 32%,var(--dim) 67%);
 animation:rail-power-rotate 8s linear infinite;
}
#railPower[data-state=warning] .rail-power-ring{animation:rail-power-pulse 2.4s ease-in-out infinite}
#railPower[data-state=critical] .rail-power-ring{animation:rail-power-pulse .8s ease-in-out infinite}
#railPower:is([data-state=off],[data-state=offline]) .rail-power-ring{opacity:.4}
.rail-power-progress{background:conic-gradient(var(--rail-ring) var(--rail-fill),transparent var(--rail-fill))}
.rail-power-metrics:not([hidden]){display:grid;gap:3px}
.rail-power-metrics>span{display:flex;align-items:center;gap:7px;font-size:15px;font-weight:500;font-variant-numeric:tabular-nums}
.orbit-power-area .rail-power-icon{width:22px;height:22px;color:var(--orbit-muted)}
.rail-power-centre{max-width:76px;font-size:12px;line-height:1.2;text-align:center;overflow-wrap:anywhere}
.rail-power-centre[data-symbol]{color:var(--rail-ring);font-size:30px}
 .rail-power-caption{min-width:0;color:var(--orbit-muted);font-size:11px;line-height:1.35}
 .rail-power-caption strong{display:block;color:var(--orbit-ink);font-weight:600}
 .rail-power-status{display:block;color:var(--orbit-muted)}
 .rail-power-caption small{display:block;margin-top:2px;color:inherit;font-size:inherit}
 .rail-power-notice{display:block;margin-top:4px;overflow-wrap:anywhere}
 .rail-power-notice[data-kind=error]{color:var(--red)}
 .rail-power-notice[data-kind=pending]{color:var(--orange)}
 .rail-power-notice:empty{margin-top:0}
 @keyframes rail-power-rotate{to{transform:rotate(360deg)}}
 @keyframes rail-power-pulse{0%,100%{opacity:1}50%{opacity:.27}}
 @media(prefers-reduced-motion:reduce){
  #railPower[data-state] .rail-power-ring{animation:none}
 }
 .orbit-rail .orbit-theme{margin-top:auto;display:flex;justify-content:center;align-items:center;min-height:44px;padding:4px;border:0;background:transparent;color:var(--orbit-muted);cursor:pointer}
 .orbit-theme-track{display:grid;grid-template-columns:1fr 1fr;align-items:center;width:68px;height:32px;flex:none;border:1px solid var(--orbit-line);border-radius:99px;text-align:center;background:var(--orbit-bg)}
 .orbit-theme-track>span{border-radius:99px;font-size:18px;line-height:28px}
 .orbit-theme[aria-checked=false] .orbit-theme-track>span:first-child,.orbit-theme[aria-checked=true] .orbit-theme-track>span:last-child{background:var(--orbit-accent);color:var(--orbit-bg)}
 @media(max-width:760px){
  .orbit-power-area>#railPower{width:76px;height:76px;margin:0;box-shadow:none}
  #railPower :is(.rail-power-track,.rail-power-ring,.rail-power-progress){
   mask:radial-gradient(farthest-side,transparent calc(100% - 4px),#000 calc(100% - 3px));
  }
  .rail-power-metrics:not([hidden]){gap:2px}
  .rail-power-metrics>span{gap:3px;font-size:11px}
  .orbit-power-area .rail-power-icon{width:14px;height:14px}
  .rail-power-centre{max-width:57px;font-size:10px}
  .rail-power-centre[data-symbol]{font-size:24px}
  .orbit-theme-track{width:60px;height:30px}
  .orbit-theme-track>span{font-size:17px;line-height:26px}
 }
</style>
<style>
.settings-tabs{display:flex;gap:7px;flex-wrap:wrap;margin:18px 0}.settings-tabs button{background:#263640;color:#dce7ee;border-radius:12px;padding:10px 15px}.settings-tabs button[aria-selected="true"]{background:#69d8c3;color:#10251f}.tab-panel{min-width:0}.feature-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(240px,1fr));gap:12px}.feature-tile{margin:0;min-width:0}.feature-grid #collectorSettings,.feature-grid #thermostatSettings{grid-column:1/-1}.feature-tile h2{margin-top:0}.feature-tile p{line-height:1.45}.tab-panel fieldset{padding:14px}.tab-panel legend{font-weight:750;color:#dbe9ee}.tab-panel .grid{grid-template-columns:repeat(auto-fit,minmax(190px,1fr))}.settings-actions{position:sticky;bottom:0;z-index:3;display:flex;align-items:center;gap:12px;flex-wrap:wrap;background:#182833f5;border:1px solid #38515e;border-radius:14px;padding:10px 16px}.settings-actions p{margin:0;font-size:.77rem}.knx-console{max-height:265px;min-height:140px;overflow:auto;background:#0a151d;border:1px solid #334b58;border-radius:12px;padding:12px}.thermostat-hero{display:grid;place-items:center;gap:10px;padding:35px;background:radial-gradient(circle at center,#2a4a5155,transparent 26rem),#182630}.thermostat-hero .thermostat{width:min(100%,360px);padding:24px}.thermostat-hero .dial{width:190px}.thermostat-hero .current{font-size:2.7rem}.thermostat-hero .step{width:46px;height:46px;flex-basis:46px}.hint{font-size:.78rem;color:#a8bbc6}@media(max-width:600px){.settings-tabs{display:grid;grid-template-columns:repeat(2,minmax(0,1fr))}.settings-tabs button{width:100%}}
</style>
<style>
@media(max-width:520px){#rooms{gap:8px}#rooms .thermostat{padding:9px}#rooms .thermostat .setpoint{padding:8px 4px 6px}#rooms .thermostat .setpoint-controls{display:grid;grid-template-columns:30px minmax(0,1fr) 30px;gap:2px}#rooms .thermostat .step{width:30px;height:30px;flex:initial;font-size:1.1rem}#rooms .thermostat .setpoint-value{min-width:0;overflow:hidden}#rooms .thermostat .setpoint-value .label{font-size:.52rem;white-space:nowrap}#rooms .thermostat .setpoint-value output{font-size:clamp(.78rem,3.4vw,.95rem);letter-spacing:-.04em;white-space:nowrap}}
.pump-phase[data-phase="thermal-cold"] span{background:#f07168}.pump-phase[data-phase="thermal-return"] span{background:#ef9c5a}
</style>
<style>
.pump-phase{width:100%;max-width:125px;height:9px;margin:7px auto 0;overflow:hidden;border-radius:10px;background:#31424b}.pump-phase span{display:block;height:100%;width:0;transition:width .3s;background:#71808a}.pump-phase[data-phase="waiting"] span{background:#f07168}.pump-phase[data-phase="service"] span{background:#68d69c}.pump-phase[data-phase="pause"] span{background:#efb35a}.pump-phase-label{min-height:24px;margin-top:4px;color:#b9cbd3;font-size:.63rem}.collector-circuit[data-opening="true"] .collector-valve{border-color:#efb35a;box-shadow:0 0 12px #efb35a75}.collector-opening{align-self:end;min-height:12px;font-size:.54rem;color:#efb35a;white-space:nowrap}.collector-circuit[data-opening="true"] .collector-valve-led{color:#efb35a;animation:collector-blink .65s ease-in-out infinite}@media(max-width:720px){.collector-pump-graphic{grid-row:1/6}.pump-phase,.pump-phase-label{grid-column:2;margin-left:0;text-align:left}}
</style>
<style>.history-nav{display:flex;gap:8px;margin:10px 0 18px}.history-nav button{background:#263c48;color:#e8eef5}.history-card{overflow:hidden}.history-chart{width:100%;height:auto;display:block;background:#101c25;border-radius:12px}.history-legend{display:flex;gap:14px;flex-wrap:wrap;font-size:.82rem}.history-legend span::before{content:"";display:inline-block;width:11px;height:11px;margin-right:5px;background:var(--c);border-radius:50%}</style>
<style>.history-nav{align-items:center;flex-wrap:wrap}.nav-actions{display:flex;gap:8px;margin-left:auto}.nav-actions .settings-icon{flex:0 0 42px}.room-thermostat .header{position:relative;justify-content:center;min-height:22px}.room-thermostat .header h3{max-width:calc(100% - 26px);text-align:center}.room-thermostat .status{position:absolute;right:0;top:50%;transform:translateY(-50%);font-size:0;gap:0}.room-thermostat .status::before{width:9px;height:9px}.room-thermostat .dial{background:radial-gradient(circle,transparent 65%,#091118 66% 72%,transparent 73%),conic-gradient(from 220deg,var(--ring) 0 var(--ring-fill,0%),#344550 var(--ring-fill,0%) 86%,transparent 86%)}.room-thermostat .setpoint-controls{display:grid;grid-template-columns:1fr 1fr;justify-items:center;gap:5px}.room-thermostat .setpoint-value{grid-column:1/-1;grid-row:1}.room-thermostat .setpoint-value output{font-size:1.28rem}.room-thermostat .decrease{grid-column:1;grid-row:2}.room-thermostat .increase{grid-column:2;grid-row:2}@media(max-width:520px){#rooms .room-thermostat .setpoint-controls{display:grid;grid-template-columns:1fr 1fr;gap:4px}#rooms .room-thermostat .setpoint-value output{font-size:1.15rem}}</style>
<style>/* Shared Orbit surfaces. Heating and thermostat components keep their existing styles. */
:root{color-scheme:dark;--orbit-bg:#0b1119;--orbit-panel:#111b27;--orbit-raised:#182532;--orbit-line:#293a49;--orbit-ink:#e9f0f7;--orbit-muted:#a1b2c2;--orbit-accent:#61d5b0;--orange:#f0b374;--red:#ed9ba6;--soft-accent:#193a33;--bar-height:64px}
body.orbit.light{color-scheme:light;--orbit-bg:#f1f5f3;--orbit-panel:#ffffff;--orbit-raised:#eaf1ed;--orbit-line:#cedbd4;--orbit-ink:#192f28;--orbit-muted:#536b60;--orbit-accent:#18755f;--orange:#865306;--red:#b43d56;--soft-accent:#e1f2e9}
html{min-height:100%;background:var(--orbit-bg)}
body.orbit{--bg:var(--orbit-bg);--panel:var(--orbit-panel);--line:var(--orbit-line);--accent:var(--orbit-accent);margin:0;padding:0 24px 0 166px;max-width:none;min-height:100vh;background:var(--orbit-bg);color:var(--orbit-ink);font:14px/1.5 system-ui,sans-serif}
/* Preserve the operational card styles, including their pre-existing light variants. */
.orbit :is(#heating,#thermostatView){font-size:16px;line-height:normal;--panel:#1b2631;--line:#344351;--accent:#78d9c0;--orbit-bg:#111820;--orbit-raised:#20303c;--orbit-line:#344351;--orbit-ink:#e9f0f7;--orbit-muted:#9eafc2;--orbit-accent:#78d9c0}
.orbit.light :is(#heating,#thermostatView){--panel:#fff;--line:#bacfc9;--accent:#18755f;--orbit-bg:#edf4f2;--orbit-raised:#deede8;--orbit-line:#bacfc9;--orbit-ink:#18362f;--orbit-muted:#516b62;--orbit-accent:#18755f}
.orbit .shell{max-width:1380px;margin:0 auto;padding:24px 0;min-width:0}
.orbit-rail{position:fixed;inset:0 auto 0 0;width:142px;padding:24px 12px;background:var(--orbit-panel);border-right:1px solid var(--orbit-line);display:flex;flex-direction:column;gap:12px;z-index:30;overflow-y:auto}
.orbit-logo{text-align:center;color:var(--orbit-muted);letter-spacing:.22em;font-size:11px;font-weight:750;margin-bottom:12px}
.orbit-power-area{display:grid;justify-items:center;gap:12px;text-align:center}
.orbit-nav{display:grid;gap:4px}
.orbit-nav a{display:flex;align-items:center;gap:8px;min-height:44px;padding:8px;border:1px solid transparent;border-radius:8px;color:var(--orbit-muted);text-decoration:none;font-size:12px}
.orbit-nav a:is([aria-current=page],:hover){background:var(--orbit-raised);border-color:var(--orbit-line);color:var(--orbit-accent)}
.orbit-nav svg{width:18px;height:18px;fill:none;stroke:currentColor;stroke-width:1.4;flex:none}
.orbit .page-header{display:block;margin:0 0 24px}
.orbit .page-header h1{font-size:28px;line-height:1.2;letter-spacing:-.03em;overflow-wrap:anywhere}
.device-status{margin:6px 0 0;font-size:12px;color:var(--orbit-muted);overflow-wrap:anywhere}
.orbit-footer{display:flex;align-items:center;justify-content:space-between;gap:8px 16px;flex-wrap:wrap;border-top:1px solid var(--orbit-line);color:var(--orbit-muted);font-size:11px;padding:12px 0 0;margin:24px 0 0}
.orbit-footer a{display:inline-flex;align-items:center;min-height:44px;color:var(--orbit-muted);text-underline-offset:3px}
.orbit-footer a:is(:hover,[aria-current=page]){color:var(--orbit-accent)}
.orbit :is(#helpArea,#sensorArea,#historyArea,.orbit-config) h2{font-size:16px;margin:0 0 12px}
.orbit :is(#helpArea,#sensorArea,#historyArea,.orbit-config) :is(p,small,legend){color:var(--orbit-muted)}
.orbit :is(#sensorReadings .card,.orbit-config section){min-width:0}
.orbit-config section{border-radius:12px;padding:20px}
.orbit-config h3{font-size:14px}
.orbit-config label{font-size:13px;color:var(--orbit-muted)}
.orbit-config :is(input:not([type=checkbox]):not([type=range]),select,textarea){max-width:100%;min-height:44px;border:1px solid var(--orbit-line);border-radius:8px;padding:9px 12px;background:var(--orbit-bg);color:var(--orbit-ink);font:14px system-ui}
.orbit-config input[type=checkbox]{accent-color:var(--orbit-accent);width:18px;height:18px;vertical-align:middle}
.orbit-config button,#outputs button{min-height:44px;max-width:100%;border:1px solid var(--orbit-line);border-radius:8px;padding:9px 14px;background:var(--orbit-raised);color:var(--orbit-ink);font:600 13px system-ui}
.orbit-config button[type=submit]{background:var(--orbit-accent);color:var(--orbit-bg);border-color:transparent}
.orbit-config button:not([type=submit]):hover:not(:disabled),#outputs button:hover:not(:disabled){border-color:var(--orbit-accent)}
.orbit .settings-tabs{display:flex;flex-wrap:wrap;gap:6px;border-bottom:1px solid var(--orbit-line);padding-bottom:12px;margin:16px 0 20px}
.orbit .settings-tabs button{width:auto;background:transparent;color:var(--orbit-muted)}
.orbit .settings-tabs [aria-selected=true]{background:var(--soft-accent);color:var(--orbit-accent);border-color:var(--orbit-accent)}
.orbit .settings-actions{background:var(--orbit-panel);border-color:var(--orbit-line);border-radius:12px}
.orbit-config pre{max-height:320px;overflow:auto;background:var(--orbit-bg);border:1px solid var(--orbit-line);padding:12px;border-radius:8px}
.orbit :is(button,a,input,select,summary):focus-visible{outline:2px solid var(--orbit-accent);outline-offset:2px}
.orbit-config :is(.grid,.plan-fields,.feature-grid){gap:16px;grid-template-columns:repeat(auto-fit,minmax(min(100%,220px),1fr))}
.orbit-config fieldset{min-width:0}
.orbit-config input[type=file]{width:100%}
.orbit #msg{position:sticky;top:0;z-index:10;background:var(--orbit-raised);border:1px solid var(--orbit-line);color:var(--orbit-ink);overflow-wrap:anywhere}
.orbit .top-tiles .value{font-size:clamp(16px,2.35vw,28px);overflow-wrap:anywhere}
.history-plot{display:grid;grid-template-columns:48px minmax(0,1fr);align-items:start}
.history-axis{width:48px;height:232px}
.history-scroll{overflow-x:auto;overscroll-behavior-inline:contain}
.history-scroll .history-chart{max-width:none;height:232px;border-radius:0 8px 8px 0;background:var(--orbit-bg)}
.history-legend{margin:10px 0 22px;color:var(--orbit-muted)}
@media(max-width:760px){
 body.orbit{padding:0 14px calc(var(--bar-height) + 18px + env(safe-area-inset-bottom))}
 .orbit-rail{position:static;width:auto;padding:16px 0 0;border:0;background:none;display:grid;grid-template-columns:minmax(0,1fr) auto;align-items:start;gap:8px;overflow:visible}
 .orbit-logo{display:none}
 .orbit-power-area{grid-template-columns:auto minmax(0,1fr);align-items:center;justify-items:start;gap:10px;text-align:left}
 .orbit-rail .orbit-theme{margin-top:16px}
 .orbit-nav{position:fixed;inset:auto 0 0;z-index:30;display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:2px;min-height:var(--bar-height);padding:4px 6px calc(4px + env(safe-area-inset-bottom));border-top:1px solid var(--orbit-line);background:var(--orbit-panel)}
 .orbit-nav a{justify-content:center;flex-direction:column;gap:4px;padding:4px 1px;min-width:0;font-size:clamp(10px,2.85vw,12px);text-align:center}
 .orbit .shell{padding:20px 0 0}
 .orbit .page-header{margin-bottom:16px}
 .orbit .page-header h1{font-size:26px}
 .orbit-config section{padding:16px}
 .orbit .settings-actions{bottom:calc(var(--bar-height) + env(safe-area-inset-bottom))}
 .orbit .top-tiles .card{padding:12px}
 .orbit .top-tiles .value{font-size:clamp(16px,5vw,24px)}
 .orbit .settings-tabs button{padding:8px 10px;font-size:12px}
}
.orbit-actions{display:flex;flex-wrap:wrap;gap:10px;margin:0 0 14px}
.orbit-action{display:inline-flex;align-items:center;justify-content:center;gap:8px;min-height:44px;padding:9px 14px;border:1px solid var(--orbit-line);border-radius:11px;background:var(--orbit-raised);color:var(--orbit-ink);font:600 .9rem system-ui}
.orbit-action svg{width:19px;height:19px;fill:none;stroke:currentColor;stroke-width:2;stroke-linecap:round;stroke-linejoin:round}
.orbit-action:hover:not(:disabled){border-color:var(--orbit-accent)}.orbit-action:disabled{opacity:.5}
.orbit-switch-track{position:relative;width:36px;height:20px;border-radius:20px;background:var(--orbit-muted)}
.orbit-switch-track::after{content:"";position:absolute;top:3px;left:3px;width:14px;height:14px;border-radius:50%;background:var(--orbit-bg)}
.orbit-switch[aria-checked=true] .orbit-switch-track{background:var(--orbit-accent)}
.orbit-switch[aria-checked=true] .orbit-switch-track::after{transform:translateX(16px)}
.sensor-trend{font-size:.85em;margin-left:.35em;color:var(--orbit-muted)}.sensor-trend[data-direction=up]{color:#f29d70}.sensor-trend[data-direction=down]{color:#74c8ee}
.collector-pump-detail:empty{display:none}
</style></head><body class="orbit"><div class="orbit-rail"><div class="orbit-logo" aria-hidden="true">ORBIT</div><div class="orbit-power-area"><button type="button" id="railPower" class="rail-power" data-state="offline" aria-label="Comando geral · a aguardar estado" aria-disabled="true">—</button><div class="rail-power-caption"><strong>Comando geral</strong><span class="rail-power-status" id="railPowerStatus">A aguardar estado</span><small>Mantém premido 1,5 s</small><span class="rail-power-notice" id="railPowerNotice" role="status"></span></div></div>
<nav aria-label="Navegação principal" class="orbit-nav">
<a id="viewPanel" data-view="dashboard" href="/#painel"><svg aria-hidden="true" viewBox="0 0 24 24"><path d="M3 3h7v7H3z M14 3h7v7h-7z M3 14h7v7H3z M14 14h7v7h-7z"/></svg>Painel</a>
<a id="viewSensors" data-view="sensors" href="/#sensores"><svg aria-hidden="true" viewBox="0 0 24 24"><path d="M10 14V5a2 2 0 0 1 4 0v9a4 4 0 1 1-4 0z"/></svg>Sensores</a>
<a id="viewHistory" data-view="history" href="/#historico"><svg aria-hidden="true" viewBox="0 0 24 24"><path d="M3 20V10m6 10V6m6 14V12m6 8V3"/></svg>Histórico</a>
<a id="viewSettings" data-view="settings" href="/config.html"><svg aria-hidden="true" viewBox="0 0 24 24"><path d="M3 6h18M3 18h18M8 3v6m8 6v6"/></svg>Configuração</a></nav><button type="button" id="orbitTheme" class="orbit-theme" role="switch" aria-checked="false" aria-label="Tema claro" title="Alternar tema claro e escuro"><span class="orbit-theme-track" aria-hidden="true"><span>☾</span><span>☀</span></span></button></div>
<div class="shell"><header class="page-header"><h1 id="deviceName">Sensor KNX</h1><p class="device-status" id="connection">A ligar ao dispositivo…</p></header><div id="msg" role="status"></div><main hidden="" id="helpArea"><section><h2>Ajuda</h2><p>Ativa o controlo geral após confirmar o estado da bomba e dos circuitos.</p><p><strong>Automático:</strong> a divisão pede calor de acordo com a temperatura e o setpoint. Sem temperatura válida ou setpoint, deixa de pedir calor e fica cinzenta. Ao regressar uma leitura válida, o automático retoma se o controlo estiver ativo. Se estiver configurado, o BME serve de alternativa quando falta uma temperatura KNX válida.</p><p><strong>Manual:</strong> o botão de ligar/desligar aparece ao lado do modo. O pedido mantém-se até o desligares, mudares de modo ou o controlo parar por falha.</p><p><strong>Bomba e válvulas:</strong> o ESP envia o comando e aguarda o feedback KNX; a bomba espera o tempo de abertura e faz os ciclos de serviço e pausa definidos. Falhas KNX ou de feedback podem parar o controlo e exigir nova ativação.</p><p><strong>Proteção térmica:</strong> a bomba tem 5 minutos de circulação com água de entrada abaixo do mínimo. Se a água não atingir o limite nesse período, para primeiro a bomba e depois fecha os circuitos, aguarda a pausa configurada e tenta novamente, com uma nova janela de 5 minutos. Se a água de retorno se mantiver acima do limite durante o tempo definido, também pausa. A contagem aparece na barra da bomba. As sondas DS18B20 têm de estar configuradas e a enviar leituras recentes.</p><p><strong>Reiniciar:</strong> aguarda confirmação OFF das saídas. Se o controlo estava ativo, retoma no arranque após confirmar as saídas desligadas.</p><p><strong>Histórico:</strong> regista sensores e amostras do estado da bomba de 20 em 20 minutos. Não participa no controlo.</p><p id="webMcpStatus" role="status">A verificar ferramentas WebMCP…</p></section></main><main id="sensorArea" hidden><section><h2>Sensores</h2><div class="grid" id="sensorReadings"></div></section></main><main hidden id="historyArea"><section class="history-card">
<h2>Histórico</h2><p>Desliza para consultar as últimas 48 horas. Cada ecrã mostra cerca de duas horas. Leituras de 20 em 20 minutos, na hora local do navegador.</p>
<p id="historyMessage" role="status">A carregar…</p>
<h3>Temperaturas</h3><div class="history-plot"><svg class="history-axis" id="temperatureAxis" aria-hidden="true"></svg><div class="history-scroll" id="temperatureScroll" tabindex="0" role="region" aria-label="Histórico de temperaturas com deslocação horizontal"><svg aria-label="Temperaturas da ida, retorno e ambiente" class="history-chart" id="historyChart" role="img"></svg></div></div>
<div class="history-legend"><span style="--c:#ff705c">Sonda 1 · entrada</span><span style="--c:#4db9e7">Sonda 2 · retorno</span><span style="--c:#69d8c3">Temperatura ambiente</span><span style="--c:#d694f5">Bomba ligada · amostra</span></div>
<h3>Humidade</h3><div class="history-plot"><svg class="history-axis" id="humidityAxis" aria-hidden="true"></svg><div class="history-scroll" id="humidityScroll" tabindex="0" role="region" aria-label="Histórico de humidade com deslocação horizontal"><svg aria-label="Humidade relativa" class="history-chart" id="humidityChart" role="img"></svg></div></div>
<div class="history-legend"><span style="--c:#efb35a">Humidade relativa</span></div>
</section></main><main id="dashboardArea"><div class="grid top-tiles" id="readings"></div><section id="heating"><h2>Aquecimento · divisões</h2><div class="orbit-actions"><button class="orbit-action orbit-switch" type="button" id="toggleHeat" role="switch" aria-checked="false" aria-label="Controlo do aquecimento" disabled><span class="orbit-switch-track" aria-hidden="true"></span><span id="toggleHeatLabel">Desligado</span></button><button class="orbit-action" type="button" id="refreshHeat" aria-label="Atualizar valores KNX" title="Reler valores KNX" disabled><svg aria-hidden="true" viewBox="0 0 24 24"><path d="M20 11a8 8 0 1 0-2.3 6M20 5v6h-6"/></svg>Atualizar</button></div><div id="rooms"></div><p id="heatMessage" role="status"></p>
<article aria-labelledby="collector-title" class="collector-card" id="collectorCard">
<header class="collector-header"><div class="collector-title-group"><span aria-hidden="true" class="collector-title-icon"><svg viewbox="0 0 24 24"><path d="M4 6h16M4 18h16M7 6v12M12 6v12M17 6v12"></path><circle cx="7" cy="10" r="1.4"></circle><circle cx="12" cy="14" r="1.4"></circle><circle cx="17" cy="9" r="1.4"></circle></svg></span><div><h2 id="collector-title">Coletor de piso radiante</h2><p class="collector-subtitle" id="collectorSubtitle">A carregar circuitos…</p></div></div><span aria-live="polite" class="collector-summary" id="collectorSummary">A aguardar estado KNX</span></header>
<div class="collector-system"><div aria-label="Bomba: estado desconhecido" class="collector-pump" data-state="off" id="pumpState" role="img"><div class="collector-pump-graphic"><span class="collector-pump-shell"><svg aria-hidden="true" viewbox="0 0 100 100"><circle class="collector-pump-inner" cx="50" cy="50" r="38"></circle><g class="collector-pump-rotor"><path d="M50 47C36 45 29 36 31 27c12 0 19 7 22 19Z"></path><path d="M50 47C36 45 29 36 31 27c12 0 19 7 22 19Z" transform="rotate(90 50 50)"></path><path d="M50 47C36 45 29 36 31 27c12 0 19 7 22 19Z" transform="rotate(180 50 50)"></path><path d="M50 47C36 45 29 36 31 27c12 0 19 7 22 19Z" transform="rotate(270 50 50)"></path></g><circle class="collector-pump-hub" cx="50" cy="50" r="10"></circle><circle class="collector-pump-led" cx="50" cy="50" r="3"></circle></svg></span></div><strong class="collector-pump-name">Bomba de circulação</strong><span class="collector-pump-status" id="pumpStatus">—</span><span class="collector-pump-detail" id="pumpDetail">A aguardar feedback KNX</span><div aria-label="Ciclo da bomba" aria-valuemax="100" aria-valuemin="0" aria-valuenow="0" class="pump-phase" data-phase="idle" id="pumpPhase" role="progressbar"><span id="pumpPhaseFill"></span></div></div>
<div aria-label="Circuitos do coletor" class="collector-manifold"><div class="collector-rail collector-supply"><span class="collector-rail-label"><span aria-hidden="true" class="collector-flow-arrow">→</span>Entrada · água quente</span><strong class="collector-temperature" id="supplyTemperature">Sem leitura</strong></div><div class="collector-circuits" id="branches"></div><div class="collector-rail collector-return"><span class="collector-rail-label"><span aria-hidden="true" class="collector-flow-arrow">←</span>Saída · retorno</span><strong class="collector-temperature" id="returnTemperature">Sem leitura</strong></div></div></div>
<footer class="collector-footer"><div aria-label="Legenda" class="collector-legend"><span class="collector-legend-item" style="--legend:#69d8c3">Ligado</span><span class="collector-legend-item" style="--legend:#657681">Desligado</span><span class="collector-legend-item" style="--legend:#efb35a">A confirmar</span></div><span class="collector-safety">Bomba protegida · exige circuito ativo</span></footer></article></section>
<section id="relaySection"><h2>Saídas · relés</h2><div class="grid" id="outputs"></div><p>O feedback KNX indica o estado comandado. Durante falhas de rede, o comando local continua disponível e o estado é publicado ao recuperar a ligação.</p></section>
<section hidden="" id="inputSection"><h2>Inputs locais</h2><div class="grid" id="inputStates"></div></section>

</main><main hidden="" id="thermostatView"><section class="thermostat-hero"><p class="hint">Termóstato da divisão · sensor BME280</p><article class="thermostat" data-demand="false" data-powered="false" id="thermoTile"><header class="header"><h2 id="thermoName">Termóstato</h2><span class="status" id="thermoState">A aguardar</span></header><div class="dial" id="thermoDial" role="img"><div class="dial-content"><span class="label">Ambiente</span><output class="current" id="thermoTemperature">—<small>°C</small></output><span class="heat-state" id="thermoDemand">Sem leitura</span></div></div><section class="setpoint"><div class="setpoint-controls"><button aria-label="Diminuir setpoint" class="step" id="thermoDown" type="button">−</button><div class="setpoint-value"><span class="label">Pretendida</span><output id="thermoTarget">— °C</output></div><button aria-label="Aumentar setpoint" class="step" id="thermoUp" type="button">+</button></div><p aria-live="polite" class="pending" id="thermoPending"></p></section><footer class="tile-footer"><button aria-label="Ligar ou desligar termóstato" class="power" id="thermoPower" type="button"><svg aria-hidden="true" viewbox="0 0 24 24"><path d="M12 2v10M5.6 5.6a9 9 0 1 0 12.8 0"></path></svg></button><span class="hint">Automático · BME local</span></footer></article><p class="hint" id="thermoHelp">O pedido KNX aguarda confirmação do feedback.</p></section>
</main><main class="orbit-config" hidden="" id="settingsArea">
<p><button id="backDashboard" type="button">← Voltar ao painel</button></p>
<section class="login-panel" id="loginPanel"><h2>Configuração</h2><p>Inicia sessão para editar as configurações.</p><form id="loginForm"><label>Utilizador<input autocomplete="username" id="loginUser" required="" value="admin"/></label><label>Palavra-passe<input autocomplete="current-password" id="loginPassword" required="" type="password"/></label><button type="submit">Entrar</button></form><p id="loginError" role="alert"></p></section>
<div hidden="" id="privateSettings"><button type="button" id="logoutAdmin">Sair</button>
<nav aria-label="Áreas de configuração" class="settings-tabs" role="tablist"><button id="settings-tab-config" aria-controls="tab-config" aria-selected="false" data-tab="config" role="tab" type="button">Técnico</button><button id="settings-tab-diagnostics" aria-controls="tab-diagnostics" aria-selected="false" data-tab="diagnostics" role="tab" type="button">Diagnóstico</button><button id="settings-tab-hardware" aria-controls="tab-hardware" aria-selected="false" data-tab="hardware" role="tab" type="button">Hardware</button><button id="settings-tab-knx" aria-controls="tab-knx" aria-selected="false" data-tab="knx" role="tab" type="button">KNX</button><button id="settings-tab-features" aria-controls="tab-features" aria-selected="false" data-tab="features" role="tab" type="button">Configurações</button></nav>
<form id="settings" novalidate=""><div class="tab-panel" hidden="" id="tab-diagnostics" role="tabpanel" aria-labelledby="settings-tab-diagnostics"><section><h2>Registos</h2><button id="refreshDiagnostics" type="button">Atualizar</button><pre id="orbitDiagnostics"></pre><pre id="orbitLogs"></pre><p>Registo em RAM; este firmware não dispõe de cartão SD.</p></section><section id="diagnosticSection"><h2>Estado e diagnóstico</h2><pre id="status"></pre><details><summary>Telegramas recebidos</summary><pre id="telegrams">Sem dados</pre></details><p>Sondas detetadas: <span id="roms">—</span></p></section></div><div class="tab-panel" id="tab-features" role="tabpanel" aria-labelledby="settings-tab-features">
<section><h2>Função principal</h2><p>Escolhe uma função exclusiva. O mapa do coletor permanece guardado ao mudar para termóstato.</p><label>Modo do dispositivo<select id="profileMode"><option value="collector">Controlo de piso radiante</option><option value="thermostat">Termóstato de divisão</option></select></label></section>
<div class="feature-grid">
<section class="feature-tile" id="collectorSettings"><h2>Controlo de piso radiante</h2><p>Primeiro cria as divisões e associa circuitos. Os endereços de grupo aparecem no separador KNX.</p><div class="grid" id="heatSettings"></div><fieldset><legend>Proteção térmica da bomba</legend><div class="plan-fields" id="pumpGuardSettings"></div><p class="hint">Sonda 1 mede a entrada de água quente; sonda 2 mede o retorno. A proteção só atua com uma leitura DS18B20 recente.</p></fieldset><p><button id="addRoom" type="button">Adicionar divisão</button></p><div class="plan-fields" id="zoneFields"></div><h3>Posições do coletor</h3><div class="plan-fields" id="circuitFields"></div><p><button id="saveHeating" type="button">Guardar mapa e reiniciar</button></p><small>O ESP para a bomba e os circuitos, aguarda feedback OFF, grava o mapa e reinicia.</small></section>
<section class="feature-tile" hidden="" id="thermostatSettings"><h2>Termóstato de divisão</h2><p>Lê a temperatura do BME e publica temperatura, setpoint e pedido no KNX. Não comanda o coletor.</p><div id="thermostatFeature"></div><p class="hint">Configura BME e os endereços KNX antes de ativar este modo.</p></section>
<section class="feature-tile"><h2>Sensores locais</h2><p>Seleciona os valores enviados ao KNX. Preenche os endereços de grupo no separador KNX antes de guardar. Os GPIO e as ROM ficam em Hardware.</p><div id="sensorFeature"></div><button id="sensorKnxSettings" type="button">Configurar envio KNX dos sensores</button></section>
<section class="feature-tile"><h2>Actuadores locais</h2><p>Relés independentes do coletor.</p><div id="relayFeature"></div></section>
<section class="feature-tile"><h2>Inputs locais</h2><p>Quatro entradas digitais; publicam mudanças no KNX se tiverem grupo.</p><div id="inputFeature"></div></section>
</div></div>
<div class="tab-panel" hidden="" id="tab-knx" role="tabpanel" aria-labelledby="settings-tab-knx">
<section><h2>Ligação KNX</h2><div class="grid" id="knx"></div></section>
<section><h2>Validar grupos KNX</h2><p>Verifica os endereços dos canais ativos antes de guardar. Não envia telegramas nem altera a configuração.</p><button id="validateKnx" type="button">Validar endereços KNX</button><pre aria-live="polite" id="knxValidation" role="status"></pre></section>
<section><h2>Sensores · envio</h2><p>Mostram-se os canais selecionados em Configurações.</p><div id="channels"></div></section>
<section id="heatKnxSection"><h2>Piso radiante · receção e comandos</h2><p>Temperatura e setpoint recebidos; pedido, comandos e feedback separados por divisão/circuito.</p><div class="plan-fields" id="zoneKnxFields"></div><h3>Saídas KNX · circuitos e bomba</h3><div class="plan-fields" id="circuitKnxFields"></div></section>
<section hidden="" id="thermostatKnxSection"><h2>Termóstato · grupos KNX</h2><div id="thermostatKnx"></div></section>
<section><h2>Actuadores locais · comando e feedback</h2><div id="relayKnx"></div></section>
<section><h2>Inputs locais · envio</h2><div id="inputKnx"></div></section><section><h2>Terminal KNX</h2><p>Últimos telegramas recebidos e envio manual. Enfileirar um telegrama não confirma a atuação.</p><pre aria-live="polite" class="knx-console" id="knxConsole">A aguardar telegramas…</pre><div class="grid"><label>Grupo KNX<input id="knxGroup" placeholder="3/4/10"/></label><label>Operação<select id="knxOperation"><option value="read">Ler grupo</option><option value="switch">Escrever DPT 1.001</option><option value="float">Escrever DPT 9.xxx</option></select></label><label hidden="" id="knxValueLabel">Valor<input id="knxValue" placeholder="0 ou 1"/></label></div><button id="knxSend" type="button">Enviar ao KNX</button></section></div>
<div class="tab-panel" hidden="" id="tab-hardware" role="tabpanel" aria-labelledby="settings-tab-hardware">
<section><h2>Pinos dos sensores e OLED</h2><div class="grid" id="hardware"></div><p><button id="saveOneWire" type="button">Guardar GPIO e ROM das sondas e reiniciar</button> <button id="oledPreset" type="button">Aplicar pinos do OLED integrado · WiFi Kit 32 V2/V2.1</button></p><p>Sondas detetadas: <span id="hardwareRoms">Nenhuma; configura o GPIO e reinicia</span></p><p class="hint">Primeiro guarda o GPIO com os canais DS18B20 desativados e as ROM vazias. Depois do reinício, copia cada código de «Sondas detetadas» para a ROM correspondente e guarda novamente. O botão acima guarda o GPIO e as duas ROM; não guarda outros campos alterados.</p><p class="hint">Confirma a revisão da placa. BME280 e OLED podem usar barramentos I²C separados.</p></section>
<section><h2>GPIO dos relés</h2><div id="relayHardware"></div></section>
<section><h2>GPIO dos inputs</h2><div id="inputHardware"></div><p class="hint">GPIO 34/35/36/39 exigem resistência externa. As outras entradas usam pull-up (ativo baixo) ou pull-down (ativo alto) internos.</p></section></div>
<div class="tab-panel" hidden="" id="tab-config" role="tabpanel" aria-labelledby="settings-tab-config">
<section><h2>Rede e dispositivo</h2><button id="scanWifi" type="button">Pesquisar redes</button><select aria-label="Redes disponíveis" id="wifiNetworks"><option value="">Selecionar rede</option></select><p id="wifiScanState" role="status"></p><div class="grid" id="general"></div></section>
<article class="card ntp-panel" id="ntpPanel"><span class="eyebrow">HORA</span><h2>NTP e relógio local</h2><div class="ntp-row"><label for="ntpServer">Servidor NTP</label><input id="ntpServer" maxlength="63" value="pool.ntp.org"/></div><div class="ntp-row"><label for="ntpSecondary">Servidor alternativo</label><input id="ntpSecondary" maxlength="63" value="time.cloudflare.com"/></div><div class="ntp-row"><label for="timezone">Fuso horário</label><select id="timezone"><option value="WET0WEST,M3.5.0/1,M10.5.0">Europe/Lisbon</option><option value="UTC0">UTC</option></select></div><div class="ntp-summary"><div><small>Hora local</small><b id="rtcTimeValid">—</b></div><div><small>RTC externo</small><b id="rtcAvailable">—</b></div><div><small>Histórico interno</small><b id="sdAvailable">—</b></div></div><button type="button" id="syncTime">Sincronizar agora</button><p class="ntp-hint">Utiliza os servidores e o fuso guardados. Guarde primeiro as alterações.</p><p id="timeSyncState" role="status" aria-live="polite"></p></article><section><h2>Backup e restauro</h2><p>O backup inclui a configuração e o mapa KNX, sem passwords. O restauro conserva as passwords atuais e reinicia.</p><button id="export" type="button">Descarregar backup</button><label>Ficheiro de backup JSON<input accept=".json,application/json" id="restoreFile" type="file"/></label><button id="restore" type="button">Restaurar configuração</button></section>
<section><h2>Atualização de firmware</h2><p>O firmware deve identificar este equipamento e perfil. Ficheiros antigos sem identificação são recusados. Mantém a alimentação.</p><input accept=".bin" id="firmware" type="file"/><p><button id="update" type="button">Instalar atualização OTA</button></p><progress hidden="" id="progress" max="100" value="0"></progress></section>
<section><h2>Administração</h2><label>Nova palavra-passe<input autocomplete="new-password" id="newAdminPassword" maxlength="63" minlength="10" type="password"/></label><button id="changeAdminPassword" type="button">Alterar palavra-passe</button> <button id="restartDevice" type="button">Reiniciar dispositivo</button></section></div>
<div class="settings-actions"><button type="submit">Guardar configurações e reiniciar</button><p>Ao gravar, o ESP reinicia e confirma OFF no arranque pela ligação KNX configurada.</p></div></form>
</div></main><footer class="orbit-footer"><span id="orbitVersion">A consultar firmware…</span><a id="viewHelp" data-view="help" href="/#ajuda">Ajuda</a></footer>
<script data-orbit-controls>// Orbit control adapted from bratuga/iot-vmc-controler main 754e3b0.
function createOrbitPower(sendPower){
 const railPower=document.getElementById('railPower');
 railPower.innerHTML=`<span class="rail-power-track" aria-hidden="true"></span><span class="rail-power-ring" aria-hidden="true"></span><span class="rail-power-progress" aria-hidden="true"></span>
  <span class="rail-power-metrics" id="railPowerMetrics" aria-hidden="true" hidden>
   <span><svg class="rail-power-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" aria-hidden="true"><rect x="5" y="5" width="14" height="14" rx="2"/><rect x="9" y="9" width="6" height="6" rx="1"/><path d="M8 2v3m4-3v3m4-3v3M8 19v3m4-3v3m4-3v3M2 8h3m-3 4h3m-3 4h3m14-8h3m-3 4h3m-3 4h3"/></svg><span id="railCpu">—</span></span>
   <span><svg class="rail-power-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" aria-hidden="true"><rect x="2" y="7" width="20" height="9" rx="1.5"/><path d="M5 16v3m3-3v3m3-3v3m5-3v3m3-3v3M6 11h2m3 0h2m3 0h2"/></svg><span id="railHeap">—</span></span>
  </span>
  <span class="rail-power-centre" id="railPowerCentre" aria-hidden="true">—</span>`;
 const vmcEl=id=>document.getElementById(id);
 const vmcText=(id,text)=>{if(vmcEl(id).textContent!==text)vmcEl(id).textContent=text};
 let deviceState=null,deviceBusy=false;
 let railHold=null,railPending=false,railFrame=0,railConfirmTarget=null,railError='';
 function renderRailNotice(state,label){
  const error=railError||(state==='critical'?label:'');
  const pending=railPending?'A enviar comando…':state==='warning'?label:railConfirmTarget!==null?'A aguardar confirmação':'';
  vmcEl('railPowerNotice').dataset.kind=error?'error':pending?'pending':'';
  const notice=error||pending||(state==='offline'?label:'');
  vmcText('railPowerNotice',notice);
  vmcText('railPowerStatus',notice?'':state==='off'?'Saídas desligadas':'Saídas autorizadas');
 }
 function railPowerState(state){
  if(typeof state?.powerEnabled!=='boolean')return ['offline','Sem ligação'];
  if(state.level==='critical')return ['critical',state.label||'Falha crítica'];
  if(state.level==='warning')return ['warning',state.label||'A aguardar'];
  if(!state.powerEnabled)return ['off','Saídas desligadas'];
  return ['ok','Ligada · OK'];
 }
 function renderRailPower(){
  const enabled=deviceState?.powerEnabled;
  if(railHold&&(deviceBusy||deviceState?.canToggle===false||railHold.enabled!==enabled))clearRailHold();
  const [state,label]=railPowerState(deviceState);
  if(railConfirmTarget!==null&&enabled===railConfirmTarget&&(state==='ok'||state==='off'))railConfirmTarget=null;
  renderRailNotice(state,label);
  const working=!!railHold||railPending;
  railPower.dataset.state=state;
  railPower.toggleAttribute('data-holding',!!railHold);
  railPower.toggleAttribute('data-pending',railPending);
  railPower.setAttribute('aria-disabled',String(state==='offline'||deviceBusy||railPending||deviceState?.canToggle===false));
  const metrics=state==='ok'&&!working;
  vmcEl('railPowerMetrics').hidden=!metrics;
  const cpu=Number.isFinite(deviceState?.cpuUsePct)?deviceState.cpuUsePct+'%':Number.isFinite(deviceState?.cpuMHz)?deviceState.cpuMHz+' MHz':'—';
  const heap=Number.isFinite(deviceState?.heapUsePct)?deviceState.heapUsePct+'%':'—';
  vmcText('railCpu',cpu);vmcText('railHeap',heap);
  const centre=vmcEl('railPowerCentre');
  const powerSymbol=working||state==='off';
  centre.hidden=metrics;
  centre.toggleAttribute('data-symbol',powerSymbol);
  vmcText('railPowerCentre',powerSymbol?'⏻':state==='offline'?'—':label);
  const hint=state==='offline'?'':' · manter premido 1,5 s para '+(enabled?'desligar':'ligar');
  const accessible=label+(metrics?' · CPU '+cpu+(Number.isFinite(deviceState?.cpuUsePct)?' de utilização':' de frequência; utilização indisponível')+' · memória utilizada '+heap:'')+hint;
  railPower.setAttribute('aria-label',accessible);
  railPower.title=accessible;
 }
 function clearRailHold(){
  railHold=null;
  cancelAnimationFrame(railFrame);
  railPower.style.setProperty('--rail-fill','0%');
 }
 function cancelRailHold(){
  if(!railHold)return;
  clearRailHold();
  renderRailPower();
 }
 function startRailHold(source){
  if(railHold||deviceBusy||railPending||typeof deviceState?.powerEnabled!=='boolean'||deviceState.canToggle===false)return;
  const hold={source,enabled:deviceState.powerEnabled,started:performance.now()};
  railHold=hold;
  railConfirmTarget=null;
  railError='';
  renderRailPower();
  const fill=async now=>{
   if(railHold!==hold)return;
   const progress=Math.min(1,(now-hold.started)/1500);
   railPower.style.setProperty('--rail-fill',progress*100+'%');
   if(progress<1){railFrame=requestAnimationFrame(fill);return}
   clearRailHold();
   if(deviceBusy||deviceState?.canToggle===false||deviceState?.powerEnabled!==hold.enabled){renderRailPower();return}
   const target=!hold.enabled;
   railPending=true;
   railPower.style.setProperty('--rail-fill','100%');
   renderRailPower();
   try{
    if(await sendPower(target))railConfirmTarget=target;
    else railError='Comando falhou';
   }catch(error){railError=error.message||'Comando falhou'}
   railPending=false;
   railPower.style.setProperty('--rail-fill','0%');
   renderRailPower();
  };
  railFrame=requestAnimationFrame(fill);
 }
 railPower.addEventListener('pointerdown',event=>{
  if(!event.isPrimary||event.button!==0||railPower.getAttribute('aria-disabled')==='true')return;
  railPower.setPointerCapture(event.pointerId);
  startRailHold(event.pointerId);
 });
 for(const name of ['pointerup','pointercancel','lostpointercapture']){
  railPower.addEventListener(name,event=>{if(railHold?.source===event.pointerId)cancelRailHold()});
 }
 railPower.addEventListener('pointermove',event=>{
  if(railHold?.source!==event.pointerId)return;
  const bounds=railPower.getBoundingClientRect();
  if(event.clientX<bounds.left-8||event.clientX>bounds.right+8||event.clientY<bounds.top-8||event.clientY>bounds.bottom+8)cancelRailHold();
 });
 railPower.addEventListener('keydown',event=>{
  if(event.key==='Escape')cancelRailHold();
  if(event.key!==' '&&event.key!=='Enter')return;
  event.preventDefault();
  if(!event.repeat)startRailHold(event.key);
 });
 railPower.addEventListener('keyup',event=>{if(railHold?.source===event.key)cancelRailHold()});
 railPower.addEventListener('blur',cancelRailHold);
 railPower.addEventListener('contextmenu',event=>event.preventDefault());
 window.addEventListener('blur',cancelRailHold);
 document.addEventListener('visibilitychange',()=>{if(document.hidden)cancelRailHold()});
 return {update(state,busy=false){
  deviceState=state;
  deviceBusy=busy;
  renderRailPower();
 }};
}

function initOrbitTheme(key='orbit-theme'){
 const button=document.getElementById('orbitTheme');if(!button)return;
 const apply=light=>{document.body.classList.toggle('light',light);button.setAttribute('aria-checked',String(light));};
 try{apply(localStorage.getItem(key)==='light')}catch(e){apply(false)}
 button.addEventListener('click',()=>{const light=!document.body.classList.contains('light');apply(light);try{localStorage.setItem(key,light?'light':'dark')}catch(e){}});
}

initOrbitTheme('iot-sensor-device-theme');
let orbitLastState=null,orbitPowerBusy=false;
const orbitPower=createOrbitPower(async enabled=>{
 if(orbitPowerBusy)return false;orbitPowerBusy=true;
 try{await orbitPowerRequest(enabled);await orbitRefreshPower();return true}
 finally{orbitPowerBusy=false;orbitShowPower(orbitLastState)}
});
function orbitShowPower(s){orbitLastState=s;orbitPower.update(typeof s?.powerEnabled==='boolean'?{powerEnabled:s.powerEnabled,cpuUsePct:s.cpuUsePct,cpuMHz:s.cpuMHz,heapUsePct:s.heapUsePct,level:s.powerLevel,label:s.powerLabel,canToggle:s.powerCanToggle!==false}:null,orbitPowerBusy);}

async function orbitPowerRequest(enabled){await api('/api/power',{method:'POST',headers:{'Content-Type':'application/json','X-CSRF-Token':csrf},body:JSON.stringify({enabled}),signal:AbortSignal.timeout(5000)});}
async function orbitRefreshPower(){try{orbitShowPower(await api('/api/status',{signal:AbortSignal.timeout(5000)}))}catch(e){orbitShowPower(null)}}
</script><script>
'use strict';let cfg,csrf='',busy=false;
const $=id=>document.getElementById(id);const show=s=>{$('msg').textContent=s;$('msg').style.display='block'};
async function api(path,options={}){const r=await fetch(path,{cache:'no-store',...options});const j=await r.json();if(!r.ok){const e=Error(j.error||('HTTP '+r.status));e.status=r.status;throw e;}return j;}
function field(parent,id,label,value,type='text',step){const l=document.createElement('label');const input=document.createElement('input');input.id=id;input.name=id;input.type=type;if(type==='checkbox'){input.checked=!!value;l.append(input,document.createTextNode(label));}else{l.textContent=label;input.value=value??'';if(step)input.step=step;l.append(input);}$(parent).append(l);return input;}
const gs=[['name','Nome do dispositivo'],['showDiagnostics','Mostrar detalhes no diagnóstico','checkbox'],['ssid','Rede Wi-Fi (SSID)'],['wifiPassword','Password Wi-Fi','password'],['wifiDhcp','IP dinâmico (DHCP)','checkbox'],['wifiIp','IP fixo'],['wifiMask','Máscara'],['wifiGateway','Gateway'],['wifiDns','DNS'],['ntpServer','Servidor NTP'],['ntpSecondary','Servidor alternativo'],['timezone','Fuso horário POSIX']];
const hs=[['sda','GPIO SDA BME280 / I²C partilhado','number'],['scl','GPIO SCL BME280 / I²C partilhado','number'],['bmeAddress','Endereço BME280: 118 ou 119','number'],['oneWire','GPIO DS18B20 (-1 desativa)','number'],['oledEnabled','Ativar OLED','checkbox'],['oledSda','GPIO SDA OLED (-1 partilha I²C)','number'],['oledScl','GPIO SCL OLED (-1 partilha I²C)','number'],['oledAddress','Endereço OLED: 60 ou 61','number'],['oledReset','GPIO reset OLED (-1 sem reset)','number'],['oledHeight','Altura OLED: 32 ou 64','number'],['sampleMs','Leitura dos sensores (s)','number','0.1']];
const ks=[['knxEnabled','Ativar ligação KNX','checkbox'],['gateway','IPv4 do knxd'],['port','Porta KNX','number'],['rate','Limite de telegramas/segundo','number']];
const cs=[['name','Nome'],['group','Grupo KNX (principal 0–15)'],['offset','Correção de leitura','number','any'],['periodic','Envio periódico','checkbox'],['periodMs','Período de envio (s)','number','0.1'],['onChange','Envio por variação','checkbox'],['delta','Variação mínima','number','any'],['minGapMs','Intervalo mínimo (s)','number','0.1']];
const rsFeature=[['name','Nome'],['enabled','Ativar saída','checkbox']];
const rsHardware=[['gpio','GPIO (-1 desativa)','number'],['activeLow','Ativo a nível baixo','checkbox']];
const rsKnx=[['commandGroup','Grupo KNX de comando'],['feedbackGroup','Grupo KNX de feedback']];
const isFeature=[['name','Nome'],['enabled','Ativar entrada','checkbox']];
const isHardware=[['gpio','GPIO (-1 desativa)','number'],['activeLow','Ativo a nível baixo','checkbox']];
const isKnx=[['group','Grupo KNX · envio DPT 1.001 (opcional)']];
const tsFeature=[['thermostatName','Nome da divisão'],['thermostatMargin','Margem ± °C','number','0.1']];
const tsKnx=[['thermostatSetpointGroup','Setpoint · DPT 9.001 · envio/receção'],['thermostatDemandGroup','Pedido · DPT 1.001 · comando'],['thermostatFeedbackGroup','Pedido · DPT 1.001 · feedback']];
const pumpGuardFields=[['pumpColdGuardEnabled','Proteger quando a água de entrada está fria','checkbox'],['pumpColdLimitC','Temperatura mínima da água de entrada (°C)','number','0.1'],['pumpColdPauseMin','Pausa após água fria (min)','number','1'],['pumpReturnGuardEnabled','Proteger quando a água de retorno está quente','checkbox'],['pumpReturnLimitC','Limite da água de retorno (°C)','number','0.1'],['pumpReturnAboveMin','Tempo acima do limite até pausar (min)','number','1'],['pumpReturnPauseMin','Pausa após retorno quente (min)','number','1']];
const secondDurations=new Set(['sampleMs','periodMs','minGapMs','openMs','maxAgeMs']);
const minuteDurations=new Set(['serviceMs','pauseMs']);
function durationForUi(key,value){const factor=secondDurations.has(key)?1000:minuteDurations.has(key)?60000:1;const n=Number(value)/factor;return Number.isFinite(n)?Number(n.toFixed(1)):value;}
function durationForApi(key,value){const factor=secondDurations.has(key)?1000:minuteDurations.has(key)?60000:1;return factor===1?value:Math.round(Number(value)*factor);}
let adminSession=sessionStorage.getItem('adminSession')||'';
async function adminApi(path,options={}){const headers={...(options.headers||{}),'X-Admin-Session':adminSession};try{return await api(path,{...options,headers});}catch(e){if(e.status===401){adminSession='';sessionStorage.removeItem('adminSession');$('privateSettings').hidden=true;$('loginPanel').hidden=false;}throw e;}}
const publicHashes={dashboard:'painel',sensors:'sensores',history:'historico',help:'ajuda'};
function routeFromHash(){return Object.keys(publicHashes).find(name=>'#'+publicHashes[name]===location.hash)||'dashboard';}
function view(name,updateHash=true){
 const settings=name==='settings';
 if(settings&&location.pathname!='/config.html'){location.href='/config.html';return;}
 if(!settings&&location.pathname==='/config.html'){location.href='/#'+publicHashes[name];return;}
 $('dashboardArea').hidden=name!=='dashboard'||currentThermostatMode;
 $('thermostatView').hidden=name!=='dashboard'||!currentThermostatMode;
 $('sensorArea').hidden=name!=='sensors';$('settingsArea').hidden=!settings;
 $('historyArea').hidden=name!=='history';$('helpArea').hidden=name!=='help';
 for(const link of document.querySelectorAll('[data-view]')){
  if(link.dataset.view===name)link.setAttribute('aria-current','page');else link.removeAttribute('aria-current');
 }
 const hash=publicHashes[name];
 if(updateHash&&hash&&location.hash!=='#'+hash)location.hash=hash;
}
let currentThermostatMode=false;
const settingsHashes={config:'tecnico',diagnostics:'diagnostico',hardware:'hardware',knx:'knx',features:'configuracoes'};
const settingsTabs=Array.from(document.querySelectorAll('.settings-tabs [role="tab"]'));
function tabFromHash(){return Object.keys(settingsHashes).find(name=>'#'+settingsHashes[name]===location.hash)||'config';}
function selectTab(name){
 for(const b of settingsTabs){
  const selected=b.dataset.tab===name,panel=$(b.getAttribute('aria-controls'));
  b.setAttribute('aria-selected',String(selected));b.tabIndex=selected?0:-1;
  panel.hidden=!selected;
 }
}
function goToTab(name){
 selectTab(name);
 const hash='#'+settingsHashes[name];
 if(location.pathname==='/config.html'&&location.hash!==hash)location.hash=hash;
}
settingsTabs.forEach((tab,index)=>{
 tab.onclick=()=>goToTab(tab.dataset.tab);
 tab.onkeydown=e=>{
  const next=e.key==='ArrowLeft'?settingsTabs[(index+settingsTabs.length-1)%settingsTabs.length]:
   e.key==='ArrowRight'?settingsTabs[(index+1)%settingsTabs.length]:e.key==='Home'?settingsTabs[0]:e.key==='End'?settingsTabs.at(-1):null;
  if(!next)return;
  e.preventDefault();goToTab(next.dataset.tab);next.focus();
 };
});
function visibility(){if(!cfg)return;const mode=$('profileMode').value==='thermostat';$('collectorSettings').hidden=mode;$('thermostatSettings').hidden=!mode;$('heatKnxSection').hidden=mode;$('thermostatKnxSection').hidden=!mode;for(let i=0;i<5;i++)$('ch'+i).parentElement.hidden=!$('channelEnabled'+i).checked;for(let i=0;i<4;i++){$('relayK'+i).hidden=!$('r'+i+'_enabled').checked;$('inputK'+i).hidden=!$('in'+i+'_enabled').checked;}}
function validateKnxFields(){
 const problems=[],valid=s=>{const m=s.trim().match(/^(\d{1,3})\/(\d{1,3})\/(\d{1,3})$/);return !!m&&+m[1]<=15&&+m[2]<=7&&+m[3]<=255&&!(+m[1]===0&&+m[2]===0&&+m[3]===0)};
 const check=(id,label,required)=>{const el=$(id),v=el?.value.trim()||'';if((required&&!v)||(v&&!valid(v)))problems.push(label+': '+(v?'"'+v+'" inválido':'grupo obrigatório'));};
 for(let i=0;i<5;i++)if($('channelEnabled'+i)?.checked)check(i+'_group','Sensor '+(i+1),true);
 if($('profileMode')?.value==='thermostat')for(const [id,label]of [['thermostatSetpointGroup','Termóstato · setpoint'],['thermostatDemandGroup','Termóstato · pedido'],['thermostatFeedbackGroup','Termóstato · feedback']])check(id,label,true);
 for(let i=0;i<4;i++){
  if($('r'+i+'_enabled')?.checked){check('r'+i+'_commandGroup','Relé '+(i+1)+' · comando',true);check('r'+i+'_feedbackGroup','Relé '+(i+1)+' · feedback',true);}
  if($('in'+i+'_enabled')?.checked)check('in'+i+'_group','Entrada '+(i+1),false);
 }
 $('knxValidation').textContent=problems.length?problems.join('\n'):'Endereços KNX dos canais ativos válidos. As restantes regras são verificadas ao guardar.';
 return problems;
}
$('validateKnx').onclick=()=>{selectTab('knx');validateKnxFields();};
$('sensorKnxSettings').onclick=()=>{visibility();goToTab('knx');const first=Array.from({length:5},(_,i)=>i).find(i=>$('channelEnabled'+i).checked);if(first!==undefined)$((first)+'_group').focus();else show('Seleciona primeiro um sensor para enviar ao KNX.');};

function dashboard(){view('dashboard');}
async function openSettings(){view('settings');if(location.pathname!='/config.html')return;$('privateSettings').hidden=true;$('loginPanel').hidden=false;if(!adminSession)return;try{const r=await adminApi('/api/config');cfg=r.config;heatingPlan=await adminApi('/api/heating/layout');render();renderRelays();renderInputs();renderHeating();visibility();selectTab(tabFromHash());$('loginPanel').hidden=true;$('privateSettings').hidden=false;}catch(e){$('loginError').textContent=e.message;}}
$('backDashboard').onclick=dashboard;
$('restartDevice').onclick=async()=>{if(busy||!confirm('Reiniciar o dispositivo? O coletor aguarda confirmação das saídas desligadas.'))return;busy=true;try{const result=await adminApi('/api/restart',{method:'POST',headers:{'X-CSRF-Token':csrf}});if(result.pending)await waitForSafeSave(()=>result,'A reiniciar o dispositivo…');else show('A reiniciar o dispositivo…');}catch(e){show('Não foi possível reiniciar: '+e.message);}finally{busy=false;}};
let lastHistoryRows=null,historyPosition=1;
const historyPanes=[['temperatureScroll','historyChart','temperatureAxis'],['humidityScroll','humidityChart','humidityAxis']];
for(const [source,chart] of historyPanes){
 $(source).onscroll=()=>{
  const pane=$(source),range=pane.scrollWidth-pane.clientWidth;
  if(!range)return;
  historyPosition=pane.scrollLeft/range;
  for(const [other] of historyPanes)if(other!==source){
   const target=$(other),offset=historyPosition*(target.scrollWidth-target.clientWidth);
   if(Math.abs(target.scrollLeft-offset)>1)target.scrollLeft=offset;
  }
 };
}
function drawHistory(rows){
 lastHistoryRows=rows;
 const ns='http://www.w3.org/2000/svg',height=232;
 const end=Math.ceil(Date.now()/1800000)*1800,start=end-48*3600;
 const values=rows.flatMap(row=>row.slice(1,4)).filter(Number.isFinite);
 const low=values.length?Math.floor(Math.min(...values)/5)*5:0;
 const high=values.length?Math.max(low+5,Math.ceil(Math.max(...values)/5)*5):40;
 const series=[['#ff705c','#4db9e7','#69d8c3'],['#efb35a']];
 historyPanes.forEach(([paneId,chartId,axisId],kind)=>{
  const pane=$(paneId),svg=$(chartId),axis=$(axisId),width=(pane.clientWidth||700)*24;
  svg.textContent='';axis.textContent='';
  svg.setAttribute('width',width);svg.style.width=width+'px';svg.setAttribute('height',height);svg.setAttribute('viewBox','0 0 '+width+' '+height);
  axis.setAttribute('viewBox','0 0 48 '+height);
  if(!rows.length)return;
  const x=time=>Math.max(0,Math.min(width,(time-start)*width/(end-start)));
  const min=kind?0:low,max=kind?100:high,y=value=>184-(value-min)/(max-min)*164;
  function el(parent,tag,attrs,content){const n=document.createElementNS(ns,tag);for(const [k,v]of Object.entries(attrs))n.setAttribute(k,String(v));if(content!==undefined)n.textContent=content;parent.append(n);return n;}
  for(let i=0;i<=4;i++){
   const v=min+(max-min)*i/4,py=y(v);
   el(svg,'line',{x1:0,x2:width,y1:py,y2:py,stroke:'var(--orbit-line)'});
   el(axis,'text',{x:43,y:py+4,fill:'var(--orbit-muted)','text-anchor':'end','font-size':12},v.toLocaleString('pt-PT',{maximumFractionDigits:1})+(kind?'%':'°'));
  }
  for(let t=start;t<=end;t+=1800){
   const px=x(t),date=new Date(t*1000);
   el(svg,'line',{x1:px,x2:px,y1:20,y2:194,stroke:'var(--orbit-line)'});
   const anchor=t===end?'end':t===start?'start':'middle';
   el(svg,'text',{x:t===end?width-5:t===start?5:px,y:216,fill:'var(--orbit-muted)','text-anchor':anchor,'font-size':11},date.toLocaleString('pt-PT',{day:'2-digit',month:'2-digit',hour:'2-digit',minute:'2-digit'}));
  }
  if(!kind)for(const row of rows)if(row[5]===1)el(svg,'circle',{cx:x(row[0]),cy:195,r:3.5,fill:'#d694f5'});
  series[kind].forEach((color,i)=>{
   let line='',previous=null;
   for(const row of rows){
    const v=row[kind?4:i+1];if(!Number.isFinite(v)){previous=null;continue;}
    const px=x(row[0]),py=y(v);
    el(svg,'circle',{cx:px,cy:py,r:3,fill:color});
    line+=(previous!==null&&row[0]-previous<=1800?' L':' M')+px.toFixed(1)+' '+py.toFixed(1);previous=row[0];
   }
   if(line)el(svg,'path',{d:line,fill:'none',stroke:color,'stroke-width':2});
  });
  pane.scrollLeft=historyPosition*(pane.scrollWidth-pane.clientWidth);
 });
}
if(typeof window!=='undefined')window.addEventListener('resize',()=>{if(lastHistoryRows&&!$('historyArea').hidden)drawHistory(lastHistoryRows);});
async function refreshHistory(){try{let h=await api('/api/history');if(!h.synced){await api('/api/history/time',{method:'POST',headers:{'Content-Type':'application/json','X-CSRF-Token':csrf},body:JSON.stringify({time:Math.floor(Date.now()/1000)})});h=await api('/api/history');}drawHistory(h.records);$('historyMessage').textContent=!h.storage?'Armazenamento do histórico indisponível.':!h.synced?'A aguardar hora válida.':h.records.length?h.records.length+' registos · desliza para consultar as horas anteriores.':'Ainda não há leituras guardadas. A primeira pode demorar até 20 minutos.';}catch(e){$('historyMessage').textContent='Histórico indisponível: '+e.message;}}
for(const [id,name] of [['viewPanel','dashboard'],['viewSensors','sensors'],['viewHistory','history'],['viewHelp','help']]){
 $(id).onclick=e=>{e?.preventDefault();view(name,false);if(location.pathname==='/config.html')return;
  if(typeof history!=='undefined'&&location.hash!=='#'+publicHashes[name])history.pushState(null,'','#'+publicHashes[name]);
  if(name==='history')return refreshHistory();
 };
}
$('loginForm').onsubmit=async e=>{e.preventDefault();$('loginError').textContent='';try{const result=await api('/api/admin/login',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({username:$('loginUser').value,password:$('loginPassword').value})});$('loginPassword').value='';adminSession=result.session;sessionStorage.setItem('adminSession',adminSession);await openSettings();}catch(err){$('loginError').textContent=err.message;}};
const initialSettings=location.pathname==='/config.html'||location.hash==='#configuracoes';
view(initialSettings?'settings':routeFromHash(),false);
let heatingPlan=null,heatingDraft=null;const setpointEdits=new Map();
async function heatAction(action,index,mode){if(busy)return;busy=true;try{await api('/api/heating/control',{method:'POST',headers:{'Content-Type':'application/json','X-CSRF-Token':csrf},body:JSON.stringify({action,index,mode})});await refreshHeat();}catch(e){show(e.message);}finally{busy=false;}}
$('refreshHeat').onclick=()=>heatAction('refresh');$('toggleHeat').onclick=()=>heatAction(heatingPlan&&(heatingPlan.controlRequested??heatingPlan.armed)?'stop':'arm');
function lamp(state){const dot=document.createElement('span');dot.className='state-dot '+(state==='ligado'?'on':state==='desligado'?'off':state==='pendente'?'pending':'');dot.title=state;dot.setAttribute('aria-label',state);return dot;}
function formatDegrees(v){return Number(v).toFixed(1).replace('.',',')+' °C';}
function changeSetpoint(index,delta){
 if(busy||!heatingPlan)return;
 const room=(heatingPlan.rooms||heatingPlan.control.rooms)[index];
 if(!room||!room.hasSetpoint&&!room.setpoint){show('Configura o endereço KNX do setpoint desta divisão.');return;}
 const existing=setpointEdits.get(index);const previous=existing?existing.value:room.setpointKnown?Number(room.target):20;
 const value=Math.max(5,Math.min(30,Math.round((previous+delta)*2)/2));if(value===previous)return;
 stageRoomSetpoint(index,value,true);
}
function stageRoomSetpoint(index,value,schedule=true){
 const existing=setpointEdits.get(index);const edit=existing||{value,version:0,timer:null,sending:false,name:heatingPlan.zones[index]};
 edit.value=value;edit.version++;clearTimeout(edit.timer);
 const version=edit.version;edit.timer=schedule?setTimeout(()=>sendSetpoint(index,version),5000):null;
 setpointEdits.set(index,edit);renderHeatStatus();return {edit,version};
}
async function sendSetpoint(index,version,throwOnError=false){
 const edit=setpointEdits.get(index);if(!edit||edit.version!==version)return;
 if(edit.sending){edit.timer=setTimeout(()=>sendSetpoint(index,version),250);return;}
 if(!heatingPlan||heatingPlan.zones[index]!==edit.name){setpointEdits.delete(index);renderHeatStatus();show('A divisão mudou; o setpoint pendente não foi enviado.');return;}
 const value=edit.value;edit.timer=null;edit.sending=true;renderHeatStatus();
 try{
  const result=await api('/api/heating/setpoint',{method:'POST',headers:{'Content-Type':'application/json','X-CSRF-Token':csrf},body:JSON.stringify({index,value})});
  if(edit.version===version)setpointEdits.delete(index);
  await refreshHeat();
  return {...result,value};
 }catch(e){if(edit.version===version)setpointEdits.delete(index);show('Setpoint: '+e.message);await refreshHeat().catch(()=>{});if(throwOnError)throw e;return {queued:false,error:e.message};}
 finally{edit.sending=false;if(setpointEdits.get(index)===edit&&edit.version!==version&&!edit.timer)edit.timer=setTimeout(()=>sendSetpoint(index,edit.version),5000);}
}
function renderCollector(h){
 const thermalPause=!!h.thermalPause;
 const pump=h.pumpState;const visual=thermalPause?'blocked':pump==='ligado'?'running':pump==='desligado'||pump==='desativado'?'off':pump==='pendente'?'pending':pump==='sem confirmação'?'blocked':'unknown';
 const pumpCard=$('pumpState');pumpCard.dataset.state=visual;pumpCard.setAttribute('aria-label','Bomba: '+pump);
 $('pumpStatus').textContent=thermalPause?'Pausa térmica':pump==='ligado'?'Ligada':pump==='desligado'?'Parada':pump==='pendente'?'A confirmar…':pump==='sem confirmação'?'Sem confirmação':pump==='desativado'?'Desativada':'Estado desconhecido';
 $('pumpDetail').textContent=thermalPause?(h.controlMessage||'A aguardar a pausa térmica'):visual==='running'||pump==='desligado'?'':visual==='pending'?'A aguardar feedback KNX':visual==='blocked'?'Verifica o feedback da bomba':'Sem estado confirmado';
 const phase=h.pumpPhase||'idle',total=Number(h.pumpPhaseTotalMs)||0,left=Number(h.pumpRemainingMs)||0,percent=total?Math.max(0,Math.min(100,Math.round(100*(total-left)/total))):0;
 $('pumpPhase').dataset.phase=phase;$('pumpPhase').setAttribute('aria-valuenow',String(percent));$('pumpPhaseFill').style.width=percent+'%';
 const phaseText={waiting:'À espera do arranque',service:'Tempo de serviço',pause:'Pausa de serviço','thermal-cold':'Água de entrada fria','thermal-return':'Retorno quente',idle:'Sem pedido'};
 $('pumpPhase').setAttribute('aria-valuetext',phaseText[phase]||phaseText.idle);
 const placed=h.circuits.filter(c=>c.position>=0&&c.zone>=0),count=placed.filter(c=>c.state==='ligado').length;
 $('collectorSubtitle').textContent=placed.length+' '+(placed.length===1?'circuito configurado':'circuitos configurados')+' · '+(8-placed.length)+' '+(8-placed.length===1?'posição não ligada':'posições não ligadas');
 $('collectorSummary').textContent=(thermalPause?'Pausa de proteção':pump==='ligado'?'Bomba ligada':pump==='pendente'?'Bomba a confirmar':'Bomba '+pump)+' · '+count+' '+(count===1?'circuito ativo':'circuitos ativos');
 $('branches').textContent='';
 for(let pos=0;pos<8;pos++){
  const i=h.circuits.findIndex(c=>c.position===pos&&c.zone>=0);const c=i>=0?h.circuits[i]:null;const state=c?c.state:'não ligada';
  const card=document.createElement('div');card.className='collector-circuit';card.dataset.available=String(!!c);card.dataset.active=String(state==='ligado');card.dataset.pending=String(state==='pendente'||state==='sem confirmação');card.dataset.opening=String(!!c&&c.openingRemainingMs>0);
  const room=c?h.zones[c.zone]||'Divisão':'Não ligada';card.setAttribute('role','img');card.setAttribute('aria-label',(c?'Circuito '+(i+1)+', '+room+', estado '+state+(c.openingRemainingMs>0?', válvula a abrir durante mais '+Math.ceil(c.openingRemainingMs/1000)+' segundos':''):'Posição '+(pos+1)+' não ligada'));card.title=c?'Circuito '+(i+1)+' · '+room+' · '+state+(c.openingRemainingMs>0?' · a abrir '+Math.ceil(c.openingRemainingMs/1000)+' s':''):'Posição '+(pos+1)+' não ligada';
  card.innerHTML='<span class="collector-valve" aria-hidden="true"><span class="collector-valve-led"></span></span><span class="collector-pipe" aria-hidden="true"><span class="collector-flow"></span></span>';
  const label=document.createElement('strong'),name=document.createElement('span'),opening=document.createElement('span');label.className='collector-circuit-id';label.textContent=c?'C'+(i+1):'—';name.className='collector-room';name.textContent=room;opening.className='collector-opening';opening.textContent=c&&c.openingRemainingMs>0?'A abrir '+Math.ceil(c.openingRemainingMs/1000)+' s':'';card.append(label,name,opening);$('branches').append(card);
 }
}
function renderHeatStatus(){
 const h=heatingPlan;if(!h||currentThermostatMode)return;
 renderCollector(h);
 $('heatMessage').textContent=h.controlMessage||'';const requested=h.controlRequested??h.armed;
 const toggle=$('toggleHeat');toggle.setAttribute('aria-checked',String(!!requested));
 toggle.disabled=!h.outputsEnabled||(!requested&&h.canArm===false);
 $('toggleHeatLabel').textContent=h.resumePending?'A retomar…':requested?(h.thermalPause?'Em pausa':h.armed?'Ligado':'A aguardar'):'Desligado';
 $('refreshHeat').disabled=!h.outputsEnabled;
 $('rooms').textContent=h.zones.length?'':'Adiciona divisões em Configurações.';
 h.zones.forEach((name,z)=>{const edit=setpointEdits.get(z),actual=(h.rooms||h.control.rooms)[z],r=edit?{...actual,target:edit.value,setpointKnown:true}:actual,card=document.createElement('article');card.className='thermostat room-thermostat';const on=(h.armed||h.thermalPause)&&(r.mode===2||(r.mode===0&&r.enabled&&!r.autoFault));card.dataset.powered=String(on);card.dataset.demand=String(on&&r.demand);card.style.cssText='--ring-fill:'+(r.setpointKnown?Math.round(Math.max(0,Math.min(1,(Number(r.target)-16)/14))*86):0)+'%;';card.setAttribute('aria-labelledby','room-name-'+z);
 const header=document.createElement('header');header.className='header';const title=document.createElement('h3');title.id='room-name-'+z;title.textContent=name;const status=document.createElement('span');status.className='status';status.setAttribute('aria-live','polite');status.textContent=h.thermalPause?'Pausa térmica':r.mode===0&&r.autoFault?(r.autoFaultReason||'Sem leitura válida'):!on?'Desligado':r.demand?(h.pumpState==='ligado'?'A aquecer':'A aguardar'):'Sem pedido';status.title=status.textContent;header.append(title,status);card.append(header);
 const dial=document.createElement('div');dial.className='dial';dial.setAttribute('role','img');dial.setAttribute('aria-label',r.temperatureKnown?'Temperatura ambiente: '+formatDegrees(r.value):'Temperatura ambiente desconhecida');const content=document.createElement('div');content.className='dial-content';const label=document.createElement('span');label.className='label';label.textContent='Ambiente';const current=document.createElement('output');current.className='current';current.textContent=r.temperatureKnown?Number(r.value).toFixed(1).replace('.',','):'—';const unit=document.createElement('small');unit.textContent='°C';current.append(unit);current.title=r.temperatureKnown?(r.temperatureFresh?'Origem: '+(r.source||'KNX'):r.defaultTemperature?'Valor de recurso; aguarda sensor':'Última leitura guardada; aguarda KNX'):'Aguarda temperatura KNX';const heat=document.createElement('span');heat.className='heat-state';heat.innerHTML='<svg class="flame" viewBox="0 0 24 24" aria-hidden="true"><path d="M13.7 2.2c.7 3.1-.8 4.5-2.1 5.8-1 1-1.8 1.9-1.3 3.5.8-.4 1.5-1.2 2-2.3 2.2 1.7 3.7 4 3.7 6.6a4.7 4.7 0 1 1-9.4 0c0-3.6 2.4-6.1 4.4-8.2 1.5-1.5 2.8-2.9 2.7-5.4Z"/></svg>';const heatText=document.createElement('span');heatText.textContent=h.thermalPause?'Pausa de proteção':r.mode===0&&r.autoFault?'Aguarda condições':!on?'Desligado':r.demand?'Pedido ativo':'Em espera';heat.append(heatText);content.append(label,current,heat);dial.append(content);card.append(dial);
 const setpoint=document.createElement('section');setpoint.className='setpoint';setpoint.setAttribute('aria-label','Temperatura pretendida');const controls=document.createElement('div');controls.className='setpoint-controls';const decrease=document.createElement('button'),increase=document.createElement('button');for(const [b,sign,delta,word]of [[decrease,'−',-0.5,'Diminuir'],[increase,'+',0.5,'Aumentar']]){b.type='button';b.className='step '+(delta<0?'decrease':'increase');b.textContent=sign;b.setAttribute('aria-label',word+' o setpoint de '+name+' em 0,5 graus');b.disabled=!(r.hasSetpoint||r.setpoint)||!h.outputsEnabled;b.onclick=()=>changeSetpoint(z,delta);}const target=document.createElement('div');target.className='setpoint-value';const targetOutput=document.createElement('output');targetOutput.setAttribute('aria-live','polite');targetOutput.textContent=r.setpointKnown?formatDegrees(r.target):'— °C';target.append(targetOutput);controls.append(decrease,target,increase);const pending=document.createElement('p');pending.className='pending';pending.dataset.visible=String(!!r.setpointPending||!!edit?.sending);pending.setAttribute('aria-live','polite');pending.textContent=edit?(edit.sending?'A enviar ao KNX…':'Envia ao KNX após 5 s sem alterações'):r.setpointPending?'A confirmar envio KNX…':r.setpointFailed?'Falha no envio KNX':'';setpoint.append(controls,pending);card.append(setpoint);
 const footer=document.createElement('footer');footer.className='tile-footer';const power=document.createElement('button');power.type='button';power.className='power';power.hidden=r.mode===0;power.disabled=!h.armed;power.setAttribute('aria-pressed',String(on));power.setAttribute('aria-label',(on?'Desligar':'Ligar')+' o aquecimento de '+name);power.innerHTML='<svg viewBox="0 0 24 24" aria-hidden="true"><path d="M12 2v10M5.6 5.6a9 9 0 1 0 12.8 0"/></svg>';power.onclick=()=>heatAction('room',z,on?1:2);const mode=document.createElement('button');mode.type='button';mode.className='mode';mode.disabled=!h.armed;mode.dataset.mode=r.mode===0?(r.autoFault?'fault':'automatic'):'manual';mode.setAttribute('aria-pressed',String(r.mode===0));mode.textContent=r.mode===0?'Automático':'Manual';mode.title=r.autoFault?(r.autoFaultReason||'A aguardar temperatura e setpoint válidos'):r.external?'Seguir os pedidos do termostato KNX':'Controlar pela temperatura e setpoint';mode.onclick=()=>heatAction('room',z,r.mode===0?1:0);footer.append(power,mode);card.append(footer);$('rooms').append(card);});

}
async function refreshHeat(){heatingPlan=await api('/api/heating/status');renderHeatStatus();}
function captureHeating(){
 const h=heatingDraft;if(!h)return;
 h.control.enabled=$('heatEnabled').checked;h.control.openMs=durationForApi('openMs',$('heatOpen').value);h.control.maxAgeMs=durationForApi('maxAgeMs',$('heatAge').value);h.control.serviceMs=durationForApi('serviceMs',$('heatService').value);h.control.pauseMs=durationForApi('pauseMs',$('heatPause').value);
 h.zones=h.zones.map((_,i)=>$('zone'+i).value.trim());
 h.control.rooms=h.control.rooms.map((r,i)=>({...Object.fromEntries(['enable','status','temperature','setpoint'].map(k=>[k,$('hz'+i+k).value.trim()])),external:$('hz'+i+'external').checked,bmeFallback:$('hz'+i+'bmeFallback').checked,margin:$('hz'+i+'margin').value}));
 h.control.actuators=h.control.actuators.map((a,i)=>Object.fromEntries(['command','feedback'].map(k=>[k,$('ha'+i+k).value.trim()])));
 h.circuits.forEach((c,i)=>{c.position=Number($('circuitPosition'+i).value);});
}
$('addRoom').onclick=()=>{captureHeating();if(heatingDraft.zones.length>=16){show('Máximo de 16 divisões nesta versão.');return;}heatingDraft.zones.push('Divisão '+(heatingDraft.zones.length+1));heatingDraft.control.rooms.push({enable:'',status:'',temperature:'',setpoint:''});renderHeating(false);};
function heatingControlForSave(zones,control){
 const rooms=control.rooms.map((room,i)=>{
  const raw=room.margin.trim().replace(',','.');
  const margin=/^\d+(?:\.\d+)?$/.test(raw)?Number(raw):NaN;
  if(!Number.isFinite(margin)||margin<0.1||margin>5){
   selectTab('features');$('hz'+i+'margin').focus();
   throw Error('Histerese de «'+(zones[i]||('Divisão '+(i+1)))+'»: indica um valor entre 0,1 e 5,0 °C.');
  }
  return {...room,margin:Number(margin.toFixed(1))};
 });
 return {...control,rooms};
}
$('saveHeating').onclick=async()=>{if(busy||!heatingDraft)return;if(setpointEdits.size||thermoEdit){show('Aguarda o envio e a confirmação dos setpoints antes de guardar.');return;}busy=true;try{captureHeating();const {zones,circuits,control}=heatingDraft;const payload={zones,circuits,control:heatingControlForSave(zones,control)};await waitForSafeSave(()=>adminApi('/api/heating/layout',{method:'POST',headers:{'Content-Type':'application/json','X-CSRF-Token':csrf},body:JSON.stringify(payload)}),'Mapa do aquecimento guardado após feedback OFF. O ESP está a reiniciar.');}catch(e){show('Mapa do aquecimento não guardado: '+e.message);}finally{busy=false;}};
function renderHeating(reset=true){
 if(!heatingPlan)return;
 renderHeatStatus();if(reset)heatingDraft=JSON.parse(JSON.stringify(heatingPlan));const h=heatingDraft;
 for(const id of ['heatSettings','zoneFields','zoneKnxFields','circuitFields','circuitKnxFields'])$(id).textContent='';
 field('heatSettings','heatEnabled','Ativar controlo KNX do coletor',h.control.enabled,'checkbox');
 field('heatSettings','heatOpen','Tempo de abertura das válvulas (s)',durationForUi('openMs',h.control.openMs),'number','0.1');
 field('heatSettings','heatService','Tempo de serviço da bomba (min)',durationForUi('serviceMs',h.control.serviceMs??900000),'number','1');
 field('heatSettings','heatPause','Pausa de serviço da bomba (min)',durationForUi('pauseMs',h.control.pauseMs??300000),'number','1');
 field('heatSettings','heatAge','Validade da temperatura KNX (s)',durationForUi('maxAgeMs',h.control.maxAgeMs),'number','0.1');
 h.zones.forEach((name,i)=>{
  const box=document.createElement('fieldset');box.id='hz'+i;const legend=document.createElement('legend');legend.textContent='Divisão '+(i+1);box.append(legend);$('zoneFields').append(box);
  field(box.id,'zone'+i,'Nome da divisão',name);
  field(box.id,'hz'+i+'external','Termóstato KNX externo (desmarcado: automático ESP)',h.control.rooms[i].external||false,'checkbox');
  const margin=h.control.rooms[i].margin??0.3;
  const marginInput=field(box.id,'hz'+i+'margin','Margem ± °C (histerese)',typeof margin==='number'?margin.toFixed(1).replace('.',','):margin);
  marginInput.inputMode='decimal';
  marginInput.placeholder='0,3';
  field(box.id,'hz'+i+'bmeFallback','Usar BME local quando faltar temperatura KNX válida',h.control.rooms[i].bmeFallback||false,'checkbox');
  const list=document.createElement('div');box.append(list);
  h.circuits.forEach((c,j)=>{if(c.zone!==i)return;const row=document.createElement('p'),remove=document.createElement('button');remove.type='button';remove.textContent='Retirar circuito '+(j+1);remove.onclick=()=>{captureHeating();h.circuits[j].zone=-1;renderHeating(false);};row.append(remove);list.append(row);});
  const select=document.createElement('select');h.circuits.forEach((c,j)=>{if(c.zone<0){const o=document.createElement('option');o.value=j;o.textContent='Circuito '+(j+1);select.append(o);}});
  const add=document.createElement('button');add.type='button';add.textContent='Associar circuito';add.disabled=!select.options.length;add.onclick=()=>{captureHeating();h.circuits[Number(select.value)].zone=i;renderHeating(false);};
  const removeRoom=document.createElement('button');removeRoom.type='button';removeRoom.textContent='Remover divisão';removeRoom.onclick=()=>{captureHeating();h.zones.splice(i,1);h.control.rooms.splice(i,1);h.circuits.forEach(c=>{if(c.zone===i)c.zone=-1;else if(c.zone>i)c.zone--;});renderHeating(false);};box.append(select,add,document.createElement('hr'),removeRoom);
  const knx=document.createElement('fieldset');knx.id='hzk'+i;const title=document.createElement('legend');title.textContent=name;knx.append(title);$('zoneKnxFields').append(knx);
  for(const [k,label] of [['enable','Pedido/ativação · DPT 1.001'],['status','Feedback do pedido · DPT 1.001'],['temperature','Temperatura recebida · DPT 9.001'],['setpoint','Setpoint recebido · DPT 9.001']])field(knx.id,'hz'+i+k,label,h.control.rooms[i][k]);
 });
 h.control.actuators.forEach((a,i)=>{
  if(i<8){const box=document.createElement('fieldset');box.id='hap'+i;const title=document.createElement('legend');title.textContent='Circuito '+(i+1);box.append(title);$('circuitFields').append(box);const select=document.createElement('select');select.id='circuitPosition'+i;['Por atribuir',...Array.from({length:8},(_,j)=>'Posição '+(j+1))].forEach((label,j)=>{const option=document.createElement('option');option.value=j-1;option.textContent=label;select.append(option);});select.value=h.circuits[i].position;box.append(select);}
  const knx=document.createElement('fieldset');knx.id='ha'+i;knx.hidden=i<8&&h.circuits[i].zone<0;const title=document.createElement('legend');title.textContent=i===8?'Bomba':'Circuito '+(i+1);knx.append(title);$('circuitKnxFields').append(knx);
  for(const k of ['command','feedback'])field(knx.id,'ha'+i+k,'Grupo KNX · '+(k==='command'?'comando':'feedback'),a[k]);
 });
}
function renderRelays(){
 for(const id of ['relayFeature','relayHardware','relayKnx'])$(id).textContent='';
 cfg.relays.forEach((r,i)=>{
  for(const [parent,prefix,list] of [['relayFeature','rf',rsFeature],['relayHardware','rh',rsHardware],['relayKnx','relayK',rsKnx]]){
   const f=document.createElement('fieldset');f.id=prefix+i;const title=document.createElement('legend');title.textContent='Relé '+(i+1);f.append(title);$(parent).append(f);for(const[k,label,t]of list)field(f.id,'r'+i+'_'+k,label,r[k],t);
  }
  $('r'+i+'_enabled').onchange=visibility;
 });
}
function renderInputs(){
 for(const id of ['inputFeature','inputHardware','inputKnx'])$(id).textContent='';
 (cfg.inputs||[]).forEach((input,i)=>{
  for(const [parent,prefix,list] of [['inputFeature','if',isFeature],['inputHardware','ih',isHardware],['inputKnx','inputK',isKnx]]){
   const f=document.createElement('fieldset');f.id=prefix+i;const title=document.createElement('legend');title.textContent='Entrada '+(i+1);f.append(title);$(parent).append(f);for(const[k,label,t]of list)field(f.id,'in'+i+'_'+k,label,input[k],t);
  }
  $('in'+i+'_enabled').onchange=visibility;
 });
}
async function switchRelay(index,on){if(busy)return;busy=true;try{await api('/api/relay',{method:'POST',headers:{'Content-Type':'application/json','X-CSRF-Token':csrf},body:JSON.stringify({index,on})});show('Comando aplicado; feedback KNX será enviado quando houver ligação.');}catch(e){show(e.message);}finally{busy=false;await refresh();}}
function value(id,type){return type==='checkbox'?$(id).checked:type==='number'?Number($(id).value):$(id).value;}
async function waitForSafeSave(startRequest,successMessage){
 const started=await startRequest();
 if(started&&started.saved){if(location.pathname!='/config.html')dashboard();show(successMessage||'Configuração guardada; ESP a reiniciar.');return;}
 const deadline=Date.now()+100000;let disconnectedAt=0;
 show('A parar a bomba e a aguardar confirmação OFF dos circuitos…');
 while(Date.now()<deadline){
  await new Promise(resolve=>setTimeout(resolve,350));
  try{
   const status=await adminApi('/api/config/save-status');disconnectedAt=0;
   if(status.state==='saved'){if(location.pathname!='/config.html')dashboard();show(successMessage||status.message||'Configuração guardada; ESP a reiniciar.');return;}
   if(status.state==='failed'){const failure=Error(status.message||'A configuração não foi gravada.');failure.saveFailure=true;throw failure;}
   if(status.message)show(status.message);
  }catch(e){
   if(e.status===401||e.saveFailure)throw e;
   if(!disconnectedAt)disconnectedAt=Date.now();
   if(Date.now()-disconnectedAt>=8000){if(location.pathname!='/config.html')dashboard();show('A ligação caiu após iniciar a gravação. Confirma se o ESP reiniciou e verifica o endereço de rede configurado.');return;}
  }
 }
 throw Error('Tempo esgotado à espera de confirmação OFF. A configuração não foi confirmada.');
}
async function applyConfiguration(next,successMessage){
 return waitForSafeSave(()=>adminApi('/api/config',{method:'POST',headers:{'Content-Type':'application/json','X-CSRF-Token':csrf},body:JSON.stringify(next)}),successMessage);
}
function render(){
 $('ntpServer').value=cfg.ntpServer||'pool.ntp.org';$('ntpSecondary').value=cfg.ntpSecondary||'time.cloudflare.com';orbitTimezone('timezone',cfg.timezone||'WET0WEST,M3.5.0/1,M10.5.0');refreshTime();
 for(const [list,parent]of [[gs.filter(x=>!['ntpServer','ntpSecondary','timezone'].includes(x[0])),'general'],[hs,'hardware'],[ks,'knx'],[tsFeature,'thermostatFeature'],[tsKnx,'thermostatKnx']]){
  $(parent).textContent='';for(const [key,label,type,step]of list)field(parent,key,label,durationForUi(key,cfg[key]),type,step);
 }
 const staticNetwork=()=>{for(const id of ['wifiIp','wifiMask','wifiGateway','wifiDns'])$(id).disabled=$('wifiDhcp').checked};$('wifiDhcp').onchange=staticNetwork;staticNetwork();
 $('pumpGuardSettings').textContent='';for(const [key,label,type,step]of pumpGuardFields)field('pumpGuardSettings',key,label,cfg[key],type,step);
 $('profileMode').value=cfg.thermostatMode?'thermostat':'collector';$('profileMode').onchange=visibility;
 $('sensorFeature').textContent='';
 field('sensorFeature','bmeEnabled','Ativar BME280',cfg.bmeEnabled,'checkbox');
 for(let i=0;i<2;i++)field('hardware','rom'+i,'ROM da sonda '+(i+1),cfg.rom[i]);
 $('channels').textContent='';cfg.channels.forEach((c,i)=>{
  const b=field('sensorFeature','channelEnabled'+i,['Temperatura BME280','Humidade BME280','Pressão BME280','DS18B20 1','DS18B20 2'][i]+' · enviar KNX',c.enabled,'checkbox');b.onchange=visibility;
  const f=document.createElement('fieldset');const title=document.createElement('legend');title.textContent=['Temperatura BME280 · °C','Humidade · %','Pressão · Pa','DS18B20 1 · °C','DS18B20 2 · °C'][i];f.append(title);$('channels').append(f);
  const g=document.createElement('div');g.id='ch'+i;g.className='grid';f.append(g);for(const [key,label,type,step]of cs)field(g.id,i+'_'+key,label,durationForUi(key,c[key]),type,step);
 });
}
$('settings').onsubmit=async e=>{e.preventDefault();if(busy)return;if(setpointEdits.size||thermoEdit){show('Aguarda 5 segundos pelo envio do setpoint antes de guardar.');return;}const knxProblems=validateKnxFields();if(knxProblems.length){visibility();selectTab('knx');show(knxProblems.join('; '));return;}busy=true;try{
 const next=JSON.parse(JSON.stringify(cfg));
 for(const list of [gs,hs,ks,tsFeature,tsKnx])for(const [k,,t]of list)next[k]=durationForApi(k,value(k,t));
 for(const [k,,t]of pumpGuardFields)next[k]=value(k,t);
 next.bmeEnabled=$('bmeEnabled').checked;next.thermostatMode=$('profileMode').value==='thermostat';
 next.relays=cfg.relays.map((r,i)=>{const n={...r};for(const[k,,t]of [...rsFeature,...rsHardware,...rsKnx])n[k]=value('r'+i+'_'+k,t);return n;});
 next.inputs=(cfg.inputs||[]).map((r,i)=>{const n={...r};for(const[k,,t]of [...isFeature,...isHardware,...isKnx])n[k]=value('in'+i+'_'+k,t);return n;});
 next.rom=[value('rom0'),value('rom1')];
 next.channels=cfg.channels.map((c,i)=>{const n={...c,enabled:$('channelEnabled'+i).checked};for(const[k,,t]of cs)n[k]=durationForApi(k,value(i+'_'+k,t));return n;});
 await applyConfiguration(next,'Configuração guardada. O ESP reinicia e confirma OFF no arranque.');
}catch(e){show(e.message);}finally{busy=false;}};
$('saveOneWire').onclick=async()=>{if(busy)return;const pin=Number($('oneWire').value);if($('oneWire').value.trim()===''||!Number.isInteger(pin)){show('Indica um GPIO inteiro para DS18B20 (ou -1 para desativar).');return;}busy=true;try{
 const next=JSON.parse(JSON.stringify(cfg));next.oneWire=pin;next.rom=[value('rom0').trim(),value('rom1').trim()];
 await applyConfiguration(next,'GPIO e ROM das sondas guardados. O ESP reinicia e confirma OFF no arranque.');
}catch(e){show('GPIO e ROM das sondas não guardados: '+e.message);}finally{busy=false;}};
$('oledPreset').onclick=()=>{for(const [id,value]of [['oledSda',4],['oledScl',15],['oledReset',16],['oledAddress',60],['oledHeight',64]])$(id).value=value;$('oledEnabled').checked=true;show('Pinos do OLED integrado aplicados. Guarda e reinicia para testar a deteção.');};
$('export').onclick=async()=>{try{const backup=await adminApi('/api/backup');const b=new Blob([JSON.stringify(backup,null,2)],{type:'application/json'}),url=URL.createObjectURL(b),a=document.createElement('a');a.href=url;a.download='iot-sensor-backup.json';a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);}catch(e){show(e.message);}};
$('restore').onclick=async()=>{const file=$('restoreFile').files?.[0];if(!file){show('Escolhe um backup JSON.');return;}if(busy)return;if(setpointEdits.size||thermoEdit){show('Aguarda o envio do setpoint antes de restaurar.');return;}try{const data=await file.text();const parsed=JSON.parse(data);if(parsed.format!=='iot-sensor-backup-v1')throw Error('Formato de backup inválido');if(!confirm('Restaurar a configuração e reiniciar?'))return;busy=true;await adminApi('/api/backup',{method:'POST',headers:{'Content-Type':'application/json','X-CSRF-Token':csrf},body:data});if(location.pathname!='/config.html')dashboard();show('Backup restaurado. O dispositivo vai reiniciar.');}catch(e){show(e.message);}finally{busy=false;}};
$('knxOperation').onchange=()=>{$('knxValueLabel').hidden=$('knxOperation').value==='read';};
$('knxSend').onclick=async()=>{if(busy)return;const operation=$('knxOperation').value,raw=$('knxValue').value.trim();let val;
 if(operation==='switch'){if(!['0','1'].includes(raw)){show('Para DPT 1.001 usa 0 ou 1.');return;}val=raw==='1';}
 if(operation==='float'){val=Number(raw.replace(',','.'));if(raw===''||!Number.isFinite(val)){show('Indica um valor numérico.');return;}}
 busy=true;try{await adminApi('/api/knx/command',{method:'POST',headers:{'Content-Type':'application/json','X-CSRF-Token':csrf},body:JSON.stringify({group:$('knxGroup').value.trim(),operation,value:val})});show('Telegrama colocado na fila KNX; aguarda confirmação.');}catch(e){show(e.message);}finally{busy=false;}};
$('update').onclick=async()=>{if(busy)return;const file=$('firmware').files[0];if(!file){show('Seleciona um ficheiro .bin.');return;}let version;try{version=await validateFirmware(file,'iot-sensor-device','sensor-esp32',0);}catch(e){show(e.message);return;}if(!confirm('Instalar '+version+' neste equipamento e reiniciar?'))return;busy=true;const data=new FormData();data.append('firmware',file);const x=new XMLHttpRequest();x.open('POST','/api/update');x.setRequestHeader('X-CSRF-Token',csrf);x.setRequestHeader('X-Admin-Session',adminSession);x.setRequestHeader('X-Firmware-Size',String(file.size));x.upload.onprogress=e=>{if(e.lengthComputable){$('progress').hidden=false;$('progress').value=e.loaded/e.total*100;}};x.onload=()=>{show(x.status===200?'Atualização concluída. O dispositivo vai reiniciar.':'Atualização recusada ou falhou: '+x.responseText);busy=false;};x.onerror=()=>{show('Ligação interrompida. Confirma o estado do dispositivo.');busy=false;};x.send(data);};
function updateClock(){const clock=$('localClock');if(clock)clock.textContent=new Date().toLocaleTimeString('pt-PT',{hour:'2-digit',minute:'2-digit',second:'2-digit'});}
let thermoEdit=null;
function renderThermostat(t){
 if(!t)return;
 const target=thermoEdit?thermoEdit.value:t.setpoint,on=t.on,demand=on&&t.demand;
 $('thermoName').textContent=t.name||'Termóstato';$('thermoTile').dataset.powered=String(on);$('thermoTile').dataset.demand=String(demand);
 $('thermoTemperature').textContent=t.temperatureValid?formatDegrees(t.temperature):'— °C';
 $('thermoDial').setAttribute('aria-label',t.temperatureValid?'Temperatura ambiente: '+formatDegrees(t.temperature):'Sem leitura BME280');
 $('thermoTarget').textContent=formatDegrees(target);
 $('thermoState').textContent=!on?'Desligado':!t.temperatureValid?'Sem leitura':!t.knxConnected?'KNX desligado':t.failed?'Sem feedback':t.pending?'A confirmar':demand?'A aquecer':'Em espera';
 $('thermoDemand').textContent=!on?'Desligado':!t.temperatureValid?'Aguardar BME280':demand?'Pedido de calor':'Sem pedido';
 $('thermoPower').setAttribute('aria-pressed',String(on));
 $('thermoPower').setAttribute('aria-label',on?'Desligar termóstato':'Ligar termóstato');
 $('thermoPending').textContent=thermoEdit?(thermoEdit.sending?'A enviar ao KNX…':'Envia após 5 s sem alterações'):t.failed?'Sem confirmação do atuador':t.pending?'A aguardar feedback KNX…':!t.knxConnected?'A aguardar ligação KNX':'';
 $('thermoHelp').textContent=t.feedbackKnown?'Último feedback KNX: '+(t.feedback?'pedido ativo':'pedido inativo'):'Ainda não foi recebido feedback KNX';
}
function stageThermoSetpoint(value,schedule=true){
 const edit=thermoEdit||{value,version:0,timer:null,sending:false};edit.value=value;edit.version++;
 clearTimeout(edit.timer);const version=edit.version;edit.timer=schedule?setTimeout(()=>sendThermo(version),5000):null;thermoEdit=edit;renderThermostat(latestThermo);return {edit,version};
}
function changeThermo(delta){if(busy||!latestThermo)return;
 const previous=thermoEdit?thermoEdit.value:latestThermo.setpoint;
 const target=Math.max(5,Math.min(30,Math.round((previous+delta)*2)/2));if(target===previous)return;
 stageThermoSetpoint(target,true);
}
async function sendThermo(version,throwOnError=false){const edit=thermoEdit;if(!edit||edit.version!==version)return;
 if(edit.sending){edit.timer=setTimeout(()=>sendThermo(version),250);return;}
 const value=edit.value;edit.timer=null;edit.sending=true;renderThermostat(latestThermo);
 try{const result=await api('/api/thermostat',{method:'POST',headers:{'Content-Type':'application/json','X-CSRF-Token':csrf},body:JSON.stringify({setpoint:value})});if(edit.version===version)thermoEdit=null;await refresh();return {...result,value};}
 catch(e){if(edit.version===version)thermoEdit=null;show('Termóstato: '+e.message);await refresh().catch(()=>{});if(throwOnError)throw e;return {queued:false,error:e.message};}
 finally{edit.sending=false;if(thermoEdit===edit&&edit.version!==version&&!edit.timer)edit.timer=setTimeout(()=>sendThermo(edit.version),5000);}
}
let latestThermo=null;
$('thermoDown').onclick=()=>changeThermo(-0.5);$('thermoUp').onclick=()=>changeThermo(0.5);
$('thermoPower').onclick=async()=>{if(busy||!latestThermo)return;busy=true;try{await api('/api/thermostat',{method:'POST',headers:{'Content-Type':'application/json','X-CSRF-Token':csrf},body:JSON.stringify({on:!latestThermo.on})});}catch(e){show(e.message);}finally{busy=false;await refresh();}};
let clockSyncAttempt=0;
function sensorDisplay(channel,index){
 if(!channel?.valid)return 'Sem leitura';
 const value=index===2?channel.value/1000:channel.value;
 return value.toLocaleString('pt-PT',{minimumFractionDigits:1,maximumFractionDigits:1})+' '+(index===2?'kPa':channel.unit);
}
function appendTrend(element,channel,index){
 const delta=channel?.trend30m;
 if(!channel?.valid||!Number.isFinite(delta)||index>1)return;
 const threshold=index===0?0.1:1;
 const direction=delta>=threshold?'up':delta<=-threshold?'down':'flat';
 const label={up:'A subir',down:'A descer',flat:'Estável'}[direction]+' nos últimos 30 minutos';
 const arrow=document.createElement('span');arrow.className='sensor-trend';arrow.dataset.direction=direction;
 arrow.textContent={up:'↑',down:'↓',flat:'→'}[direction];arrow.title=label;arrow.setAttribute('role','img');arrow.setAttribute('aria-label',label);element.append(arrow);
}
async function refresh(){if(busy)return;try{
 const s=adminSession&&location.pathname==='/config.html'?await adminApi('/api/status'):await api('/api/status');orbitShowPower(s);csrf=s.csrf;if(s.historyTime===false&&Date.now()-clockSyncAttempt>300000){clockSyncAttempt=Date.now();api('/api/history/time',{method:'POST',headers:{'Content-Type':'application/json','X-CSRF-Token':csrf},body:JSON.stringify({time:Math.floor(Date.now()/1000)})}).catch(()=>{});}currentThermostatMode=!!s.thermostatMode;latestThermo=s.thermostat;
 if($('settingsArea').hidden&&$('historyArea').hidden&&$('helpArea').hidden&&$('sensorArea').hidden){$('dashboardArea').hidden=currentThermostatMode;$('thermostatView').hidden=!currentThermostatMode;}
 renderThermostat(latestThermo);
 for(const [id,index] of [['supplyTemperature',3],['returnTemperature',4]]){const c=s.channels[index];$(id).textContent=c&&c.valid?c.value.toFixed(1)+' °C':'Sem leitura';}
 $('deviceName').textContent=s.name||'Sensor KNX';document.title=(s.name||'Sensor KNX')+' · MySmartHouse';
 $('orbitVersion').textContent='Firmware '+(s.firmware||'—')+(s.build?' · '+s.build:'');$('connection').textContent=s.ip+' · KNX '+s.knx;
 $('status').textContent='Wi-Fi: '+s.rssi+' dBm | memória livre: '+s.freeHeap+' bytes\nUptime: '+Math.floor(s.uptimeMs/1000)+' s | BME280: '+(s.bme?'detetado':'ausente/desativado')+' | OLED: '+(s.oled?'ativo':'ausente/desativado')+'\nI²C principal: '+(s.i2cMain||'não iniciado')+' | I²C OLED: '+(s.i2cOled||'não iniciado')+'\nKNX individual: '+s.individual+' | enviados: '+s.sent+' | confirmados: '+s.acked+' | falhas: '+s.failed+'\n'+s.lastEvent;
 $('readings').textContent='';for(const [i,name]of [[0,'Temperatura'],[1,'Humidade'],[2,'Pressão'],[-1,'Hora']]){const d=document.createElement('div');d.className='card';const title=document.createElement('div'),v=document.createElement('div');title.textContent=name;v.className='value';if(i<0){v.id='localClock';v.title='Hora local do dispositivo que mostra esta página';}else{const c=s.channels[i];v.textContent=sensorDisplay(c,i);appendTrend(v,c,i);}d.append(title,v);$('readings').append(d);}updateClock();
 const sensorReadings=$('sensorReadings');sensorReadings.textContent='';
 for(const [i,label] of ['Temperatura ambiente','Humidade','Pressão','Ida do coletor','Retorno do coletor'].entries()){
  const c=s.channels[i],tile=document.createElement('article'),heading=document.createElement('h3'),value=document.createElement('div');
  tile.className='card';heading.textContent=label;value.className='value';value.textContent=sensorDisplay(c,i);appendTrend(value,c,i);tile.append(heading,value);sensorReadings.append(tile);
 }
 $('outputs').textContent='';$('relaySection').hidden=!s.relays.some(r=>r.enabled);$('diagnosticSection').hidden=s.showDiagnostics===false;
 s.relays.forEach((r,i)=>{if(!r.enabled)return;const d=document.createElement('div');d.className='card';const title=document.createElement('strong'),p=document.createElement('p'),b=document.createElement('button');title.textContent=r.name;p.textContent=(r.on?'Ligado':'Desligado')+(r.gpio===undefined?'':' · GPIO '+r.gpio)+(r.feedbackPending?' · feedback pendente':' · feedback enviado');b.textContent=r.on?'Desligar':'Ligar';b.onclick=()=>switchRelay(i,!r.on);d.append(title,p,b);$('outputs').append(d);});
 $('inputStates').textContent='';$('inputSection').hidden=!(s.inputs||[]).some(i=>i.enabled);
 for(const item of s.inputs||[]){if(!item.enabled)continue;const tile=document.createElement('div');tile.className='card';tile.textContent=item.name+' · '+(item.active?'Ativo':'Inativo')+(item.gpio===undefined?'':' · GPIO '+item.gpio);$('inputStates').append(tile);}
 $('roms').textContent=(s.roms||[]).join(', ')||'Nenhuma; configura o GPIO e reinicia';
 $('hardwareRoms').textContent=$('roms').textContent;
 const log=(s.telegrams||[]).join('\n')||'Ainda não foram recebidos telegramas';$('telegrams').textContent=log;if(adminSession&&location.pathname==='/config.html')$('knxConsole').textContent=log;return s;
 }catch(e){orbitShowPower(null);$('connection').textContent='Sem ligação — '+e.message;for(const id of ['supplyTemperature','returnTemperature'])$(id).textContent='Sem ligação';}
}
let webMcpController=null;
function webMcpSchema(properties,required=Object.keys(properties)){return {type:'object',properties,required,additionalProperties:false};}
function webMcpTool(name,title,description,inputSchema,execute,readOnly=false){return {name,title,description,inputSchema,execute,annotations:{readOnlyHint:readOnly,destructiveHint:false,untrustedContentHint:true}};}
async function webMcpFetchStatus(){const status=await api('/api/status');if(status.csrf)csrf=status.csrf;return status;}
async function webMcpFetchHeating(){const state=await api('/api/heating/status');heatingPlan=state;renderHeatStatus();return state;}
async function webMcpEnsureCsrf(){if(csrf)return;const status=await webMcpFetchStatus();if(!status.csrf)throw Error('O dispositivo não forneceu token CSRF; atualiza a página e tenta novamente.');}
function webMcpRoomIndex(state,name){
 if(typeof name!=='string'||!name.trim())throw Error('Indica o nome da divisão.');
 const wanted=name.trim().toLocaleLowerCase('pt-PT');const matches=state.zones.map((room,index)=>({room,index})).filter(x=>String(x.room).trim().toLocaleLowerCase('pt-PT')===wanted);
 if(!matches.length)throw Error('Não encontrei essa divisão na configuração atual.');
 if(matches.length>1)throw Error('Há mais do que uma divisão com esse nome; renomeia-as para poderes escolher sem ambiguidade.');
 return matches[0];
}
function webMcpRoomSummary(state,index){const room=(state.rooms||[])[index]||{};return {name:state.zones[index],mode:room.mode===0?'automatic':room.mode===1?'off':room.mode===2?'on':'unknown',temperatureC:room.temperatureKnown&&room.temperatureFresh?room.value:null,setpointC:room.setpointKnown?room.target:null,demand:!!room.demand,setpointPending:!!room.setpointPending,setpointFailed:!!room.setpointFailed,fault:room.autoFault?room.autoFaultReason||'Controlo automático indisponível':null};}
function webMcpHeatingSummary(state){return {controlEnabled:!!state.outputsEnabled,armed:!!state.armed,canArm:!!state.canArm,fault:!!state.fault,controlMessage:state.controlMessage||'',pump:{state:state.pumpState||'desconhecido',phase:state.pumpPhase||'idle',remainingMs:Number(state.pumpRemainingMs)||0,thermalPause:!!state.thermalPause,thermalReason:state.thermalReason||'none'},rooms:(state.zones||[]).map((_,i)=>webMcpRoomSummary(state,i)),circuits:(state.circuits||[]).map((c,i)=>({circuit:i+1,position:c.position>=0?c.position+1:null,room:c.zone>=0?state.zones[c.zone]||null:null,state:c.state||'desconhecido',valveOpeningRemainingMs:Number(c.openingRemainingMs)||0}))};}
function webMcpDelay(ms,signal){return new Promise((resolve,reject)=>{
 if(signal&&signal.aborted){reject(new DOMException('Operação cancelada antes do envio.','AbortError'));return;}
 const timer=setTimeout(done,ms);function cleanup(){if(signal)signal.removeEventListener('abort',cancel);}
 function done(){cleanup();resolve();}function cancel(){clearTimeout(timer);cleanup();reject(new DOMException('Operação cancelada antes do envio.','AbortError'));}
 if(signal)signal.addEventListener('abort',cancel,{once:true});
});}
async function webMcpMutation(path,payload,refreshTarget='device'){
 if(busy)throw Error('Há outra operação ativa na página; espera que termine e tenta novamente.');
 await webMcpEnsureCsrf();if(busy)throw Error('Há outra operação ativa na página; espera que termine e tenta novamente.');
 busy=true;let response;
 try{response=await api(path,{method:'POST',headers:{'Content-Type':'application/json','X-CSRF-Token':csrf},body:JSON.stringify(payload)});}
 finally{busy=false;}
 if(refreshTarget==='heating'){await refreshHeat();return {response,state:heatingPlan};}
 const state=await refresh();return {response,state};
}
async function webMcpSetRoomSetpoint(input,signal){
 if(busy)throw Error('Há outra operação ativa na página; espera que termine antes de alterar o setpoint.');
 const target=input?.temperatureC;if(typeof target!=='number'||!Number.isFinite(target)||target<5||target>30||Math.abs(target*2-Math.round(target*2))>0.001)throw Error('O setpoint tem de estar entre 5 e 30 °C, em passos de 0,5 °C.');
 const state=await webMcpFetchHeating();if(busy)throw Error('Há outra operação ativa na página; tenta novamente quando terminar.');if(!state.outputsEnabled)throw Error('O controlo de piso radiante está desativado ou em modo termóstato.');
 const {room,index}=webMcpRoomIndex(state,input.room);const config=(state.rooms||[])[index]||{};
 if(!config.hasSetpoint)throw Error('Esta divisão não tem endereço KNX de setpoint configurado.');
 const staged=stageRoomSetpoint(index,target,false);
 try{
  await webMcpDelay(5000,signal);if(signal&&signal.aborted)throw new DOMException('Operação cancelada antes do envio.','AbortError');if(busy)throw Error('Outra operação começou durante a espera; o setpoint não foi enviado.');
  if(setpointEdits.get(index)?.version!==staged.version)throw Error('O setpoint foi alterado durante a espera; não enviei o valor antigo.');
  const sent=await sendSetpoint(index,staged.version,true);if(!sent||!sent.queued)throw Error(sent?.error||'O setpoint não foi colocado na fila KNX.');
  const updated=heatingPlan||await webMcpFetchHeating();return {accepted:true,room,valueC:target,knxQueued:true,feedbackPending:!!(updated.rooms||[])[index]?.setpointPending,message:'Setpoint guardado e colocado na fila KNX; a confirmação do barramento pode ainda estar pendente.'};
 }catch(error){const edit=setpointEdits.get(index);if(edit===staged.edit&&edit.version===staged.version&&!edit.sending){clearTimeout(edit.timer);setpointEdits.delete(index);renderHeatStatus();}throw error;}
}
async function webMcpSetThermostatSetpoint(input,signal){
 if(busy)throw Error('Há outra operação ativa na página; espera que termine antes de alterar o setpoint.');
 const target=input?.temperatureC;if(typeof target!=='number'||!Number.isFinite(target)||target<5||target>30||Math.abs(target*2-Math.round(target*2))>0.001)throw Error('O setpoint tem de estar entre 5 e 30 °C, em passos de 0,5 °C.');
 const status=await webMcpFetchStatus();latestThermo=status.thermostat;currentThermostatMode=!!status.thermostatMode;renderThermostat(latestThermo);
 if(busy)throw Error('Há outra operação ativa na página; tenta novamente quando terminar.');if(!currentThermostatMode)throw Error('O modo termóstato local não está ativo.');
 const staged=stageThermoSetpoint(target,false);
 try{await webMcpDelay(5000,signal);if(signal&&signal.aborted)throw new DOMException('Operação cancelada antes do envio.','AbortError');if(busy)throw Error('Outra operação começou durante a espera; o setpoint não foi enviado.');if(thermoEdit?.version!==staged.version)throw Error('O setpoint foi alterado durante a espera; não enviei o valor antigo.');
  const sent=await sendThermo(staged.version,true);if(!sent||(!sent.queued&&!sent.ok))throw Error(sent?.error||'O setpoint do termóstato não foi colocado na fila KNX.');return {accepted:true,valueC:target,knxQueued:true,message:'Setpoint guardado e colocado na fila KNX; a confirmação do barramento pode ainda estar pendente.'};
 }catch(error){if(thermoEdit===staged.edit&&thermoEdit.version===staged.version&&!thermoEdit.sending){clearTimeout(thermoEdit.timer);thermoEdit=null;renderThermostat(latestThermo);}throw error;}
}
async function registerWebMcpTools(){
 const context=document.modelContext;if(!context||typeof context.registerTool!=='function'){const status=$('webMcpStatus');if(status)status.textContent=window.isSecureContext===false?'WebMCP requer uma origem segura HTTPS; este endereço HTTP não pode expor as ferramentas.':'WebMCP não está ativo neste navegador; ativa o suporte WebMCP numa versão compatível do Chromium.';return false;}if(webMcpController)return true;
 const controller=new AbortController();webMcpController=controller;
 const tools=[
  webMcpTool('get_device_status','Consultar estado do dispositivo','Lê o estado atual dos sensores, KNX, relés e entradas locais. Não inclui passwords nem dados de sessão.',webMcpSchema({}),async()=>{const s=await webMcpFetchStatus();return {name:s.name,firmware:s.firmware,ip:s.ip,knx:s.knx,individualAddress:s.individual,uptimeSeconds:Math.floor((Number(s.uptimeMs)||0)/1000),sensors:(s.channels||[]).map(c=>({name:c.name,valid:!!c.valid,value:c.valid?c.value:null,unit:c.unit})),relays:(s.relays||[]).flatMap((r,i)=>r.enabled?[{name:r.name,configuredIndex:i+1,on:!!r.on,feedbackPending:!!r.feedbackPending}]:[]),inputs:(s.inputs||[]).filter(i=>i.enabled).map(i=>({name:i.name,active:!!i.active})),thermostatMode:!!s.thermostatMode,thermostat:s.thermostatMode?s.thermostat:null};},true),
  webMcpTool('get_heating_status','Consultar estado do aquecimento','Lê o estado confirmado da bomba, das divisões e dos circuitos do coletor.',webMcpSchema({}),async()=>webMcpHeatingSummary(await webMcpFetchHeating()),true),
  webMcpTool('get_recent_knx_telegrams','Consultar telegramas KNX recentes','Lê os telegramas KNX recentes com uma sessão de administração.',webMcpSchema({}),async()=>{if(!adminSession)throw Error('Inicia sessão nas Configurações para consultar telegramas.');const s=await adminApi('/api/status');return {knx:s.knx,telegrams:(s.telegrams||[]).slice(-20),lastEvent:s.lastEvent||''};},true),
  webMcpTool('set_heating_control','Ativar ou parar o controlo de piso radiante','Ativa o controlador quando as saídas estão seguras ou pede a paragem e devolve os estados KNX atualizados.',webMcpSchema({action:{type:'string',enum:['start','stop'],description:'Ativa o controlo ou pede a paragem do aquecimento.'}}),async({action})=>{
   if(!['start','stop'].includes(action))throw Error('Ação de controlo inválida.');const h=await webMcpFetchHeating();if(!h.outputsEnabled)throw Error('O controlo de piso radiante está desativado ou em modo termóstato.');
   if(action==='start'&&h.armed)return {...webMcpHeatingSummary(h),accepted:true,message:'O controlo já estava ativo.'};if(action==='start'&&!h.canArm)throw Error(h.controlMessage||'As saídas ainda não estão confirmadas desligadas.');
   const {response,state}=await webMcpMutation('/api/heating/control',{action:action==='start'?'arm':'stop'},'heating');return {...webMcpHeatingSummary(state),accepted:!!response.ok,requestedAction:action,message:action==='stop'?'Paragem pedida; verifica os feedbacks OFF dos atuadores.':'Controlo ativado.'};
  }),
  webMcpTool('set_room_heating_mode','Alterar modo de uma divisão','Define uma divisão como automática, ligada manualmente ou desligada manualmente. O controlo do aquecimento tem de estar ativo.',webMcpSchema({room:{type:'string',description:'Nome da divisão tal como aparece no painel.'},mode:{type:'string',enum:['automatic','on','off'],description:'Modo de controlo pretendido.'}}),async({room,mode})=>{
   const h=await webMcpFetchHeating();if(!h.outputsEnabled||!h.armed)throw Error('Ativa primeiro o controlo de piso radiante.');if(!['automatic','on','off'].includes(mode))throw Error('Modo de divisão inválido.');const found=webMcpRoomIndex(h,room);const commandMode=mode==='automatic'?0:mode==='off'?1:2;
   const {response,state}=await webMcpMutation('/api/heating/control',{action:'room',index:found.index,mode:commandMode},'heating');return {accepted:!!response.ok,room:found.room,requestedMode:mode,state:webMcpRoomSummary(state,found.index),message:'Pedido aplicado; o estado dos circuitos depende do feedback KNX.'};
  }),
  webMcpTool('set_room_setpoint','Alterar setpoint de uma divisão','Atualiza e guarda o setpoint da divisão. O envio ao KNX só acontece após cinco segundos sem alterações.',webMcpSchema({room:{type:'string',description:'Nome da divisão tal como aparece no painel.'},temperatureC:{type:'number',description:'Temperatura pretendida em graus Celsius.'}}),async(input,{signal}={})=>webMcpSetRoomSetpoint(input,signal)),
  webMcpTool('set_configured_relay','Comandar um relé configurado','Liga ou desliga um dos relés locais configurados e devolve o estado e o feedback atuais.',webMcpSchema({relay:{type:'integer',minimum:1,maximum:4,description:'Número do relé apresentado na configuração, de 1 a 4.'},on:{type:'boolean',description:'true para ligar; false para desligar.'}}),async({relay,on})=>{
   if(!Number.isInteger(relay)||relay<1||relay>4||typeof on!=='boolean')throw Error('Indica um relé de 1 a 4 e o estado pretendido.');const s=await webMcpFetchStatus();if(!s.relays?.[relay-1]?.enabled)throw Error('Esse relé não está configurado.');const {response,state}=await webMcpMutation('/api/relay',{index:relay-1,on},'device');const current=state?.relays?.[relay-1];return {accepted:!!response.applied,relay,name:current?.name||s.relays[relay-1].name,requestedOn:on,reportedOn:!!current?.on,feedbackPending:!!current?.feedbackPending,message:current?.feedbackPending?'Comando aplicado; aguarda confirmação KNX.':'Estado atualizado.'};
  }),
  webMcpTool('set_local_thermostat_power','Ligar ou desligar o termóstato local','Altera o funcionamento do termóstato local baseado no BME280 e atualiza o pedido KNX.',webMcpSchema({on:{type:'boolean',description:'true para ligar; false para desligar.'}}),async({on})=>{
   if(typeof on!=='boolean')throw Error('Indica se o termóstato deve ficar ligado.');const s=await webMcpFetchStatus();if(!s.thermostatMode)throw Error('O modo termóstato local não está ativo.');const {response,state}=await webMcpMutation('/api/thermostat',{on},'device');return {accepted:!!response.ok,thermostat:state?.thermostat||null,message:'Pedido enviado; a confirmação do atuador KNX pode ainda estar pendente.'};
  }),
  webMcpTool('set_local_thermostat_setpoint','Alterar setpoint do termóstato local','Atualiza e guarda o setpoint do termóstato local. O envio ao KNX só acontece após cinco segundos sem alterações.',webMcpSchema({temperatureC:{type:'number',description:'Temperatura pretendida em graus Celsius.'}}),async(input,{signal}={})=>webMcpSetThermostatSetpoint(input,signal))
 ];
 try{for(const tool of tools)await context.registerTool(tool,{signal:controller.signal});const status=$('webMcpStatus');if(status)status.textContent='WebMCP ativo: '+tools.length+' ferramentas registadas neste navegador.';return true;}
 catch(error){controller.abort();webMcpController=null;const status=$('webMcpStatus');if(status)status.textContent='Falha ao registar as ferramentas WebMCP: '+error.message;console.warn('WebMCP: não foi possível registar as ferramentas',error);return false;}
}
if(typeof window!=='undefined'&&window.addEventListener)window.addEventListener('pagehide',()=>{if(webMcpController){webMcpController.abort();webMcpController=null;}},{once:true});


$('scanWifi').onclick=async()=>{
 const button=$('scanWifi'),state=$('wifiScanState');button.disabled=true;state.textContent='A pesquisar…';
 try{
  for(let i=0;i<20;i++){
   const result=await adminApi('/api/network/scan');
   if(!result.pending){
    const networks=result.networks||[];
    $('wifiNetworks').replaceChildren(new Option('Selecionar rede',''),...networks.map(n=>new Option(n.ssid+' · '+n.rssi+' dBm',n.ssid)));
    state.textContent=networks.length?networks.length+' redes':'Nenhuma rede encontrada. Podes escrever o SSID.';
    return;
   }
   await new Promise(resolve=>setTimeout(resolve,600));
  }
  throw Error('A pesquisa de redes demorou demasiado.');
 }catch(e){state.textContent=e.message;show(e.message);}finally{button.disabled=false;}
};
$('wifiNetworks').onchange=()=>{if($('wifiNetworks').value)$('ssid').value=$('wifiNetworks').value;};
async function orbitDiagnostics(){try{const r=await adminApi('/api/diagnostics');$('orbitDiagnostics').textContent=r.version+' · '+r.build+'\nUptime '+r.uptime+' s · RAM livre '+r.freeHeap;$('orbitLogs').textContent=r.logs;$('knxConsole').textContent=r.telegrams.join('\n');$('orbitVersion').textContent='Firmware '+r.version+' · '+r.build}catch(e){show(e.message)}}
$('refreshDiagnostics').onclick=orbitDiagnostics;
$('changeAdminPassword').onclick=async()=>{try{await adminApi('/api/admin/password',{method:'POST',headers:{'Content-Type':'application/json','X-CSRF-Token':csrf},body:JSON.stringify({password:$('newAdminPassword').value})});adminSession='';sessionStorage.removeItem('adminSession');location.reload()}catch(e){show(e.message)}};
selectTab(tabFromHash());
$('logoutAdmin').onclick=async()=>{try{await adminApi('/api/admin/logout',{method:'POST',headers:{'Content-Type':'application/json','X-CSRF-Token':csrf},body:'{}'});adminSession='';sessionStorage.removeItem('adminSession');location.reload()}catch(e){show(e.message)}};
if(typeof window!=='undefined')window.addEventListener('hashchange',()=>{if(location.pathname==='/config.html'){selectTab(tabFromHash());return;}const page=location.hash==='#configuracoes'?'settings':routeFromHash();view(page,false);if(page==='history')refreshHistory();});
(async()=>{try{await refresh();await registerWebMcpTools();try{await refreshHeat();}catch(e){$('heatMessage').textContent='Mapa do coletor indisponível: '+e.message;}if(initialSettings&&location.pathname==='/config.html')await openSettings();else if(location.hash==='#historico')await refreshHistory();setInterval(updateClock,1000);setInterval(()=>{refresh();if(!busy)refreshHeat().catch(e=>{if(heatingPlan){heatingPlan.armed=false;heatingPlan.pumpState='desconhecido';heatingPlan.circuits.forEach(c=>c.state='desconhecido');renderHeatStatus();}$('pumpState').dataset.state='unknown';$('pumpStatus').textContent='Sem ligação';$('pumpDetail').textContent='Estados indisponíveis';$('toggleHeat').disabled=true;$('refreshHeat').disabled=true;});},3000);}catch(e){show(e.message);}})();
</script><script>async function timeRequest(sync=false){const controller=new AbortController(),timer=setTimeout(()=>controller.abort(),5000);try{return await adminApi(sync?'/api/time/sync':'/api/time',{method:sync?'POST':'GET',headers:{'X-CSRF-Token':csrf},signal:controller.signal})}finally{clearTimeout(timer)}}
function timeDraftChanged(){return !cfg||['ntpServer','ntpSecondary','timezone'].some(id=>$(id).value!==(cfg[id]||(id==='ntpSecondary'?'time.cloudflare.com':'')))}

function orbitTimezone(id,value){const select=document.getElementById(id);if(![...select.options].some(o=>o.value===value))select.add(new Option('Personalizado',value));select.value=value;}
function renderTimeStatus(t){
 document.getElementById('rtcTimeValid').textContent=t.valid?(t.localTime||'Hora válida'):'Por sincronizar';
 document.getElementById('rtcAvailable').textContent=t.rtcAvailable?'Disponível':'Não equipado';
 document.getElementById('sdAvailable').textContent=t.storageAvailable?'Disponível':'Indisponível';
 document.getElementById('timeSyncState').textContent=({pending:'A aguardar resposta NTP…',synced:'Hora sincronizada por NTP.',timeout:'Não foi recebida resposta NTP. Verifique a rede e os servidores.',idle:'Ainda sem confirmação NTP neste arranque.'})[t.syncState]||'';
}
async function refreshTime(){try{renderTimeStatus(await timeRequest())}catch(e){document.getElementById('timeSyncState').textContent=e.message}}
document.getElementById('syncTime').onclick=async()=>{
 const button=document.getElementById('syncTime'),state=document.getElementById('timeSyncState');
 if(button.disabled)return;
 if(timeDraftChanged()){state.textContent='Guarde primeiro as alterações de NTP e fuso horário.';return;}
 button.disabled=true;
 try{await timeRequest(true);state.textContent='A aguardar resposta NTP…';
  for(let i=0;i<32;i++){await new Promise(resolve=>setTimeout(resolve,1000));const t=await timeRequest();renderTimeStatus(t);if(t.syncState!=='pending')return}
  state.textContent='Não foi recebida confirmação NTP.';
 }catch(e){state.textContent=e.message}finally{button.disabled=false}
};
async function validateFirmware(file,product,hardware,chip){
 if(!file||file.size<392||file.size>16*1024*1024)throw Error('Selecione uma aplicação firmware.bin compatível.');
 const bytes=new Uint8Array(await file.slice(0,392).arrayBuffer());
 if(bytes.length!==392||bytes[0]!==0xe9||bytes[1]<1||bytes[1]>16||(bytes[12]|bytes[13]<<8)!==chip||new DataView(bytes.buffer).getUint32(32,true)!==0xabcd5432)throw Error('Formato de firmware ou processador incompatível.');
 const text=(start,size)=>{const raw=bytes.slice(start,start+size),end=raw.indexOf(0);if(end<1)throw Error('Identificação de firmware inválida.');for(const c of raw.slice(0,end))if(c<32||c>126)throw Error('Identificação inválida.');return new TextDecoder().decode(raw.slice(0,end));};
 if(text(288,8)!=='IOTFW01'||text(296,32)!==product||text(328,32)!==hardware)throw Error('Firmware de outro equipamento ou sem identificação compatível.');
 return text(360,32);
}
</script></div></body></html>)HTML";
