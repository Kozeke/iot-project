const express = require('express');
const router = express.Router();
const IotSensor = require('../model/iotSensor')
const mongoose = require('mongoose');

// var request = require('request');



// var form = {
//     "dataSource": 'Cluster0',
//     "database": 'test',
//     "collection": 'iot_sensors',
//     "filter": { "truck": "wokwi-weather" }

// };

// request.post({
//     headers: {
//         'api-key': 'aMEZyLmeNjyAigvrYdy4nIKZLIXtjqJ8F6CDjSarplf76Osdww6yDBgvO3bHqdZI',
//         'Content-Type': 'application/json',
//         'Access-Control-Request-Headers': '*'
//     },
//     url: 'https://data.mongodb-api.com/app/data-tgpkz/endpoint/data/v1/action/findOne',
//     body: form,
//     json: true
// }, function (error, response, body) {
//     if (!error && response.statusCode == 200) {
//         console.log(body.document)
//     }
// });
// console.log(Math.round(new Date().getTime()/1000))
// request.post({
//     headers: {
//         'api-key': 'aMEZyLmeNjyAigvrYdy4nIKZLIXtjqJ8F6CDjSarplf76Osdww6yDBgvO3bHqdZI',
//         'Content-Type': 'application/json',
//         'Access-Control-Request-Headers': '*'
//     },
//     url: 'https://industrial.api.ubidots.com/api/v2.0/variables/<variable_id>/_/values/delete/',
//     body: form,
//     json: true
// }, function (error, response, body) {
//     if (!error && response.statusCode == 200) {
//         console.log(body.document)
//     }
// });

// var formData = querystring.stringify(form);
// var contentLength = formData.length;


router.get('/light', (req, res, next) => {
    IotSensor.find().lean()
        .exec((err, items) => {
            if (!err) {
                let obj = items.find(o => o.truck === '/v1.6/devices/garden/light');
                return res.json({ data: obj.m })
            }
        })
})

router.get('/soil', (req, res, next) => {
    IotSensor.find().lean()
        .exec((err, items) => {
            if (!err) {
                let obj = items.find(o => o.truck === '/v1.6/devices/garden/soil');
                return res.json({ data: obj.m })
            }
        })
})

router.get('/temp', (req, res, next) => {
    IotSensor.find().lean()
        .exec((err, items) => {
            if (!err) {
                let obj = items.find(o => o.truck === '/v1.6/devices/garden/temp');
                return res.json({ data: obj.m })
            }
        })
})

router.get('/motor', (req, res, next) => {
    IotSensor.find().lean()
        .exec((err, items) => {
            if (!err) {
                let obj = items.find(o => o.truck === '/v1.6/devices/garden/motor');
                return res.json({ data: obj.m })
            }
        })
})

module.exports = router;