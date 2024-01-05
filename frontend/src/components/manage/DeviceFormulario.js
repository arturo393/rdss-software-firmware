import { useState, useEffect, useRef } from "react"
import axios from "axios"

const Formulario = () => {
  const [deviceData, setDeviceData] = useState({})
  //   const refType = useRef()


  useEffect(() => {
    // if (refType.current) {
    //    console.log(refType)
    // }
    getFormulario()
  }, [])

  async function getFormulario() {
    // Obtiene los campos desde la API
    const deviceId = 6
    // const uri = process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/devices/" 
    const uri = process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/devices/devicestype/"
    axios
      .get(uri)
      .then(
        (result) => {
          //   console.log("TYPEFORMULARIO ",result)
          setDeviceData(result.data[0])
       
          return result.data[0]

        },
        (error) => {
          console.log(error)
        }
      )
  }

  // useEffect(() => {
  //     getCampos().then((formulario) => {
  //         setCampos(formulario)
  //     })
  // }, [])
  return (




     // MEMO FORMULARIO QUE EXISTE

    <div className="">
      <div>
        <div className="">Select Items</div>
        <label>Din </label>
        <div className="input-group mb-3">
          <span className="input-group-text">tipo</span>
          <input type="text" className="" id="type1" value={deviceData?.din?.tipo} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">Name</span>
          <input type="text" className="" id="name1" value={deviceData?.din?.name}  />
        </div>
        <div className="input-group mb-4">
          <span className="input-group-text">stateon</span>
          <input type="text" className="" id="stateon1" name="stateon" value={deviceData?.din?.stateon} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">stateon</span>
          <input type="text" className="" id="stateoff1" value={deviceData?.din?.stateoff} onChange={() => console.log("memo")} />
        </div>
        <label>Ain </label>
        <div className="input-group mb-3">
          <span className="">tipo</span>
          <input type="text" className="" id="tipo2" value={deviceData?.ain?.tipo} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">Name</span>
          <input type="text" className="" id="name2" value={deviceData?.ain?.name} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">unit</span>
          <input type="text" className="" id="unit2" value={deviceData?.ain?.unit} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">maximumanalog</span>
          <input type="text" className="" id="maximumanalog2" value={deviceData?.ain?.maximumanalog} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">maximumconverted</span>
          <input type="text" className="" id="maximumconverted2" value={deviceData?.ain?.maximumconverted} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">minimumconverted</span>
          <input type="text" className="" id="minimumconverted2" value={deviceData?.ain?.minimumconverted} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">maximumalert</span>
          <input type="text" className="" id="maximumalert2" value={deviceData?.ain?.maximumalert} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">minimumalert</span>
          <input type="text" className="" id="minimumalert2" value={deviceData?.ain?.minimumalert} onChange={() => console.log("memo")} />
        </div>

        <br></br>
        <label>Aout </label>

        <div className="input-group mb-3">
          <span className="input-group-text">tipo</span>
          <input type="text" className="" id="tipo3" value={deviceData?.aout?.tipo} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">Name</span>
          <input type="text" className="" id="name3" value={deviceData?.aout?.name}  onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">minimumanalog</span>
          <input type="text" className="" id="minimumanalog3" value={deviceData?.aout?.minimumanalog}  onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">maximumanalog</span>
          <input type="text" className="" id="maximumanalog3" value={deviceData?.aout?.maximumanalog}  onChange={() => console.log("memo")} />
        </div>

        <label>modbus </label>

        <div className="input-group mb-3">
          <span className="input-group-text">tipo</span>
          <input type="text" className="" id="tipo4"value={deviceData?.modbus?.tipo} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">Name</span>
          <input type="text" className="" id="name4"value={deviceData?.modbus?.name} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">query</span>
          <input type="text" className="" id="query4" value={deviceData?.modbus?.query} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">answer</span>
          <input type="text" className="" id="answer4" value={deviceData?.modbus?.answer} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">startindex</span>
          <input type="text" className="" id="startindex4" value={deviceData?.modbus?.startindex} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">datalength</span>
          <input type="text" className="" id="datalength4" value={deviceData?.modbus?.datalength} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">dinimumalert</span>
          <input type="text" className="" id="dinimumalert4" value={deviceData?.modbus?.maximumanalog} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">dinimumalert</span>
          <input type="text" className="" id="maximumalert4" value={deviceData?.modbus?.maximumanalog} onChange={() => console.log("memo")} />
        </div>
        <div>    
            <button className="btn btn-primary" type="button"  onClick={ console.log("memo")}>
         Save
        </button>
            </div>

      </div>

    </div>
  )



}

export default Formulario