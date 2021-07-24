import Head from "next/head"
import "bootstrap/dist/css/bootstrap.css"
import "../styles/global.css"
import "../styles/common/Header.css"
import "../styles/common/Footer.css"

import Header from "../components/common/Header"
import Footer from "../components/common/Footer"

import { wrapper } from "../redux/store"

const MyApp = ({ Component, pageProps }) => {
  return (
    <>
      <Head>
        <meta name="viewport" content="width=device-width, initial-scale=1" />
      </Head>
      <Header />
      <Component {...pageProps} />
      <Footer />
    </>
  )
}
export default wrapper.withRedux(MyApp)
