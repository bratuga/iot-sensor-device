// Chromium integration checks for the embedded UI; all device I/O is simulated.
// Run with playwright-core on NODE_PATH and ORBIT_CHROMIUM pointing to Chromium.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const { chromium } = require('playwright-core');

const html = fs.readFileSync(path.join(__dirname, '../include/WebUi.h'), 'utf8')
  .split('R"HTML(')[1].split(')HTML"')[0];
const origin = 'http://orbit.test';
const zones = ['Quarto principal', 'Quarto de hóspedes', 'Casa de banho principal', 'Casa de banho', 'Closet'];
const layout = {
  zones, outputsEnabled: true, armed: true, canArm: true, pumpState: 'ligado',
  pumpPhase: 'service', pumpRemainingMs: 500000, pumpPhaseTotalMs: 900000,
  controlMessage: 'Controlo ativo.',
  circuits: Array.from({ length: 8 }, (_, i) => ({ position: i, zone: i < 7 ? i % zones.length : -1, state: i < 3 ? 'ligado' : 'desligado' })),
  control: {
    enabled: true, openMs: 120000, maxAgeMs: 300000, serviceMs: 900000, pauseMs: 300000,
    rooms: zones.map((_, i) => ({
      enable: '4/0/' + (i+1), status: '4/1/' + (i+1), temperature: '4/2/' + (i+1), setpoint: '4/3/' + (i+1),
      margin: 0.3000000119, enabled: true, mode: 0, demand: i < 3, temperatureKnown: true, temperatureFresh: true,
      value: 21 + i / 10, setpointKnown: true, target: 23, hasSetpoint: true,
    })),
    actuators: Array.from({ length: 9 }, (_, i) => ({ command: '5/0/' + (i+1), feedback: '5/1/' + (i+1) })),
  },
};
const config = {
  schema: 2, name: 'orbit-teste', ssid: 'Rede original', showDiagnostics: true,
  wifiDhcp: true, wifiIp: '192.168.1.80', wifiMask: '255.255.255.0', wifiGateway: '192.168.1.1',
  wifiDns: '1.1.1.1', ntpServer: 'pool.ntp.org', timezone: 'WET0WEST,M3.5.0/1,M10.5.0',
  knxEnabled: true, gateway: '192.168.1.12', port: 3671, rate: 5,
  bmeEnabled: true, sda: 23, scl: 22, bmeAddress: 119, oneWire: 27, rom: ['',''],
  oledEnabled: false, oledSda: 4, oledScl: 15, oledAddress: 60, oledReset: 16, oledHeight: 64, sampleMs: 5000,
  pumpColdGuardEnabled: true, pumpColdLimitC: 25, pumpColdPauseMin: 15,
  pumpReturnGuardEnabled: true, pumpReturnLimitC: 28, pumpReturnAboveMin: 5, pumpReturnPauseMin: 15,
  thermostatMode: false, thermostatName: 'Quarto', thermostatMargin: 0.3,
  thermostatSetpointGroup: '6/0/1', thermostatDemandGroup: '6/0/2', thermostatFeedbackGroup: '6/0/3',
  channels: Array.from({ length: 5 }, (_, i) => ({
    name: ['Ambiente','Humidade','Pressão','Ida','Retorno'][i], enabled: i === 0, group: i === 0 ? '3/1/1' : '',
    offset: 0, periodic: true, periodMs: 60000, onChange: true, delta: 0.2, minGapMs: 5000,
  })),
  relays: Array.from({ length: 4 }, (_, i) => ({ name: 'Relé '+(i+1), enabled: i === 0, gpio: 25+i, activeLow: true, commandGroup: '7/0/'+(i+1), feedbackGroup: '7/1/'+(i+1) })),
  inputs: Array.from({ length: 4 }, (_, i) => ({ name: 'Entrada '+(i+1), enabled: i === 0, gpio: 32+i, activeLow: true, group: '8/1/'+(i+1) })),
};
const logs = ['1.1.1 → 3/1/1 · 21,5 °C'];
const backup = { format: 'iot-sensor-backup-v1', config, heating: layout };
const epoch = Math.floor(Date.now()/1000);

