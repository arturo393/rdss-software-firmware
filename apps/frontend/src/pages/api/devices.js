import { connectToDatabase } from "../../util/mongodb"
import axios from "axios"

export default async function (req, res) {
  const { db } = await connectToDatabase()
  const devices = await db
    .collection("devices")
    .find({})
    .sort({ metacritic: -1 })
    .limit(20)
    .toArray()

  axios.get("http://backend:4200/?name=" + devices[0].name).then((res) => {
    // console.log(res)
  })

  res.json({ status: "name set" })
}
