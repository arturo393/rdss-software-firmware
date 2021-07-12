import { connectToDatabase } from "../../../components/db/util/mongodb"
import { ObjectID } from "bson"

export default async function (req, res, next) {
    const { db } = await connectToDatabase()
    db.collection('config').updateMany(
        {},
        {
            $set: 
                {
                    image: req.body.photo
                }
        }
    );

  const inserted = true
  res.json(inserted)
}
