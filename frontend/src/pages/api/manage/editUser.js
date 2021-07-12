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
                    name: req.body.email,
                    company: req.body.company
                }
        }
    );

  const inserted = true
  res.json(inserted)
}
