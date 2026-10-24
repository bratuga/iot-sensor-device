const {JSDOM}=require('jsdom');const fs=require('fs');const assert=require('assert');
let html=fs.readFileSync(require('path').join(__dirname,'../include/WebUi.h'),'utf8').split('R"HTML(')[1].split(')HTML"')[0];html=html.replace(/\(async\(\)=>\{try\{const r=await api\('\/api\/config'\)[\s\S]*?<\/script>/,'</script>');const dom=new JSDOM(html,{runScripts:'dangerously',url:'http://sensor.local'});const w=dom.window,d=w.document;
w.eval(`heatingPlan={outputsEnabled:false,armed:false,fault:false,pumpState:'desconhecido',zones:[],circuits:Array.from({length:8},(_,i)=>({zone:-1,position:i,state:'desconhecido'})),control:{enabled:false,openMs:120000,maxAgeMs:300000,rooms:[],actuators:Array.from({length:9},()=>({command:'',feedback:''}))}};renderHeating();`);
d.getElementById('addRoom').click();d.getElementById('addRoom').click();
function click(id,text){[...d.querySelectorAll('#'+id+' button')].find(b=>b.textContent===text).click();}
click('hz0','Associar circuito');click('hz0','Associar circuito');assert.equal(w.eval('heatingDraft.circuits.filter(c=>c.zone===0).length'),2);
d.getElementById('zone0').value='Sala';w.eval('captureHeating();heatingPlan.pumpState="ligado";renderHeatStatus();');assert.equal(d.getElementById('zone0').value,'Sala');assert(d.querySelector('#pumpState .on'));
click('hz1','Associar circuito');click('hz0','Remover divisão');assert.equal(w.eval('heatingDraft.zones.length'),1);assert.equal(w.eval('heatingDraft.circuits.filter(c=>c.zone===0).length'),1);assert.equal(w.eval('heatingDraft.circuits.filter(c=>c.zone===-1).length'),7);
click('hz0','Retirar');assert.equal(w.eval('heatingDraft.circuits.filter(c=>c.zone===-1).length'),8);
for(let i=0;i<8;i++)click('hz0','Associar circuito');assert.equal(w.eval('heatingDraft.circuits.filter(c=>c.zone===0).length'),8);assert([...d.querySelectorAll('#hz0 button')].find(b=>b.textContent==='Associar circuito').disabled);
w.eval('heatingPlan=JSON.parse(JSON.stringify(heatingDraft));heatingPlan.circuits[7].state="ligado";renderHeatStatus();');assert.equal(d.querySelectorAll('#branches .on').length,2);assert.equal(d.querySelectorAll('#branches .branch').length,8);
console.log('PASS DOM: dynamic rooms, multiple/eight associations, remove/reindex/release, draft preserved during refresh, pump and head lights');

w.eval('heatingPlan.armed=true;heatingPlan.control.rooms[0].mode=2;renderHeatStatus();');assert(d.querySelector('#rooms .power-on'));assert.equal(d.querySelector('#rooms .power-toggle').getAttribute('aria-pressed'),'true');
w.eval('heatingPlan.control.rooms[0].mode=1;renderHeatStatus();');assert(!d.querySelector('#rooms .power-on'));console.log('PASS room power tile: red ON, grey OFF and accessible state');

assert(!d.getElementById('rooms').textContent.includes('Circuito'));console.log('PASS room tiles omit circuit assignments');

w.eval('heatingPlan.controlMessage="A aguardar feedback OFF da bomba em 3/5/10";Object.assign(heatingPlan.control.rooms[0],{temperatureKnown:true,temperatureFresh:false,value:21.5,setpointKnown:true,target:23});renderHeatStatus();');
assert(d.getElementById('rooms').textContent.includes('21,5º'));assert(d.querySelector('.room-temperature').title.includes('Última leitura guardada'));assert(d.getElementById('rooms').textContent.includes('Setpoint: 23º'));assert(d.getElementById('heatMessage').textContent.includes('3/5/10'));
w.eval('heatingPlan.control.rooms[0].temperatureKnown=false;renderHeatStatus();');assert(d.getElementById('rooms').textContent.includes('Setpoint: 23º'));console.log('PASS independent setpoint/temperature, stale cache tooltip and actionable pump feedback diagnostic');

d.getElementById('hz0external').checked=true;d.getElementById('hz0bmeFallback').checked=true;d.getElementById('hz0margin').value='0.4';w.eval('captureHeating();renderHeating(false)');assert(d.getElementById('hz0external').checked);assert(d.getElementById('hz0bmeFallback').checked);assert.equal(d.getElementById('hz0margin').value,'0.4');console.log('PASS room control and BME fallback survive redraw');

w.eval('heatingPlan.outputsEnabled=true;heatingPlan.armed=false;heatingPlan.canArm=false;heatingPlan.fault=true;heatingPlan.faultOutput=3;heatingPlan.pumpState="desligado";heatingPlan.controlMessage="Falha: Circuito 4 (feedback 3/5/14)";renderHeatStatus();');
assert(d.getElementById('armHeat').disabled);
assert(d.getElementById('heatMessage').textContent.includes('Circuito 4'));
assert(!d.getElementById('pumpState').textContent.includes('Falha'));
assert(d.getElementById('pumpState').textContent.includes('desligado'));
w.eval('heatingPlan.canArm=true;renderHeatStatus();');assert(!d.getElementById('armHeat').disabled);
w.eval('heatingPlan.circuits[0].zone=-1;heatingPlan.circuits[0].state="nao utilizado";renderHeatStatus();');
assert(d.querySelector('#branches .branch.unused').textContent.includes('Sem circuito'));
console.log('PASS unused circuit, specific fault separate from pump, arm only after used outputs confirmed OFF');

// The same state still drives the commands; visible text stays compact.
w.eval('heatingPlan.armed=true;Object.assign(heatingPlan.control.rooms[0],{temperatureKnown:true,value:20,setpointKnown:true,target:25,demand:true,mode:2});renderHeatStatus();');
assert.equal(d.querySelector('.room-temperature').textContent,'20º');
assert.equal(d.querySelectorAll('.room-detail')[0].textContent,'Setpoint: 25º');
assert.equal(d.querySelectorAll('.room-detail')[1].textContent,'Pedido: ligado');
assert.equal(d.querySelector('.power-toggle').textContent,'');
assert(d.querySelector('.power-toggle').getAttribute('aria-label').startsWith('Desligar '));
assert.equal(w.getComputedStyle(d.querySelector('.power-toggle')).width,'64px');
assert.equal(w.getComputedStyle(d.querySelector('.room-temperature')).fontSize,'34px');
w.eval('var lastRoomAction;heatAction=(...args)=>{lastRoomAction=args;};');
d.querySelector('.room-auto').click();assert.equal(w.eval('JSON.stringify(lastRoomAction)'),JSON.stringify(['room',0,0]));
d.querySelector('.power-toggle').click();assert.equal(w.eval('JSON.stringify(lastRoomAction)'),JSON.stringify(['room',0,1]));
console.log('PASS compact room tile, icon-only accessible power button, temperature/setpoint/request, unchanged mode actions');
