import { connectToDatabase } from "../../../components/db/util/mongodb"
import { DateTime } from "luxon"

export default async function (req, res, next) {
  const { db } = await connectToDatabase()

  var start = DateTime.fromISO(req.body.dateFrom)
  var end = DateTime.fromISO(req.body.dateTo)

  var diffInDays = end.diff(start, "days").as("days")

  // console.log("diffInDays", diffInDays)

  let dynId = {
    device: "$id",
    year: "$year",
    month: "$month",
    day: "$day",
  }

  if (diffInDays > 4 && diffInDays <= 31) {
    dynId.hour = "$hour"
  } else if (diffInDays > 0.25 && diffInDays <= 4) {
    dynId.hour = "$hour"
    dynId.minute = "$minute"
  } else if (diffInDays <= 0.25) {
    dynId.hour = "$hour"
    dynId.minute = "$minute"
    dynId.second = "$second"
  }

  
  const pipeline = [
    {
      $match: {
        $and: [{ id: req.body.id }, { sampleTime: { $gte: new Date(start).toISOString(), } }, { sampleTime: { $lte: new Date(end).toISOString() } }],
      },
    },
    {
      $project: {
        id: "$id",
        year: { $year: { $toDate: "$sampleTime" } },
        month: { $month: { $toDate: "$sampleTime" } },
        day: { $dayOfMonth: { $toDate: "$sampleTime" } },
        hour: { $hour: { $toDate: "$sampleTime" } },
        minute: { $minute: { $toDate: "$sampleTime" } },
        seconds: { $second: { $toDate: "$sampleTime" } },
        connected: "$connected",
        field_values: {
          $objectToArray: "$field_values",
        },
      },
    },
    {
      $unwind: "$field_values",
    },
    {
      $group: {
        _id: dynId,
        connected: {
          $last: "$connected",
        },
        values: {
          $push: {
            k: "$field_values.k",
            value: "$field_values.v.value",
            alert: "$field_values.v.alert",
          },
        },
      },
    },
    {
      $project: {
        _id: "$_id",
        connected: "$connected",
        field_values: {
          $arrayToObject: {
            $map: {
              input: "$values",
              as: "value",
              in: {
                k: "$$value.k",
                v: {
                  value: {
                    $avg: "$$value.value",
                  },
                  alert: {
                    $allElementsTrue: {
                      $map: {
                        input: ["$$value.alert"],
                        as: "alert",
                        in: "$$alert",
                      },
                    },
                  },
                },
              },
            },
          },
        },
      },
    },
    {
      $sort: {
        _id: 1,
      },
    },
  ]

  const fields = await db.collection("fields").find({ plottable: true }).toArray()
  const rtData = await db.collection("rtData").aggregate(pipeline).toArray()

  const filteredResponse = rtData.map(data => {
    const filteredFieldValues = Object.fromEntries(Object.entries(data.field_values).filter(([key]) => fields.find(def => String(def._id) === key && def.plottable)));
    return Object.keys(filteredFieldValues).length > 0 ? { ...data, field_values: filteredFieldValues } : null;
  }).filter(item => item !== null);
  
  res.json(filteredResponse)
}
