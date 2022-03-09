const express = require('express')
const axios = require('axios')
const app = express()
const port = 3210
const config = {
  debug: true,
  // token: 'rYuJtEkyQSTGDUUE-ADR_ypPDKrmjYiy',
  token: 'l6323ydKzOhq2xk3bex_FQ2EZiCtjnHD',
  deviceId: 'V5',
}
let average = 0;
let last_sent = 0;
let last_try = 0;
let default_response;
let nodeLists = [];
let startTime = new Date();
app.use(express.json())
app.use(express.urlencoded({ extended: true }))
app.all('*', (req, res, next) => {
  res.header("Access-Control-Allow-Origin", "*");
  res.header("Access-Control-Allow-Headers", "X-Requested-With");
  default_response = {
    success: true,
    debug: config.debug,
    hostname: 'hidden',
    message: 'PM2 Sensor Project (yue.sh)',
    node_id: 0,
    average: average,
    node_list: nodeLists,
    last_try: last_try,
    last_sent: last_sent,
    startTime: startTime,
    uptime: process.uptime(),
  }
  next();
})
app.get('/', (req, res) => {
  res.json(default_response);
})
app.post('/', (req, res) => {
  if (config.debug) console.log('[DEBUG] Request body', req.body);
  if (!req.body.node_id || !req.body.value) {
    if (config.debug) console.log('[DEBUG] Rejected:', req.body.node_id, req.body.value);
    return res.json(default_response);
  }
  //Check if value is number
  if (isNaN(req.body.value)) {
    if (config.debug) console.log('[DEBUG] Rejected value:', req.body.node_id, req.body.value);
    return res.json(default_response);
  }
  let node = nodeLists.find(node => node.node_id == req.body.node_id);
  if (!node) {
    req.body.last_update = new Date();
    nodeLists.push(req.body);
    if (config.debug) console.log('[DEBUG] New node added to list:', req.body);
  } else {
    //Delete old and add new
    nodeLists = nodeLists.filter(node => node.node_id != req.body.node_id);
    req.body.last_update = new Date();
    nodeLists.push(req.body);
    if (config.debug) console.log('[DEBUG] Node edited to list:', node);
  }
  submitToBlynk();
  res.json({
    success: true,
    message: 'Wakatta!',
  });
});

app.listen(port, () => {
  console.log(`[APP] Express listening on port ${port}`)
})

function submitToBlynk() {
  last_try = new Date();
  //Calc average value
  let sum = 0;
  for (const node of nodeLists) {
    sum += node.value;
  }
  average = sum / nodeLists.length;
  //Send to Blynk
  let str_average = average.toString();
  if (str_average.length >= 4) {
    str_average = str_average.substring(0, 6);
  }
  axios.put(`http://blynk2.iot-cm.com:8080/${config.token}/update/${config.deviceId}?value=${str_average}`, JSON.stringify([str_average]), {
    headers: {
      'Content-Type': 'application/json'
    }
  });
  last_sent = new Date();
}
