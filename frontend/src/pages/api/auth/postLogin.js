import { connectToDatabase } from "../../../components/db/util/mongodb"

export default async function (req, res, next) {
  const { db } = await connectToDatabase()

  const user = await db.collection("users").findOne({
    name: req.body.email,
    password: req.body.password,
  })
  // .toArray()

  // const result = user.data[0] || null
  res.status(200).json(user)
}
