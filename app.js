const express = require('express');
const app = express();
const mongoose = require('mongoose');
const config = require('config');
const morgan = require('morgan');

const userRouter = require('./routes/iotSensor')
const dbConfig = config.get('project.dbConfig.dbName');
const ubidots = require('./ubidots/ubidots')

mongoose.connect(dbConfig).then(() => {
    console.log('Database connected');
}).catch(err => {
    console.log('Database not connected' + err)
});

app.use(morgan('dev'));

app.use(express.json());
app.use('/user', userRouter);


app.use('/', (req, res, next) => {
    res.status(200).json({
        message: "hello world"
    });
});


module.exports = app;