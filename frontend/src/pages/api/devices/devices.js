import { connectToDatabase } from "../../../components/db/util/mongodb"
export default async function (req, res) {
  const { db } = await connectToDatabase()
  const filterRtdata = {
    $project: {
      id: 1,
      status: 1,
      type: 1,
      name: 1,
    },
  }
  const pipeline = [filterRtdata]
 //const devices = await db.collection("devices").aggregate(pipeline).toArray()
 // res.json(devices)
  const type = await db.collection("type").find({type: 'sniffer'}).toArray()
  const devices = await db.collection("devices").find({type: 'sniffer'}).toArray()
//  console.log("Disposii:",type)
// const devices2 = Object.assign({}, type, devices);
  res.json(devices)
}