async function device(context, mode = 'collector') {
  const requests = [], violations = [];
  const thermostat = { name: 'Quarto', on: true, demand: true, setpoint: 22, temperature: 20.5, temperatureValid: true, pending: false, knxConnected: true };
  let loggedIn = false, scans = 0, powerAttempts = 0;
  const power={powerEnabled:true,powerCanToggle:true,powerLevel:'ok',powerLabel:'',cpuMHz:240,heapUsePct:42};
  await context.route(origin+'/**', async route => {
    const req = route.request(), pathname = new URL(req.url()).pathname;
    if (pathname === '/' || pathname === '/config.html') return route.fulfill({ contentType:'text/html; charset=utf-8', body:html });
    if (pathname === '/favicon.ico') return route.fulfill({ status:204 });
    const headers=req.headers(), post=req.method()==='POST';
    const admin=loggedIn && headers['x-admin-session']==='browser-test-session';
    const body=post&&headers['content-type']?.startsWith('application/json')?req.postDataJSON():null;
    requests.push({ pathname, post, body, headers });
    const json=(value,status=200)=>route.fulfill({json:value,status});
    const protectedPath=/^\/api\/(config|diagnostics|network|backup|knx|update|restart)(?:\/|$)/.test(pathname)
      || pathname==='/api/heating/layout' || /^\/api\/admin\/(password|logout)$/.test(pathname);
    if(protectedPath&&!admin) return json({error:'Inicia sessão'},401);
    if(post&&pathname!=='/api/admin/login'&&headers['x-csrf-token']!=='orbit-test-csrf') {
      violations.push('Missing CSRF on '+pathname);return json({error:'Sessão inválida'},403);
    }
    switch(pathname) {
      case '/api/status': return json({
        name:config.name, build:'browser fixture', firmware:'0.3.20-orbit', ip:'192.168.1.80', csrf:'orbit-test-csrf', historyTime:true,
        knx:'ligado', uptimeMs:20000, rssi:-58, thermostatMode:mode==='thermostat', thermostat,...power,
        channels:config.channels.map((c,i)=>({name:c.name,valid:i!==4,value:[21.5,65,100632,31.4,null][i],unit:['°C','%','Pa','°C','°C'][i],trend30m:[0.5,-2][i]})),
        relays:config.relays.map(r=>({name:r.name,enabled:r.enabled,on:true,feedbackPending:false})),
        inputs:config.inputs.map(i=>({name:i.name,enabled:i.enabled,active:true})),
        showDiagnostics:admin, roms:admin?['28FF000012345678']:[], telegrams:admin?logs:[],
        ...(admin?{individual:'1.1.203',freeHeap:124000,sent:12,acked:12,failed:0,lastEvent:'KNX ligado',bme:true,oled:false}:{}),
      });
      case '/api/power':
        if(++powerAttempts===1)return json({error:'Operação em curso'},409);
        Object.assign(power,{powerEnabled:body.enabled,powerCanToggle:false,powerLevel:'warning',powerLabel:'A parar'});
        return json({accepted:true});
      case '/api/history': return json({synced:true,storage:true,records:[[epoch-171200,30,25,21,64,0],[epoch-170000,31,26,21,65,1],[epoch-86200,31,25,22,65,1],[epoch-85000,32,25,22,66,0],[epoch-2400,32,25,22,65,0],[epoch-1200,33,24,23,64,1]]});
      case '/api/heating/status': return json(layout);
      case '/api/heating/layout': return json(post?{pending:true}:layout,post?202:200);
      case '/api/config': return json(post?{saved:true,reboot:true}:{config:{...config,thermostatMode:mode==='thermostat'}});
      case '/api/config/save-status': return json({state:'saved'});
      case '/api/admin/login':
        if(body.password!=='valid-fixture-password')return json({error:'Utilizador ou palavra-passe incorretos'},401);
        loggedIn=true;return json({session:'browser-test-session'});
      case '/api/admin/logout':
      case '/api/admin/password': loggedIn=false;return json({});
      case '/api/network/scan': return json(++scans===1?{pending:true}:{pending:false,networks:[{ssid:'Rede encontrada',rssi:-55}]});
      case '/api/diagnostics': return json({version:'0.3.20-orbit',build:'browser fixture',uptime:20,freeHeap:124000,logs:'AUDIT: login administrador',telegrams:logs});
      case '/api/backup': return json(post?{saved:true}:backup);
      case '/api/thermostat':
        if(typeof body?.setpoint==='number')thermostat.setpoint=body.setpoint;
        if(typeof body?.on==='boolean')thermostat.on=body.on;
        return json({ok:true,queued:true,value:thermostat.setpoint});
      case '/api/heating/control': return json({ok:true});
      case '/api/heating/setpoint': return json({queued:true,value:body.value});
      case '/api/relay': return json({ok:true});
      case '/api/knx/command': return json({queued:true});
      case '/api/update': return json({updated:true});
      case '/api/restart': return json({restarting:true});
      default: violations.push('Unexpected request '+pathname);return json({error:'Mock: not found'},404);
    }
  });
  return { requests, violations, power };
}

