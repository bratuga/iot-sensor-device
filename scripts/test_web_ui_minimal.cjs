// Dependency-free dashboard smoke test; the full DOM test uses jsdom when installed.
const {readFileSync}=require('node:fs');
const {join}=require('node:path');
const vm=require('node:vm');
const assert=require('node:assert/strict');
const html=readFileSync(join(__dirname,'../include/WebUi.h'),'utf8').split('R"HTML(')[1].split(')HTML"')[0];
const original=html.split('<script>')[1].split('</script>')[0];
const js=original.replace(/\(async\(\)=>\{try\{await refresh\(\);[\s\S]*?\}\)\(\);\s*$/,'');
assert(js.length<original.length,'bootstrap extracted');
class Element{
  constructor(tag='div'){this.tagName=tag;this.children=[];this.dataset={};this.style={};this.attributes={};this.hidden=false;this._text='';this.disabled=false;}
  set textContent(text){this._text=String(text);this.children=[];}
  get textContent(){return this._text+this.children.map(c=>c.textContent).join('');}
  set innerHTML(text){this._html=text;this.children=[];this._text='';}
  append(...nodes){this.children.push(...nodes);}
  setAttribute(key,value){this.attributes[key]=String(value);}
  getAttribute(key){return this.attributes[key];}
  querySelector(selector){const cls=selector.replace('.','');for(const child of this.children){if(child.className===cls||child.className?.split(' ').includes(cls))return child;const found=child.querySelector(cls);if(found)return found;}return null;}
}
const elements=new Map();const document={getElementById(id){if(!elements.has(id))elements.set(id,new Element());return elements.get(id);},createElement(tag){return new Element(tag);},createTextNode(text){const e=new Element('#text');e.textContent=text;return e;},querySelectorAll(){return [];}};
const calls=[];let layout={outputsEnabled:true,armed:true,canArm:true,pumpState:'desligado',controlMessage:'Controlo ativo.',zones:['Sala'],circuits:Array.from({length:8},(_,i)=>({position:i,zone:i===0?0:-1,state:'desligado'})),control:{rooms:[{setpoint:'3/4/2',temperatureKnown:true,temperatureFresh:true,value:20,setpointKnown:true,target:23,mode:0,enabled:true,demand:true}]}};
let session='';const fetch=async(path,options={})=>{calls.push({path,options});let body={};let status=200;
 if(path==='/api/heating/setpoint'){const {index,value}=JSON.parse(options.body);assert.equal(index,0);assert.equal(options.headers['X-CSRF-Token'],'test-csrf');layout.control.rooms[index].target=value;layout.control.rooms[index].setpointPending=true;body={queued:true,value};}
 else if(path==='/api/heating/status')body=layout;
 else if(path==='/api/admin/login'){body={session:'session-token'};session='session-token';}
 else if(path==='/api/config'){status=options.headers['X-Admin-Session']===session&&session?200:401;body=status===200?{config:{name:'Teste'}}:{error:'Inicia sessao nas Configuracoes'};}
 else if(path==='/api/status')body={csrf:'test-csrf'};
 return {ok:status===200,status,json:async()=>body};};
const timers=new Map();let timerId=0;
const context=vm.createContext({document,fetch,sessionStorage:{getItem:()=>null,setItem:()=>{},removeItem:()=>{}},location:{hash:''},console,setInterval:()=>{},setTimeout:(fn,ms)=>{const id=++timerId;timers.set(id,{fn,ms});return id;},clearTimeout:id=>timers.delete(id),Date,Number,String,Math,JSON,Error});
vm.runInContext(js,context);
(async()=>{
 vm.runInContext('csrf="test-csrf";heatingPlan=globalLayout;renderHeatStatus();',Object.assign(context,{globalLayout:layout}));
 assert.equal(document.getElementById('pumpState').dataset.state,'off');
 assert.equal(document.getElementById('branches').children.length,8);
 assert.equal(document.getElementById('branches').children[0].dataset.available,'true');
 assert.equal(document.getElementById('branches').children[1].dataset.available,'false');
 const rooms=document.getElementById('rooms');assert.equal(rooms.children.length,1);const card=rooms.children[0];assert.equal(card.className,'thermostat');assert.equal(card.dataset.demand,'true');assert.equal(card.children[0].children[1].textContent,'A aguardar');assert.equal(card.children[0].children[0].textContent,'Sala');
 const dial=card.children[1];assert(dial.textContent.includes('20,0'));assert.equal(card.children[2].children[0].children[1].children[1].textContent,'23,0 °C');
 const plus=card.children[2].children[0].children[2];plus.onclick();rooms.children[0].children[2].children[0].children[2].onclick();
 assert.equal(calls.filter(c=>c.path==='/api/heating/setpoint').length,0);assert.equal(timers.size,1);assert.equal([...timers.values()][0].ms,5000);
 assert(rooms.children[0].children[2].textContent.includes('24,0 °C'));assert(rooms.children[0].children[2].textContent.includes('5 s'));await vm.runInContext('refreshHeat()',context);assert(rooms.children[0].children[2].textContent.includes('24,0 °C'));
 const delayed=[...timers.values()][0].fn;timers.clear();await delayed();
 assert.equal(calls.filter(c=>c.path==='/api/heating/setpoint').length,1);assert.equal(calls.find(c=>c.path==='/api/heating/setpoint').options.body,'{"index":0,"value":24}');assert.equal(layout.control.rooms[0].target,24);assert(rooms.children[0].children[2].textContent.includes('A confirmar envio KNX'));
 vm.runInContext('render=()=>{};renderRelays=()=>{};renderInputs=()=>{};renderHeating=()=>{};visibility=()=>{};selectTab=()=>{};',context);
 await document.getElementById('viewSettings').onclick();assert.equal(document.getElementById('privateSettings').hidden,true);assert.equal(document.getElementById('loginPanel').hidden,false);
 document.getElementById('loginUser').value='admin';document.getElementById('loginPassword').value='secret';await document.getElementById('loginForm').onsubmit({preventDefault(){}});assert.equal(document.getElementById('privateSettings').hidden,false);assert.equal(document.getElementById('loginPanel').hidden,true);
 assert(calls.filter(c=>c.path==='/api/config').every(c=>c.options.headers['X-Admin-Session']==='session-token'||!c.options.headers['X-Admin-Session']));
 vm.runInContext('latestThermo={name:"Quarto",on:true,setpoint:21,temperature:20,demand:true,temperatureValid:true,pending:false,failed:false,feedbackKnown:false};currentThermostatMode=true;renderThermostat(latestThermo);csrf="test-csrf";changeThermo(0.5);changeThermo(0.5);',context);
 assert.equal(calls.filter(c=>c.path==='/api/thermostat').length,0);assert.equal([...timers.values()][0].ms,5000);assert.equal(document.getElementById('thermoTarget').textContent,'22,0 °C');
 const thermoTimer=[...timers.values()][0].fn;timers.clear();await thermoTimer();
 assert.equal(calls.filter(c=>c.path==='/api/thermostat').length,1);assert.equal(JSON.parse(calls.find(c=>c.path==='/api/thermostat').options.body).setpoint,22);
 assert(html.includes('data-tab="features"')&&html.includes('data-tab="knx"')&&html.includes('data-tab="hardware"')&&html.includes('data-tab="config"'));
 assert(html.includes('id="knxConsole"')&&html.includes('id="restoreFile"')&&html.includes('id="thermostatView"'));
 assert(!html.includes('id="viewDashboard"'));assert(html.includes('id="deviceName"'));assert(html.includes('id="localClock"')===false);assert(html.includes('id="connection"'));
 console.log('PASS: public dashboard, both 5-second setpoint debounces, tabs, collector state, integrated admin login');
})().catch(e=>{console.error(e);process.exitCode=1;});
