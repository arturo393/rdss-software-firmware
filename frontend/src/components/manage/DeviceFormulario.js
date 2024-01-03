import { useState, useEffect ,useRef} from "react"
import axios from "axios"




const Formulario = () => {

  const [deviceData, setDeviceData] = useState({})
//   const refType = useRef()


//   useEffect(() => {
//     if (refType.current) {
//        console.log(refType)
//     }
// }, [])


  async function getFormulario() {
    // Obtiene los campos desde la API
    const deviceId = 1
    const uri = process.env.NEXT_PUBLIC_APIPROTO + "://" + process.env.NEXT_PUBLIC_APIHOST + ":" + process.env.NEXT_PUBLIC_APIPORT + "/api/devicetype/" + deviceId
    axios
      .get(uri)
      .then(
        (result) => {
  console.log(result)
          setDeviceData(result.data)
          return result.data

        },
        (error) => {
          console.log(error)
        }
      )



  }

  console.log(getFormulario())

  getFormulario()
  // useEffect(() => {
  //     getCampos().then((formulario) => {
  //         setCampos(formulario)
  //     })
  // }, [])
  return (
    <div className="">

      {/* <input type="text" className="" id="type" value={"tipo"} onChange={() => console.log("memo")} />
            <input type="text" className="" id="type" value={"tipo"} onChange={() => console.log("memo")} />
            <input type="text" className="" id="type" value={"tipo"} onChange={() => console.log("memo")} /> */}
      <div>
        <div className="">Select Items</div>
        <label>Din </label>

        <div className="input-group mb-3">
          <span className="input-group-text">tipo</span>

        <input type="text" className="" id="type2" value={deviceData?.type} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">Name</span>
          <input type="text" className="" id="name" value={deviceData?.id} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">stateon</span>
          <input type="text" className="" id="type1" name="stateon" value={deviceData?.name} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">stateon</span>
          <input type="text" className="" id="type" value={"stateoff"} onChange={() => console.log("memo")} />
        </div>

        <label>Ain </label>

        <div className="input-group mb-3">
          <span className="">tipo</span>
          <input type="text" className="" id="tipo" value={"tipo"} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">Name</span>
          <input type="text" className="" id="name" value={"name"} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">unit</span>
          <input type="text" className="" id="unit" value={"unit"} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">maximumanalog</span>
          <input type="text" className="" id="maximumanalog" value={"maximumconverted"} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">maximumconverted</span>
          <input type="text" className="" id="maximumconverted" value={"maximumconverted"} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">minimumconverted</span>
          <input type="text" className="" id="minimumconverted" value={"minimumconverted"} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">maximumalert</span>
          <input type="text" className="" id="maximumalert" value={"maximumalert"} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">minimumalert</span>
          <input type="text" className="" id="minimumalert" value={"minimumalert"} onChange={() => console.log("memo")} />
        </div>

        <br></br>
        <label>Aout </label>

        <div className="input-group mb-3">
          <span className="input-group-text">tipo</span>
          <input type="text" className="" id="tipo" value={"tipo"} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">Name</span>
          <input type="text" className="" id="name" value={"name"} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">minimumanalog</span>
          <input type="text" className="" id="minimumanalog" value={"minimumanalog"} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">maximumanalog</span>
          <input type="text" className="" id="maximumanalog" value={"maximumanalog"} onChange={() => console.log("memo")} />
        </div>

        <label>Aout </label>

        <div className="input-group mb-3">
          <span className="input-group-text">tipo</span>
          <input type="text" className="" id="tipo" value={"modbus"} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">Name</span>
          <input type="text" className="" id="name" value={"circuito"} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">query</span>
          <input type="text" className="" id="query" value={"query"} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">answer</span>
          <input type="text" className="" id="answer" value={"answer"} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">startindex</span>
          <input type="text" className="" id="startindex" value={"startindex"} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">datalength</span>
          <input type="text" className="" id="datalength" value={"datalength"} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">dinimumalert</span>
          <input type="text" className="" id="dinimumalert" value={"dinimumalert"} onChange={() => console.log("memo")} />
        </div>
        <div className="input-group mb-3">
          <span className="input-group-text">dinimumalert</span>
          <input type="text" className="" id="maximumalert" value={"maximumalert"} onChange={() => console.log("memo")} />
        </div>

      </div>

    </div>
  )


  // return ( 
  //     <>
  //         <div className="container-fluid ">
  //             <div className="text-center mt-2 mb-2">
  //                 <h5>Select Item</h5>
  //             </div>
  //             <div className="card h-100">

  //                 <form id="camposForm">
  //                     <div className="container-fluid">
  //                         {/* Genera un input por cada campo */}
  //                         {formulario && formulario.map((formulario) => (
  //                             <div className="row">
  //                                 <div className="col-md-12">
  //                                     <div className="form-floating input-group mb-3">
  //                                         <input type="text" className="form-control" id={formulario?.type} placeholder={0} required={formulario?.required} />
  //                                         <label htmlFor={formulario?.type}>{formulario?.din}</label>
  //                                     </div>
  //                                 </div>
  //                             </div>
  //                         ))
  //                         }
  //                     </div>
  //                 </form>
  //             </div>
  //         </div>

  //     </>
  // )

}

export default Formulario