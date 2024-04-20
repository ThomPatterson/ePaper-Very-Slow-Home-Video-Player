const express = require('express');
const fs = require('fs');
const path = require('path');
const config = process.env.hasOwnProperty('CONFIG') ? JSON.parse(process.env.CONFIG) : require('./config.js');
const StateController = require('./stateController');
const processVideos = require('./processVideos.js');

const app = express();
const PORT = process.env.PORT || 8080;

processVideos();//invoke immediately
setInterval(processVideos, 5 * 60 * 1000);  // Invoke again every 5 minutes

app.get('/image', (req, res) => {

  if (!req.query.displayId) {
    res.status(400).send('Missing displayId query parameter');
    return;
  }

  if (!StateController.hasDisplayWithId(req.query.displayId)) {
    res.status(400).send('No display found for Id = ' + req.query.displayId);
    return;
  }

  try {
    let bitmapData = StateController.getNextFrame(req.query.displayId);
    res.setHeader('Content-Type', 'image/bmp');
    res.end(bitmapData);
  } catch (err) {
    res.status(500).send('Error reading BMP file: ' + err);
    console.error(err);
  }

});

app.listen(PORT, () => {
  console.log(`Server running at http://localhost:${PORT}/`);
});