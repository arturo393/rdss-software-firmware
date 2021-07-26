import { connectToDatabase } from "../../../components/db/util/mongodb"
//import  {ISODate} from 'isodate'

export default async function (req, res, next) {
  const { db } = await connectToDatabase()

  const device = await db.collection("devices").find({id: req.body.id}).toArray()

  res.json(device)
}
