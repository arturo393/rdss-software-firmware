import { connectToDatabase } from "../../../components/db/util/mongodb"
//import  {ISODate} from 'isodate'

export default async function (req, res, next) {
  const { db } = await connectToDatabase()


  const today = new Date()
const yesterday = new Date(today)

yesterday.setDate(yesterday.getDate() - 1)

const filterRtdata = 
{
  '$project': {
    'rtData': {
      '$filter': {
        'input': '$rtData',
        'as': 'item',
        'cond': {
          '$gte': [
            '$$item.sampleTime',
            req.body.fechaHaciaAdelante
          ]
        }
      }
    },
    'id': 1,
    'status':1,
    'type' : 1
  }
}
;


const filterId =   { '$match': { 'id' : req.body.id } } ;


const pipeline = [filterId, filterRtdata ];

const devices = await db
.collection("devices")
.aggregate(pipeline).toArray()


  res.json(devices)
}
