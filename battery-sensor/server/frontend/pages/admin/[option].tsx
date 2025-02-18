import React, { Dispatch, useEffect } from "react";
import styles from "../../styles/Home.module.css";
import { useAppDispatch } from "../../features/hooks";
import { useSelector } from "react-redux";
import { selectUserStatus } from "../../features/user/userSelector";
import { getProfile } from "../../features/user/userSlice";
import NavBar from "../../components/Home/NavBar";
import { Box } from "@mui/material";
import DesktopSideBar from "../../components/Home/DesktopSideBar";
import useRenderDesktopView from "../../hooks/shouldRenderDesktopView";
import { useRouter } from "next/router";
import BoatAdministration from "../../components/admin/parentComponents/BoatAdministration";
import BookingPeriodAdministration from "../../components/admin/parentComponents/BookingPeriodAdministration";
import KeyEventLogContainer from "../../components/admin/parentComponents/KeyEventLogContainer";
import UserAdministration from "../../components/admin/parentComponents/UserAdministration";
import StatusAdministration from "../../components/admin/parentComponents/StatusAdministration";
import OpenKeyAdministration from "../../components/admin/parentComponents/OpenKeyAdministration";

const actionDispatch = (dispatch: Dispatch<any>) => ({
  fetchProfile: () => dispatch(getProfile()),
});

function AdminOption() {
  const profileStatus = useSelector(selectUserStatus);
  const { fetchProfile } = actionDispatch(useAppDispatch());
  const shouldRenderDesktopView = useRenderDesktopView();

  useEffect(() => {
    if (profileStatus === "idle") {
      fetchProfile();
    }
  }, []);

  const router = useRouter();
  const { option } = router.query;

  return (
    <main className={styles.main}>
      <title>Admin - Kystlaget</title>
      <meta name="description" content="Page for admin functionality." />

      <NavBar handleOpenShow={() => null} showWeather={false} />
      <Box display="flex" flexDirection="row">
        {shouldRenderDesktopView && <DesktopSideBar />}
        <Box width="100%">
          {option == "users" && <UserAdministration />}
          {option == "keys" && <KeyEventLogContainer />}

          {option == "bookingPeriods" && <BookingPeriodAdministration />}
          {option == "boats" && <BoatAdministration />}
          {option == "status" && <StatusAdministration />}
          {option == "open" && <OpenKeyAdministration />}
        </Box>
      </Box>
    </main>
  );
}

export default AdminOption;
