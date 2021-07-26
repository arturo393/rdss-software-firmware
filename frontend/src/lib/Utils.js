export function convertISODateToTimeFormat(ISODate) {
    const newDateObj = new Date(ISODate);
    const toMonth = newDateObj.getMonth() + 1;
    const toYear = newDateObj.getFullYear();
    const toDate = newDateObj.getDate();
    const toHours = newDateObj.getHours();
    const toHoursProcessed = (toHours < 10 ? '0' : '') + toHours;
    const toMin = newDateObj.getMinutes();
    const toMinProcessed = (toMin < 10 ? '0' : '') + toMin;
    const dateTemplate = `${toYear}-${toMonth}-${toDate} ${toHoursProcessed}:${toMinProcessed}`;
    // console.log(dateTemplate)
    return dateTemplate;
  }