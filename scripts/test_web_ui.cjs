const {JSDOM}=require('jsdom');const fs=require('fs');const assert=require('assert');
let html=fs.readFileSync(require('path').join(__dirname,'../include/WebUi.h'),'utf8').split('R"HTML(')[1].split(')HTML"')[0];html=html.replace(/\(async\(\)=>\{try\{await refresh\(\);[\s\S]*?<\/script>/,'</script>');const dom=new JSDOM(html,{runScripts:'dangerously',url:'http://sensor.local'});const w=dom.window,d=w.document;
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

w.eval('heatingPlan.armed=true;heatingPlan.control.rooms[0].mode=2;renderHeatStatus();');assert.equal(d.querySelector('#rooms .power').getAttribute('aria-pressed'),'true');
w.eval('heatingPlan.control.rooms[0].mode=1;renderHeatStatus();');assert.equal(d.querySelector('#rooms .power').getAttribute('aria-pressed'),'false');console.log('PASS thermostat power states and accessible control');
assert(!d.getElementById('rooms').textContent.includes('Circuito'));console.log('PASS room tiles omit circuit assignments');
w.eval('heatingPlan.controlMessage="A aguardar feedback OFF da bomba em 3/5/10";Object.assign(heatingPlan.control.rooms[0],{temperatureKnown:true,temperatureFresh:false,value:21.5,setpointKnown:true,target:23});renderHeatStatus();');
assert(d.getElementById('rooms').textContent.includes('21,5'));assert(d.querySelector('.current').title.includes('Última leitura guardada'));assert(d.getElementById('rooms').textContent.includes('23,0 °C'));assert(d.getElementById('heatMessage').textContent.includes('3/5/10'));
w.eval('heatingPlan.control.rooms[0].temperatureKnown=false;renderHeatStatus();');assert(d.getElementById('rooms').textContent.includes('23,0 °C'));console.log('PASS independent setpoint/temperature and stale cache tooltip');

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

// Commands still come from the real heating state, and setpoint buttons use the dedicated API.
w.eval('heatingPlan.armed=true;Object.assign(heatingPlan.control.rooms[0],{temperatureKnown:true,value:20,setpointKnown:true,target:25,setpoint:"3/4/2",demand:true,mode:2});renderHeatStatus();');
assert.equal(d.querySelector('.current').textContent,'20,0°C');
assert.equal(d.querySelector('.setpoint-value output').textContent,'25,0 °C');
assert.equal(d.querySelector('.heat-state').textContent,'Pedido ativo');
assert.equal(d.querySelector('.thermostat').dataset.demand,'true');
assert(d.querySelector('.power').getAttribute('aria-label').startsWith('Desligar '));
w.eval('var lastRoomAction;heatAction=(...args)=>{lastRoomAction=args;};');
d.querySelector('.mode').click();assert.equal(w.eval('JSON.stringify(lastRoomAction)'),JSON.stringify(['room',0,0]));
d.querySelector('.power').click();assert.equal(w.eval('JSON.stringify(lastRoomAction)'),JSON.stringify(['room',0,1]));
w.eval('heatingPlan.control.rooms[0].mode=0;renderHeatStatus();');
d.querySelector('.mode').click();assert.equal(w.eval('JSON.stringify(lastRoomAction)'),JSON.stringify(['room',0,2]));
w.eval('heatingPlan.control.rooms[0].mode=2;renderHeatStatus();');
w.eval('var lastSetpoint;changeSetpoint=(...args)=>{lastSetpoint=args;};');
d.querySelector('.increase').click();assert.equal(w.eval('JSON.stringify(lastSetpoint)'),JSON.stringify([0,0.5]));
d.querySelector('.decrease').click();assert.equal(w.eval('JSON.stringify(lastSetpoint)'),JSON.stringify([0,-0.5]));
console.log('PASS thermostat layout, demand, power/mode and setpoint events');
