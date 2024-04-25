import { connectToDatabase } from "../../../components/db/util/mongodb"
import { ObjectID } from "bson"

export default async function (req, res) {
  const { db } = await connectToDatabase()

  const rol = await db
    .collection("roles")
    .find({_id: ObjectID(req.body.rolId)})
    .sort({ metacritic: -1 })
    .limit(20)
    .toArray()

  res.json(rol)
}
