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
  export function setGraficoData(state,labelArray,dataArray){
    state.labels = labelArray
    state.datasets[0].data = dataArray
    return state;
  }

  export function addGraficoData(state,labelObject,dataObject){
    state.labels.push(labelObject)
    state.datasets[0].data.push(dataObject)
    return state;
  }
  
  export function setBorderColor(state,borderColor){
    state.datasets[0].borderColor = borderColor
    return state;
  }
  

  export function initConfig(backgroundColor,borderColor, label) {
    let objectConfigGraph = {
      labels: [],
      datasets: [
        {
          label: label,
          fill: false,
          lineTension: 0.5,
          backgroundColor: backgroundColor,
          borderColor: borderColor,
          borderWidth: 2,
          data: []
        }
      ]
    }
    return objectConfigGraph
  }

  export function initOptions(text) {
    let initialOptions = {
      maintainAspectRatio: true,
      scales: {
        xAxes: [{
          gridLines: {
           display: false,
           color: '#03A5C5',
           lineWidth: 8,
           },
          ticks: {
            fontColor: "white",
          },
        }],
        yAxes: [{
          gridLines: {
           display: false,
           color: '#03A5C5',
           lineWidth: 8,
           },
          ticks: {
            fontColor: "white",
            beginAtZero: true,
          }
        }]
      },
      title: {
        display: true,
        text: text,
        fontSize: 20
      },
      legend: {
        display: true,
        position: 'right'
      },
      plugins: {
        zoom: {
          zoom: {
            wheel: {
              enabled: true, // SET SCROOL ZOOM TO TRUE 
            },
            mode: "xy",
            speed: 100,
          },
          pan: {
            enabled: true,
            mode: "xy",
            speed: 100,
          },
        },
      },
    }
    return initialOptions
  }
 