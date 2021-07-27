import { connectToDatabase } from "../../../components/db/util/mongodb"

export default async function (req, res, next) {
  const { db } = await connectToDatabase()

  let user = { name: req.body.name, admin: req.body.admin }
  if (req.body.password) user.password = req.body.password
  const query = { email: req.body.email }
  const update = { $set: user }
  const options = { upsert: true }

  db.collection("users").updateOne(query, update, options)

  const inserted = true
  res.json(inserted)
}
