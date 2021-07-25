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
                    'status.provisioned': true,
                    'status.connected': false,
                }
        }
    );

  const inserted = true
  res.json(inserted)
}
