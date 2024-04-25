import { connectToDatabase } from "../../../components/db/util/mongodb"
import { DateTime } from "luxon"

export default async function (req, res, next) {
  const { db } = await connectToDatabase()

  const dateFrom = DateTime.fromISO(req.body.dateFrom).toFormat("yyyy-LL-dd HH:mm:ss")
  const dateTo = DateTime.fromISO(req.body.dateTo).toFormat("yyyy-LL-dd HH:mm:ss")

  var end = DateTime.fromISO(req.body.dateFrom)
  var start = DateTime.fromISO(req.body.dateTo)

  var diffInDays = start.diff(end, "days").as("days")

  let dynId = {
    device: "$id",
    year: "$year",
    month: "$month",
    day: "$day",
    // hour: "$hour",
    // minute: "$minute",
    // second: "$second",
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

  // const pipeline = [
  //   {
  //     $match: {
  //       id: req.body.id,
  //     },
  //   },
  //   {
  //     $project: {
  //       rtData: {
  //         $filter: {
  //           input: "$rtData",
  //           as: "item",
  //           cond: {
  //             $and: [
  //               {
  //                 $gte: ["$$item.sampleTime", dateFrom],
  //               },
  //               {
  //                 $lte: ["$$item.sampleTime", dateTo],
  //               },
  //             ],
  //           },
  //         },
  //       },
  //       id: 1,
  //       status: 1,
  //       type: 1,
  //     },
  //   },
  //   {
  //     $unwind: {
  //       path: "$rtData",
  //     },
  //   },
  //   {
  //     $project: {
  //       year: {
  //         $year: {
  //           $dateFromString: {
  //             dateString: "$rtData.sampleTime",
  //           },
  //         },
  //       },
  //       month: {
  //         $month: {
  //           $dateFromString: {
  //             dateString: "$rtData.sampleTime",
  //           },
  //         },
  //       },
  //       day: {
  //         $dayOfMonth: {
  //           $dateFromString: {
  //             dateString: "$rtData.sampleTime",
  //           },
  //         },
  //       },
  //       hour: {
  //         $hour: {
  //           $dateFromString: {
  //             dateString: "$rtData.sampleTime",
  //           },
  //         },
  //       },
  //       minute: {
  //         $minute: {
  //           $dateFromString: {
  //             dateString: "$rtData.sampleTime",
  //           },
  //         },
  //       },
  //       second: {
  //         $second: {
  //           $dateFromString: {
  //             dateString: "$rtData.sampleTime",
  //           },
  //         },
  //       },
  //       voltage: "$rtData.voltage",
  //       current: "$rtData.current",
  //       power: "$rtData.power",
  //       alerts: "$rtData.alerts",
  //       id: "$id",
  //     },
  //   },
  //   {
  //     $group: {
  //       _id: dynId,
  //       voltage: {
  //         $avg: "$voltage",
  //       },
  //       current: {
  //         $avg: "$current",
  //       },
  //       power: {
  //         $avg: "$power",
  //       },
  //       alerts: {
  //         $push: "$alerts",
  //       },
  //     },
  //   },
  //   {
  //     $sort: {
  //       _id: 1,
  //     },
  //   },
  // ]

  const pipeline = [
    {
      $match: {
        $and: [{ "metaData.deviceId": req.body.id }, { sampleTime: { $gte: new Date(end) } }, { sampleTime: { $lte: new Date(start) } }],
      },
    },
    {
      $project: {
        id: "$id",
        year: { $year: "$sampleTime" },
        month: { $month: "$sampleTime" },
        day: { $dayOfMonth: "$sampleTime" },
        hour: { $hour: "$sampleTime" },
        minute: { $minute: "$sampleTime" },
        seconds: { $second: "$sampleTime" },
        voltage: { $toDecimal: "$voltage" },
        current: { $toDecimal: "$current" },
        power: { $toDecimal: "$power" },
        alerts: "$alerts",
      },
    },
    {
      $group: {
        _id: dynId,
        voltage: { $avg: "$voltage" },
        current: { $avg: "$current" },
        power: { $avg: "$power" },
        alerts: { $push: "$alerts" },
      },
    },
    {
      $sort: {
        _id: 1,
      },
    },
  ]

  // console.log(JSON.stringify(pipeline))

  const devices = await db.collection("rtData").aggregate(pipeline).toArray()

  res.json(devices)
}
