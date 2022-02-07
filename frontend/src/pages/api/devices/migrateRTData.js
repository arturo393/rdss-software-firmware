import fs from "fs"
import path from "path"
import { connectToDatabase } from "../../../components/db/util/mongodb"

export default async (req, res) => {
  const { db } = await connectToDatabase()
  const dirRelativeToMigrationFolder = "data"
  const dir = path.resolve("./migration", dirRelativeToMigrationFolder)
  const filenames = fs.readdirSync(dir)
  const deviceId = req.query.deviceId || 0
  let ans = "Device: " + deviceId

  if (!deviceId) {
    res.json("No device id provided")
  }

  if (filenames.length === 0) {
    res.json("No file found")
  }

  console.log("==========================")
  fs.readFile(path.join(dir, filenames[0]), "utf8", (error, data) => {
    if (error) {
      console.log(error)
      res.json("Error opening file")
    }
    const obj = JSON.parse(JSON.stringify(data))
    const arrDevices = obj.split("\n")

    const deviceData = JSON.parse(arrDevices[deviceId - 1])
    const { id, rtData } = deviceData
    insertDocuments(db, id, rtData)
  })

  res.json(ans + "...Migration started\n Please do not close this window until you verify all data is migrated")
}

function insertDocuments(db, id = 0, rtData = {}) {
  if (!id || rtData.length === 0) return {}
  // ensure id as integer
  console.log("Device: " + id)
  console.log("Inserting documents...")
  console.log(rtData.length)
  let counter = 1
  //   var bulk = db.collection("rtData").initializeUnorderedBulkOp()
  rtData.map((item) => {
    const newTSDoc = {
      sampleTime: new Date(item.sampleTime),
      alerts: item.alerts,
      metaData: { deviceId: parseInt(id) },
    }
    if (item.voltage !== undefined) {
      newTSDoc.voltage = item.voltage.toString()
    }
    if (item.current !== undefined) {
      newTSDoc.current = item.current.toString()
    }
    if (item.power !== undefined) {
      newTSDoc.power = item.power.toString()
    }
    if (item.gupl !== undefined) {
      newTSDoc.gupl = item.gupl.toString()
    }
    if (item.gdwl !== undefined) {
      newTSDoc.gdwl = item.gdwl.toString()
    }

    try {
      db.collection("rtData").insertOne(newTSDoc)
    } catch (error) {
      console.log("============ERROR===========")
      console.log(newTSDoc)
      console.log(error)
    }
  })
  console.log("Done")
}
