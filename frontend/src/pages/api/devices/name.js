import { connectToDatabase } from "../../../components/db/util/mongodb"

export default async function (req, res) {
  const { db } = await connectToDatabase()
  const name = req.query.name

  const device = await db.collection("devices").find({ name: name }).toArray()

  res.json(device)
}
