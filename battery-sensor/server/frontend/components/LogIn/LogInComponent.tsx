import React, { Dispatch, useEffect } from "react";
import { Box, Container, Typography } from "@mui/material";
import Head from "next/head";
import Link from "next/link";
import GoogleButton from "../Buttons/GoogleButton";
import { useCookies } from "react-cookie";
import { logIn } from "../../features/auth/authSlice";
import { useAppDispatch } from "../../features/hooks";
import kystlaget_logo from "../../public/images/kystlaget_logo.jpeg"
import Image from "next/image";

const actionDispatch = (dispatch: Dispatch<any>) => ({
  logIn: (accessToken: string) => dispatch(logIn(accessToken)),
});

const LoginComponent = () => {
  const [cookies, _] = useCookies();
  const { logIn } = actionDispatch(useAppDispatch());

  useEffect(() => {
    if (cookies["token"] !== undefined) {
      logIn(cookies["token"]);
    }
  }, [cookies]);
  return (
    <>
      <Head>
        <title>Logg inn - Kystlaget</title>
        <meta name="description" content="Page for booking a trip." />
      </Head>
      <Container style={{ display: "flex", flexDirection: "column", alignItems: "center", justifyContent: "center", margin: "auto" }}>
        <Image width="300" height="300" src={kystlaget_logo} />
        <form
          action={`${process.env.NODE_ENV === "production"
            ? process.env.API_URL_PROD
            : process.env.API_URL_DEV
            }Auth/account/external-login?provider=Google&returnUrl=/`}
          className="items-center flex flex-col"
          method="post"
        >
          <Typography className="w-1/2 text-center text-gray-400">
            Her kan du logge inn på Kystlaget Trondheims booking system. For å
            kunne logge inn må du være medlem av Kystlaget. Hvis du ikke er
            medlem, kan du registrere deg her:
            <Link href="https://www.kystlaget-trh.no/elfryd/">
              <a className="hover:underline transition-all ease-in-out">
                {" "}
                https://www.kystlaget-trh.no/elfryd/
              </a>
            </Link>
          </Typography>
          <GoogleButton />
        </form>
      </Container>
    </>
  );
};

export default LoginComponent;
