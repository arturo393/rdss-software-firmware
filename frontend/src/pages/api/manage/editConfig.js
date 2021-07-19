import { connectToDatabase } from "../../../components/db/util/mongodb"
import { ObjectID } from "bson"

export default async function (req, res, next) {
    const { db } = await connectToDatabase()
    db.collection('config').updateMany(
        {},
        {
            $set: 
                {
                    minVoltage: req.body.minVoltage,
                    maxVoltage: req.body.maxVoltage,
                    minCurrent: req.body.minCurrent,
                    maxCurrent: req.body.maxCurrent,
                    minUplink: req.body.minUplink,
                    maxUplink: req.body.maxUplink,
                    minDownlink: req.body.minDownlink,
                    maxDownlink: req.body.maxDownlink,
                    minDownlinkOut: req.body.minDownlinkOut,
                    maxDownlinkOut: req.body.maxDownlinkOut
                }
        }
    );

  const inserted = true
  res.json(inserted)
}
