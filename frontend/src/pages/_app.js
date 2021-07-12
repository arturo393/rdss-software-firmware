import Head from "next/head"
import "bootstrap/dist/css/bootstrap.css"
import '../styles/global.css'
import '../styles/common/Footer.css'
import '../styles/common/Header.css'

import Header from "../components/common/Header"
import Footer from "../components/common/Footer"
import Logo from "../images/logoSigma.png"
import Url from "../images/Url.png"

import { wrapper } from "../redux/store"

const MyApp = ({ Component, pageProps }) => (
  <>
    <Head>
      <meta name="viewport" content="width=device-width, initial-scale=1" />
    </Head>
    <Header logo={Logo}/>
    <Component {...pageProps} />
    <Footer url={Url}/>
  </>
)

export default wrapper.withRedux(MyApp)
