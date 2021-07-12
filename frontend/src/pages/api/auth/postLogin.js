import { connectToDatabase } from "../../../components/db/util/mongodb"

export default async function (req, res, next) {
  const { db } = await connectToDatabase()

  const users = await db
    .collection("users")
    .find(
      {
        name: req.body.email,
        password: req.body.password
      }
    )
    .sort({ metacritic: -1 })
    .limit(20)
    .toArray()
    
  res.json(users)
}
