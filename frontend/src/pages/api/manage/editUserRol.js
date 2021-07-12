import { connectToDatabase } from "../../../components/db/util/mongodb"
import { ObjectID } from "bson"

export default async function (req, res, next) {
    const { db } = await connectToDatabase()
    db.collection('users').updateOne(
        {
            _id: ObjectID(req.body.id)
        },
        {
            $set: 
                {
                    rolname: req.body.rolname
                }
        }
    );

  const inserted = true
  res.json(inserted)
}
