export default async function (req, res) {
  const { db } = await connectToDatabase()
  const id = req.query.id

  const device = await db.collection("devices").find({ id: id }).toArray()

  res.json(device)
}
