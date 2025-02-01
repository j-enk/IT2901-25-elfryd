import React, { Dispatch, useEffect, useState } from "react";
import { useSelector } from "react-redux";
import {
  selectSelectedBoat,
} from "../features/boat/boatSelector";
import { useAppDispatch } from "../features/hooks";
import styles from "../styles/Home.module.css";
import {
  fetchLockBoxes,
  setSelectedLockBox,
} from "../features/lockbox/lockboxSlice";
import { selectLockStatus } from "../features/lockbox/lockboxSelector";
import { getProfile } from "../features/user/userSlice";
import { selectUserStatus } from "../features/user/userSelector";
import Head from "next/head";
import UnlockComponent from "../components/Home/UnlockComp";
import NavBar from "../components/Home/NavBar";
import Calendar from "../components/Bookings/Calendar";
import useRenderDesktopView from "../hooks/shouldRenderDesktopView";
import { Box } from "@mui/material";
import DesktopSideBar from "../components/Home/DesktopSideBar";

const actionDispatch = (dispatch: Dispatch<any>) => ({
  fetchProfile: () => dispatch(getProfile()),
  fetchLockBoxes: () => dispatch(fetchLockBoxes()),
  setLockBox: (query: number) => dispatch(setSelectedLockBox(query)),
});

function Booking() {
  const lockBoxStatus = useSelector(selectLockStatus);
  const selectedBoat = useSelector(selectSelectedBoat);
  const profileStatus = useSelector(selectUserStatus);

  const [unlockOpen, setUnlockOpen] = useState(false);

  function handleShow() {
    setUnlockOpen((wasOpen) => !wasOpen);
  }

  const { fetchProfile, fetchLockBoxes, setLockBox } =
    actionDispatch(useAppDispatch());

  useEffect(() => {
    if (lockBoxStatus === "succeeded") {
      if (selectedBoat) {
        setLockBox(selectedBoat.lockBoxID);
      }
    }
  }, [lockBoxStatus]);

  useEffect(() => {
    if (profileStatus === "idle") {
      fetchProfile();
    }
    if (lockBoxStatus === "idle") {
      fetchLockBoxes();
    }
  }, []);

  const shouldRenderDesktopView = useRenderDesktopView();

  return (
    <>
      <Head>
        <title>Book din neste tur - Kystlaget</title>
        <meta name="description" content="Page for booking a trip." />
      </Head>
      <main className={styles.main}>
        <NavBar handleOpenShow={handleShow} showWeather={shouldRenderDesktopView} />
        <Box display="flex" flexDirection="row">
          {shouldRenderDesktopView && <DesktopSideBar />}
          <Box px={2} py={2}>
            <Calendar />
          </Box>
        </Box>
        <UnlockComponent handleShow={handleShow} open={unlockOpen} />
      </main>
    </>
  );
}

export default Booking;