async function checkLayout(page, label) {
  const shape=await page.evaluate(()=>{
    const mains=[...document.querySelectorAll('main')].filter(e=>!e.hidden);
    return {
      width:document.documentElement.clientWidth,
      scroll:document.documentElement.scrollWidth,
      mains:mains.map(e=>e.id),
      nav:[...document.querySelectorAll('.orbit-nav [data-view]')].map(el=>{
        const r=el.getBoundingClientRect();return r.width>0&&r.left>=0&&r.right<=innerWidth+1&&r.bottom<=innerHeight+1;
      }),
      duplicates:[...document.querySelectorAll('[id]')].map(e=>e.id).filter((id,i,ids)=>ids.indexOf(id)!==i),
      wider:[...document.body.querySelectorAll('*')].filter(e=>e.clientWidth&&e.getBoundingClientRect().left+e.scrollWidth>innerWidth+1&&getComputedStyle(e).overflowX==='visible').slice(0,15).map(e=>[e.tagName,e.id,e.className?.baseVal??e.className,Math.round(e.getBoundingClientRect().left+e.scrollWidth)]),
      overflowing:mains.flatMap(e=>[...e.querySelectorAll('section,fieldset,button,input,select')]).filter(e=>{
        const r=e.getBoundingClientRect();return r.width>0&&(r.right>innerWidth+1||r.left<0);
      }).map(e=>e.id||e.tagName).slice(0,12),
    };
  });
  assert.equal(shape.mains.length,1,label+': only one page visible');
  assert.deepEqual(shape.duplicates,[],label+': duplicate IDs');
  assert(shape.scroll<=shape.width+1,label+': horizontal overflow '+JSON.stringify(shape));
  assert.deepEqual(shape.overflowing,[],label+': controls outside viewport');
  assert.equal(shape.nav.length,4,label+': only four navigation destinations');
  assert(shape.nav.every(Boolean),label+': navigation outside viewport');
  if(process.env.ORBIT_SCREENSHOTS)await page.screenshot({path:path.join(process.env.ORBIT_SCREENSHOTS,label+'.png'),animations:'disabled'});
}
async function visible(page, selector) { await page.locator(selector).first().waitFor({state:'visible'}); }
async function msg(page, value) { await page.waitForFunction(expected=>document.getElementById('msg').textContent.includes(expected),value); }
async function openTab(page, name) {
  await page.locator('[data-tab="'+name+'"]').click();
  await visible(page,'#tab-'+name);
  assert.equal(await page.locator('[role=tab][aria-selected=true]').count(),1);
}

