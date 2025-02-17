import { connectToDatabase } from "../../../components/db/util/mongodb"

export default async function (req, res) {
  const { db } = await connectToDatabase()
  const { id } = req.query

  const device = await db
    .collection("devices")
    .find({ id: parseInt(id) })
    .project({ name: 1, type: 1, id: 1 })
    .limit(1)
    .toArray()
  res.json(device)
}
