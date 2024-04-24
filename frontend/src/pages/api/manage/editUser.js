import { connectToDatabase } from "../../../components/db/util/mongodb"
import { ObjectID } from "bson"

export default async function (req, res, next) {
  const { db } = await connectToDatabase()

  let user = {}
  user.name = req.body.name
  user.email = req.body.email
  if (req.body.password) user.password = req.body.password

  console.log(user)
  console.log(req.body.id)
  //   db.collection("users").updateOne(
  //     {
  //       _id: ObjectID(req.body.id),
  //     },
  //     {
  //       $set: user,
  //     }
  //   )

  const inserted = true
  res.json(inserted)
}
