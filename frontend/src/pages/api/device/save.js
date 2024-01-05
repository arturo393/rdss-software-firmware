import { connectToDatabase } from "../../../components/db/util/mongodb"

export default async function (req, res, next) {
  const { db } = await connectToDatabase()
  let inserted = false

  console.log(req)
  if (req.body.id) {
    db.collection("devices").updateOne(
      {
        id: req.body.id,
      },
      {
        $set: {
          name: req.body.name,
          type: req.body.type,
          attenuation: req.body.attenuation,
          changed: true,
          data : req.body.data
        },
      }
    )

    inserted = true
  }
  res.json(inserted)
}
