import { connectToDatabase } from "../../../components/db/util/mongodb"

export default async function (req, res) {
  const { db } = await connectToDatabase()

  console.log(req.body)
  const filter = req.body.filter

  const filterRtdata = 
    {
      '$project': {
        'id': 1,
        'status':1,
        'type' : 1
      }
    }
  ;


  const pipeline = [filterRtdata];

  const devices = await db
    .collection("devices")
    .aggregate(pipeline).toArray()


  res.json(devices)
}
