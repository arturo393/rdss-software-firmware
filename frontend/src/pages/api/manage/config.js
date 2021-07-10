import { connectToDatabase } from "../../../components/db/util/mongodb"

export default async function (req, res, next) {
  const { db } = await connectToDatabase()

  const config = await db
    .collection("config")
    .find({})
    .sort({ metacritic: -1 })
    .limit(20)
    .toArray()

  res.json(config)
}
