const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const {JSDOM}=require('jsdom');
const dom=new JSDOM('<button id="railPower"></button><button id="orbitTheme" aria-checked="false"></button>',{url:'http://orbit.test',runScripts:'outside-only'});
const w=dom.window,d=w.document;let now=0,frame,commands=[];
w.requestAnimationFrame=fn=>{frame=fn;return 1};w.cancelAnimationFrame=()=>{frame=null};
Object.defineProperty(w.performance,'now',{value:()=>now});
const raw=fs.readFileSync(path.join(__dirname,'../include/WebUi.h'),'utf8');
const actual=new JSDOM(raw.split('R"HTML(')[1].split(')HTML"')[0]);
assert.equal(actual.window.document.querySelectorAll('#orbitTheme').length,1);
assert.equal(actual.window.document.querySelectorAll('#railPower').length,1);
actual.window.close();
w.eval(raw.split('<script data-orbit-controls>')[1].split("initOrbitTheme('iot-")[0]);
let state={powerEnabled:true,cpuUsePct:20,heapUsePct:40};
let succeeds=true;
const power=w.createOrbitPower(async target=>{commands.push(target);if(!succeeds)return false;state={...state,powerEnabled:target};power.update(state);return true;});
const button=d.getElementById('railPower');
function key(type,value){button.dispatchEvent(new w.KeyboardEvent(type,{key:value,cancelable:true}))}
async function advance(ms){now+=ms;const fn=frame;frame=null;if(fn)await fn(now);}
(async()=>{
 power.update(state);assert.equal(d.getElementById('railCpu').textContent,'20%');
 key('keydown',' ');await advance(1499);key('keyup',' ');await advance(20);assert.equal(commands.length,0);
 key('keydown','Enter');await advance(1500);assert.deepEqual(commands,[false]);assert.equal(button.dataset.state,'off');
 key('keyup','Enter');key('keydown',' ');key('keydown','Escape');await advance(1600);assert.equal(commands.length,1);
 key('keydown',' ');power.update(null);await advance(1600);assert.equal(commands.length,1);assert.equal(button.getAttribute('aria-disabled'),'true');
 power.update(state);key('keydown',' ');power.update({...state,canToggle:false});await advance(1600);assert.equal(commands.length,1);
 succeeds=false;power.update(state);key('keydown',' ');await advance(1500);assert.equal(commands.length,2);assert.equal(button.dataset.state,'off');assert.equal(d.getElementById('railPowerNotice').textContent,'Comando falhou');
 power.update({...state,powerEnabled:true,cpuUsePct:null,cpuMHz:240});assert.equal(d.getElementById('railCpu').textContent,'240 MHz');assert.match(button.getAttribute('aria-label'),/utilização indisponível/);
 power.update({...state,level:'warning',label:'A parar',canToggle:false});assert.equal(button.dataset.state,'warning');assert.match(button.getAttribute('aria-label'),/A parar/);
 w.initOrbitTheme('test-theme');d.getElementById('orbitTheme').click();assert(w.document.body.classList.contains('light'));assert.equal(w.localStorage.getItem('test-theme'),'light');
 console.log('PASS hold/cancel/keyboard/offline/busy/failure/metrics/theme');w.close();
})().catch(e=>{console.error(e);w.close();process.exitCode=1});
