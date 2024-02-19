import { connectToDatabase } from "../../../components/db/util/mongodb"
import { DateTime } from "luxon"

export default async function (req, res, next) {
  const { db } = await connectToDatabase()

  var start = DateTime.fromISO(req.body.dateFrom)
  var end = DateTime.fromISO(req.body.dateTo)

  var diffInDays = end.diff(start, "days").as("days")

  console.log("diffInDays",diffInDays)

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
        $and: [
          {
            id: req.body.id ,
          },
          {
            sampleTime: {
              $gte: new Date(start).toISOString(),
            },
          },
          {
            sampleTime: {
              $lte: new Date(end).toISOString(),
            },
          },
        ],
      },
    },
    {
      $project: {
        id: "$id",
        year: {
          $year: {
            $toDate: "$sampleTime",
          },
        },
        month: {
          $month: {
            $toDate: "$sampleTime",
          },
        },
        day: {
          $dayOfMonth: {
            $toDate: "$sampleTime",
          },
        },
        hour: {
          $hour: {
            $toDate: "$sampleTime",
          },
        },
        minute: {
          $minute: {
            $toDate: "$sampleTime",
          },
        },
        seconds: {
          $second: {
            $toDate: "$sampleTime",
          },
        },
        connected: "$connected",
        field_values_array: {
          $objectToArray: "$field_values",
        },
      },
    },
    {
      $unwind: "$field_values_array",
    },
    {
      $addFields: {
        field_values_array: {
          k: {
            $toObjectId: "$field_values_array.k",
          },
          v: "$field_values_array.v",
        },
      },
    },
    {
      $lookup: {
        from: "fields",
        localField: "field_values_array.k",
        foreignField: "_id",
        as: "field_info",
      },
    },
    {
      $unwind: "$field_info",
    },
    {
      $match: {
        "field_info.plottable": true,
      },
    },
    {
      $group: {
        _id: {...dynId, connected: {$and: "$connected"}, field: "$field_values_array.k"},
        averageValue: { $avg: "$field_values_array.v.value" },
        alerts: { $push: "$field_values_array.v.alert" },
      },
    },
    {
      $project: {
        _id: 1,
        connected: {
          $ifNull: ["$connected", false]
        },
        field_values: {
          k: {$toString: "$_id.field"},
          v: {
            value: "$averageValue",
            alert: {
              $reduce: {
                input: "$alerts",
                initialValue: true,
                in: { $and: ["$$value", "$$this"] },
              },
            },
          },
        },
      },
    },
    {
      $group: {
        _id: {
          device: "$_id.device",
          year: "$_id.year",
          month: "$_id.month",
          day: "$_id.day",
          hour: {
            $cond: {
              if: "$_id.hour",  // Check if dynId.hour exists
              then: "$_id.hour",  // Include dynId.hour if it exists
              else: null  // Otherwise, set it to null or omit it
            }
          },
          minute: {
            $cond: {
              if: "$_id.minute",  // Check if dynId.hour exists
              then: "$_id.minute",  // Include dynId.hour if it exists
              else: null  // Otherwise, set it to null or omit it
            }
          },
          second: {
            $cond: {
              if: "$_id.second",  // Check if dynId.hour exists
              then: "$_id.second",  // Include dynId.hour if it exists
              else: null  // Otherwise, set it to null or omit it
            }
          },
        },
        connected: {$last: "$connected"},
        field_values: { $push: "$field_values" },
      },
    },
    {
      $project: {
        _id: 1,
        connected: 1,
        field_values: {
          $arrayToObject: "$field_values",
        },
      },
    },
    {
      $sort: {
        "_id": 1,
      },
    }
  ]


  console.log(JSON.stringify(pipeline))

  const devices = await db.collection("rtData").aggregate(pipeline).toArray()

  res.json(devices)
}

/**
 * 
 [
  {
    $match: {
      $and: [
        { id: 8 },
        {
          sampleTime: {
            $gte: "2021-02-18T21:20:00.000Z",
          },
        },
        {
          sampleTime: {
            $lte: "2024-02-18T22:39:39.488Z",
          },
        },
      ],
    },
  },
  {
    $project: {
      id: "$id",
      year: { $year: { $toDate: "$sampleTime" } },
      month: {
        $month: { $toDate: "$sampleTime" },
      },
      day: {
        $dayOfMonth: { $toDate: "$sampleTime" },
      },
      hour: { $hour: { $toDate: "$sampleTime" } },
      minute: {
        $minute: { $toDate: "$sampleTime" },
      },
      seconds: {
        $second: { $toDate: "$sampleTime" },
      },
      connected: 1,
      field_values_array: {
        $objectToArray: "$field_values",
      },
    },
  },
  { $unwind: "$field_values_array" },
  {
      $addFields: {
        field_values_array: {
          k: { $toObjectId: "$field_values_array.k" },
          v: "$field_values_array.v"
        }
      }
  },
  {
    $lookup: {
      from: "fields",
      localField: "field_values_array.k",
      foreignField: "_id",
      as: "field_info",
    },
  },
  {
    $unwind: "$field_info"
  },
  {
    $match: {
      "field_info.plottable": true
    }
  },
]
 * 
 */