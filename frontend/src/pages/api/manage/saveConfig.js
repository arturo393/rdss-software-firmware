import { connectToDatabase } from "../../../components/db/util/mongodb"

export default async function (req, res, next) {
  const { db } = await connectToDatabase()
  if (req.body) {
    var response = await db.collection("config").updateOne(
      {},
      {
        $set: req.body,
      }
    )
  }
  const inserted = true
  res.json(inserted)
}
