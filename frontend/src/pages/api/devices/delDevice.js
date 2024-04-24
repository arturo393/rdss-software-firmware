import { connectToDatabase } from "../../../components/db/util/mongodb"

export default async function (req, res, next) {
  const { db } = await connectToDatabase()
  let inserted = false
  if (req.body.id) {
    db.collection("devices").updateOne(
      {
        id: req.body.id,
      },
      {
        $set: {
          "status.provisioned": false,
          "status.connected": false,
          "status.x": 100,
          "status.y": 100,
        },
      }
    )

    inserted = true
  }
  res.json(inserted)
}
