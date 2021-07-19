import { connectToDatabase } from "../../../components/db/util/mongodb"

export default async function (req, res, next) {
    const { db } = await connectToDatabase()
    db.collection('devices').updateOne(
        {
            'id': req.body.id
        },
        {
            $set: 
                {
                    'status.x': req.body.x,
                    'status.y': req.body.y
                }
        }
    );

  const inserted = true
  res.json(inserted)
}
