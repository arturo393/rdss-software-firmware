import { connectToDatabase } from "../../../components/db/util/mongodb"

export default async function (req, res, next) {
  const { db } = await connectToDatabase()

  const user = await db.collection("users").findOne(
    {
      email: req.body.email,
      password: req.body.password,
    },
    { password: 0, _id: 0 }
  )
  console.log(user)
  // .toArray()
  res.status(200).json(user)
}
