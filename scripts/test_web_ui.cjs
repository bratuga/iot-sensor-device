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
