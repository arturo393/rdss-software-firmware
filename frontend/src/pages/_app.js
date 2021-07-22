import Head from "next/head"
import "bootstrap/dist/css/bootstrap.css"
import '../styles/global.css'
import '../styles/common/Footer.css'
import '../styles/common/Header.css'

import Header from "../components/common/Header"
import Footer from "../components/common/Footer"
import SiteLayout from "../components/common/SiteLayout"

import { setMonitorDataEvent } from "../redux/actions/main"
import { connect } from "react-redux"
import axios from "axios"

import { wrapper } from "../redux/store"

const MyApp = ({ Component, pageProps }) => {
  console.log("pageProps: ", pageProps)
  return (
    <div>
    <Head>
      <meta name="viewport" content="width=device-width, initial-scale=1" />
    </Head>
    <Header />
    <SiteLayout config={pageProps.config} devices={pageProps.devices}>
      <Component {...pageProps} />
    </SiteLayout>
    <Footer />
    </div>
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


export default wrapper.withRedux(MyApp)
