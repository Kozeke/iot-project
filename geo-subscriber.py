##########################################################################################
#
# This is an example subcriber to an MQTT Broker like HiveMQ.
#
# Note: It is not intended for production use since no authentication
#	   and encrytpion is in place.
#
# For more information, please have a look at:
#		MongoDB Atlas: https://cloud.mongodb.com
#		MongoDB Client for python: https://api.mongodb.com/python/current/
#		HiveMQ MQTT Broker: https://www.hivemq.com/
#		Eclipse Paho MQTT Client for python: https://www.eclipse.org/paho/clients/python/
#											 and https://pypi.org/project/paho-mqtt/
#
###########################################################################################
import json
import os
import json
import paho.mqtt.client as mqtt
from datetime import datetime
import pymongo

BATCH_SIZE_MESSAGES = 10
MONGO_BUCKET_SIZE = 60
message_batch = []

MQTT_HOST = 'industrial.api.ubidots.com'
USERNAME = 'BBFF-8jMRx8MB8JNxxZuMGLJ8fSocMFRHGe'
PASSWORD = ''
if MQTT_HOST == None:
    raise ValueError('No MQTT Broker provided. Will exit.')
    exit(-1)

MONGO_URI = 'mongodb+srv://kevin_cox:HP7mEsdYbMf6rXrO@cluster0.afuif.mongodb.net/?retryWrites=true&w=majority'
if MONGO_URI == None:
    raise ValueError('No MongoDB Cluster provided. Will exit.')
    exit(-1)

# The callback for when the client receives a CONNACK response from the MQTT server.


def on_connect(client, userdata, flags, rc):
    print('Connected to MQTT broker with result code ' + str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    # We want to subscribe to the status topic of all stations
    client.subscribe('/v1.6/devices/garden/light')
    client.subscribe('/v1.6/devices/garden/temp')
    client.subscribe('/v1.6/devices/garden/soil')
    client.subscribe('/v1.6/devices/garden/motor')

# The callback for when a PUBLISH message is received from the MQTT server.
# Optimization: As we receive many messages in one shot, the results should be processed in a batched manner.


def on_message(client, userdata, message):
    print('Received message ' + str(message.payload) + ' on topic ' +
          message.topic + ' with QoS ' + str(message.qos))
    # We generate a timestamp here, this should be included by the trucks themselves so that we have both
    # timestamps: captured ts, and written ts
    ts = datetime.today()
    # payload = json.loads(message.payload)

    is_parsable = is_json(message.payload)
    if is_parsable:
        payload = json.loads(message.payload)
    #  {"location":{"lat":7.628477821925232,"lon":51.48995060174505},"routeId":"5bd3c108-2681-4135-80fb-1c73632b98fd","speed":140.01392213036448,"speedLimit":120.0,"break":false}
        if(message.topic == '/v1.6/devices/garden/light'):
            message_batch.append(pymongo.UpdateOne(

                {
                    'truck': message.topic,
                    'bktSize': {'$lt': MONGO_BUCKET_SIZE}
                },
                {
                    '$push': {
                        'm': {
                            'light': payload['value'],
                            'timestamp': ts
                        }
                    },
                    '$max': {'max_ts': ts},
                    '$min': {'min_ts': ts},
                    '$inc': {'bktSize': 1}
                },
                upsert=True))
        if(message.topic == '/v1.6/devices/garden/temp'):
            message_batch.append(pymongo.UpdateOne(

                {
                    'truck': message.topic,
                    'bktSize': {'$lt': MONGO_BUCKET_SIZE}
                },
                {
                    '$push': {
                        'm': {
                            'temp': payload['value'],
                            'timestamp': ts

                        }
                    },
                    '$max': {'max_ts': ts},
                    '$min': {'min_ts': ts},
                    '$inc': {'bktSize': 1}
                },
                upsert=True))
        if(message.topic == '/v1.6/devices/garden/soil'):
            message_batch.append(pymongo.UpdateOne(

                {
                    'truck': message.topic,
                    'bktSize': {'$lt': MONGO_BUCKET_SIZE}
                },
                {
                    '$push': {
                        'm': {
                            'soil': payload['value'],
                            'timestamp': ts

                        }
                    },
                    '$max': {'max_ts': ts},
                    '$min': {'min_ts': ts},
                    '$inc': {'bktSize': 1}
                },
                upsert=True))
        if(message.topic == '/v1.6/devices/garden/motor'):
            message_batch.append(pymongo.UpdateOne(

                {
                    'truck': message.topic,
                    'bktSize': {'$lt': MONGO_BUCKET_SIZE}
                },
                {
                    '$push': {
                        'm': {
                            'motor': payload['value'],
                            'timestamp': ts

                        }
                    },
                    '$max': {'max_ts': ts},
                    '$min': {'min_ts': ts},
                    '$inc': {'bktSize': 1}
                },
                upsert=True))

    write_batch(batch=message_batch, collection=status_collection,
                batch_size=BATCH_SIZE_MESSAGES, full_batch_required=True)


def write_batch(batch, collection, batch_size=100, full_batch_required=False):
    # Writes batch of pymongo Bulk operations into the provided collection.
    # Full_batch_required can be used to write smaller amounts of data, e.g. the last batch that does not fill the batch_size
    # print(len(batch))
    # print(batch)
    if len(batch) > 0 and ((full_batch_required and len(batch) >= batch_size) or full_batch_required):
        try:
            result = collection.bulk_write(batch)
            print(str(datetime.today()) + ' Wrote ' + str(len(batch)) +
                  ' to MongoDB (' + str(collection.name) + ').')
            batch.clear()
        except pymongo.errors.BulkWriteError as err:
            print(str(datetime.today()) +
                  ' ERROR Writing to MongoDB: ' + str(err.details))


def is_json(myjson):
    try:
        json.loads(myjson)
    except ValueError as e:
        return False
    return True


# Setup MQTT broker connectionci
client = mqtt.Client(client_id='garden-subscriber')
client.on_connect = on_connect
client.on_message = on_message
client.username_pw_set(USERNAME, PASSWORD)
client.connect(MQTT_HOST, 1883, 60)

# Setup MongoDB connection
mongo_client = pymongo.MongoClient(MONGO_URI)
db = mongo_client.test
status_collection = db.iot_sensors

# Efficient queries per device and timespan
status_collection.create_index([('truck', pymongo.ASCENDING),
                                ('min_ts', pymongo.ASCENDING),
                                ('max_ts', pymongo.ASCENDING)])

# # Efficient queries per truck, route and timespan
# status_collection.create_index([('truck', pymongo.ASCENDING),
#                                 ('routeId', pymongo.ASCENDING),
#                                 ('min_ts', pymongo.ASCENDING),
#                                 ('max_ts', pymongo.ASCENDING)])

# # Efficient retreival of open buckets per device
# status_collection.create_index([('truck', pymongo.ASCENDING),
#                                 ('routeId', pymongo.ASCENDING),
#                                 ('bktSize', pymongo.ASCENDING)],
#                                partialFilterExpression={'bktSize': {'$lt': MONGO_BUCKET_SIZE}})

# Start to listen to the HiveMQ Broker
client.loop_forever()
