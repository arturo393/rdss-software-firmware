import { connectToDatabase } from "../../../components/db/util/mongodb"

export default async function (req, res) {
  const { db } = await connectToDatabase()

  const devices = await db
    .collection("devices")
    .find({})
    .sort({ metacritic: -1 })
    .toArray()

  res.json(devices)
}
