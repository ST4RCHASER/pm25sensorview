const express = require('express')
const axios = require('axios')
const app = express()
const port = 3210
const config = {
  debug: true,
}
let average = 0;
let default_response;
let nodeLists = [];
let groups = [];
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
    all_average: average,
    node_list: nodeLists,
    groups: groups,
    startTime: startTime,
    uptime: process.uptime(),
  }
  next();
})
app.get('/', (req, res) => {
  res.json(default_response);
})
app.get('/view', (req, res) => {
  //View html file
  res.sendFile(__dirname + '/view.html');
});
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
  calcAvg();
  res.json({
    success: true,
    message: 'Wakatta!',
  });
});

app.listen(port, () => {
  console.log(`[APP] Express listening on port ${port}`)
})

function calcAvg() {
  //Calc average value
  let sum = 0;
  for (const node of nodeLists) {
    sum += node.value;
  }
  average = sum / nodeLists.length;
  groups = [];
  for (const node of nodeLists) {
    let group = groups.find(group => group.name == node.group);
    if (!group) {
      groups.push({
        name: node.group,
        nodes: [node],
      });
    } else {
      group.nodes.push(node);
    }
  }
  for (const group of groups) {
    let sum = 0;
    for (const node of group.nodes) {
      sum += node.value;
    }
    group.average = sum / group.nodes.length;
  }
  if (config.debug) console.log('[DEBUG] Groups:', groups);
}
