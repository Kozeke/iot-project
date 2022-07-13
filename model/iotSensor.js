const mongoose = require('mongoose');
const Schema = mongoose.Schema;
const ObjectId = Schema.ObjectId;

const IotSensorSchema = new Schema({
    _id: ObjectId,
    truck: String,
    bktSize: Number,
    m: Array,
    max_ts: Date
});

const IotSensor = mongoose.model('Iot_sensor', IotSensorSchema);


module.exports = IotSensor;