import { connectToDatabase } from "../../../components/db/util/mongodb"

export default async function (req, res) {
  const { db } = await connectToDatabase()

  const users = await db
    .collection("users")
    .find({})
    .sort({ metacritic: -1 })
    .limit(20)
    .toArray()
    
  res.json(users)
}
