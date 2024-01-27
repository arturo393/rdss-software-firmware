import { connectToDatabase } from "../../../components/db/util/mongodb"
import { ObjectId } from "mongodb"

const COLLECTION = "fields"
export default async function (req, res) {
    try {
        const { db } = await connectToDatabase()
        if (req.method === "GET") {
            const fields = await db
            .collection(COLLECTION)
            .find({})
            .sort({ metacritic: -1 })
            .limit(10000)
            .toArray()

            res.status(200).json(fields)
        } else if (req.method === "POST") {
            const field = req.body // Obtén los datos del cuerpo de la solicitud
            await db.collection(COLLECTION).insertOne(field)
            res.status(201).end("Field created successfully")
        } else if (req.method === "PUT") {
            const fieldId = req.query.id // Obtén el ID del campo de la consulta
            const updatedField = req.body // Obtén los datos actualizados del cuerpo de la solicitud
            await db.collection(COLLECTION).updateOne({ _id: fieldId }, { $set: updatedField })
            res.status(200).end("Field updated successfully")
        } else if (req.method === "DELETE") {
            const fieldId = req.query.id // Obtén el ID del campo de la consulta
            console.log("API DEl", fieldId)
            await db.collection(COLLECTION).deleteOne({ _id: ObjectId(fieldId) })
            res.status(200).end("Field deleted successfully")
        } else {
            res.status(405).end(`Method ${req.method} Not Allowed`)
        }
    } catch (error) {
        console.error(error)
        res.status(500).end("Internal Server Error")
    }
}
