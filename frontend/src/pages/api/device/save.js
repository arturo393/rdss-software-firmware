import { connectToDatabase } from "../../../components/db/util/mongodb"

export default async function (req, res, next) {
  const { db } = await connectToDatabase()
  let inserted = false

  if (req.body.id) {
    const { id, _id,  type, data, name, attenuation, changed, ...fields_values } = req.body
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
          data : req.body.data,
          ...fields_values
        },
      }
    )

    inserted = true
  }
  res.json(inserted)
}
