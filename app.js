const express = require('express');
const app = express();
const mongoose = require('mongoose');
const config = require('config');
const morgan = require('morgan');


app.use((req, res, next) => {
    res.header('Access-Control-Allow-Origin', '*');
    res.header(
        'Access-Control-Allow-Headers',
        'Origin,X-Requested-With,Content-Type,Accept,Authorization'
    );

    if (req.method === 'OPTIONS') {
        res.header('Accept-Control-Methods', 'PUT,POST,PATCH,DELETE,GET');
        return res.status(200).json({

        })
    }
    next();
});


const userRouter = require('./routes/iotSensor')
const dbConfig = config.get('project.dbConfig.dbName');
// const ubidots = require('./ubidots/ubidots')

mongoose.connect(dbConfig).then(() => {
    console.log('Database connected');
}).catch(err => {
    console.log('Database not connected' + err)
});

app.use(morgan('dev'));

app.use(express.json());
app.set('json spaces', 40);

app.use('/iot-sensor', userRouter);


app.use('/', (req, res, next) => {
    res.status(200).json({
        message: "hello world"
    });
});


module.exports = app;