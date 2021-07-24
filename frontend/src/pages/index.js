import axios from "axios"

import Schema from "../components/Schema"
import Status from "../components/common/Status"

import { connect } from "react-redux"
import DynamicComponent from "../components/DynamicComponent"

const Home = (props) => {
  // Cargar información desde DB (ServerSideProps)
  //Acá va a lógica de despliegue de componentes en forma dinámica, por ejemplo:
  // tomar el botón pulsado en el menú
  // si boton.valor == x mostrar componente Y

  const { activeComponent } = props

  return (
    <main>
      <div className="container-fluid">
        <div className="row">
          <div className="col-md-6">
            <Schema />
          </div>
          <div className="col-md-6">
            <div className="row">
              <div className="col-md-12">
                <Status />
              </div>
            </div>
            <div className="row">
              <div className="col-md-12 lightBgColor hundredHeight">
                <DynamicComponent activeComponent="alerts" />
              </div>
            </div>
          </div>
        </div>
      </div>
    </main>
  )
}

// export async function getServerSideProps() {
//   const config = await axios
//     .get("http://localhost:3000/api/manage/config")
//     .then((res) => {
//       return res.data
//     })

//   const devices = await axios
//     .get("http://localhost:3000/api/devices/devices")
//     .then((res) => {
//       return res.data
//     })

//   return {
//     props: { config, devices },
//   }
// }

const mapDispatchToProps = {}

const mapStateToProps = (state) => {
  return { activeComponent: state.main.activeComponent }
}

export default connect(mapStateToProps, mapDispatchToProps)(Home)
