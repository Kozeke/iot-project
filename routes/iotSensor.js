const express = require('express');
const router = express.Router();
const IotSensor = require('../model/iotSensor')
const mongoose = require('mongoose');

router.get('/', (req, res, next) => {
    IotSensor.find()
        .then(data => {
            console.log(data)
        });
    res.status(200).json({
        message: "got data"
    })
})

router.post('/', (req, res, next) => {
    const user_1 = new User({
        user_id: mongoose.Types.ObjectId(),
        mail: req.body.mail,
        name: req.body.name,
        age: req.body.age
    });

    user_1.save().then(result => {
        console.log(result);
        res.status(200).json({
            message: result
        })
    }).catch(err => {
        console.log(err);
        res.status(500).json({
            message: err
        })
    });

});


module.exports = router;