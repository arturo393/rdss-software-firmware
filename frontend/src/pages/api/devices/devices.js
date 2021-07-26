import { connectToDatabase } from "../../../components/db/util/mongodb"

export default async function (req, res) {
  const { db } = await connectToDatabase()

  console.log(req.body)
  const filter = req.body.filter

  const devices = await db
    .collection("devices")
    .find({ filter })
    .sort({ metacritic: -1 })
    .toArray()

  res.json(devices)
}
