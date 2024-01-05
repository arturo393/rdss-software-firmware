import { connectToDatabase } from "../../../components/db/util/mongodb"
export default async function (req, res) {
  const { db } = await connectToDatabase()
  const type = await db.collection("type").find({type: 'sniffer'}).toArray()
  // const type = await db
  // .collection("type")
  // .find({})
  // .sort({ metacritic: -1 })
  // .limit(20)
  // .toArray()
  //console.log("typeMEmo",type)
res.json(type)
}
  // const filterRtdata = {
  //   $project: {
  //     id: 1,
  //     status: 1,
  //     type: 1,
  //     name: 1,
  //   },
  // }
  // const pipeline = [filterRtdata]
 //const devices = await db.collection("devices").aggregate(pipeline).toArray()
 // res.json(devices)
  // const type = await db.collection("type").find()
  // console.log("typeMEmo",type)
//  console.log("Disposii:",type)
// const devices2 = Object.assign({}, type, devices);
  // res.json(type)
//}
