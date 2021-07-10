import { connectToDatabase } from "../../../components/db/util/mongodb"
import { ObjectID } from "bson"

export default async function (req, res, next) {
    const { db } = await connectToDatabase()
    db.collection('users').deleteOne(
        {
            _id: ObjectID(req.body.id)
        }
    );

  const inserted = true
  res.json(inserted)
}
