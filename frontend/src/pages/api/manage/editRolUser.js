import { connectToDatabase } from "../../../components/db/util/mongodb"
import { ObjectID } from "bson"

export default async function (req, res, next) {
    const { db } = await connectToDatabase()
    db.collection('roles').updateOne(
        {
            _id: ObjectID(req.body.id)
        },
        {
            $set: 
                {
                    users: req.body.users
                }
        }
    );

  const inserted = true
  res.json(inserted)
}
