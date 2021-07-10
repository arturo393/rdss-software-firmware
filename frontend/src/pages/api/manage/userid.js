import { connectToDatabase } from "../../../components/db/util/mongodb"
import { ObjectID } from "bson"

export default async function (req, res) {
  const { db } = await connectToDatabase()

  const user = await db
    .collection("users")
    .find({_id: ObjectID(req.body.userId)})
    .sort({ metacritic: -1 })
    .limit(20)
    .toArray()

  res.json(user)
}
