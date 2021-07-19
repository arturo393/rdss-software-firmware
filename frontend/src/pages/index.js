import Image from "next/image"
import axios from "axios"
import { setRTDataEvent } from "../redux/actions/main"
import { connect } from "react-redux"
import { useEffect } from "react"

import Diagram from './manage/diagram'


const Home = (props) => {

  const { setRTDataEvent } = props

  useEffect(() => {
    setRTDataEvent(props.socket)
  }, [])

  console.log("props: ", props)
  return (
      <main>
        <div className="container-fluid">
          <div className="row">
            <div className="col-md-6">
              <h3>
                {/* <Diagram props={image}/> */}
              </h3>
            </div>
            <div className="col-md-6">
              <div className="row" style={{ bgcolor: "red" }}>
                <div className="col-md-12">
                  <h3>ACA VA EL RESUMEN DEL STATUS.</h3>
                </div>
              </div>
              <div className="row">
                <div className="col-md-12">
                  <Image src="/status.png" alt="mapa" width={817} height={132} />
                </div>
              </div>
            </div>
          </div>
        </div>
      </main>
  )
}


export async function getServerSideProps() {
  const config = await axios
    .get("http://localhost:3000/api/manage/config")
    .then((res) => {
      return res.data
    })

  const devices = await axios
    .get("http://localhost:3000/api/devices/devices")
    .then((res) => {
      return res.data
    })

  

  return {
    props: { config, devices },
  }
}

const mapStateToProps = (state) => {
  return {
    rtData: state.main.rtdata,
  }
}

const mapDispatchToProps = {
  setRTDataEvent
}

export default connect(mapStateToProps, mapDispatchToProps)(Home)