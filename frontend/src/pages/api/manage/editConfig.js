import { connectToDatabase } from "../../../components/db/util/mongodb"

export const config = {
  api: {
    bodyParser: {
      sizeLimit: "5mb",
    },
  },
}

export default async function (req, res, next) {
  const { db } = await connectToDatabase()
  if (req.body) {
    var response = await db.collection("config").updateOne(
      {},
      {
        $set: {
          minVoltage: req.body.minVoltage,
          maxVoltage: req.body.maxVoltage,
          minCurrent: req.body.minCurrent,
          maxCurrent: req.body.maxCurrent,
          minUplink: req.body.minUplink,
          maxUplink: req.body.maxUplink,
          minDownlink: req.body.minDownlink,
          maxDownlink: req.body.maxDownlink,
          minDownlinkOut: req.body.minDownlinkOut,
          maxDownlinkOut: req.body.maxDownlinkOut,
          image: req.body.image,
        },
      }
    )
  }
  const inserted = true
  res.json(inserted)
}
