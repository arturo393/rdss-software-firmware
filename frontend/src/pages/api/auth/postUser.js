import { connectToDatabase } from "../../../components/db/util/mongodb"

export default async function (req, res, next) {
  const { db } = await connectToDatabase()
  db.collection('users').insertOne({
    name: req.body.email,
    company: req.body.company,
    password: req.body.password
  });

  const inserted = true
  res.json(inserted)
}
