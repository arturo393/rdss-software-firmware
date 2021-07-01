import Head from "next/head"
import "bootstrap/dist/css/bootstrap.css"
import "../styles/globals.css"
import { wrapper } from "../redux/store"

const MyApp = ({ Component, pageProps }) => (
  <>
    <Head>
      <meta name="viewport" content="width=device-width, initial-scale=1" />
    </Head>
    <Component {...pageProps} />
  </>
)

export default wrapper.withRedux(MyApp)