(async()=>{
  if(process.env.ORBIT_SCREENSHOTS) fs.mkdirSync(process.env.ORBIT_SCREENSHOTS,{recursive:true});
  const browser=await chromium.launch({executablePath:process.env.ORBIT_CHROMIUM,headless:true,args:['--no-sandbox','--disable-dev-shm-usage','--disable-gpu']});
  try{
    for(const width of [320,390,768,1440]){
      const context=await browser.newContext({viewport:{width,height:width<500?844:1000},locale:'pt-PT',acceptDownloads:true});
      const sim=await device(context),page=await context.newPage(),errors=[];
      page.on('pageerror',e=>errors.push(e.message));page.on('dialog',d=>d.accept());
      await page.goto(origin);await visible(page,'#rooms .room-thermostat');
      assert.equal(await page.locator('#rooms .room-thermostat').count(),5);
      assert.equal(await page.locator('#branches .collector-circuit').count(),8);
      assert.equal(await page.locator('#viewPanel').getAttribute('aria-current'),'page');
      await page.locator('#readings .sensor-trend').first().waitFor();
      assert.equal(await page.locator('#readings .sensor-trend[data-direction=up]').count(),1);
      assert.equal(await page.locator('#readings .sensor-trend[data-direction=down]').count(),1);
      assert.match(await page.locator('#readings').innerText(),/100,6 kPa/);
      assert.equal(await page.locator('#toggleHeat').getAttribute('aria-checked'),'true');
      assert.doesNotMatch(await page.locator('#collectorCard').innerText(),/Feedback ON KNX confirmado|Tempo de serviço ·/);
      await checkLayout(page,width+'-painel');
      if(width===390){
        const button=page.locator('#railPower'),theme=page.locator('#orbitTheme'),notice=page.locator('#railPowerNotice');
        const box=await button.boundingBox(),navBox=await page.locator('.orbit-nav').boundingBox();
        assert(box.width>=76&&box.height>=76&&box.y+box.height<navBox.y);
        assert((await theme.boundingBox()).y<navBox.y);
        await theme.click();
        assert.equal(await theme.getAttribute('aria-checked'),'true');
        assert.equal(await page.locator('body').evaluate(el=>getComputedStyle(el).getPropertyValue('--orbit-accent').trim()),'#18755f');
        await page.reload();
        await visible(page,'#rooms .room-thermostat');
        assert.equal(await theme.getAttribute('aria-checked'),'true');
        await button.hover();await page.mouse.down();await page.waitForTimeout(100);await page.mouse.up();
        assert(!sim.requests.some(x=>x.pathname==='/api/power'));
        await button.hover();await page.mouse.down();
        await notice.filter({hasText:'Operação em curso'}).waitFor({state:'visible'});
        await page.mouse.up();
        const feedback=await notice.boundingBox();assert(feedback.width>20&&feedback.height>10&&feedback.y<navBox.y);
        assert.equal(await button.getAttribute('data-state'),'ok');
        await button.hover();await page.mouse.down();
        await notice.filter({hasText:'A parar'}).waitFor({state:'visible'});
        await page.mouse.up();
        assert.equal(await button.getAttribute('data-state'),'warning');
        Object.assign(sim.power,{powerCanToggle:true,powerLevel:'ok',powerLabel:''});
        await page.waitForFunction(()=>document.getElementById('railPower').dataset.state==='off');
        assert.equal(await notice.textContent(),'');
        await theme.click();
      }
      if(width===1440){
        await Promise.all([page.waitForResponse(r=>r.url().endsWith('/api/heating/control')&&r.request().method()==='POST'),page.locator('#toggleHeat').click()]);
        await page.waitForFunction(()=>!busy);
        assert(sim.requests.some(x=>x.pathname==='/api/heating/control'&&x.body.action==='stop'));
        await Promise.all([page.waitForResponse(r=>r.url().endsWith('/api/heating/control')&&r.request().method()==='POST'),page.locator('#refreshHeat').click()]);
        await page.waitForFunction(()=>!busy);
        assert(sim.requests.some(x=>x.pathname==='/api/heating/control'&&x.body.action==='refresh'));
        await Promise.all([page.waitForResponse(r=>r.url().endsWith('/api/relay')&&r.request().method()==='POST'),page.locator('#outputs button').click()]);
        await page.waitForFunction(()=>!busy);
        assert(sim.requests.some(x=>x.pathname==='/api/relay'&&x.body.index===0));
      }
      await page.locator('#viewSensors').click();await visible(page,'#sensorReadings article');
      assert.equal(await page.locator('#sensorReadings article').count(),5);
      assert.match(await page.locator('#sensorReadings').innerText(),/21,5 °C/);
      assert.match(await page.locator('#sensorReadings').innerText(),/Sem leitura/);
      await checkLayout(page,width+'-sensores');
      await page.locator('#viewHistory').click();await visible(page,'#historyChart path');
      await visible(page,'#humidityChart path');
      const zoom=await page.locator('#temperatureScroll').evaluate(el=>el.scrollWidth/el.clientWidth);
      assert(Math.abs(zoom-24)<0.05);
      await page.locator('#temperatureScroll').evaluate(el=>{el.scrollLeft=el.clientWidth*5;});
      await page.waitForFunction(()=>Math.abs(document.getElementById('temperatureScroll').scrollLeft-document.getElementById('humidityScroll').scrollLeft)<2);
      await checkLayout(page,width+'-historico');
      await page.locator('#viewHelp').click();await visible(page,'#helpArea');await checkLayout(page,width+'-ajuda');
      await page.goBack();await visible(page,'#historyArea');assert.equal(await page.locator('#viewHistory').getAttribute('aria-current'),'page');
      await page.locator('#viewSettings').click();await page.waitForURL('**/config.html');await visible(page,'#loginForm');
      assert.equal(await page.locator('#privateSettings').isVisible(),false);
      assert.equal(await page.evaluate(async()=> (await fetch('/api/diagnostics')).status),401);
      await checkLayout(page,width+'-login');
      if(width===1440){
        await page.locator('#loginPassword').fill('incorrect-fixture-password');
        await page.locator('#loginForm button').click();
        await page.waitForFunction(()=>document.getElementById('loginError').textContent.includes('incorretos'));
      }
      await page.locator('#loginPassword').fill('valid-fixture-password');await page.locator('#loginForm button').click();
      await visible(page,'#privateSettings');assert.equal(await page.locator('#loginForm').isVisible(),false);
      for(const tab of ['config','diagnostics','hardware','knx','features']){
        await openTab(page,tab);await checkLayout(page,width+'-'+tab);
      }
      assert.equal(await page.locator('#hz0margin').inputValue(),'0,3');
      if(width===1440){
        await page.locator('#hz0margin').fill('0,4');await page.locator('#saveHeating').click();await msg(page,'Mapa do aquecimento guardado');
        assert.equal(sim.requests.find(x=>x.pathname==='/api/heating/layout'&&x.post).body.control.rooms[0].margin,0.4);
        await page.locator('#sensorKnxSettings').click();await visible(page,'#tab-knx');
        assert.equal(await page.locator('input[id="0_group"]').evaluate(el=>el===document.activeElement),true);
        await page.locator('#validateKnx').click();assert.match(await page.locator('#knxValidation').innerText(),/válidos/);
        await page.locator('#knxGroup').fill('3/1/1');await page.locator('#knxOperation').selectOption('float');
        await page.locator('#knxValue').fill('21,5');await page.locator('#knxSend').click();await msg(page,'Telegrama colocado');
        assert.equal(sim.requests.find(x=>x.pathname==='/api/knx/command').body.value,21.5);
        await openTab(page,'hardware');await page.locator('#oledPreset').click();assert.equal(await page.locator('#oledReset').inputValue(),'16');
        await page.locator('#oneWire').fill('26');await page.locator('#saveOneWire').click();await msg(page,'GPIO e ROM das sondas guardados');
        assert.equal(sim.requests.filter(x=>x.pathname==='/api/config'&&x.post).at(-1).body.oneWire,26);
        await openTab(page,'diagnostics');await page.locator('#refreshDiagnostics').click();
        await page.waitForFunction(()=>document.getElementById('orbitLogs').textContent.includes('AUDIT:'));
        assert.match(await page.locator('#knxConsole').innerText(),/3\/1\/1/);
        await openTab(page,'config');assert(await page.locator('#wifiIp').isDisabled());await page.locator('#wifiDhcp').uncheck();assert(await page.locator('#wifiIp').isEnabled());
        await page.locator('#scanWifi').click();await page.locator('#wifiNetworks option[value="Rede encontrada"]').waitFor({state:'attached'});
        await page.locator('#wifiNetworks').selectOption('Rede encontrada');assert.equal(await page.locator('#ssid').inputValue(),'Rede encontrada');
        await page.locator('.settings-actions button').click();await msg(page,'Configuração guardada');
        const saved=sim.requests.filter(x=>x.pathname==='/api/config'&&x.post).at(-1);
        assert.equal(saved.body.wifiDhcp,false);assert.equal(saved.body.ssid,'Rede encontrada');
        const dl=page.waitForEvent('download');await page.locator('#export').click();assert.equal((await dl).suggestedFilename(),'iot-sensor-backup.json');
        await page.locator('#restoreFile').setInputFiles({name:'teste.json',mimeType:'application/json',buffer:Buffer.from(JSON.stringify(backup))});
        await page.locator('#restore').click();await msg(page,'Backup restaurado');
        await page.locator('#firmware').setInputFiles({name:'teste.bin',mimeType:'application/octet-stream',buffer:Buffer.from('fixture somente')});
        await page.locator('#update').click();await msg(page,'Atualização concluída');
        const ota=sim.requests.find(x=>x.pathname==='/api/update');assert.match(ota.headers['content-type'],/multipart\/form-data/);assert.equal(ota.headers['x-admin-session'],'browser-test-session');
        await page.locator('#restartDevice').click();await msg(page,'A reiniciar');
        await page.locator('#newAdminPassword').fill('nova-fixture-password');await page.locator('#changeAdminPassword').click();await visible(page,'#loginForm');
        assert.equal(sim.requests.find(x=>x.pathname==='/api/admin/password').body.password,'nova-fixture-password');
        await page.locator('#loginPassword').fill('valid-fixture-password');await page.locator('#loginForm button').click();await visible(page,'#privateSettings');
      }
      await page.locator('#logoutAdmin').click();await visible(page,'#loginForm');assert.equal(await page.locator('#privateSettings').isVisible(),false);
      assert.deepEqual(errors,[],width+': JS exceptions');assert.deepEqual(sim.violations,[]);
      console.log('PASS Chromium '+width+' px: all public views, login and five protected tabs'+(width===1440?' + KNX, hardware, diagnostics, network, backup, OTA and administration':''));
      await context.close();
    }
    for(const width of [390,1440]){
      const context=await browser.newContext({viewport:{width,height:844},locale:'pt-PT'});
      const sim=await device(context,'thermostat'),page=await context.newPage(),errors=[];
      page.on('pageerror',e=>errors.push(e.message));await page.goto(origin);
      await visible(page,'#thermoTile');assert.equal(await page.locator('#collectorCard').isVisible(),false);
      await checkLayout(page,width+'-termostato');
      await page.locator('#thermoUp').click();assert.match(await page.locator('#thermoTarget').innerText(),/22,5/);
      if(width===1440){
        await page.waitForResponse(r=>r.url().endsWith('/api/thermostat')&&r.request().method()==='POST');
        assert(sim.requests.some(x=>x.pathname==='/api/thermostat'&&x.body?.setpoint===22.5));
      }
      await page.locator('#viewSensors').click();await visible(page,'#sensorReadings article');assert.equal(await page.locator('#thermoTile').isVisible(),false);
      await page.locator('#viewSettings').click();await visible(page,'#loginForm');
      await page.locator('#loginPassword').fill('valid-fixture-password');await page.locator('#loginForm button').click();await visible(page,'#privateSettings');
      await openTab(page,'features');await visible(page,'#thermostatSettings');assert.equal(await page.locator('#collectorSettings').isVisible(),false);
      await checkLayout(page,width+'-termostato-config');
      await openTab(page,'knx');await visible(page,'#thermostatKnxSection');assert.equal(await page.locator('#heatKnxSection').isVisible(),false);
      await checkLayout(page,width+'-termostato-knx');
      assert.deepEqual(errors,[]);assert.deepEqual(sim.violations,[]);
      console.log('PASS Chromium '+width+' px: thermostat dashboard, setpoint and thermostat-only settings');
      await context.close();
    }
  }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
